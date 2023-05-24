#include "server.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define PORT 8080

HTTPServer::HTTPServer() {
    serverSocket = -1;
    isRunning = false;
    dict = Dict();
}

std::string HTTPServer::getJSONResponseHeader(const std::string& json) {
    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n";
    oss << "Content-Type: application/json\r\n";
    oss << "Content-Length: " << json.length() << "\r\n";
    oss << "\r\n";
    oss << json;

    return oss.str();
}

std::string decodePercentEncoding(const std::string& encodedString) {
    std::string decodedString;
    std::stringstream ss;

    for (size_t i = 0; i < encodedString.length(); ++i) {
        if (encodedString[i] == '%') {
            std::string hexCode = encodedString.substr(i + 1, 2);
            int charCode;

            ss.clear();
            ss << std::hex << hexCode;
            ss >> charCode;

            decodedString += static_cast<char>(charCode);

            i += 2; // Skip the two hex digits
        } else {
            decodedString += encodedString[i];
        }
    }

    return decodedString;
}

void HTTPServer::start() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress{}, clientAddress{};
    socklen_t clientLength;

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        return;
    }

    // Bind socket to IP and port
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);
    if (bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Failed to bind socket" << std::endl;
        return;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Failed to listen on socket" << std::endl;
        return;
    }

    std::cout << "Server running on port " << PORT << std::endl;

    while (true) {
        clientLength = sizeof(clientAddress);

        // Accept a new client connection
        clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddress, &clientLength);
        if (clientSocket == -1) {
            std::cerr << "Failed to accept client connection" << std::endl;
            return;
        }

        // Receive request from the client
        char buffer[BUFFER_SIZE];
        ssize_t bytesRead = read(clientSocket, buffer, BUFFER_SIZE);
        if (bytesRead == -1) {
            std::cerr << "Failed to read request from client" << std::endl;
            close(clientSocket);
            continue;
        }

        // Parse the request
        std::string request(buffer, bytesRead);
        std::istringstream iss(request);
        std::string method, path;
        iss >> method >> path;

        // Ensure the request is valid
        if (method != "GET") {
            std::cerr << "Invalid request method: " << method << std::endl;
            close(clientSocket);
            continue;
        }
        
        if (path.starts_with("/search/")) {
            std::string query = decodePercentEncoding(path.substr(8));  // Remove leading '/search/'
            std::cout << query << std::endl;
            
            if (query.empty()) {
                std::cerr << "Empty search query" << std::endl;
                close(clientSocket);
                continue;
            }
            
            SearchResult result = dict.search(query);
            
            // Prepare the response
            std::string responseHeader = getJSONResponseHeader(Dict::searchResultToJson(result).dump());
            if (responseHeader.empty()) {
                close(clientSocket);
                continue;
            }
            
            // Send the response header
            ssize_t bytesSent = write(clientSocket, responseHeader.c_str(), responseHeader.size());
            if (bytesSent == -1) {
                std::cerr << "Failed to send response header" << std::endl;
                close(clientSocket);
                continue;
            }
            
            // Close the client connection
            close(clientSocket);
        }
    }
}

void HTTPServer::stop() {
    isRunning = false;
}
