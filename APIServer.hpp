#pragma once

#include"ServerSocket.hpp"

class APIServer{
private:
    static APIServer* _instance;
    ServerSocket _socket;


    APIServer(){}
public:
    APIServer(const APIServer&) = delete;
    APIServer& operator=(const APIServer&) = delete;

    static APIServer* getInstance();
    

    bool connectToClient( unsigned short port);

    bool sendResponse(const std::string& response);
    void receiveRequest();

};