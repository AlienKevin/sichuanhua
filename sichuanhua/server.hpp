#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <string>
#include "dict.hpp"

class HTTPServer {
public:
    HTTPServer();

    void start();
    void stop();

private:
    std::string getAudioFileName(int fileId);
    std::string getAudioResponseHeader(const std::string& fileName);
    std::string getJSONResponseHeader(const std::string& json);
    std::string readAudioFile(const std::string& fileName);

    int serverSocket;
    bool isRunning;
    Dict dict;
};

#endif  // HTTP_SERVER_H
