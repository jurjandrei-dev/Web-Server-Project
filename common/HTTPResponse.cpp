#include "HTTPResponse.hpp"

std::string HTTPResponse::serialize(){
    std::string data;
    
    data += _protocol + "/" + _HTTPv + " ";
    data += _status_code + " " + _status + "\r\n";
    
    data += "Content-Type: " + _content_type + "\r\n";
    data += "Content-Length: " + std::to_string(_content_length) + "\r\n";
    
    for (const auto& [key, value] : _headers) {
        data += key + ": " + value + "\r\n";
    }
    
    data += "\r\n";
    
    data += _content;
    
    return data;
}

void HTTPResponse::deserialize(const std::string &data) {
    std::istringstream ss(data);
    std::string line;

    // Parsare status line
    std::getline(ss, line);
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
    
    std::istringstream statusLine(line);
    std::string protocolFull;
    statusLine >> protocolFull >> _status_code;
    
    std::getline(statusLine, _status);
    if (!_status.empty() && _status[0] == ' ') {
        _status = _status.substr(1);
    }
    
    size_t slashPos = protocolFull.find('/');
    if (slashPos != std::string::npos) {
        _protocol = protocolFull.substr(0, slashPos);
        _HTTPv = protocolFull.substr(slashPos + 1);
    }

    // Parsare headers
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
            
            if (key == "Content-Type") {
                _content_type = value;
            } else if (key == "Content-Length") {
                _content_length = std::stoi(value);
            } else {
                _headers[key] = value;
            }
        }
    }
    
    _content.resize(_content_length);
    ss.read(_content.data(), _content_length);
}