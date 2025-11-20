#include "AppManager.hpp"

AppManager* AppManager::_instance = nullptr;

AppManager *AppManager::getInstance(){
    if(!_instance){
        _instance=new AppManager();
    }

    return _instance;
}

void AppManager::runApplication(){
    auto ip=sf::IpAddress::resolve("172.20.10.2");

    if(ip.has_value()){
        if(!_api->connectToServer(ip.value(),54000)){
            std::cerr << "Failed to connect to server" << std::endl;
        }else{
            std::cout << "Successfully connected to server" << std::endl;
        }
    }
    else{
        std::cerr << "Failed to resolve IP address" << std::endl;
    }


    _api->sendRequest("Salut lume!");
    _api->receiveResponse();

    _api->closeConnection();
}

void AppManager::getInstanceParser() {
    if(_parser == nullptr){
        _parser = ParseHTML::getInstance();
    }
}

void AppManager::getInstanceApiClient() {
    if(_api == nullptr){
        _api = APIClient::getInstance();
    }
}