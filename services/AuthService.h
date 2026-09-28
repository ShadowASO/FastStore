// services/AuthService.h
#pragma once
#include "repositories/UserRepository.h"
#include "auth/JwtService.h"
#include "dto/AuthDto.h"
#include <functional>
#include <variant>
#include <string>
namespace faststore::services
{
    struct AuthError
    {
        std::string code;
        std::string message;
    };
    class AuthService
    {
    public:
        AuthService(std::shared_ptr<faststore::repositories::UserRepository> userRepo,
                    std::shared_ptr<faststore::auth::JwtService> jwtService)
            : userRepo_(std::move(userRepo)), jwtService_(std::move(jwtService))
        {
        }
        void registerUser(
            const faststore::dto::RegisterRequest &request,
            std::function<void(std::variant<faststore::dto::AuthTokens, AuthError>)> callback);
        void login(
            const faststore::dto::LoginRequest &request,
            std::function<void(std::variant<faststore::dto::AuthTokens, AuthError>)> callback);

    private:
        std::shared_ptr<faststore::repositories::UserRepository> userRepo_;
        std::shared_ptr<faststore::auth::JwtService> jwtService_;
    };
}