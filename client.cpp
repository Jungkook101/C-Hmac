#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <cstring>
#include <winsock2.h>  // Include the Winsock header

// Set the server address and port
constexpr const char* HOST = "127.0.0.1";  // Server IP address
constexpr int PORT = 5001;                // Server port

void run_client() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return;
    }

    // Benchmark settings
    constexpr size_t message_size = 1024 * 1024;  // 1 MB per message
    constexpr int num_messages = 100;             // Total of 100 messages
    constexpr size_t total_bytes = message_size * num_messages;
    
    // Create a socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        WSACleanup();
        return;
    }
    
    // Configure server address
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
    // Convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, HOST, &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address or address not supported" << std::endl;
        close(client_socket);
        WSACleanup();
        return;
    }
    
    // Connect to the server
    if (connect(client_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        close(client_socket);
        WSACleanup();
        return;
    }
    
    std::cout << "Connected to server at " << HOST << ":" << PORT << std::endl;
    
    // Create a 1 MB block of data
    std::vector<char> data(message_size, 'a');
    
    // Start timer for the benchmark
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Send the data in a loop
    for (int i = 0; i < num_messages; ++i) {
        size_t total_sent = 0;
        
        // Send may not send all data at once, so we loop until all data is sent
        while (total_sent < message_size) {
            ssize_t bytes_sent = send(client_socket, data.data() + total_sent, message_size - total_sent, 0);
            
            if (bytes_sent == -1) {
                std::cerr << "Send failed" << std::endl;
                close(client_socket);
                WSACleanup();
                return;
            }
            
            total_sent += bytes_sent;
        }
        
        std::cout << "Sent message " << i + 1 << "/" << num_messages << std::endl;
    }
    
    // Shutdown the sending side to indicate completion
    shutdown(client_socket, SHUT_WR);
    
    // Optionally, wait for an acknowledgment from the server
    char ack_buffer[1024];
    ssize_t ack_size = recv(client_socket, ack_buffer, sizeof(ack_buffer) - 1, 0);
    
    if (ack_size > 0) {
        ack_buffer[ack_size] = '\0';  // Null-terminate the received data
        std::cout << "Received acknowledgment: " << ack_buffer << std::endl;
    }
    
    // End timer
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    
    // Calculate throughput
    double throughput = total_bytes / elapsed.count();
    
    std::cout << "Sent " << total_bytes << " bytes in " << elapsed.count() << " seconds." << std::endl;
    std::cout << "Throughput: " << throughput / 1024 << " KB/s" << std::endl;
    
    // Close the socket
    close(client_socket);
    
    // Cleanup Winsock
    WSACleanup();
}

int main() {
    run_client();
    return 0;
}
