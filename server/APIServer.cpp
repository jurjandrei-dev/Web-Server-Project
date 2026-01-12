#include "APIServer.hpp"
#include <iostream>
#include <sstream>
#include <random>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;
APIServer* APIServer::_instance = nullptr;

APIServer* APIServer::getInstance() {
    if (_instance == nullptr) {
        _instance = new APIServer();
    }
    return _instance;
}

APIServer::~APIServer() {
    stop();
}

void APIServer::createUserDirectories(const std::string& email) {
    std::string userFolder = getUserFolder(email);
    
    fs::create_directories(userFolder);
    
    // Creez user_data/email/uploaded/
    fs::create_directories(userFolder + "/uploaded");
    
    std::cout << "Directoare create pentru: " << email << std::endl;
}

std::string APIServer::getUserFolder(const std::string& email) {
    std::string safeEmail = email;
    std::replace(safeEmail.begin(), safeEmail.end(), '@', '_');
    std::replace(safeEmail.begin(), safeEmail.end(), '.', '_');
    return "./user_data/" + safeEmail;
}

bool APIServer::connectToClient(unsigned short port) {
    loadUsers();
    
    for (int i = 0; i < NUM_WORKERS; i++) {
        _workers[i] = std::thread(&APIServer::workerThread, this);
    }
    
    return _socket.start(port);
}

void APIServer::workerThread() {
    while (_running) {
        sf::TcpSocket* client = nullptr;
        
        {
            std::unique_lock<std::mutex> lock(_queueMutex);
            _queueCV.wait(lock, [this] { return !_clientQueue.empty() || !_running; });
            
            if (!_running && _clientQueue.empty()) {
                return;
            }
            
            if (!_clientQueue.empty()) {
                client = _clientQueue.front();
                _clientQueue.pop();
            }
        }
        
        if (client) {
            handleClient(client);
        }
    }
}

void APIServer::run() {
    _running = true;
    
    while (_running) {
        sf::TcpSocket* client = _socket.acceptClient();
        
        if (client != nullptr) {
            std::lock_guard<std::mutex> lock(_queueMutex);
            _clientQueue.push(client);
            _queueCV.notify_one();
        }
    }
}

void APIServer::stop() {
    _running = false;
    _queueCV.notify_all();
    
    for (int i = 0; i < NUM_WORKERS; i++) {
        if (_workers[i].joinable()) {
            _workers[i].join();
        }
    }
    
    _socket.close();
}

void APIServer::handleClient(sf::TcpSocket* clientSocket) {
    std::unique_ptr<sf::TcpSocket> socket(clientSocket);
    
    std::string rawRequest = _socket.receive(*socket);
    
    if (rawRequest.empty()) {
        return;
    }
    
    HTTPRequest request;
    request.deserialize(rawRequest);
    
    if (request.getRequestURI() == "/favicon.ico") {
        socket->disconnect();
        return;
    }
    
    std::cout << "Request: " << request.getMethod() << " " << request.getRequestURI() << std::endl;
    
    HTTPResponse response = processRequest(request);
    
    std::string responseStr = response.serialize();
    _socket.send(*socket, responseStr);
    
    socket->disconnect();
}

HTTPResponse APIServer::processRequest(const HTTPRequest& request) {
    std::string method = request.getMethod();
    std::string uri = request.getRequestURI();
    std::string sessionId = extractSessionCookie(request);
    
    std::cout << "=== processRequest ===" << std::endl;
    std::cout << "Method: " << method << std::endl;
    std::cout << "URI: " << uri << std::endl;
    std::cout << "Session: " << sessionId << std::endl << std::endl;
    
    if (method == "POST") {
        std::map<std::string, std::string> formData = parseFormData(request.getBody());
        
        if (uri == "/login") {
            return handleLogin(formData);
        } else if (uri == "/signup") {
            return handleSignup(formData);
        } else if (uri == "/logout") {
            return handleLogout(sessionId);
        } else if (uri == "/download") {
            return handleDownload(formData, sessionId);
        } else if (uri == "/upload") {
            return handleUpload(request, sessionId);
        }
    } else if (method == "GET") {
        // Request pentru imaginile user-ului
        if (uri.find("/user-uploaded/") == 0) {
            std::string filename = uri.substr(15);
            return handleUserImage(sessionId, "uploaded", filename);
        }
        return handleFileRequest(uri, sessionId);
    }
    
    return create404();
}

std::string APIServer::getEmailFromSession(const std::string& sessionId) {
    std::lock_guard<std::mutex> lock(_sessionMutex);
    auto it = _sessions.find(sessionId);
    if (it == _sessions.end()) {
        return "";
    }
    return it->second;
}

bool APIServer::addImageToUser(const std::string& email, const std::string& filename) {
    std::lock_guard<std::mutex> lock(_userMutex);
    
    auto it = _users.find(email);
    if (it == _users.end()) {
        return false;
    }
    
    it->second.uploadedImages.push_back(filename);
    
    saveUsers();
    return true;
}

HTTPResponse APIServer::handleUpload(const HTTPRequest& request, const std::string& sessionId) {
    std::string email = getEmailFromSession(sessionId);
    
    if (email.empty()) {
        return createRedirect("/login.html");
    }
    
    std::string contentType = request.getContentType();
    std::string body = request.getBody();
    
    std::cout << "=== UPLOAD DEBUG ===" << std::endl;
    std::cout << "Email: " << email << std::endl;
    std::cout << "Content-Type: [" << contentType << "]" << std::endl;
    std::cout << "Body length: " << body.length() << " bytes" << std::endl;
    std::cout << "Body preview (first 200 chars): " << body.substr(0, 200) << std::endl << std::endl;
    
    if (contentType.find("multipart/form-data") != std::string::npos) {
        std::string boundary = parseMultipartBoundary(contentType);
        
        if (boundary.empty()) {
            return createRedirect("/upload.html");
        }
        
        auto [filename, fileData] = parseMultipartFile(body, boundary);
        
        if (filename.empty() || fileData.empty()) {
            return createRedirect("/upload.html");
        }
        
        std::string timestamp = std::to_string(std::time(nullptr));
        std::string uniqueFilename = timestamp + "_" + filename;
        
        std::string userFolder = getUserFolder(email);
        createUserDirectories(email);
        
        std::string savePath = userFolder + "/uploaded/" + uniqueFilename;
        
        std::ofstream outFile(savePath, std::ios::binary);
        
        if (outFile.is_open()) {
            outFile.write(fileData.c_str(), fileData.length());
            outFile.close();
            
            addImageToUser(email, uniqueFilename);
            
            std::cout << "Imagine salvata cu succes: " << fileData.length() << " bytes" << std::endl;
        } else {
            std::cout << "EROARE: Nu pot deschide fisierul: " << savePath << std::endl;
        }
    } else {
        std::cout << "Content-Type nu este multipart/form-data!" << std::endl;
    }
    
    return createRedirect("/my-photos.html");
}

HTTPResponse APIServer::handleUserImage(const std::string& sessionId, const std::string& imageType, const std::string& filename) {
    std::string email = getEmailFromSession(sessionId);
    
    std::cout << "=== handleUserImage ===" << std::endl;
    std::cout << "Session: " << sessionId << std::endl;
    std::cout << "Email: " << email << std::endl;
    std::cout << "Type: " << imageType << std::endl;
    std::cout << "Filename: " << filename << std::endl << std::endl;
    
    if (email.empty()) {
        return create404();
    }
    
    std::string userFolder = getUserFolder(email);
    std::string imagePath = userFolder + "/" + imageType + "/" + filename;
    
    std::cout << "Path complet: " << imagePath << std::endl;
    
    std::ifstream file(imagePath, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "EROARE: Nu pot deschide fisierul!" << std::endl;
        return create404();
    }
    
    std::ostringstream ss;
    ss << file.rdbuf();
    std::string content = ss.str();
    file.close();
    
    std::cout << "Imagine citita: " << content.length() << " bytes" << std::endl << std::endl;
    
    HTTPResponse response;
    response.setProtocol("HTTP");
    response.setHTTPv("1.1");
    response.setStatusCode("200");
    response.setStatus("OK");
    response.setContentType(getContentType(filename));
    response.setContent(content);
    
    return response;
}

std::map<std::string, std::string> APIServer::parseFormData(const std::string& body) {
    std::map<std::string, std::string> formData;
    std::istringstream stream(body);
    std::string pair;
    
    while (std::getline(stream, pair, '&')) {
        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);
            
            std::replace(value.begin(), value.end(), '+', ' ');
            
            std::string decoded;
            for (size_t i = 0; i < value.length(); i++) {
                if (value[i] == '%' && i + 2 < value.length()) {
                    std::string hex = value.substr(i + 1, 2);
                    char ch = (char)strtol(hex.c_str(), nullptr, 16);
                    decoded += ch;
                    i += 2;
                } else {
                    decoded += value[i];
                }
            }
            
            formData[key] = decoded;
        }
    }
    
    return formData;
}

std::string APIServer::extractSessionCookie(const HTTPRequest& request) {
    auto headers = request.getHeaders();
    auto it = headers.find("Cookie");
    
    if (it == headers.end()) {
        return "";
    }
    
    std::string cookieHeader = it->second;
    size_t pos = cookieHeader.find("session_id=");
    
    if (pos == std::string::npos) {
        return "";
    }
    
    pos += 11;
    size_t endPos = cookieHeader.find(';', pos);
    
    if (endPos == std::string::npos) {
        return cookieHeader.substr(pos);
    }
    
    return cookieHeader.substr(pos, endPos - pos);
}

std::string APIServer::generateSessionId() {
    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string sessionId;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);
    
    for (int i = 0; i < 32; ++i) {
        sessionId += alphanum[dis(gen)];
    }
    
    return sessionId;
}

std::string APIServer::parseMultipartBoundary(const std::string& contentType) {
    std::cout << "parseMultipartBoundary input: [" << contentType << "]" << std::endl;
    
    size_t pos = contentType.find("boundary=");
    if (pos == std::string::npos) {
        std::cout << "Nu contine 'boundary='" << std::endl;
        return "";
    }
    
    std::string boundary = contentType.substr(pos + 9);
    
    while (!boundary.empty() && (boundary[0] == ' ' || boundary[0] == '"')) {
        boundary = boundary.substr(1);
    }
    while (!boundary.empty() && (boundary.back() == ' ' || boundary.back() == '"' || boundary.back() == '\r' || boundary.back() == '\n')) {
        boundary.pop_back();
    }
    
    return boundary;
}

std::pair<std::string, std::string> APIServer::parseMultipartFile(const std::string& body, const std::string& boundary) {
    std::string delimiter = "--" + boundary;
   
    size_t start = body.find(delimiter);
    if (start == std::string::npos) {
        std::cout << "Boundary nu a fost gasit in body!" << std::endl;
        return {"", ""};
    }
    
    // sar peste primul boundary
    start += delimiter.length();
    
    // sar peste \r\n după boundary
    if (start + 2 < body.length() && body[start] == '\r' && body[start + 1] == '\n') {
        start += 2;
    }
    
    size_t end = body.find(delimiter, start);
    if (end == std::string::npos) {
        std::cout << "Boundary de final nu a fost găsit!" << std::endl;
        return {"", ""};
    }
    
    std::string part = body.substr(start, end - start);
    
    std::cout << "Part length: " << part.length() << std::endl;
    
    // cauta filename
    size_t filenamePos = part.find("filename=\"");
    if (filenamePos == std::string::npos) {
        std::cout << "filename nu a fost găsit!" << std::endl;
        return {"", ""};
    }
    
    filenamePos += 10; // lungimea lui "filename=\""
    size_t filenameEnd = part.find("\"", filenamePos);
    if (filenameEnd == std::string::npos) {
        std::cout << "Sfârșitul filename-ului nu a fost găsit!" << std::endl;
        return {"", ""};
    }
    
    std::string filename = part.substr(filenamePos, filenameEnd - filenamePos);
    std::cout << "Filename găsit: " << filename << std::endl;
    
    size_t dataStart = part.find("\r\n\r\n");
    if (dataStart == std::string::npos) {
        std::cout << "Separator header-body nu a fost găsit!" << std::endl;
        return {"", ""};
    }
    
    dataStart += 4; // sar peste \r\n\r\n
    
    size_t dataEnd = part.length();
    
    if (dataEnd >= 2 && part[dataEnd - 2] == '\r' && part[dataEnd - 1] == '\n') {
        dataEnd -= 2;
    }
    
    std::string fileData = part.substr(dataStart, dataEnd - dataStart);
    
    std::cout << "File data length: " << fileData.length() << " bytes" << std::endl;
    
    return {filename, fileData};
}

std::string APIServer::readFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    
    if (!file.is_open()) {
        std::string altPath = "../pages" + filepath.substr(1);
        file.open(altPath, std::ios::binary);
        
        if (!file.is_open()) {
            return "";
        }
    }
    
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

std::string APIServer::getContentType(const std::string& filepath) {
    if (filepath.find(".html") != std::string::npos) return "text/html; charset=UTF-8";
    if (filepath.find(".css") != std::string::npos) return "text/css";
    if (filepath.find(".jpg") != std::string::npos) return "image/jpeg";
    if (filepath.find(".jpeg") != std::string::npos) return "image/jpeg";
    if (filepath.find(".png") != std::string::npos) return "image/png";
    if (filepath.find(".gif") != std::string::npos) return "image/gif";
    return "application/octet-stream";
}

HTTPResponse APIServer::handleLogin(const std::map<std::string, std::string>& formData) {
    auto emailIt = formData.find("email");
    auto passIt = formData.find("password");
    
    if (emailIt != formData.end() && passIt != formData.end()) {
        if (authenticateUser(emailIt->second, passIt->second)) {
            std::string sessionId = generateSessionId();
            
            {
                std::lock_guard<std::mutex> lock(_sessionMutex);
                _sessions[sessionId] = emailIt->second;
            }
            
            std::cout << "Login SUCCESS: " << emailIt->second << std::endl;
            
            std::string cookie = "session_id=" + sessionId + "; Path=/; HttpOnly";
            return createRedirect("/gallery.html", cookie);
        }
    }
    
    return createRedirect("/login.html");
}

HTTPResponse APIServer::handleSignup(const std::map<std::string, std::string>& formData) {
    auto nameIt = formData.find("name");
    auto emailIt = formData.find("email");
    auto passIt = formData.find("password");
    
    if (nameIt != formData.end() && emailIt != formData.end() && passIt != formData.end()) {
        if (registerUser(nameIt->second, emailIt->second, passIt->second)) {
            std::string sessionId = generateSessionId();
            
            {
                std::lock_guard<std::mutex> lock(_sessionMutex);
                _sessions[sessionId] = emailIt->second;
            }
            
            std::cout << "Signup SUCCESS: " << emailIt->second << std::endl;
            
            std::string cookie = "session_id=" + sessionId + "; Path=/; HttpOnly";
            return createRedirect("/gallery.html", cookie);
        }
    }
    
    return createRedirect("/signup.html");
}

HTTPResponse APIServer::handleLogout(const std::string& sessionId) {
    if (!sessionId.empty()) {
        std::lock_guard<std::mutex> lock(_sessionMutex);
        _sessions.erase(sessionId);
    }
    return createRedirect("/login.html");
}

HTTPResponse APIServer::handleDownload(const std::map<std::string, std::string>& formData, const std::string& sessionId) {
    std::string email = getEmailFromSession(sessionId);
    
    if (email.empty()) {
        return createRedirect("/login.html");
    }
    
    auto filenameIt = formData.find("filename");
    auto typeIt = formData.find("type");
    
    if (filenameIt == formData.end()) {
        std::cout << "Filename lipsa in form data" << std::endl;
        return create404();
    }
    
    std::string filename = filenameIt->second;
    std::string type = (typeIt != formData.end()) ? typeIt->second : "public";
    
    std::cout << "Download request: " << filename << " (type: " << type << ") de la " << email << std::endl;
    
    if (type == "public") {
        std::string sourcePath = "./public_images/" + filename;
        
        std::cout << "Incercare citire din: " << sourcePath << std::endl;
        
        std::ifstream sourceFile(sourcePath, std::ios::binary);
        if (!sourceFile.is_open()) {
            sourcePath = "../public_images/" + filename;
            std::cout << "Reîncercare din: " << sourcePath << std::endl;
            sourceFile.open(sourcePath, std::ios::binary);
            
            if (!sourceFile.is_open()) {
                std::cout << "EROARE: Imagine publică negăsită: " << filename << std::endl;
                return create404();
            }
        }
        
        std::ostringstream ss;
        ss << sourceFile.rdbuf();
        std::string content = ss.str();
        sourceFile.close();
        
        std::cout << "Imagine citită: " << content.length() << " bytes" << std::endl;
        
        if (content.empty()) {
            std::cout << "EROARE: Fișier gol!" << std::endl;
            return create404();
        }
        
        HTTPResponse response;
        response.setProtocol("HTTP");
        response.setHTTPv("1.1");
        response.setStatusCode("200");
        response.setStatus("OK");
        response.setContentType("image/jpeg");
        response.addHeader("Content-Disposition", "attachment; filename=\"" + filename + "\"");
        response.setContent(content);
        
        return response;
    }
    
    std::cout << "Tip download necunoscut: " << type << std::endl;
    return create404();
}

HTTPResponse APIServer::createRedirect(const std::string& location, const std::string& setCookie) {
    HTTPResponse response;
    response.setProtocol("HTTP");
    response.setHTTPv("1.1");
    response.setStatusCode("302");
    response.setStatus("Found");
    response.addHeader("Location", location);
    
    if (!setCookie.empty()) {
        response.addHeader("Set-Cookie", setCookie);
    }
    
    response.setContentType("text/html");
    response.setContent("<html><body>Redirecting...</body></html>");
    
    return response;
}

HTTPResponse APIServer::create404() {
    HTTPResponse response;
    response.setProtocol("HTTP");
    response.setHTTPv("1.1");
    response.setStatusCode("404");
    response.setStatus("Not Found");
    response.setContentType("text/html");
    response.setContent("<html><body><h1>404 - Not Found</h1></body></html>");
    
    return response;
}

bool APIServer::loadUsers() {
    std::lock_guard<std::mutex> lock(_userMutex);
    std::ifstream file("users.json");
    
    if (!file.is_open()) {
        std::ofstream newFile("users.json");
        newFile << "{}";
        newFile.close();
        return true;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    size_t startPos = content.find('{');
    if (startPos == std::string::npos) {
        std::cout << "EROARE: JSON invalid - lipsește '{'" << std::endl;
        return false;
    }
    
    size_t pos = startPos + 1;
    int usersFound = 0;
    
    while (pos < content.length()) {
        // cauta urmatorul '"' care ar putea fi inceputul unui email
        pos = content.find("\"", pos);
        if (pos == std::string::npos) break;
        
        size_t emailStart = pos + 1;
        size_t emailEnd = content.find("\"", emailStart);
        if (emailEnd == std::string::npos) break;
        
        std::string potentialKey = content.substr(emailStart, emailEnd - emailStart);
        
        // verifica ce urmeaza dupa ":"
        size_t colonPos = content.find(":", emailEnd);
        if (colonPos == std::string::npos) break;
        
        // sar peste spatii/newlines
        size_t afterColon = colonPos + 1;
        while (afterColon < content.length() && 
               (content[afterColon] == ' ' || content[afterColon] == '\n' || 
                content[afterColon] == '\r' || content[afterColon] == '\t')) {
            afterColon++;
        }
        
        if (afterColon >= content.length() || content[afterColon] != '{') {
            std::cout << "  Skip cheie internă: \"" << potentialKey << "\"\n";
            pos = emailEnd + 1;
            continue;
        }
        
        std::string email = potentialKey;
        usersFound++;
        std::cout << "\n[" << usersFound << "] Găsit user: " << email << std::endl;
        
        size_t braceStart = afterColon;
        int braceCount = 1;
        size_t braceEnd = braceStart + 1;
        
        while (braceEnd < content.length() && braceCount > 0) {
            if (content[braceEnd] == '{') braceCount++;
            else if (content[braceEnd] == '}') braceCount--;
            braceEnd++;
        }
        
        if (braceCount != 0) {
            std::cout << "EROARE: JSON invalid - paranteză neînchisă pentru user: " << email << std::endl;
            break;
        }
        
        std::string userBlock = content.substr(braceStart, braceEnd - braceStart);
        std::cout << "  User block length: " << userBlock.length() << " bytes\n";
        
        // name
        std::string name = "";
        size_t namePos = userBlock.find("\"name\"");
        if (namePos != std::string::npos) {
            size_t nameValStart = userBlock.find("\"", namePos + 6);
            if (nameValStart != std::string::npos) {
                nameValStart++;
                size_t nameValEnd = userBlock.find("\"", nameValStart);
                if (nameValEnd != std::string::npos) {
                    name = userBlock.substr(nameValStart, nameValEnd - nameValStart);
                }
            }
        }
        
        // password
        std::string password = "";
        size_t passPos = userBlock.find("\"password\"");
        if (passPos != std::string::npos) {
            size_t passValStart = userBlock.find("\"", passPos + 10);
            if (passValStart != std::string::npos) {
                passValStart++;
                size_t passValEnd = userBlock.find("\"", passValStart);
                if (passValEnd != std::string::npos) {
                    password = userBlock.substr(passValStart, passValEnd - passValStart);
                }
            }
        }
        
        // uploded images
        std::vector<std::string> uploadedImages;
        size_t imgsPos = userBlock.find("\"uploaded_images\"");
        if (imgsPos != std::string::npos) {
            size_t arrStart = userBlock.find("[", imgsPos);
            size_t arrEnd = userBlock.find("]", arrStart);
            if (arrStart != std::string::npos && arrEnd != std::string::npos) {
                std::string arrContent = userBlock.substr(arrStart + 1, arrEnd - arrStart - 1);
                
                size_t imgPos = 0;
                while (imgPos < arrContent.length()) {
                    imgPos = arrContent.find("\"", imgPos);
                    if (imgPos == std::string::npos) break;
                    
                    size_t imgStart = imgPos + 1;
                    size_t imgEnd = arrContent.find("\"", imgStart);
                    if (imgEnd == std::string::npos) break;
                    
                    std::string imageName = arrContent.substr(imgStart, imgEnd - imgStart);
                    uploadedImages.push_back(imageName);
                    
                    imgPos = imgEnd + 1;
                }
            }
        }
        
        User user = {name, email, password, uploadedImages};
        _users[email] = user;
        
        pos = braceEnd;
    }
    
    return true;
}

bool APIServer::saveUsers() {
    std::ofstream file("users.json");
    if (!file.is_open()) return false;
    
    file << "{\n";
    bool first = true;
    for (const auto& [email, user] : _users) {
        if (!first) file << ",\n";
        first = false;
        file << "  \"" << email << "\": {\n";
        file << "    \"name\": \"" << user.name << "\",\n";
        file << "    \"email\": \"" << email << "\",\n";
        file << "    \"password\": \"" << user.password << "\",\n";
        
        // uploaded images
        file << "    \"uploaded_images\": [";
        bool firstImg = true;
        for (const auto& img : user.uploadedImages) {
            if (!firstImg) file << ", ";
            firstImg = false;
            file << "\"" << img << "\"";
        }
        file << "]\n";
        
        file << "  }";
    }
    file << "\n}";
    file.close();
    
    std::cout << "Useri salvati: " << _users.size() << std::endl;
    return true;
}
bool APIServer::registerUser(const std::string& name, const std::string& email, const std::string& password) {
    std::lock_guard<std::mutex> lock(_userMutex);
    
    if (_users.find(email) != _users.end()) {
        return false;
    }
    
    User newUser = {name, email, password, {}};
    _users[email] = newUser;
    
    saveUsers();
    
    return true;
}

bool APIServer::authenticateUser(const std::string& email, const std::string& password) {
    std::lock_guard<std::mutex> lock(_userMutex);
    
    auto it = _users.find(email);
    if (it == _users.end()) {
        return false;
    }
    
    return it->second.password == password;
}

HTTPResponse APIServer::handleFileRequest(const std::string& uri, const std::string& sessionId) {
    std::string filepath = "." + uri;
    
    if (uri == "/" || uri.empty()) {
        filepath = "./login.html";
    }
    
    std::cout << "File request: " << uri << " -> " << filepath << std::endl;
    
    if (uri == "/my-photos.html") {
        std::string email = getEmailFromSession(sessionId);
        
        if (email.empty()) {
            return createRedirect("/login.html");
        }
        
        std::string content = readFile(filepath);
        
        if (content.empty()) {
            std::cout << "EROARE: Nu pot citi " << filepath << std::endl;
            return create404();
        }
        
        std::string uploadedHTML;
        
        {
            std::lock_guard<std::mutex> lock(_userMutex);
            auto it = _users.find(email);
            if (it != _users.end()) {
                std::cout << "User gasit: " << email << std::endl;
                std::cout << "  Uploaded images: " << it->second.uploadedImages.size() << std::endl;
                
                // Uploaded images
                if (it->second.uploadedImages.empty()) {
                    uploadedHTML = "<p style='text-align: center; color: #999; padding: 30px;'>Nu ai încărcat încă nicio poza</p>";
                } else {
                    for (const auto& img : it->second.uploadedImages) {
                        uploadedHTML += 
                            "<div class=\"pin-card\">"
                            "<img src=\"/user-uploaded/" + img + "\" alt=\"Uploaded\">"
                            "<div class=\"pin-info\">"
                            "<h3>" + img + "</h3>"
                            "<div class=\"pin-actions\">"
                            "<a href=\"/user-uploaded/" + img + "\" class=\"btn-download\" download>Download</a>"
                            "</div>"
                            "</div>"
                            "</div>";
                    }
                }
            }
        }
        
        size_t pos = content.find("<!--UPLOADED_IMAGES_PLACEHOLDER-->");
        if (pos != std::string::npos) {
            content.replace(pos, 35, uploadedHTML);
        }
        
        size_t downloadedStart = content.find("<!-- Pozele descărcate de user -->");
        if (downloadedStart != std::string::npos) {
            size_t downloadedEnd = content.find("</div>", content.find("<!--DOWNLOADED_IMAGES_PLACEHOLDER-->"));
            if (downloadedEnd != std::string::npos) {
                content.erase(downloadedStart, downloadedEnd - downloadedStart + 6);
            }
        }
        
        HTTPResponse response;
        response.setProtocol("HTTP");
        response.setHTTPv("1.1");
        response.setStatusCode("200");
        response.setStatus("OK");
        response.setContentType("text/html; charset=UTF-8");
        response.setContent(content);
        
        return response;
    }
    
    std::string content = readFile(filepath);
    
    if (content.empty()) {
        std::cout << "Fișier negăsit: " << filepath << std::endl;
        return create404();
    }
    
    HTTPResponse response;
    response.setProtocol("HTTP");
    response.setHTTPv("1.1");
    response.setStatusCode("200");
    response.setStatus("OK");
    response.setContentType(getContentType(filepath));
    response.setContent(content);
    
    return response;
}