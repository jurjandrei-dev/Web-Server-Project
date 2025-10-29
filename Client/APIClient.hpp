#pragma once

#include"ClientSocket.hpp"


class APIClient {
private:
    static APIClient* _instance;
    ClientSocket _socket;

    APIClient(){}
public:
    APIClient(const APIClient&) = delete;
    APIClient& operator=(const APIClient&) = delete;

    static APIClient* getInstance();
    bool connectToServer(sf::IpAddress& ip, unsigned short port);
    
    bool sendRequest(const std::string& request);
    void receiveResponse();
    void closeConnection();
};