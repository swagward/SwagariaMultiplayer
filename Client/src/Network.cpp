#include "../include/Network.h"
#include "../include/Game.h"
#include <algorithm>

Network::Network()
{
    if (SDLNet_Init() < 0)
        std::cerr << "[SDL_NET] failed to initialize: " << SDLNet_GetError() << std::endl;
}

Network::~Network()
{
    disconnect();
    SDLNet_Quit();
}

bool Network::connectToServer(const std::string& host, const int port)
{
    std::cout << "[NETWORK] Attempting to connect to " << host << ":" << port << "..." << std::endl;

    IPaddress ip;
    if (SDLNet_ResolveHost(&ip, host.c_str(), port) < 0)
    {
        std::cerr << "[NETWORK] Failed to resolve host: " << SDLNet_GetError() << std::endl;
        return false;
    }

    socket = SDLNet_TCP_Open(&ip);
    if (!socket)
    {
        //debugs in case the player cant connect for whatever reason
        std::cerr << "[NETWORK] Connection failed. Possible causes: \n"
                  << "1. Firewall blocking port " << port << " on Host\n"
                  << "2. Players are on different subnets\n"
                  << "3. Wrong IP address entered.\n"
                  << "SDL_Net Error: " << SDLNet_GetError() << std::endl;
        return false;
    }

    connected = true;
    std::cout << "[NETWORK] Connected successfully!" << std::endl;

    if (recvThread.joinable()) recvThread.join();
    if (sendThread.joinable()) sendThread.join();

    recvThread = std::thread(&Network::receiveLoop, this);
    sendThread = std::thread(&Network::sendLoop, this);
    return true;
}

void Network::disconnect()
{
    connected = false;

    if (socket)
    {
        SDLNet_TCP_Close(socket);
        socket = nullptr;
    }

    if (recvThread.joinable()) recvThread.join();
    if (sendThread.joinable()) sendThread.join();
}

void Network::queueMessage(const std::string& msg)
{
    std::lock_guard lock(sendMutex);
    sendQueue.push(msg);
}

void Network::receiveLoop()
{
    char buffer[4096];
    std::string partial;

    while (connected)
    {
        const int bytes = SDLNet_TCP_Recv(socket, buffer, sizeof(buffer) - 1);
        if (bytes <= 0)
        {
            std::cerr << "[NETWORK] Connection lost or closed\n";
            connected = false;
            break;
        }

        buffer[bytes] = '\0';
        partial += buffer;

        size_t pos;
        while ((pos = partial.find('\n')) != std::string::npos)
        {
            std::string line = partial.substr(0, pos);
            partial.erase(0, pos + 1);

            line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
            line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
            if (!line.empty() && game)
                game->pushNetworkMessage(line);
        }
    }
}

void Network::sendLoop(){
    while (connected)
    {
        std::string msg;
        {
            std::lock_guard lock(sendMutex);
            if (!sendQueue.empty())
            {
                msg = sendQueue.front();
                sendQueue.pop();
            }
        }

        if (!msg.empty())
        {
            if (msg.back() != '\n') msg.push_back('\n');
            const int len = static_cast<int>(msg.size());

            if (const int result = SDLNet_TCP_Send(socket, msg.c_str(), len); result < len)
            {
                std::cerr << "[NETWORK] Send failed: " << SDLNet_GetError() << std::endl;
                connected = false;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}