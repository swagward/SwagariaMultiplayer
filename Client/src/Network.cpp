#include "../include/Network.h"
#include <iostream>

bool Network::init() {
    if (SDLNet_Init() < 0) {
        std::cerr << "[Network] Failed to initialize SDL_net: "
                  << SDLNet_GetError() << std::endl;
        return false;
    }
    std::cout << "[Network] SDL_net initialized successfully.\n";
    return true;
}

bool Network::connectToServer(const std::string& host, Uint16 port) {
    IPaddress ip;
    if (SDLNet_ResolveHost(&ip, host.c_str(), port) < 0) {
        std::cerr << "[Network] Failed to resolve host '" << host << "': "
                  << SDLNet_GetError() << std::endl;
        return false;
    }

    socket = SDLNet_TCP_Open(&ip);
    if (!socket) {
        std::cerr << "[Network] Could not open TCP connection: "
                  << SDLNet_GetError() << std::endl;
        return false;
    }

    //create a socket (set for polling the incoming data to stop blocking the network)
    socketSet = SDLNet_AllocSocketSet(1);
    SDLNet_TCP_AddSocket(socketSet, socket);

    std::cout << "[Network] Connected to " << host << ":" << port << std::endl;
    return true;
}

void Network::sendMessage(const std::string& msg) {
    if (!socket) return;
    std::string formatted = msg + "\n";
    int len = formatted.size();
    if (SDLNet_TCP_Send(socket, formatted.c_str(), len) < len) {
        std::cerr << "[Network] Failed to send message: " << SDLNet_GetError() << std::endl;
    }
}

void Network::receiveMessage() {
    if (!socket || !socketSet) return;

    //poll the socket set for up to a millisecond
    const int activity = SDLNet_CheckSockets(socketSet, 1);
    if (activity <= 0) return; // nothing to read

    if (SDLNet_SocketReady(socket)) {
        char buffer[512];
        const int received = SDLNet_TCP_Recv(socket, buffer, sizeof(buffer) - 1);

        if (received <= 0) {
            std::cerr << "[Network] Connection closed or receive error.\n";
            SDLNet_TCP_DelSocket(socketSet, socket);
            SDLNet_TCP_Close(socket);
            socket = nullptr;
            return;
        }

        buffer[received] = '\0';
        std::cout << "[Server] " << buffer << std::endl;
    }
}

void Network::cleanup() {
    if (socketSet) {
        SDLNet_FreeSocketSet(socketSet);
        socketSet = nullptr;
    }
    if (socket) {
        SDLNet_TCP_Close(socket);
        socket = nullptr;
    }

    SDLNet_Quit();
    std::cout << "[Network] Cleaned up SDL_net.\n";
}
