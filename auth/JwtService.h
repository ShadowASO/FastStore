// auth/JwtService.h

#pragma once

#include <string>
#include <optional>
#include <chrono>

namespace faststore::auth
{

    struct JwtClaims
    {
        std::string userId;
        std::string email;
        std::string role;
        int64_t issuedAt;
        int64_t expiresAt;
    };

    class JwtService
    {
    public:
        explicit JwtService(std::string secret, int accessTokenTtlSeconds = 900, // 15 minutes
                            int refreshTokenTtlSeconds = 1209600);               // 14 days
        std::string issueAccessToken(const std::string &userId,
                                     const std::string &email,
                                     const std::string &role) const;
        std::string issueRefreshToken(const std::string &userId) const;
        // Returns std::nullopt for any invalid, expired, or tampered token.
        // Deliberately does not distinguish *why* verification failed in
        // its return type   that distinction belongs in logs, not in
        // information we hand back to a potential attacker.
        std::optional<JwtClaims> verify(const std::string &token) const;
        [[nodiscard]] int accessTokenTtlSeconds() const { return accessTokenTtlSeconds_; }

    private:
        std::string secret_;
        int accessTokenTtlSeconds_;
        int refreshTokenTtlSeconds_;
        std::string sign(const std::string &headerAndPayload) const;
        static std::string base64UrlEncode(const std::string &input);
        static std::string base64UrlDecode(const std::string &input);
    };
}