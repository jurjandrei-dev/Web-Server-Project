#include "HTTPRequest.hpp"

std::string HTTPRequest::serialize() const{
    std::string data;
    data += _method + " ";
    data += _request_URI + " ";
    data += "HTTP/" + _HTTPv + "\r\n";
    data += "Host: " + _host + "\r\n";
    
    if (!_content_type.empty()) {
        data += "Content-Type: " + _content_type + "\r\n";
    }
    
    if (!_body.empty()) {
        data += "Content-Length: " + std::to_string(_body.length()) + "\r\n";
    }
    
    data += "\r\n";
    
    if (!_body.empty()) {
        data += _body;
    }
    
    return data;
}

void HTTPRequest::deserialize(const std::string &data){
    std::istringstream ss(data);
    std::string line;
    
    // Parsare prima linie: GET /path HTTP/1.1
    if (std::getline(ss, line)) {
        
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        std::istringstream lineStream(line);
        std::string httpFull;
        lineStream >> _method >> _request_URI >> httpFull;
        
        // Extrage versiunea din HTTP/1.1
        if (httpFull.find("HTTP/") == 0) {
            _HTTPv = httpFull.substr(5);
        }
    }
    
    while (std::getline(ss, line) && line != "\r" && !line.empty()) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            
            while (!value.empty() && value[0] == ' ') {
                value = value.substr(1);
            }
            
            _headers[key] = value;
            
            if (key == "Host") {
                _host = value;
            } else if (key == "Content-Type") {
                _content_type = value;
            }
        }
    }
    
    std::string bodyContent;
    while (std::getline(ss, line)) {
        bodyContent += line;
        if (ss.peek() != EOF) {
            bodyContent += "\n";
        }
    }
    _body = bodyContent;
}