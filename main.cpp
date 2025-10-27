#include<iostream>
#include<string>

#include "APIServer.hpp"


int main(int argc,char** argv){
    APIServer* api =APIServer::getInstance();
    
    if(!api->connectToClient(54000)){
        std::cerr << "Failed to connect to client" << std::endl;
    }
    else{
        std::cout << "Successfully connected to client" << std::endl;
    }

    api->receiveRequest();
    api->sendResponse("Hello world!");

    return 0;
}