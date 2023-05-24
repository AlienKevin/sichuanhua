#include "server.hpp"
#include <iostream>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPServerRequest.h>

void MyRequestHandler::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
{
    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");

    std::ostream &ostr = response.send();

    std::string keyword = parseKeyword(request.getURI());
    std::string decodedKeyword;
    Poco::URI::decode(keyword, decodedKeyword);

    SearchResult result = dict.search(decodedKeyword);

    ostr << Dict::searchResultToJson(result).dump();
}

std::string MyRequestHandler::parseKeyword(const std::string &uri)
{
    std::string keyword;
    std::size_t startPos = uri.find("/search/");
    if (startPos != std::string::npos)
    {
        keyword = uri.substr(startPos + 8);
    }
    return keyword;
}

HTTPRequestHandler *MyRequestHandlerFactory::createRequestHandler(const HTTPServerRequest &)
{
    return new MyRequestHandler(dict);
}

int MyServerApp::main(const std::vector<std::string> &)
{
    Dict dict;
    HTTPServer server(new MyRequestHandlerFactory(dict), ServerSocket(8080), new HTTPServerParams);
    server.start();
    std::cout << "Server started on port 8080." << std::endl;
    waitForTerminationRequest();
    server.stop();
    std::cout << "Server stopped." << std::endl;
    return Application::EXIT_OK;
}
