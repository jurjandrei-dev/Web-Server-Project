#include "AppManager.hpp"

static AppManager* _instance = nullptr;

AppManager *AppManager::getInstance(){
    if(!_instance){
        _instance=new AppManager();
    }

    return _instance;
}

void AppManager::runApplication(){
    APIServer* api =APIServer::getInstance();
    if(!api->connectToClient(54000)){
        std::cerr << "Failed to connect to client" << std::endl;
    }
    else{
        std::cout << "Successfully connected to client" << std::endl;
    }

    while(1){
        
    }



    api->closeConnection();


}
