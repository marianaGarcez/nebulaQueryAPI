#include <iostream>
#include <memory>
#include <string>
#include <chrono>
#include <thread>
#include <fstream>
#include <vector>
#include <atomic>
#include <iomanip>

// Include NebulaStream headers (based on working query1.cpp)
#include <API/QueryAPI.hpp>
#include <API/Query.hpp>
#include <API/WindowedQuery.hpp>
#include <API/Windowing.hpp>
#include <Operators/LogicalOperators/Windows/LogicalWindowDescriptor.hpp>
#include <Types/SlidingWindow.hpp>
#include <Types/WindowType.hpp>
#include <Util/TimeMeasurement.hpp>
#include <Client/ClientException.hpp>
#include <Client/QueryConfig.hpp>
#include <Client/RemoteClient.hpp>

using namespace std;
using namespace NES;

struct PerformanceMetrics {
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point endTime;
    uint64_t totalTuples = 0;
    uint64_t totalResults = 0;
    double throughputTuplesPerSecond = 0.0;
    double latencyMs = 0.0;
    double processingTimeMs = 0.0;
    bool querySuccess = false;
};

class Query1PerformanceBenchmark {
private:
    std::shared_ptr<Client::RemoteClient> client;
    const std::string coordinatorIp;
    const int coordinatorPort;
    const std::string sourceName;
    
public:
    Query1PerformanceBenchmark(const std::string& ip = "127.0.0.1", 
                               int port = 8081, 
                               const std::string& source = "sncb") 
        : coordinatorIp(ip), coordinatorPort(port), sourceName(source) {}
    
    bool connectToCoordinator() {
        try {
            std::cout << "\n=== SNCB Query1 Performance Benchmark ===" << std::endl;
            std::cout << "Connecting to NebulaStream coordinator at " << coordinatorIp << ":" << coordinatorPort << std::endl;
            
            client = std::make_shared<Client::RemoteClient>(coordinatorIp, coordinatorPort, std::chrono::seconds(30), true);
            bool connected = client->testConnection();
            
            if (connected) {
                std::cout << "✓ Successfully connected to NebulaStream coordinator!" << std::endl;
                return true;
            } else {
                std::cerr << "✗ Failed to connect to NebulaStream coordinator" << std::endl;
                return false;
            }
        } catch (const std::exception& e) {
            std::cerr << "✗ Connection error: " << e.what() << std::endl;
            return false;
        }
    }
    
    Query createGeospatialQuery() {
        std::cout << "\nCreating geospatial query with:" << std::endl;
        std::cout << "  - Source: " << sourceName << std::endl;
        std::cout << "  - Filter: (Code1 != 0 || Code2 != 0)" << std::endl;
        std::cout << "  - Window: ThresholdWindow with teintersects geospatial function" << std::endl;
        std::cout << "  - Aggregation: Sum(speed)" << std::endl;
        
        // Your actual geospatial query with ThresholdWindow and teintersects
        Query query = Query::from(sourceName)
                .filter((Attribute("Code1") != 0 || Attribute("Code2") != 0))
                .window(ThresholdWindow::of(teintersects(Attribute("longitude", BasicType::FLOAT64),
                                Attribute("latitude", BasicType::FLOAT64),
                                Attribute("timestamp", BasicType::UINT64)) == 1))
                .apply(Sum(Attribute("speed", BasicType::UINT64)))
                .sink(FileSinkDescriptor::create("query1_performance_results.csv", "CSV_FORMAT", "APPEND"));
        
        return query;
    }
    
    PerformanceMetrics runPerformanceTest(int durationSeconds = 60) {
        PerformanceMetrics metrics;
        
        try {
            // Create the actual geospatial query
            Query query = createGeospatialQuery();
            
            // Configure query execution
            Client::QueryConfig queryConfig;
            queryConfig.setPlacementType(Optimizer::PlacementStrategy::TopDown);
            
            std::cout << "\n=== Starting Performance Test ===" << std::endl;
            std::cout << "Test duration: " << durationSeconds << " seconds" << std::endl;
            
            // Record start time
            metrics.startTime = std::chrono::high_resolution_clock::now();
            
            // Submit the query
            std::cout << "Submitting geospatial query..." << std::endl;
            QueryId queryId = client->submitQuery(query, queryConfig);
            std::cout << "✓ Query submitted with ID: " << queryId << std::endl;
            
            // Monitor query during execution
            std::cout << "\nMonitoring query execution..." << std::endl;
            std::cout << "Time(s) | Status" << std::endl;
            std::cout << "--------|--------" << std::endl;
            
            for (int i = 0; i < durationSeconds; i += 5) {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                
                try {
                    std::string status = client->getQueryStatus(queryId);
                    std::cout << std::setw(7) << (i+5) << " | " << status << std::endl;
                } catch (const std::exception& e) {
                    std::cout << std::setw(7) << (i+5) << " | ERROR: " << e.what() << std::endl;
                }
            }
            
            // Record processing time before stopping
            auto processingEndTime = std::chrono::high_resolution_clock::now();
            metrics.processingTimeMs = std::chrono::duration<double, std::milli>(processingEndTime - metrics.startTime).count();
            
            // Stop the query
            std::cout << "\nStopping query..." << std::endl;
            bool stopResult = client->stopQuery(queryId);
            
            // Record end time
            metrics.endTime = std::chrono::high_resolution_clock::now();
            
            metrics.querySuccess = stopResult;
            
            if (stopResult) {
                std::cout << "✓ Query stopped successfully" << std::endl;
            } else {
                std::cout << "✗ Failed to stop query properly" << std::endl;
            }
            
        } catch (const std::exception& e) {
            std::cerr << "✗ Performance test error: " << e.what() << std::endl;
            metrics.endTime = std::chrono::high_resolution_clock::now();
            metrics.querySuccess = false;
        }
        
        return metrics;
    }
    
    void calculateMetrics(PerformanceMetrics& metrics) {
        auto totalDuration = std::chrono::duration<double>(metrics.endTime - metrics.startTime);
        double totalTimeSeconds = totalDuration.count();
        
        // Try to get tuple count from output file
        metrics.totalResults = countResultTuples("query1_performance_results.csv");
        
        // Calculate throughput (results per second)
        if (totalTimeSeconds > 0) {
            metrics.throughputTuplesPerSecond = metrics.totalResults / totalTimeSeconds;
        }
        
        // Latency is processing time
        metrics.latencyMs = metrics.processingTimeMs;
    }
    
    uint64_t countResultTuples(const std::string& filename) {
        std::ifstream file(filename);
        uint64_t count = 0;
        std::string line;
        
        if (file.is_open()) {
            while (std::getline(file, line)) {
                if (!line.empty() && line.find(',') != std::string::npos) {
                    count++;
                }
            }
            file.close();
        }
        
        return count;
    }
    
    void printResults(const PerformanceMetrics& metrics) {
        std::cout << "\n=== SNCB Query1 Performance Results ===" << std::endl;
        std::cout << "Query Success: " << (metrics.querySuccess ? "YES" : "NO") << std::endl;
        std::cout << "Processing Time: " << std::fixed << std::setprecision(2) << metrics.processingTimeMs << " ms" << std::endl;
        std::cout << "Total Results: " << metrics.totalResults << " tuples" << std::endl;
        std::cout << "Throughput: " << std::fixed << std::setprecision(2) << metrics.throughputTuplesPerSecond << " results/second" << std::endl;
        std::cout << "Average Latency: " << std::fixed << std::setprecision(2) << metrics.latencyMs << " ms" << std::endl;
        
        // Additional metrics
        auto totalDuration = std::chrono::duration<double>(metrics.endTime - metrics.startTime);
        std::cout << "Total Test Duration: " << std::fixed << std::setprecision(2) << totalDuration.count() << " seconds" << std::endl;
        
        std::cout << "\n=== Query Details ===" << std::endl;
        std::cout << "Source: " << sourceName << std::endl;
        std::cout << "Filter: (Code1 != 0 || Code2 != 0)" << std::endl;
        std::cout << "Window: ThresholdWindow with teintersects geospatial function" << std::endl;
        std::cout << "Aggregation: Sum(speed)" << std::endl;
        std::cout << "Output: query1_performance_results.csv" << std::endl;
    }
    
    void saveResultsToFile(const PerformanceMetrics& metrics, const std::string& filename = "query1_benchmark_summary.csv") {
        std::ofstream file(filename, std::ios::app);
        
        if (file.is_open()) {
            // Write header if file is empty
            file.seekp(0, std::ios::end);
            if (file.tellp() == 0) {
                file << "timestamp,query_success,processing_time_ms,total_results,throughput_results_per_sec,latency_ms,source,filter,window,aggregation\n";
            }
            
            // Write results
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            
            file << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << ","
                 << (metrics.querySuccess ? "true" : "false") << ","
                 << std::fixed << std::setprecision(2) << metrics.processingTimeMs << ","
                 << metrics.totalResults << ","
                 << std::fixed << std::setprecision(2) << metrics.throughputTuplesPerSecond << ","
                 << std::fixed << std::setprecision(2) << metrics.latencyMs << ","
                 << sourceName << ","
                 << "\"(Code1 != 0 || Code2 != 0)\"" << ","
                 << "\"ThresholdWindow(teintersects)\"" << ","
                 << "Sum(speed)" << std::endl;
            
            file.close();
            std::cout << "\n✓ Results saved to " << filename << std::endl;
        } else {
            std::cerr << "✗ Failed to save results to " << filename << std::endl;
        }
    }
};

int main() {
    try {
        // Create benchmark instance
        Query1PerformanceBenchmark benchmark;
        
        // Connect to coordinator
        if (!benchmark.connectToCoordinator()) {
            return 1;
        }
        
        // Run performance test (60 seconds)
        PerformanceMetrics metrics = benchmark.runPerformanceTest(60);
        
        // Calculate final metrics
        benchmark.calculateMetrics(metrics);
        
        // Print results
        benchmark.printResults(metrics);
        
        // Save results to file
        benchmark.saveResultsToFile(metrics);
        
        return metrics.querySuccess ? 0 : 1;
        
    } catch (const std::exception& e) {
        std::cerr << "Benchmark error: " << e.what() << std::endl;
        return 1;
    }
} 