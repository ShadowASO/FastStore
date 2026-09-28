// controllers/AuthController.cc
#include "AuthController.h"
#include "services/AuthService.h"
#include "repositories/UserRepository.h"
#include "auth/JwtService.h"
#include "utils/ResponseBuilder.h"
#include "validation/Validator.h"
#include "config/AppConfig.h"

using namespace faststore::utils;
using namespace faststore::validation;
using namespace faststore::services;
using namespace faststore::repositories;
using namespace faststore::auth;
using namespace faststore::dto;

//using namespace faststore::config;

namespace
{
    std::shared_ptr<JwtService> makeJwtService()
    {
        return std::make_shared<JwtService>(appConfig().get("JWT_SECRET"));
    }
}
void AuthController::registerUser(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback)
{
    auto jsonBody = req->getJsonObject();
    if (!jsonBody || !jsonBody->isObject())
    {
        callback(ResponseBuilder::error("INVALID_JSON", "Request body must be valid JSON.", k400BadRequest));
        return;
    }
    Validator v;
    v.required(*jsonBody, "email").isEmail(*jsonBody, "email")
        .required(*jsonBody, "password").minLength(*jsonBody, "password", 8)
        .required(*jsonBody, "firstName").minLength(*jsonBody, "firstName", 1)
        .required(*jsonBody, "lastName").minLength(*jsonBody, "lastName", 1);
    if (v.hasErrors())
    {
        callback(ResponseBuilder::validationFailed(v.issues()));
        return;
    }
    RegisterRequest request{
        (*jsonBody)["email"].asString(),
        (*jsonBody)["password"].asString(),
        (*jsonBody)["firstName"].asString(),
        (*jsonBody)["lastName"].asString()};
    auto dbClient = drogon::app().getDbClient("default");
    auto repo = std::make_shared<UserRepository>(dbClient);
    auto jwtService = makeJwtService();
    auto authService = std::make_shared<AuthService>(repo, jwtService);
    authService->registerUser(
        request,
        [callback, authService](std::variant<AuthTokens, AuthError> result)
        {
            if (std::holds_alternative<AuthError>(result))
            {
                auto err = std::get<AuthError>(result);
                auto status = (err.code == "CONFLICT") ? k409Conflict : k500InternalServerError;
                callback(ResponseBuilder::error(err.code, err.message, status));
                return;
            }
            auto tokens = std::get<AuthTokens>(result);
            Json::Value data;
            data["accessToken"] = tokens.accessToken;
            data["refreshToken"] = tokens.refreshToken;
            data["expiresIn"] = tokens.expiresInSeconds;
            callback(ResponseBuilder::success(data, k201Created));
        });
}
void AuthController::login(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback)
{
    auto jsonBody = req->getJsonObject();
    if (!jsonBody || !jsonBody->isObject())
    {
        callback(ResponseBuilder::error("INVALID_JSON", "Request body must be valid JSON.", k400BadRequest));
        return;
    }
    Validator v;
    v.required(*jsonBody, "email").isEmail(*jsonBody, "email")
        .required(*jsonBody, "password").minLength(*jsonBody, "password", 1);
    if (v.hasErrors())
    {
        callback(ResponseBuilder::validationFailed(v.issues()));
        return;
    }
    LoginRequest request{(*jsonBody)["email"].asString(), (*jsonBody)["password"].asString()};
    auto dbClient = drogon::app().getDbClient("default");
    auto repo = std::make_shared<UserRepository>(dbClient);
    auto jwtService = makeJwtService();
    auto authService = std::make_shared<AuthService>(repo, jwtService);
    authService->login(
        request,
        [callback, authService](std::variant<AuthTokens, AuthError> result)
        {
            if (std::holds_alternative<AuthError>(result))
            {
                auto err = std::get<AuthError>(result);
                callback(ResponseBuilder::error(err.code, err.message,
                    err.code == "UNAUTHORIZED" ? k401Unauthorized : k500InternalServerError));
                return;
            }
            auto tokens = std::get<AuthTokens>(result);
            Json::Value data;
            data["accessToken"] = tokens.accessToken;
            data["refreshToken"] = tokens.refreshToken;
            data["expiresIn"] = tokens.expiresInSeconds;
            callback(ResponseBuilder::success(data));
        });
}