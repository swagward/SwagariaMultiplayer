#include "../include/network.h"
#include "../include/Game.h"
#include <iostream>
#include <sstream>
#include <algorithm>

Network::Network() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
}

Network::~Network() {
    disconnect();
    WSACleanup();
}

bool Network::connectToServer(const std::string& host, int port) {
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr);

    socket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socket == INVALID_SOCKET) {
        std::cerr << "[Network] Socket creation failed.\n";
        return false;
    }

    if (::connect(socket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "[Network] Failed to connect to " << host << ":" << port << "\n";
        closesocket(socket);
        return false;
    }

    connected = true;
    std::cout << "[Network] Connected to " << host << ":" << port << std::endl;

    recvThread = std::thread(&Network::receiveLoop, this);
    sendThread = std::thread(&Network::sendLoop, this);

    return true;
}

void Network::disconnect() {
    connected = false;

    if (socket != INVALID_SOCKET) {
        shutdown(socket, SD_BOTH);
        closesocket(socket);
        socket = INVALID_SOCKET;
    }

    if (recvThread.joinable()) recvThread.join();
    if (sendThread.joinable()) sendThread.join();
}

void Network::queueMessage(const std::string& msg) {
    std::lock_guard<std::mutex> lock(sendMutex);
    sendQueue.push(msg);
}

void Network::receiveLoop() {
    char buffer[1024];
    while (connected) {
        int bytes = recv(socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            connected = false;
            break;
        }
        buffer[bytes] = '\0';
        std::string msg(buffer);

        std::istringstream iss(msg);
        std::string line;
        while (std::getline(iss, line)) {
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
            line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
            if (!line.empty()) {
                std::cout << "[Client] Received: " << line << std::endl;
                if (game) {
                    game->pushNetworkMessage(line);
                }
            }
        }
    }
}

void Network::sendLoop() {
    while (connected) {
        std::string msg;
        {
            std::lock_guard<std::mutex> lock(sendMutex);
            if (!sendQueue.empty()) {
                msg = sendQueue.front();
                sendQueue.pop();
            }
        }

        if (!msg.empty()) {
            if (msg.back() != '\n') msg.push_back('\n');
            std::cout << "[Network] Sending: " << msg;
            send(socket, msg.c_str(), (int)msg.size(), 0);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
