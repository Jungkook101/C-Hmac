#include <iostream>
#include <chrono>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

const char* HOST = "0.0.0.0";  // Listen on all network interfaces
const int PORT = 5001;

void start_server() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;  // Listen on all interfaces
    address.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on " << HOST << ":" << PORT << std::endl;

    // Accept a connection from a client
    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "Connected by " << inet_ntoa(address.sin_addr) << std::endl;

    char buffer[4096];
    int total_received = 0;

    // Start measuring time
    auto start_time = std::chrono::high_resolution_clock::now();

    // Keep reading data until the client signals EOF (empty read)
    while (true) {
        int bytes_received = recv(new_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            break;  // EOF or error
        }
        total_received += bytes_received;
    }

    // End measuring time
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    double throughput = total_received / elapsed.count();

    std::cout << "Received " << total_received << " bytes in " << elapsed.count() << " seconds." << std::endl;
    std::cout << "Throughput: " << throughput / 1024 << " KB/s" << std::endl;

    // Send an acknowledgment back to the client
    const char* ack_message = "ACK";
    send(new_socket, ack_message, strlen(ack_message), 0);

    // Close the sockets
    close(new_socket);
    close(server_fd);
}

int main() {
    start_server();
    return 0;
}
