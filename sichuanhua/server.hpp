#ifndef MYSERVER_H
#define MYSERVER_H

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/URI.h>
#include <nlohmann/json.hpp>
#include "dict.hpp"

using namespace Poco::Net;
using namespace Poco::Util;
using json = nlohmann::json;

class MyRequestHandler : public HTTPRequestHandler
{
private:
    Dict &dict;

public:
    MyRequestHandler(Dict &dict) : dict(dict) {}
    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response);

private:
    std::string parseKeyword(const std::string &uri);
};

class MyRequestHandlerFactory : public HTTPRequestHandlerFactory
{
private:
    Dict &dict;

public:
    MyRequestHandlerFactory(Dict &dict) : dict(dict) {}
    HTTPRequestHandler *createRequestHandler(const HTTPServerRequest &);
};

class MyServerApp : public ServerApplication
{
private:
    Dict dict;

protected:
    int main(const std::vector<std::string> &);
};

#endif // MYSERVER_H
