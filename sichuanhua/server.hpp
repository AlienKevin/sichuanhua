#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <string>
#include "dict.hpp"

class HTTPServer
{
public:
    HTTPServer();

    void start();
    void stop();

private:
    std::string getJSONResponseHeader(const std::string &json);

    int serverSocket;
    bool isRunning;
    Dict dict;
};

#endif // HTTP_SERVER_H
