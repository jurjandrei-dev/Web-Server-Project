#include "ServerSocket.hpp"
#include <iostream>

bool ServerSocket::start(unsigned short port) {
    if (listener.listen(port) != sf::Socket::Status::Done) {
        std::cerr << "Eroare la listen\n";
        return false;
    }
    if (listener.accept(socket) != sf::Socket::Status::Done) {
        std::cerr << "Eroare la accept\n";
        return false;
    }
    return true;
}


std::string ServerSocket::receive() {
    uint32_t totalSize = 0;

    std::size_t received = 0;
    if (socket.receive(&totalSize, sizeof(totalSize), received) != sf::Socket::Status::Done || received != sizeof(totalSize)) {
        std::cerr << "Eroare la primirea dimensiunii mesajului\n";
        return "";
    }//se primeste prima data dimensiunea

    std::string result;
    result.resize(totalSize); 

    std::size_t totalReceived = 0;
    while (totalReceived < totalSize) {
        std::size_t current;
        if (socket.receive(&result[totalReceived], totalSize - totalReceived, current) != sf::Socket::Status::Done) {
            std::cout << "Eroare la primirea datelor!!!";
            return "";
        }

        totalReceived += current;
    }
    //se primesc mai apoi datele utile

    return result;
}


bool ServerSocket::send(const std::string& message) {
    uint32_t totalSize = static_cast<uint32_t>(message.size());

    // trimitem intai dimensiunea
    if (socket.send(&totalSize, sizeof(totalSize)) != sf::Socket::Status::Done)
        return false;

    // trimitem in chunkuri de 1024
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