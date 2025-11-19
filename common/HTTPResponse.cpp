#include "HTTPResponse.hpp"

std::string HTTPResponse::serialize(){
    std::string data;
    data += _protocol + "/";
    data += _HTTPv + " ";
    data += _status_code + " ";
    data += _status + "\n";
    data += _content_type + "\n";
    data += std::to_string(_content_length) + "\n";
    data += _content;
    return data;
    
}

void HTTPResponse::deserialize(const std::string &data) {
    std::istringstream ss(data);
    std::string line;

    std::getline(ss, line);
    std::istringstream request_line(line);
    request_line >> _protocol;
    request_line.ignore(); // ignore '/'
    request_line >> _HTTPv;
    request_line >> _status_code;
    request_line >> _status;

    std::getline(ss, _content_type);
    std::getline(ss, line);
    _content_length = std::stoi(line);
    
    _content.resize(_content_length);
    ss.read(_content.data(), _content_length);
}
