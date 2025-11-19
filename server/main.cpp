#include<iostream>
#include<string>

#include "APIServer.hpp"
#include "HTTPRequest.hpp"


int main(int argc,char** argv){
    APIServer* api =APIServer::getInstance();

    /*HTTPRequest request;
    request.setMethod("GET");
    request.setRequestURI("/");
    request.setHost("localhost");
    request.setHTTPv("1.1");
    request.setContentType("application/json");

    std::cout<<request.serialize();
    */

    if(!api->connectToClient(54000)){
        std::cerr << "Failed to connect to client" << std::endl;
    }
    else{
        std::cout << "Successfully connected to client" << std::endl;
    }


    api->closeConnection();

    return 0;
}