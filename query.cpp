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

            // Define schema that matches your CSV file structure
            auto gpsSchema = Schema::create()
                        ->addField("timestamp", BasicType::UINT64)
                        ->addField("id", BasicType::UINT64)
                        ->addField("Vbat", BasicType::FLOAT64)
                        ->addField("PCFA_mbar", BasicType::FLOAT64)
                        ->addField("PCFF_mbar", BasicType::FLOAT64)
                        ->addField("PCF1_mbar", BasicType::FLOAT64)
                        ->addField("PCF2_mbar", BasicType::FLOAT64)
                        ->addField("T1_mbar", BasicType::FLOAT64)
                        ->addField("T2_mbar", BasicType::FLOAT64)
                        ->addField("Code1", BasicType::UINT64)
                        ->addField("Code2", BasicType::UINT64)
                        ->addField("speed", BasicType::FLOAT64)
                        ->addField("latitude", BasicType::FLOAT64)
                        ->addField("longitude", BasicType::FLOAT64);
                              
            // Set the path to your CSV file
            std::string csvFilePath = "/media/psf/Home/Parallels/nebulaQueryAPI/data/selected_columns_df.csv";
            
            // Define a logical source name
            std::string logicalSourceName = "gpsData";
            
            // Get existing logical sources to check if our source already exists
            std::string logicalSources = client->getLogicalSources();
            std::cout << "Existing logical sources: " << logicalSources << std::endl;
            
            bool sourceExists = logicalSources.find(logicalSourceName) != std::string::npos;
            bool success = true;
            
            if (!sourceExists) {
                // Register the logical source with the schema
                success = client->addLogicalSource(gpsSchema, logicalSourceName);
                
                if (!success) {
                    std::cerr << "Failed to add logical source" << std::endl;
                    return 1;
                }
                else {
                    std::cout << "Logical source added successfully" << std::endl;
                }
            } else {
                std::cout << "Logical source '" << logicalSourceName << "' already exists, using it" << std::endl;
            }
            
            // Create a query that uses the logical source
            NES::Query query = NES::Query::from(logicalSourceName)
                        .sink(FileSinkDescriptor::create("out.csv", "CSV_FORMAT", "APPEND"));

            std::cout << "Query created successfully" << std::endl;

            NES::Client::QueryConfig queryConfig;

            std::cout << "Query config created successfully" << std::endl;

            // Check if the file exists
            std::cout << "Checking if CSV file exists at: " << csvFilePath << std::endl;
            if (std::filesystem::exists(csvFilePath)) {
                std::cout << "CSV file exists!" << std::endl;
            } else {
                std::cerr << "CSV file does not exist at: " << csvFilePath << std::endl;
                std::cerr << "You need to place your CSV file at the path above." << std::endl;
                
                // List files in the directory if it exists
                auto parentPath = std::filesystem::path(csvFilePath).parent_path();
                if (std::filesystem::exists(parentPath)) {
                    std::cout << "Files in " << parentPath << ":" << std::endl;
                    for (const auto& entry : std::filesystem::directory_iterator(parentPath)) {
                        std::cout << "  " << entry.path() << std::endl;
                    }
                } else {
                    // Create directories if they don't exist
                    std::filesystem::create_directories(parentPath);
                    std::cerr << "Created directory: " << parentPath << std::endl;
                }
                
                return 1;
            }

            auto queryId = client->submitQuery(query, queryConfig);
            std::cout << "Query submitted with ID: " << queryId << std::endl;
            
            // Wait for the query to process some data
            std::cout << "Waiting for query to process data..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(10));
            
            // Optionally stop the query when done
            // client->stopQuery(queryId);
            // std::cout << "Query stopped" << std::endl;

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