#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return 1;
    }

    // Create a socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket." << std::endl;
        WSACleanup();
        return 1;
    }

    // Connect to the server
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // Change to server IP address
    serverAddress.sin_port = htons(12345); // Change to server port number

    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to the server." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to the server." << std::endl;

    std::string message;
    char buffer[1024];
    int bytesRead;

    // Enter a loop to send messages to the server   
    while (true) {
        std::cout << "Enter a message to send to the server (or 'quit' to exit): ";
        std::getline(std::cin, message);

        if (message == "quit")
            break;

        // Send the message to the server
        send(clientSocket, message.c_str(), static_cast<int>(message.length()), 0);

        // Receive a response from the server
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            std::cout << "Received: " << buffer << std::endl;
        }
    } 

    // Cleanup and close the socket
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
