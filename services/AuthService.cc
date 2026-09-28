// services/AuthService.cc
#include "AuthService.h"
#include "auth/PasswordService.h"


using namespace faststore::services;
using namespace faststore::auth;
using namespace faststore::dto;


void AuthService::registerUser(
    const RegisterRequest &request,
    std::function<void(std::variant<AuthTokens, AuthError>)> callback)
{
    userRepo_->findByEmail(
        request.email,
        //[this, request, callback](std::optional<faststore::repositories::UserRepository::UserModel> existing)
        [this, request, callback](std::optional<faststore::repositories::UserModel> existing)
        {
            if (existing.has_value())
            {
                callback(AuthError{"CONFLICT", "A user with this email already exists."});
                return;
            }
            //faststore::repositories::UserRepository::UserModel newUser;
            faststore::repositories::UserModel newUser;
            newUser.setEmail(request.email);
            newUser.setFirstName(request.firstName);
            newUser.setLastName(request.lastName);
            newUser.setPasswordHash(PasswordService::hash(request.password));
            newUser.setRole("customer");
            userRepo_->create(
                newUser,
                //[this, callback](faststore::repositories::UserRepository::UserModel created)
                [this, callback](faststore::repositories::UserModel created)
                {
                    AuthTokens tokens;
                    tokens.accessToken = jwtService_->issueAccessToken(
                        created.getValueOfId(), created.getValueOfEmail(), created.getValueOfRole());
                    tokens.refreshToken = jwtService_->issueRefreshToken(created.getValueOfId());
                    tokens.expiresInSeconds = jwtService_->accessTokenTtlSeconds();
                    callback(tokens);
                },
                [callback](const drogon::orm::DrogonDbException &)
                {
                    callback(AuthError{"INTERNAL_ERROR", "Failed to create user."});
                });
        },
        [callback](const drogon::orm::DrogonDbException &)
        {
            callback(AuthError{"INTERNAL_ERROR", "Failed to check existing user."});
        });
}
void AuthService::login(
    const LoginRequest &request,
    std::function<void(std::variant<AuthTokens, AuthError>)> callback)
{
    userRepo_->findByEmail(
        request.email,
        //[this, request, callback](std::optional<faststore::repositories::UserRepository::UserModel> user)
        [this, request, callback](std::optional<faststore::repositories::UserModel> user)
        {
            // Deliberately identical error for "no such user" and "wrong
            // password"   distinguishing them lets an attacker enumerate
            // valid registered emails, which is exactly the information
            // leak a login endpoint must never provide.
            if (!user.has_value() ||
                !PasswordService::verify(request.password, user->getValueOfPasswordHash()))
            {
                callback(AuthError{"UNAUTHORIZED", "Invalid email or password."});
                return;
            }
            if (!user->getValueOfIsActive())
            {
                callback(AuthError{"UNAUTHORIZED", "This account has been deactivated."});
                return;
            }
            AuthTokens tokens;
            tokens.accessToken = jwtService_->issueAccessToken(
                user->getValueOfId(), user->getValueOfEmail(), user->getValueOfRole());
            tokens.refreshToken = jwtService_->issueRefreshToken(user->getValueOfId());
            tokens.expiresInSeconds = jwtService_->accessTokenTtlSeconds();
            callback(tokens);
        },
        [callback](const drogon::orm::DrogonDbException &)
        {
            callback(AuthError{"INTERNAL_ERROR", "Failed to look up user."});
        });
}