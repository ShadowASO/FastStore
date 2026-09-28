//controller/HealthController.h
/*
Teste:
curl -v http://127.0.0.1:8080/HealthController/api/v1/health
*/
#pragma once

#include <drogon/HttpController.h>

//#include <functional>

class HealthController
   // : public drogon::HttpController<HealthController>
{
public:
    // METHOD_LIST_BEGIN

    // ADD_METHOD_TO(
    //     HealthController::checkHealth,
    //     "/api/v1/health",
    //     drogon::Get);

    // METHOD_LIST_END

    void checkHealth(
        const drogon::HttpRequestPtr &req,
        std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};