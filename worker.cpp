// g++ worker.cpp -o worker
// ifconfig | grep "inet "
#include <iostream>
#include <string>
#include <sstream>
#include <netinet/in.h>
#include <unistd.h>

int main() {
    // wait for a connnection from the master server
    // gets the query
    // it adds a number and attaches it to the query
    // sends back to the server

    int ID = 0;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int port = 8000;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Binding failed" << std::endl;
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_fd, 16) < 0) {
        std::cerr << "Listening failed" << std::endl;
        return 1;
    }

    std::cout << "Worker server is listening on port " << port << "...\n";

    while (true) {
        // Accept incoming connection
        int new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }

        // Receive query from master
        char buffer[1024] = {0};
        int bytesRead = recv(new_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead <= 0) {
            std::cerr << "Failed to receive query" << std::endl;
            close(new_socket);
            continue;
        }

        std::string query(buffer, bytesRead);
        std::cout << "Received the query which is " << query << '\n';

        // Append monotonically increasing integer
        std::string responseStr = query + std::to_string(ID);
        ID++;
        std::cout << "Gonna send back " << responseStr << '\n';

        // Send response back to master
        send(new_socket, responseStr.c_str(), responseStr.size(), 0);
        std::cout << "Done" << std::endl;
        // Close connection
        close(new_socket);
    }
}