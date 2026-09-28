// dto/AuthDto.h

#pragma once

#include <string>

namespace faststore::dto
{

    struct RegisterRequest
    {
        std::string email;
        std::string password;
        std::string firstName;
        std::string lastName;
    };
   
    struct LoginRequest
    {
        std::string email;
        std::string password;
    };
    
    struct AuthTokens
    {
        std::string accessToken;
        std::string refreshToken;
        int expiresInSeconds;
    };
}