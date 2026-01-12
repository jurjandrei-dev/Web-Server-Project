#include "AppManager.hpp"

AppManager* AppManager::_instance = nullptr;

AppManager* AppManager::getInstance() {
    if (!_instance) {
        _instance = new AppManager();
    }
    return _instance;
}

void AppManager::runApplication() {
    APIServer* api = APIServer::getInstance();
    
    if (!api->connectToClient(8080)) {
        std::cerr << "Eroare la pornirea serverului!" << std::endl;
        return;
    }
    
    std::cout << "Server HTTP pornit pe portul 8080" << std::endl;
    std::cout << "Deschide browserul la http://localhost:8080" << std::endl;
    
    api->run();
}