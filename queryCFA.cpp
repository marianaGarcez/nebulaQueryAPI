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
    const double slackMeters = 5.0;
    const double slackDegrees = slackMeters / 111320.0; // 1 degree ≈ 111.32 km
    try {
        const std::string coordinatorIp = "127.0.0.1";
        const int coordinatorPort = 8081;
        
        std::cout << "Connecting to NebulaStream server at " << coordinatorIp << ":" << coordinatorPort << "..." << std::endl;
        
        // Create a client to test connection to the NebulaStream server
        auto client = std::make_shared<Client::RemoteClient>(coordinatorIp, coordinatorPort, std::chrono::seconds(20), true);
        bool connected = client->testConnection();
        
        std::cout << "Connection test: " << (connected ? "successful" : "failed") << std::endl;
        
        if (connected) {
            std::cout << "Successfully connected to NebulaStream server!" << std::endl;
            
            // Get existing logical sources - should now include sncb from coordinator.yaml
            std::cout << "Available logical sources:" << std::endl;
            std::string sources = client->getLogicalSources();
            std::cout << sources << std::endl;

            
            // Create a query using the logical source defined in the coordinator config
            const std::string sourceName = "sncb";
            std::cout << "Creating query for source: '" << sourceName << "'..." << std::endl;



            // auto query = Query::from("sncb")
            //             .window(SlidingWindow::of(EventTime(Attribute("timestamp", BasicType::UINT64)), 
            //                                     Seconds(30), Seconds(30)))
            //             .apply(
            //                 Min(Attribute("PCFA_bar"))->as("min_pcfa_bar"),
            //                 Max(Attribute("PCFA_bar"))->as("max_pcfa_bar")
            //             )
            //             .map(
            //                 Attribute("variation") = Attribute("max_pcfa_bar") - Attribute("min_pcfa_bar")
            //             )
            //             .filter(Attribute("variation") > 0.4)
            //             .sink(PrintSinkDescriptor::create());

            auto query = Query::from("sncb")
                            .filter(Attribute("PCFF_bar") >= 0)
                            .window(SlidingWindow::of(EventTime(Attribute("timestamp", BasicType::UINT64)), Seconds(10), Seconds(1)))
                            .byKey(Attribute("id"))
                            .apply(Min(Attribute("PCFA_bar"))->as(Attribute("PCFA_min_value")),
                                   Max(Attribute("PCFA_bar"))->as(Attribute("PCFA_max_value")))
                            .map(Attribute("timestamp") = Attribute("sncb$start"))
                            .map(Attribute("variationPCFA") = Attribute("PCFA_max_value") - Attribute("PCFA_min_value"))
                            .filter(Attribute("variationPCFA") > 0.4)
                            .project(Attribute("timestamp"),
                                            Attribute("id"),
                                            Attribute("PCFA_min_value"),
                                            Attribute("PCFA_max_value"),
                                            Attribute("variationPCFA"))
                            //.sink(PrintSinkDescriptor::create())
                            .sink(FileSinkDescriptor::create("outputFilePath1", "CSV_FORMAT", "APPEND"));


            std::cout << "Query created successfully." << std::endl;
            
            // Configure query execution
            Client::QueryConfig queryConfig;
            queryConfig.setPlacementType(Optimizer::PlacementStrategy::TopDown);
            
            // Submit the query
            std::cout << "Submitting query..." << std::endl;
            QueryId queryId = client->submitQuery(query, queryConfig);
            std::cout << "Query submitted with ID: " << queryId << std::endl;
            
            // Wait for the query to process some data
            std::cout << "Waiting for query to process data (10 seconds)..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(20));
            
            // Check query status
            std::string status = client->getQueryStatus(queryId);
            std::cout << "Query status: " << status << std::endl;
            
            // Stop the query
            std::cout << "Stopping query..." << std::endl;
            auto stopResult = client->stopQuery(queryId);
            std::cout << "Query stopped, result: " << (stopResult ? "success" : "failed") << std::endl;


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