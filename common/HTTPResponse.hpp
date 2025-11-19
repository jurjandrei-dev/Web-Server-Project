#pragma once

#include<iostream>
#include<string>
#include<sstream>


class HTTPResponse{
private:
    std::string _protocol;
    std::string _HTTPv;
    std::string _status_code;
    std::string _status;
    std::string _content_type;
    int _content_length;
    std::string _content;
public:
    HTTPResponse() = default;
    HTTPResponse(const std::string& protocol,const std::string& HTTPv,const std::string& status_code,const std::string& status,
                 const std::string& content_type,const int& content_length,const std::string& content):
        _protocol(protocol), _HTTPv(HTTPv), _status_code(status_code), _status(status), _content_type(content_type), _content_length(content_length), _content(content){}

        
    const std::string& getProtocol() const { return _protocol; }
    const std::string& getHTTPv() const { return _HTTPv; }
    const std::string& getStatusCode() const { return _status_code; }
    const std::string& getStatus() const { return _status; }
    const std::string& getContentType() const { return _content_type; }
    int getContentLength() const { return _content_length; }
    const std::string& getContent() const { return _content; }

    
    void setProtocol(const std::string& v) { _protocol = v; }
    void setHTTPv(const std::string& v) { _HTTPv = v; }
    void setStatusCode(const std::string& v) { _status_code = v; }
    void setStatus(const std::string& v) { _status = v; }
    void setContentType(const std::string& v) { _content_type = v; }
    void setContentLength(const int& v) { _content_length = v; }
    void setContent(const std::string& v) { _content = v; }


    std::string serialize();
    void deserialize(const std::string& data);

    ~HTTPResponse() = default;
};