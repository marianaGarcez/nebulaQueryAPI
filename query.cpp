#include <iostream>
#include <memory>

// Use NebulaStream client includes
#include <Client/RemoteClient.hpp>

int main() {
    try {
        std::cout << "Connecting to NebulaStream server at 127.0.0.1:8081..." << std::endl;
        
        // Create a client to test connection to the NebulaStream server
        auto client = std::make_shared<NES::Client::RemoteClient>("127.0.0.1", 8081);
        bool connected = client->testConnection();
        
        std::cout << "Connection test: " << (connected ? "successful" : "failed") << std::endl;
        
        if (connected) {
            std::cout << "Successfully connected to NebulaStream server!" << std::endl;
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