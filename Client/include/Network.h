#pragma once
#include <SDL_net.h>
#include <string>

class Network {
public:
    bool init();
    bool connectToServer(const std::string& host, Uint16 port);
    void sendMessage(const std::string& msg);
    void receiveMessage();
    void cleanup();

private:
    TCPsocket socket = nullptr;
    SDLNet_SocketSet socketSet = nullptr;
};
