#include<iostream>
#include<string>

#include "APIClient.hpp"

int main(int argc,char** argv){
    
    APIClient *api=APIClient::getInstance();
    auto ip=sf::IpAddress::resolve("172.20.10.2");

    if(ip.has_value()){
        if(!api->connectToServer(ip.value(),54000)){
            std::cerr << "Failed to connect to server" << std::endl;
        }else{
            std::cout << "Successfully connected to server" << std::endl;
        }
    }
    else{
        std::cerr << "Failed to resolve IP address" << std::endl;
    }


    api->sendRequest("Salut lume!");
    api->receiveResponse();

    api->closeConnection();

    return 0;
}