#include "APIClient.hpp"
#include<iostream>

APIClient* APIClient::_instance = nullptr;

APIClient* APIClient::getInstance(){
    if (_instance == nullptr) {
        _instance = new APIClient();
    }
    return _instance;
}

bool APIClient::connectToServer(sf::IpAddress& ip, unsigned short port){
    return this->_socket.connect(ip, port);
}


bool APIClient::sendRequest(const std::string& request){
    if (request.empty()) 
        return false;

    bool ok = this->_socket.send(request);
    return ok;
    
}

void APIClient::receiveResponse(){
    std::cout<<this->_socket.receive();
}

void APIClient::closeConnection() {
    this->_socket.close();
}