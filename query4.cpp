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
#include <Client/ClientException.hpp>
#include <Client/QueryConfig.hpp>
#include <Client/RemoteClient.hpp>

using namespace std;
using namespace NES;

int main() {
    const double slackMeters = 5.0;
    const double slackDegrees = slackMeters / 111320.0; // 1 degree ≈ 111.32 km
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
            
            // // Get existing logical sources
            // std::cout << "Available logical sources:" << std::endl;
            // std::string sources = client->getLogicalSources();
            // std::cout << sources << std::endl;

            // Create a simplified join query based on examples from the codebase
            std::cout << "Creating join query..." << std::endl;
            
            // Query for weather data only
            auto weatherQuery = Query::from("weather")
                .map(Attribute("w_temperature") = Attribute("temperature"))
                .map(Attribute("w_timestamp") = Attribute("timestamp"))
                .map(Attribute("w_gps_lat") = Attribute("gps_lat"))
                .map(Attribute("w_gps_lon") = Attribute("gps_lon"))
                .sink(FileSinkDescriptor::create("weather_data.csv", "CSV_FORMAT", "APPEND"));

            // Query for train data only
            auto trainQuery = Query::from("sncb")
                .map(Attribute("t_timestamp") = Attribute("timestamp"))
                .map(Attribute("t_lat") = Attribute("latitude"))
                .map(Attribute("t_lon") = Attribute("longitude"))
                .map(Attribute("t_speed") = Attribute("speed"))
                .sink(FileSinkDescriptor::create("train_data.csv", "CSV_FORMAT", "APPEND"));
            
            std::cout << "Queries created successfully." << std::endl;
            
            // Configure query execution
            Client::QueryConfig queryConfig;
            queryConfig.setPlacementType(Optimizer::PlacementStrategy::TopDown);
            
            // Start measuring execution time
            auto startTime = std::chrono::high_resolution_clock::now();
            
            // Submit both queries
            std::cout << "Submitting weather query..." << std::endl;
            QueryId weatherQueryId = client->submitQuery(weatherQuery, queryConfig);
            std::cout << "Weather query submitted with ID: " << weatherQueryId << std::endl;
            
            std::cout << "Submitting train query..." << std::endl;
            QueryId trainQueryId = client->submitQuery(trainQuery, queryConfig);
            std::cout << "Train query submitted with ID: " << trainQueryId << std::endl;
            
            // Wait for the queries to process some data
         std::cout << "Waiting for query to process data (20 seconds)..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(20));
            // Stop the queries
            std::cout << "Stopping queries..." << std::endl;
            auto weatherStopResult = client->stopQuery(weatherQueryId);
            auto trainStopResult = client->stopQuery(trainQueryId);
            
            // Stop measuring execution time
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
            
            std::cout << "Weather query stopped, result: " << (weatherStopResult ? "success" : "failed") << std::endl;
            std::cout << "Train query stopped, result: " << (trainStopResult ? "success" : "failed") << std::endl;
            std::cout << "Queries execution time: " << duration.count() << " milliseconds" << std::endl;
            
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