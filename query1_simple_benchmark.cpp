#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <filesystem>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;
namespace fs = std::filesystem;

struct BenchmarkMetrics {
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point endTime;
    size_t totalResults = 0;
    size_t initialFileSize = 0;
    size_t finalFileSize = 0;
    double throughputResultsPerSecond = 0.0;
    double totalDurationSeconds = 0.0;
    bool querySuccess = false;
    std::string outputFile;
};

class Query1SimpleBenchmark {
private:
    const std::string outputFile = "query1.csv";
    const std::string resultsFile = "query1_benchmark_results.csv";
    
public:
    BenchmarkMetrics runBenchmark(int durationSeconds = 60) {
        BenchmarkMetrics metrics;
        metrics.outputFile = outputFile;
        
        std::cout << "\n=== SNCB Query1 Simple Performance Benchmark ===" << std::endl;
        std::cout << "Monitoring Query1 output file: " << outputFile << std::endl;
        std::cout << "Benchmark duration: " << durationSeconds << " seconds" << std::endl;
        
        // Check if output file exists and get initial size
        if (fs::exists(outputFile)) {
            metrics.initialFileSize = fs::file_size(outputFile);
            std::cout << "Initial output file size: " << metrics.initialFileSize << " bytes" << std::endl;
        } else {
            std::cout << "Output file doesn't exist yet - will be created by Query1" << std::endl;
            metrics.initialFileSize = 0;
        }
        
        // Record start time
        metrics.startTime = std::chrono::high_resolution_clock::now();
        
        std::cout << "\n=== Instructions ===" << std::endl;
        std::cout << "1. Start your coordinator: ./nebulaQueryAPI/coordinator.yaml" << std::endl;
        std::cout << "2. Start your worker: ./nebulaQueryAPI/worker.yaml" << std::endl;
        std::cout << "3. Run Query1: ./query1" << std::endl;
        std::cout << "4. This benchmark will monitor the output file for " << durationSeconds << " seconds" << std::endl;
        std::cout << "\nPress ENTER when your Query1 is running..." << std::endl;
        std::cin.get();
        
        // Monitor the query execution
        std::cout << "\n=== Monitoring Query1 Execution ===" << std::endl;
        std::cout << "Time(s) | File Size (bytes) | New Results | Cumulative Results" << std::endl;
        std::cout << "--------|-------------------|-------------|-------------------" << std::endl;
        
        size_t previousResults = 0;
        
        for (int i = 0; i < durationSeconds; i += 5) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            
            size_t currentFileSize = 0;
            size_t currentResults = 0;
            
            if (fs::exists(outputFile)) {
                currentFileSize = fs::file_size(outputFile);
                currentResults = countResultLines(outputFile);
            }
            
            size_t newResults = currentResults - previousResults;
            previousResults = currentResults;
            
            std::cout << std::setw(7) << (i+5) << " | " 
                      << std::setw(17) << currentFileSize << " | "
                      << std::setw(11) << newResults << " | "
                      << std::setw(17) << currentResults << std::endl;
        }
        
        // Record end time and final measurements
        metrics.endTime = std::chrono::high_resolution_clock::now();
        
        if (fs::exists(outputFile)) {
            metrics.finalFileSize = fs::file_size(outputFile);
            metrics.totalResults = countResultLines(outputFile);
            metrics.querySuccess = true;
        } else {
            metrics.querySuccess = false;
        }
        
        return metrics;
    }
    
    size_t countResultLines(const std::string& filename) {
        std::ifstream file(filename);
        size_t count = 0;
        std::string line;
        
        if (file.is_open()) {
            while (std::getline(file, line)) {
                // Skip empty lines and potential headers
                if (!line.empty() && line.find(',') != std::string::npos) {
                    count++;
                }
            }
            file.close();
        }
        
        return count;
    }
    
    void calculateMetrics(BenchmarkMetrics& metrics) {
        auto duration = std::chrono::duration<double>(metrics.endTime - metrics.startTime);
        metrics.totalDurationSeconds = duration.count();
        
        if (metrics.totalDurationSeconds > 0) {
            metrics.throughputResultsPerSecond = metrics.totalResults / metrics.totalDurationSeconds;
        }
    }
    
    void printResults(const BenchmarkMetrics& metrics) {
        std::cout << "\n=== SNCB Query1 Performance Results ===" << std::endl;
        std::cout << "Query Success: " << (metrics.querySuccess ? "YES" : "NO") << std::endl;
        std::cout << "Total Duration: " << std::fixed << std::setprecision(2) << metrics.totalDurationSeconds << " seconds" << std::endl;
        std::cout << "Initial File Size: " << metrics.initialFileSize << " bytes" << std::endl;
        std::cout << "Final File Size: " << metrics.finalFileSize << " bytes" << std::endl;
        std::cout << "Data Generated: " << (metrics.finalFileSize - metrics.initialFileSize) << " bytes" << std::endl;
        std::cout << "Total Results: " << metrics.totalResults << " tuples" << std::endl;
        std::cout << "Throughput: " << std::fixed << std::setprecision(2) << metrics.throughputResultsPerSecond << " results/second" << std::endl;
        
        std::cout << "\n=== Query Details ===" << std::endl;
        std::cout << "Source: sncb (real SNCB data)" << std::endl;
        std::cout << "Filter: (Code1 != 0 || Code2 != 0)" << std::endl;
        std::cout << "Window: ThresholdWindow with teintersects geospatial function" << std::endl;
        std::cout << "Aggregation: Sum(speed)" << std::endl;
        std::cout << "Output File: " << metrics.outputFile << std::endl;
    }
    
    void saveResults(const BenchmarkMetrics& metrics) {
        std::ofstream file(resultsFile, std::ios::app);
        
        if (file.is_open()) {
            // Write header if file is empty
            file.seekp(0, std::ios::end);
            if (file.tellp() == 0) {
                file << "timestamp,query_success,duration_seconds,initial_file_size,final_file_size,data_generated_bytes,total_results,throughput_results_per_sec,query_type\n";
            }
            
            // Write results
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            
            file << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << ","
                 << (metrics.querySuccess ? "true" : "false") << ","
                 << std::fixed << std::setprecision(2) << metrics.totalDurationSeconds << ","
                 << metrics.initialFileSize << ","
                 << metrics.finalFileSize << ","
                 << (metrics.finalFileSize - metrics.initialFileSize) << ","
                 << metrics.totalResults << ","
                 << std::fixed << std::setprecision(2) << metrics.throughputResultsPerSecond << ","
                 << "\"SNCB Query1 Geospatial (ThresholdWindow + teintersects)\"" << std::endl;
            
            file.close();
            std::cout << "\n✓ Results saved to " << resultsFile << std::endl;
        } else {
            std::cerr << "✗ Failed to save results to " << resultsFile << std::endl;
        }
    }
    
    void showSampleResults(const std::string& filename, int maxLines = 10) {
        if (!fs::exists(filename)) {
            std::cout << "\n⚠️  Output file not found: " << filename << std::endl;
            return;
        }
        
        std::ifstream file(filename);
        std::string line;
        int lineCount = 0;
        
        std::cout << "\n=== Sample Query Results (first " << maxLines << " lines) ===" << std::endl;
        
        if (file.is_open()) {
            while (std::getline(file, line) && lineCount < maxLines) {
                if (!line.empty()) {
                    std::cout << line << std::endl;
                    lineCount++;
                }
            }
            file.close();
            
            if (lineCount == 0) {
                std::cout << "(No results found in output file)" << std::endl;
            }
        }
    }
};

int main() {
    try {
        Query1SimpleBenchmark benchmark;
        
        // Run the benchmark (60 seconds)
        BenchmarkMetrics metrics = benchmark.runBenchmark(60);
        
        // Calculate final metrics
        benchmark.calculateMetrics(metrics);
        
        // Print results
        benchmark.printResults(metrics);
        
        // Show sample output
        benchmark.showSampleResults(metrics.outputFile);
        
        // Save results
        benchmark.saveResults(metrics);
        
        std::cout << "\n=== Benchmark Complete ===" << std::endl;
        std::cout << "Use this data to compare against other queries or optimizations!" << std::endl;
        
        return metrics.querySuccess ? 0 : 1;
        
    } catch (const std::exception& e) {
        std::cerr << "Benchmark error: " << e.what() << std::endl;
        return 1;
    }
} 