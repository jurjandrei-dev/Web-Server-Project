#pragma once

#include <string>
#include <vector>
#include <regex>
#include <iostream>

class ParseHTML{
private:
    static ParseHTML* _instance;
    std::vector<std::string> _references;
    
    ParseHTML() = default;
public:
    ParseHTML(const ParseHTML&) = delete;
    ParseHTML& operator=(const ParseHTML&) = delete;

    static ParseHTML* getInstance();
    static void deleteInstance(){
        free(_instance);
    }
    void findReferences(std::string contentsHTML);
    
};