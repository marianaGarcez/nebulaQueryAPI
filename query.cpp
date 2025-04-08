#include <iostream>
#include <API/Query.hpp>
#include <Client/RemoteClient.hpp>
#include <API/QueryAPI.hpp>

int main() {
    auto client = NES::Client::RemoteClient();
    auto query = Query::from("test").sink(NullOutputSinkDescriptor::create());
    client.submitQuery(query);
    std::cout << "connection : "<< client.testConnection()<< std::endl;
    return 0;
}