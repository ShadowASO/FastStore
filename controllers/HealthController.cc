//controllers/HealthController.cc
#include "HealthController.h"

#include <drogon/drogon.h>
#include <jsoncpp/json/json.h>

void HealthController::checkHealth(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    (void)req;

    Json::Value response;
    response["status"] = "ok";
    response["service"] = "faststore-api";
    response["version"] = "0.1.0";

    auto resp =drogon::HttpResponse::newHttpJsonResponse(response);

    resp->setStatusCode(drogon::k200OK);
    callback(resp);
    
}