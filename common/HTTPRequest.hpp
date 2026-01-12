#pragma once

#include<string>
#include<iostream>
#include<sstream>
#include<map>

class HTTPRequest{
private:
    std::string _method;
    std::string _request_URI;
    std::string _HTTPv;
    std::string _host;
    std::string _content_type;
    std::string _body;  
    std::map<std::string, std::string> _headers;  

public:
    HTTPRequest()=default;
    HTTPRequest(const std::string& method,const std::string& request_URI,const std::string& HTTPv,const std::string& host,const std::string& content_type):
        _method(method),_request_URI(request_URI),_HTTPv(HTTPv),_host(host),_content_type(content_type){}

    const std::string& getMethod() const { return _method; }
    const std::string& getRequestURI() const { return _request_URI; }
    const std::string& getHTTPv() const { return _HTTPv; }
    const std::string& getHost() const { return _host; }
    const std::string& getContentType() const { return _content_type; }
    const std::string& getBody() const { return _body; }
    const std::map<std::string, std::string>& getHeaders() const { return _headers; }

    void setMethod(const std::string& m) { _method = m; }
    void setRequestURI(const std::string& uri) { _request_URI = uri; }
    void setHTTPv(const std::string& v) { _HTTPv = v; }
    void setHost(const std::string& h) { _host = h; }
    void setContentType(const std::string& ct) { _content_type = ct; }
    void setBody(const std::string& b) { _body = b; }
    void addHeader(const std::string& key, const std::string& value) { _headers[key] = value; }

    std::string serialize() const;
    void deserialize(const std::string& data);

    ~HTTPRequest()=default;
};