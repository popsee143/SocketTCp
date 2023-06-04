#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <unordered_map>

std::unordered_map<SOCKET, std::thread> clientThreads;
//std::mutex socketMutex;

// Function to handle client connections
void HandleClient(SOCKET clientSocket, std::unordered_map<SOCKET, SOCKET>& clientSockets) {
    char buffer[1024];
    int bytesRead;

    while (true) {
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead == SOCKET_ERROR) {
            std::cerr << "Error in recv(). Closing connection." << std::endl;
            break;
        }

        if (bytesRead == 0) {
            std::cout << "Client disconnected." << std::endl;
            break;
        }

        // Process the received data or send a response
        // ...
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            std::cout << "Received data from client: " << buffer << std::endl;
        }
        
        // Example: Echo the received data back to the client
        //send(clientSocket, buffer, bytesRead, 0);

        // Broadcast the message to all other connected clients
        for (const auto& pair : clientSockets) {
            SOCKET otherSocket = pair.first;
            if (otherSocket != clientSocket) {
                send(otherSocket, buffer, bytesRead, 0);
            }
        }
    }

    // Close the client socket
    closesocket(clientSocket);

    // Remove the client from the threads and client sockets
    auto threadIt = clientThreads.find(clientSocket);
    if (threadIt != clientThreads.end()) {
        threadIt->second.join();
        clientThreads.erase(threadIt);
    }
    clientSockets.erase(clientSocket);
}

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return 1;
    }

    // Create a socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket." << std::endl;
        WSACleanup();
        return 1;
    }

    // Bind the socket to an address
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(12345); // Change the port number if needed

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind socket." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Failed to listen for connections." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server listening on port 12345." << std::endl;

    std::unordered_map<SOCKET, SOCKET> clientSockets;

    // Accept and handle client connections
    while (true) {
        // Accept a client connection
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Failed to accept client connection." << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        std::cout << "Client connected." << std::endl;

        // Add the client socket to the map
        clientSockets[clientSocket] = clientSocket;

        // Create a new thread to handle the client connection
        std::thread clientThread(HandleClient, clientSocket, std::ref(clientSockets));
        clientThreads[clientSocket] = std::move(clientThread);
    }

    // Cleanup and close sockets
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}