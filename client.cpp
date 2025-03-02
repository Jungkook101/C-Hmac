#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>
#include <string>
#include <cstring>

// Set the server address and port
const char* HOST = "127.0.0.1"; // Server IP address
const int PORT = 5001;          // Server port

void run_client() {
    // Benchmark settings:
    const size_t message_size = 1024 * 1024;  // 1 MB per message
    const int num_messages = 100;             // Total of 100 messages
    const size_t total_bytes = message_size * num_messages;
    
    // Create a 1 MB block of data
    char* data = new char[message_size];
    memset(data, 'a', message_size);
    
    // Create socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        std::cerr << "Error creating socket" << std::endl;
        delete[] data;
        return;
    }
    
    // Configure server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, HOST, &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address / Address not supported" << std::endl;
        close(client_socket);
        delete[] data;
        return;
    }
    
    // Connect to server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        close(client_socket);
        delete[] data;
        return;
    }
    
    std::cout << "Connected to server at " << HOST << ":" << PORT << std::endl;
    
    // Start timer for the benchmark
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Send the data in a loop
    for (int i = 0; i < num_messages; i++) {
        ssize_t bytes_sent = send(client_socket, data, message_size, 0);
        if (bytes_sent < 0) {
            std::cerr << "Error sending data" << std::endl;
            close(client_socket);
            delete[] data;
            return;
        }
        std::cout << "Sent message " << (i + 1) << "/" << num_messages << std::endl;
    }
    
    // Shutdown the sending side to indicate completion
    shutdown(client_socket, SHUT_WR);
    
    // Optionally, wait for an acknowledgment from the server
    char ack_buffer[1024];
    int bytes_received = recv(client_socket, ack_buffer, sizeof(ack_buffer) - 1, 0);
    
    if (bytes_received > 0) {
        ack_buffer[bytes_received] = '\0';
        std::cout << "Received acknowledgment: " << ack_buffer << std::endl;
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    
    double throughput = total_bytes / elapsed.count();
    
    std::cout << "Sent " << total_bytes << " bytes in " << elapsed.count() << " seconds." << std::endl;
    std::cout << "Throughput: " << throughput / 1024 << " KB/s" << std::endl;
    
    // Clean up
    close(client_socket);
    delete[] data;
}

int main() {
    run_client();
    return 0;
}
