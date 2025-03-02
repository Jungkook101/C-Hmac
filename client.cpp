#include <iostream>
#include <chrono>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

const char* SERVER_IP = "127.0.0.1";  // Server IP address
const int PORT = 5001;                // Server port

void run_client() {
    int sock = 0;
    struct sockaddr_in server_address;

    // Benchmark settings
    const int message_size = 1024 * 1024;  // 1 MB per message
    const int num_messages = 100;          // Total of 100 messages
    const long total_bytes = static_cast<long>(message_size) * num_messages;
    char data[message_size];               // Create a 1 MB block of data
    memset(data, 'a', message_size);       // Fill with 'a'

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    // Convert IP address to binary form
    if (inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr) <= 0) {
        perror("Invalid address / Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Connected to server at " << SERVER_IP << ":" << PORT << std::endl;

    // Start timer for the benchmark
    auto start_time = std::chrono::high_resolution_clock::now();

    // Send the data in a loop
    for (int i = 0; i < num_messages; ++i) {
        send(sock, data, message_size, 0);
        std::cout << "Sent message " << i + 1 << "/" << num_messages << std::endl;
    }

    // Shutdown the sending side to indicate completion
    shutdown(sock, SHUT_WR);

    // Optionally, wait for an acknowledgment from the server
    char ack[1024];
    int bytes_received = read(sock, ack, sizeof(ack));
    if (bytes_received > 0) {
        ack[bytes_received] = '\0';  // Null-terminate the received string
        std::cout << "Received acknowledgment: " << ack << std::endl;
    }

    // End timer
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    double throughput = total_bytes / elapsed.count();

    std::cout << "Sent " << total_bytes << " bytes in " << elapsed.count() << " seconds." << std::endl;
    std::cout << "Throughput: " << throughput / 1024 << " KB/s" << std::endl;

    // Close the socket
    close(sock);
}

int main() {
    run_client();
    return 0;
}

