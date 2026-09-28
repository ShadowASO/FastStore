// controllers/AuthController.h
#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

class AuthController //: public drogon::HttpController<AuthController>
{
public:
    void registerUser(const HttpRequestPtr &req,
                      std::function<void(const HttpResponsePtr &)> &&callback);
    void login(const HttpRequestPtr &req,
               std::function<void(const HttpResponsePtr &)> &&callback);
};