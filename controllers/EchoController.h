//controllers/EchoController.h
#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

class EchoController 
{

public:
    
    void echo(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback);
};
