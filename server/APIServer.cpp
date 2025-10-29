#include "APIServer.hpp"
#include<iostream>

APIServer* APIServer::_instance = nullptr;

APIServer* APIServer::getInstance(){
    if (_instance == nullptr) {
        _instance = new APIServer();
    }
    return _instance;
}


bool APIServer::connectToClient(unsigned short port){
    return this->_socket.start(port);
}

bool APIServer::sendResponse(const std::string& response){
    if (response.empty())
        return false;

    bool ok = this->_socket.send(response);
    return ok;
}

void APIServer::receiveRequest(){
    std::cout<<this->_socket.receive();
}

void APIServer::closeConnection(){
    this->_socket.close();
}
