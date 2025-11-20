#pragma once

#include "APIClient.hpp"
#include "ParseHTML.hpp"

#include<iostream>
#include <fstream>
#include <cstdlib>

class AppManager {
private:
    static AppManager* _instance;
    APIClient* _api;
    ParseHTML* _parser;

    AppManager() : _api(nullptr), _parser(nullptr){};
    ~AppManager() = default;
public:
    static AppManager* getInstance();
    void getInstanceApiClient();
    void getInstanceParser();
    void runApplication();

    void parseHTMLContent(std::string contents){
        if(_parser){
            _parser->findReferences(contents);
        }else {
            std::cerr << "Parser instance is not initialized." << std::endl;
        }
    }
    void showBrowserWindow(const std::string& htmlContent){
        const char* filename = "namePage.html";
        std::ofstream f(filename);
        if (!f) return;
        f << htmlContent;
        f.close();

        std::string cmd = std::string("xdg-open ") + filename;
        std::system(cmd.c_str());
        
    }
};