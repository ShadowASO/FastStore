//controllers/HealthController.cc
#include "HealthController.h"

#include <drogon/drogon.h>
#include <jsoncpp/json/json.h>
#include "utils/ResponseBuilder.h"

using namespace faststore::utils;

void HealthController::checkHealth(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    (void)req;

    Json::Value data;
    data["status"] = "ok";
    data["service"] = "faststore-api";
    data["version"] = "0.1.0";
    data["Threads_using"] = drogon::app().getThreadNum();
    data["Threads_sys"] = std::thread::hardware_concurrency();

    auto resp =drogon::HttpResponse::newHttpJsonResponse(data);

    //resp->setStatusCode(drogon::k200OK);
    //callback(resp);
    callback(ResponseBuilder::success(data));
}