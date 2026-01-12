#pragma once
#include <SFML/Network.hpp>
#include <string>
#include <set>

class ServerSocket {
private:
    sf::TcpListener listener;
    std::set<std::string> knownClients;
    
public:
    bool start(unsigned short port);
    sf::TcpSocket* acceptClient();  // Modificat să returneze socket-ul clientului
    std::string receive(sf::TcpSocket& socket);  // Modificat să primească socket specific
    bool send(sf::TcpSocket& socket, const std::string& message);  // Modificat
    void close();
};