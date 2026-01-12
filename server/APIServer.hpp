#pragma once

#include "ServerSocket.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include <SFML/Network.hpp>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <map>
#include <fstream>

struct User {
    std::string name;
    std::string email;
    std::string password;
    std::vector<std::string> uploadedImages; // lista imagini incarcate
};

class APIServer {
private:
    static APIServer* _instance;
    ServerSocket _socket;
    bool _running;
    
    // thread pool simplu
    static const int NUM_WORKERS = 20;
    std::thread _workers[NUM_WORKERS];
    std::queue<sf::TcpSocket*> _clientQueue;
    std::mutex _queueMutex;
    std::condition_variable _queueCV;
    
    // date utilizatori
    std::map<std::string, User> _users;
    std::map<std::string, std::string> _sessions;
    std::mutex _userMutex;
    std::mutex _sessionMutex;
    
    APIServer() : _running(false) {}
    
    void workerThread();
    void handleClient(sf::TcpSocket* clientSocket);
    HTTPResponse processRequest(const HTTPRequest& request);
    std::map<std::string, std::string> parseFormData(const std::string& body);
    std::string extractSessionCookie(const HTTPRequest& request);
    std::string generateSessionId();
    std::string readFile(const std::string& filepath);
    std::string getContentType(const std::string& filepath);
    std::string parseMultipartBoundary(const std::string& contentType);
    std::pair<std::string, std::string> parseMultipartFile(const std::string& body, const std::string& boundary);
    
    HTTPResponse handleLogin(const std::map<std::string, std::string>& formData);
    HTTPResponse handleSignup(const std::map<std::string, std::string>& formData);
    HTTPResponse handleLogout(const std::string& sessionId);
    HTTPResponse handleFileRequest(const std::string& uri, const std::string& sessionId);
    HTTPResponse handleDownload(const std::map<std::string, std::string>& formData, const std::string& sessionId);
    HTTPResponse handleUpload(const HTTPRequest& request, const std::string& sessionId);
    HTTPResponse handleUserImage(const std::string& sessionId, const std::string& imageType, const std::string& filename);
    
    HTTPResponse createRedirect(const std::string& location, const std::string& setCookie = "");
    HTTPResponse create404();
    
    bool loadUsers();
    bool saveUsers();
    bool registerUser(const std::string& name, const std::string& email, const std::string& password);
    bool authenticateUser(const std::string& email, const std::string& password);
    std::string getEmailFromSession(const std::string& sessionId);
    bool addImageToUser(const std::string& email, const std::string& filename);

public:
    APIServer(const APIServer&) = delete;
    APIServer& operator=(const APIServer&) = delete;

    static APIServer* getInstance();
    
    bool connectToClient(unsigned short port);
    void run();
    void stop();

    ~APIServer();
    void createUserDirectories(const std::string &email);
    std::string getUserFolder(const std::string &email);
};