#include <iostream>
#include <memory>
#include <string>

// Include full paths to NebulaStream headers
#include <Client/RemoteClient.hpp>
#include <API/Query.hpp>
#include <Client/QueryConfig.hpp>
#include <Operators/LogicalOperators/Sinks/FileSinkDescriptor.hpp>

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
            SchemaPtr schema = Schema::create()->addField("id", BasicType::UINT32);
            auto success = client->addLogicalSource(schema, "sourceName");
            if (!success) {
                std::cerr << "Failed to add logical source" << std::endl;
                return 1;
            }
            else{
                std::cout << "Logical source added successfully" << std::endl;
            }   
            
            NES::Query query = NES::Query::from("sourceName")
                         .sink(FileSinkDescriptor::create("out.csv", "CSV_FORMAT", "APPEND"));

            NES::Client::QueryConfig queryConfig;

            auto queryId = client->submitQuery(query, queryConfig);
            std::cout << "Query submitted with ID: " << queryId << std::endl;

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