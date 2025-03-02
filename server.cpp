#include <iostream>
#include <string>
#include <chrono>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib") // Link against Winsock library

// Configure the server to listen on all interfaces and a specified port
constexpr const char* HOST = "0.0.0.0";  // Listen on all network interfaces
constexpr int PORT = 5001;               // Port to listen on
constexpr int BUFFER_SIZE = 4096;        // 4KB buffer size for receiving

void start_server() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return;
    }

    // Create a socket
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
        WSACleanup();
        return;
    }

    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
        std::cerr << "Failed to set socket options" << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return;
    }

    // Configure server address
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket to address
    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind to port " << PORT << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return;
    }

    // Listen for connections
    if (listen(server_socket, 1) == SOCKET_ERROR) {
        std::cerr << "Listen failed" << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return;
    }

    std::cout << "Server listening on " << HOST << ":" << PORT << std::endl;

    // Accept connection
    sockaddr_in client_addr{};
    int client_addr_len = sizeof(client_addr);
    SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_addr_len);

    if (client_socket == INVALID_SOCKET) {
        std::cerr << "Accept failed" << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return;
    }

    // Get client IP
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    std::cout << "Connected by " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;

    // Start timing
    auto start_time = std::chrono::high_resolution_clock::now();

    // Receive data in chunks
    size_t total_received = 0;
    char buffer[BUFFER_SIZE];

    // Keep reading data until the client signals EOF (empty read)
    while (true) {
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);

        if (bytes_received <= 0) {
            // Connection closed or error
            break;
        }

        total_received += bytes_received;
    }

    // End timing
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    // Calculate throughput
    double throughput = total_received / elapsed.count();

    std::cout << "Received " << total_received << " bytes in " << elapsed.count() << " seconds." << std::endl;
    std::cout << "Throughput: " << throughput / 1024 << " KB/s" << std::endl;

    // Send an acknowledgment back to the client
    std::string ack_message = "ACK";
    send(client_socket, ack_message.c_str(), ack_message.length(), 0);

    // Close sockets
    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();
}

int main() {
    start_server();
    return 0;
}
