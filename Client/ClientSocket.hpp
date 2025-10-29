#pragma once

#include<SFML/Network.hpp>
class ClientSocket{
private:
    sf::TcpSocket socket;
public:
    bool connect(sf::IpAddress ip, unsigned short port);
    std::string receive();
    bool send(const std::string& message);
    void close();
};