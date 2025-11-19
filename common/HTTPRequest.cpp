#include "HTTPRequest.hpp"

std::string HTTPRequest::serialize() const{
    std::string data;
    data += _method + " ";
    data += _request_URI + " ";
    data += "HTTP/" + _HTTPv + "\n";
    data += _host + "\n";
    data += _content_type + "\n";
    return data;
}

void HTTPRequest::deserialize(const std::string &data){
    std::stringstream ss(data);
    std::string protocol;

    ss  >> this->_method >>this->_request_URI >> protocol 
        >> this ->_HTTPv >> this->_host >> this ->_content_type;

    return;
}
