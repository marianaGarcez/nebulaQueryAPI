#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <chrono>
#include <thread>
#include <filesystem>

// Include full paths to NebulaStream headers
#include <Client/RemoteClient.hpp>
#include <API/Query.hpp>
#include <API/Schema.hpp>
#include <Client/QueryConfig.hpp>
#include <Operators/LogicalOperators/Sinks/PrintSinkDescriptor.hpp>
#include <Operators/LogicalOperators/Sources/CsvSourceDescriptor.hpp>
#include <Identifiers/Identifiers.hpp>

using namespace NES;

int main() {
    try {
        const std::string coordinatorIp = "127.0.0.1";
        const int coordinatorPort = 8081;
        
        std::cout << "Connecting to NebulaStream server at " << coordinatorIp << ":" << coordinatorPort << "..." << std::endl;
        
        // Create a client to test connection to the NebulaStream server
        auto client = std::make_shared<NES::Client::RemoteClient>(coordinatorIp, coordinatorPort);
        bool connected = client->testConnection();
        
        std::cout << "Connection test: " << (connected ? "successful" : "failed") << std::endl;
        
        if (connected) {
            std::cout << "Successfully connected to NebulaStream server!" << std::endl;

            SchemaPtr sncb_schema = Schema::create()->addField("id", BasicType::UINT32)->addField("value", BasicType::UINT64);
            
            auto csvSourceType = CSVSourceType::create("sncb", "sncbmerged");
            csvSourceType->setFilePath(std::filesystem::path("selected_columns_df.csv"));
            csvSourceType->setGatheringInterval(1);                      
            csvSourceType->setSkipHeader(true);     
            
            // Use NullOutputSinkDescriptor for simpler testing
            NES::Query query = NES::Query::from("sncb")
                        .sink(PrintSinkDescriptor::create());

            std::cout << "Query created successfully" << std::endl;

            // Use the QueryConfig as shown in the documentation
            NES::Client::QueryConfig queryConfig;
            // Set the placement strategy to BottomUp 
            queryConfig.setPlacementType(Optimizer::PlacementStrategy::TopDown);

            std::cout << "Query config created successfully" << std::endl;

            NES::QueryId queryId = client->submitQuery(query, queryConfig);
            std::cout << "Query submitted with ID: " << queryId << std::endl;
            
            // Wait for the query to process some data
            std::cout << "Waiting for query to process data..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(10));
            
            // Stop the query when done
            // Note: Check the return type of stopQuery
            // auto stopResult = client->stopQuery(queryId);
            // std::cout << "Query stopped, result: " << (stopResult ? "success" : "failed") << std::endl;
                
        } else {
            std::cerr << "Failed to connect to NebulaStream server." << std::endl;
            return 1;
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}