#include "ParseHTML.hpp"
#include<iostream>

ParseHTML* ParseHTML::_instance = nullptr;

ParseHTML* ParseHTML::getInstance(){
    if(_instance == nullptr)
        _instance = new ParseHTML();
    
    return _instance;
}
void ParseHTML::findReferences(std::string contentsHTML)
{
    std::regex fileRegex(R"(<(?:link\b[^>]*href|script\b[^>]*src)=["']([^"']+\.(?:css|js))(?:[?#][^"']*)?["'])",
                            std::regex::icase);

    std::smatch matchingStructure;

    while (std::regex_search(contentsHTML, matchingStructure, fileRegex)) {
        _references.push_back(matchingStructure[1]);
        contentsHTML = matchingStructure.suffix();
    }
    
    for(auto item : _references){
        std::cout << "Found reference: " << item << std::endl;
    }
}
