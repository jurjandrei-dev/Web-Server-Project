#pragma once
#include <SFML/Network.hpp>
#include <string>

class ServerSocket {
    sf::TcpListener listener;
    sf::TcpSocket socket;
public:
    bool start(unsigned short port);
    std::string receive();
    bool send(const std::string& message);
};