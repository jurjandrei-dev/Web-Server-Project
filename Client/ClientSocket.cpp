#include "ClientSocket.hpp"

#include<iostream>
#include<SFML/Network.hpp>

bool ClientSocket::connect(sf::IpAddress ip, unsigned short port)
{
    return socket.connect(ip,port) == sf::Socket::Status::Done;
}

std::string ClientSocket::receive() {
    uint32_t totalSize = 0;

    std::size_t received = 0;
    if (socket.receive(&totalSize, sizeof(totalSize), received) != sf::Socket::Status::Done || received != sizeof(totalSize)) {
        std::cerr << "Eroare la primirea dimensiunii mesajului\n";
        return "";
    }

    std::string result;
    result.resize(totalSize);

    std::size_t totalReceived = 0;
    while (totalReceived < totalSize) {
        std::size_t current;
        if (socket.receive(&result[totalReceived], totalSize - totalReceived, current) != sf::Socket::Status::Done)
            return "";

        totalReceived += current;
    }

    return result;
}

bool ClientSocket::send(const std::string& message) {
    uint32_t totalSize = static_cast<uint32_t>(message.size());

    // Trimitem intai dimensiunea
    if (socket.send(&totalSize, sizeof(totalSize)) != sf::Socket::Status::Done)
        return false;

    // Trimitem în chunkuri de 1024
    const size_t chunkSize = 1024;
    size_t totalSent = 0;

    while (totalSent < message.size()) {
        size_t remaining = message.size() - totalSent;
        size_t currentChunk = std::min(chunkSize, remaining);

        if (socket.send(message.c_str() + totalSent, currentChunk) != sf::Socket::Status::Done)
            return false;

        totalSent += currentChunk;
    }

    return true;
}