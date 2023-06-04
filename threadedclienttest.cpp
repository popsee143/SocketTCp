#include <iostream>
#include <thread>
#include <mutex>
#include <winsock2.h>

std::mutex socketMutex;
SOCKET clientSocket;

void SendMessageThread() {
    while (true) {
        std::string message;
        std::cout << "ME: ";
        std::getline(std::cin, message);

        if (message == "quit") {
            break;
        }

        // Lock the mutex before accessing the shared socket
        //std::lock_guard<std::mutex> lock(socketMutex);

        if (send(clientSocket, message.c_str(), message.size(), 0) == SOCKET_ERROR) {
            std::cerr << "Failed to send data to server." << std::endl;
            break;
        }
    }
}

void ReceiveMessageThread() {
    while (true) {
        char buffer[4096];
        memset(buffer, 0, sizeof(buffer));

        // Lock the mutex before accessing the shared socket
        //std::lock_guard<std::mutex> lock(socketMutex);

        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead == SOCKET_ERROR) {
            std::cerr << "Failed to receive data." << std::endl;
            break;
        }

        if (bytesRead == 0) {
            std::cout << "Disconnected from the server." << std::endl;
            break;
        }

        std::cout << "\nReceived data from server: " << buffer << std::endl;

        std::cout << "ME: ";
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize winsock." << std::endl;
        return 1;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket." << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);  // Port number
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Server IP address

    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to server." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    std::cout << "Type quit to quit: "<<std::endl;
    std::thread sendThread(SendMessageThread);
    std::thread receiveThread(ReceiveMessageThread);

    sendThread.join();
    receiveThread.join();

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
