#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <chrono>
#include <cstring>
#include <arpa/inet.h>

// Configure the server to listen on all interfaces and a specified port
const char* HOST = "0.0.0.0"; // Listen on all network interfaces
const int PORT = 5001;         // Port to listen on

void start_server() {
    // Create socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return;
    }

    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error setting socket options" << std::endl;
        close(server_socket);
        return;
    }

    // Configure server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    server_addr.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error binding socket" << std::endl;
        close(server_socket);
        return;
    }

    // Listen for connections
    if (listen(server_socket, 1) < 0) {
        std::cerr << "Error listening on socket" << std::endl;
        close(server_socket);
        return;
    }

    std::cout << "Server listening on " << HOST << ":" << PORT << std::endl;

    // Accept client connection
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);

    if (client_socket < 0) {
        std::cerr << "Error accepting connection" << std::endl;
        close(server_socket);
        return;
    }

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    std::cout << "Connected by " << client_ip << std::endl;

    // Start receiving data
    size_t total_received = 0;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    const int BUFFER_SIZE = 4096; // Receive in 4KB chunks
    char buffer[BUFFER_SIZE];
    
    // Keep reading data until the client signals EOF (empty read)
    while (true) {
        int bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_read <= 0) {
            break;
        }
        total_received += bytes_read;
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    
    double throughput = total_received / elapsed.count();
    
    std::cout << "Received " << total_received << " bytes in " << elapsed.count() << " seconds." << std::endl;
    std::cout << "Throughput: " << throughput / 1024 << " KB/s" << std::endl;
    
    // Send an acknowledgment back to the client
    const char* ack_message = "ACK";
    send(client_socket, ack_message, strlen(ack_message), 0);
    
    // Clean up
    close(client_socket);
    close(server_socket);
}

int main() {
    start_server();
    return 0;
}
