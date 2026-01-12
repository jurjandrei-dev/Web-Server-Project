#include "AppManager.hpp"
#include <iostream>
#include <csignal>

APIServer* serverInstance = nullptr;

void signalHandler(int signum) {
    std::cout << "\nSemnal de întrerupere primit (" << signum << "). Oprire server...\n";
    
    if (serverInstance != nullptr) {
        serverInstance->stop();
    }
    
    exit(signum);
}

int main() {
    // seteaza handler pentru CTRL+C
    signal(SIGINT, signalHandler);
    
    std::cout << "=================================\n";
    std::cout << "  PhotoShare - Mini Web Server  \n";
    std::cout << "=================================\n\n";

    AppManager* app = AppManager::getInstance();
    
    if (app == nullptr) {
        std::cerr << "Eroare: Nu s-a putut inițializa AppManager!\n";
        return 1;
    }
    
    serverInstance = APIServer::getInstance();
    
    std::cout << "Pornire server...\n";
    app->runApplication();
    
    std::cout << "Server oprit.\n";
    return 0;
}