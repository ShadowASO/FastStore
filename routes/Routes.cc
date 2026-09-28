#include "Routes.h"
#include <drogon/drogon.h>
#include "controllers/HealthController.h"
#include "controllers/EchoController.h"
#include "controllers/UserController.h"
#include "controllers/AuthController.h"
#include "middleware/AuthFilter.h"

namespace faststore::routes
{
    void registerAll()
    {
        drogon::app().registerHandler(
            "/api/v1/health", &HealthController::checkHealth, {drogon::Get});
        drogon::app().registerHandler(
            "/api/v1/echo", &EchoController::echo, {drogon::Post});
        drogon::app().registerHandler(
            "/api/v1/auth/register", &AuthController::registerUser, {drogon::Post});
        drogon::app().registerHandler(
            "/api/v1/auth/login", &AuthController::login, {drogon::Post});
            
        // Keep the original registration URL, with the same password validation.
        drogon::app().registerHandler(
            "/api/v1/users", &AuthController::registerUser, {drogon::Post});
        drogon::app().registerHandler(
            "/api/v1/users/me", &UserController::getCurrentUser,
            {drogon::Get, "AuthFilter"});
        drogon::app().registerHandler(
            "/api/v1/users/{id}", &UserController::getById,
            {drogon::Get, "AuthFilter"});
    }
}
