#include <iostream>
#include <memory>
#include <string>
#include <chrono>
#include <thread>

// Include NebulaStream headers
#include <API/QueryAPI.hpp>
#include <API/Query.hpp>
#include <API/WindowedQuery.hpp>
#include <API/Windowing.hpp>
#include <Operators/LogicalOperators/Windows/LogicalWindowDescriptor.hpp>
#include <Types/SlidingWindow.hpp>
#include <Types/WindowType.hpp>
#include <Util/TimeMeasurement.hpp>
#include <Types/SlidingWindow.hpp>
#include <Client/ClientException.hpp>
#include <Client/QueryConfig.hpp>
#include <Client/RemoteClient.hpp>


using namespace std;
using namespace NES;

int main() {

    try {
        const std::string coordinatorIp = "192.168.0.238";
        const int coordinatorPort = 8081;
        
        std::cout << "Connecting to NebulaStream server at " << coordinatorIp << ":" << coordinatorPort << "..." << std::endl;
        
        // Create a client to test connection to the NebulaStream server
        auto client = std::make_shared<Client::RemoteClient>(coordinatorIp, coordinatorPort, std::chrono::seconds(20), true);
        bool connected = client->testConnection();
        
        std::cout << "Connection test: " << (connected ? "successful" : "failed") << std::endl;
        
        if (connected) {
            std::cout << "Successfully connected to NebulaStream server!" << std::endl;
            
            // // Get existing logical sources - should now include sncb from coordinator.yaml
            // std::cout << "Available logical sources:" << std::endl;
            // std::string sources = client->getLogicalSources();
            // std::cout << sources << std::endl;

            
            // Create a query using the logical source defined in the coordinator config
            const std::string sourceName = "nrok5";
            std::cout << "Creating query for source: '" << sourceName << "'..." << std::endl;




            auto query = Query::from("nrok5")
                        .window(SlidingWindow::of(EventTime(Attribute("timestamp", BasicType::UINT64)), Seconds(10), Milliseconds(10)))
                        .apply(Min(Attribute("PCFA_bar"))->as(Attribute("PCFA_min_value")),
                                Max(Attribute("PCFA_bar"))->as(Attribute("PCFA_max_value")),
                                Min(Attribute("PCFF_bar"))->as(Attribute("PCFF_min_value")),
                                Max(Attribute("PCFF_bar"))->as(Attribute("PCFF_max_value")))
                        .map(Attribute("wStart") = Attribute("start"))
                        .map(Attribute("wEnd") = Attribute("end"))
                        .map(Attribute("variationPCFA") = Attribute("PCFA_max_value") - Attribute("PCFA_min_value"))
                        .map(Attribute("variationPCFF") = Attribute("PCFF_max_value") - Attribute("PCFF_min_value"))
                        .filter(Attribute("variationPCFA") > 0.4 && Attribute("variationPCFF")  <= 0.1)
                        .project(Attribute("wStart"),
                                    Attribute("wEnd"),
                                    Attribute("variationPCFA"),
                                    Attribute("variationPCFF"))
                        .sink(FileSinkDescriptor::create("query6.csv", "CSV_FORMAT", "APPEND"));



            std::cout << "Query created successfully." << std::endl;
            
            // Configure query execution
            Client::QueryConfig queryConfig;
            queryConfig.setPlacementType(Optimizer::PlacementStrategy::TopDown);
            
            // Start measuring execution time
            auto startTime = std::chrono::high_resolution_clock::now();
            
            // Submit the query
            std::cout << "Submitting query..." << std::endl;
            QueryId queryId = client->submitQuery(query, queryConfig);
            std::cout << "Query submitted with ID: " << queryId << std::endl;
            
            std::cout << "Waiting for query to process data (20 seconds)..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(20));
            
            // Check query status
            std::string status = client->getQueryStatus(queryId);
            std::cout << "Query status: " << status << std::endl;
            
            // Stop the query
            std::cout << "Stopping query..." << std::endl;
            auto stopResult = client->stopQuery(queryId);
            
            // Stop measuring execution time
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
            
            std::cout << "Query stopped, result: " << (stopResult ? "success" : "failed") << std::endl;
            std::cout << "Query execution time: " << duration.count() << " milliseconds" << std::endl;


        } else {
            std::cerr << "Failed to connect to NebulaStream server." << std::endl;
            return 1;
        }
        
        return 0;
    } catch (const Client::ClientException& e) {
        std::cerr << "Client Exception: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}