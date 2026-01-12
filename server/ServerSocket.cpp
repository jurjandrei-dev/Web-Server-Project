#include "ServerSocket.hpp"
#include <iostream>

bool ServerSocket::start(unsigned short port) {
    if (listener.listen(port) != sf::Socket::Status::Done) {
        std::cerr << "Eroare la listen pe portul " << port << "\n";
        return false;
    }
    std::cout << "Server pornit pe portul " << port << std::endl;
    return true;
}

sf::TcpSocket* ServerSocket::acceptClient() {
    sf::TcpSocket* clientSocket = new sf::TcpSocket();
    
    if (listener.accept(*clientSocket) != sf::Socket::Status::Done) {
        std::cerr << "Eroare la accept client\n";
        delete clientSocket;
        return nullptr;
    }
    
    std::optional<sf::IpAddress> remoteAddr = clientSocket->getRemoteAddress();
    if (remoteAddr.has_value()) {
        std::string clientIP = remoteAddr.value().toString();
        
        if (knownClients.find(clientIP) == knownClients.end()) {
            std::cout << "Client nou conectat de la: " << clientIP << std::endl;
            knownClients.insert(clientIP);
        }
    }

    return clientSocket;
}

std::string ServerSocket::receive(sf::TcpSocket& socket) {
    const size_t CHUNK_SIZE = 1024;
    char buffer[CHUNK_SIZE];
    std::string result;
    std::size_t received;
    
    // Primește în chunk-uri de 1024
    bool headersComplete = false;
    int contentLength = -1;
    
    while (true) {
        sf::Socket::Status status = socket.receive(buffer, CHUNK_SIZE, received);
        
        if (status == sf::Socket::Status::Done) {
            result.append(buffer, received);
            
            // Verifică dacă am terminat header-ele HTTP
            if (!headersComplete && result.find("\r\n\r\n") != std::string::npos) {
                headersComplete = true;
                size_t headerEnd = result.find("\r\n\r\n");
                
                // Caută Content-Length în headers
                size_t clPos = result.find("Content-Length:");
                if (clPos != std::string::npos && clPos < headerEnd) {
                    size_t numStart = clPos + 15;
                    size_t numEnd = result.find("\r\n", numStart);
                    std::string lenStr = result.substr(numStart, numEnd - numStart);
                    
                    // Elimină spații
                    while (!lenStr.empty() && lenStr[0] == ' ') lenStr.erase(0, 1);
                    
                    contentLength = std::stoi(lenStr);
                    
                    int bodyReceived = result.length() - (headerEnd + 4);
                    
                    std::cout << "Content-Length: " << contentLength 
                              << ", Body primit până acum: " << bodyReceived << std::endl;
                    
                    // Verifica daca am primit tot body-ul
                    if (bodyReceived >= contentLength) {
                        std::cout << "Request complet primit!" << std::endl;
                        break;
                    }
                } else {
                    // Nu avem Content-Length, deci am terminat
                    break;
                }
            }
            
            // daca am headers complete si stim content length
            if (headersComplete && contentLength > 0) {
                size_t headerEnd = result.find("\r\n\r\n");
                int bodyReceived = result.length() - (headerEnd + 4);
                
                if (bodyReceived >= contentLength) {
                    std::cout << "Body complet primit: " << bodyReceived << " bytes" << std::endl;
                    break;
                }
            }
            
            // Dacă am primit mai puțin de CHUNK_SIZE și nu avem Content-Length, probabil am terminat
            if (received < CHUNK_SIZE && contentLength == -1) {
                break;
            }
        } else if (status == sf::Socket::Status::Disconnected) {
            std::cerr << "Client deconectat în timpul primirii" << std::endl;
            break;
        } else if (status == sf::Socket::Status::Error) {
            std::cerr << "Eroare la primirea datelor" << std::endl;
            break;
        } else if (status == sf::Socket::Status::NotReady) {
            // Socket nu e gata, continuăm
            continue;
        }
    }
    
    std::cout << "Total primit: " << result.length() << " bytes" << std::endl;
    return result;
}

bool ServerSocket::send(sf::TcpSocket& socket, const std::string& message) {
    const size_t CHUNK_SIZE = 1024;
    size_t totalSent = 0;
    
    // Trimite în chunk-uri de 1024
    while (totalSent < message.size()) {
        size_t remaining = message.size() - totalSent;
        size_t toSend = (remaining < CHUNK_SIZE) ? remaining : CHUNK_SIZE;
        
        if (socket.send(message.c_str() + totalSent, toSend) != sf::Socket::Status::Done) {
            return false;
        }
        
        totalSent += toSend;
    }
    
    return true;
}

void ServerSocket::close() {
    listener.close();
}