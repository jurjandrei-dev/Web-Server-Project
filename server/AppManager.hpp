#pragma once
#include "APIServer.hpp"
#include<iostream>

class AppManager {
private:
    static AppManager* _instance;

    AppManager(){}
    ~AppManager(){}
public:
    static AppManager* getInstance();
    void runApplication();
};