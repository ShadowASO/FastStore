// auth/JwtService.cc

#include "JwtService.h"

#include <json/json.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>

#include <chrono>
#include <cstdint>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace faststore::auth
{
    namespace
    {
        std::int64_t nowSeconds()
        {
            return std::chrono::duration_cast<std::chrono::seconds>(
                       std::chrono::system_clock::now().time_since_epoch())
                .count();
        }

        std::string toJson(const Json::Value &value)
        {
            Json::StreamWriterBuilder writer;
            writer["indentation"] = "";
            return Json::writeString(writer, value);
        }

        bool parseJson(const std::string &text, Json::Value &value)
        {
            Json::CharReaderBuilder reader;
            std::string errors;
            std::istringstream stream{text};

            return Json::parseFromStream(reader, stream, &value, &errors);
        }
    }

    JwtService::JwtService(
        std::string secret,
        int accessTokenTtlSeconds,
        int refreshTokenTtlSeconds)
        : secret_(std::move(secret)),
          accessTokenTtlSeconds_(accessTokenTtlSeconds),
          refreshTokenTtlSeconds_(refreshTokenTtlSeconds)
    {
        if (secret_.empty())
            throw std::invalid_argument("O segredo JWT não pode ser vazio");

        if (accessTokenTtlSeconds_ <= 0 || refreshTokenTtlSeconds_ <= 0)
            throw std::invalid_argument("Os prazos dos tokens devem ser positivos");

        if (secret_.size() > static_cast<std::size_t>(
                                 std::numeric_limits<int>::max()))
            throw std::invalid_argument("O segredo JWT é grande demais");
    }

    std::string JwtService::base64UrlEncode(const std::string &input)
    {
        static constexpr char table[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

        std::string output;
        output.reserve((input.size() / 3) * 4 + 4);

        for (std::size_t i = 0; i < input.size(); i += 3)
        {
            const auto a = static_cast<unsigned char>(input[i]);
            const auto b = i + 1 < input.size()
                               ? static_cast<unsigned char>(input[i + 1])
                               : 0;
            const auto c = i + 2 < input.size()
                               ? static_cast<unsigned char>(input[i + 2])
                               : 0;

            output.push_back(table[a >> 2]);
            output.push_back(table[((a & 0x03) << 4) | (b >> 4)]);

            if (i + 1 < input.size())
                output.push_back(table[((b & 0x0F) << 2) | (c >> 6)]);

            if (i + 2 < input.size())
                output.push_back(table[c & 0x3F]);
        }

        return output;
    }

    std::string JwtService::base64UrlDecode(const std::string &input)
    {
        // Esta função rejeita entrada inválida lançando invalid_argument.
        if (input.size() % 4 == 1)
            throw std::invalid_argument("Base64URL inválido");

        auto digit = [](char c) -> int
        {
            if (c >= 'A' && c <= 'Z')
                return c - 'A';
            if (c >= 'a' && c <= 'z')
                return c - 'a' + 26;
            if (c >= '0' && c <= '9')
                return c - '0' + 52;
            if (c == '-')
                return 62;
            if (c == '_')
                return 63;
            return -1;
        };

        std::string output;
        output.reserve((input.size() / 4) * 3 + 3);

        unsigned int bits = 0;
        int available = 0;

        for (char c : input)
        {
            const int value = digit(c);
            if (value < 0)
                throw std::invalid_argument("Base64URL contém caractere inválido");

            bits = (bits << 6) | static_cast<unsigned int>(value);
            available += 6;

            if (available >= 8)
            {
                available -= 8;
                output.push_back(
                    static_cast<char>((bits >> available) & 0xFF));
            }
        }

        // Os bits finais não utilizados devem ser zero.
        if (available > 0 && (bits & ((1U << available) - 1U)) != 0)
            throw std::invalid_argument("Base64URL contém bits finais inválidos");

        return output;
    }

    std::string JwtService::sign(const std::string &headerAndPayload) const
    {
        unsigned char digest[EVP_MAX_MD_SIZE];
        unsigned int digestLength = 0;

        if (HMAC(
                EVP_sha256(),
                secret_.data(),
                static_cast<int>(secret_.size()),
                reinterpret_cast<const unsigned char *>(
                    headerAndPayload.data()),
                headerAndPayload.size(),
                digest,
                &digestLength) == nullptr)
        {
            throw std::runtime_error("Falha ao assinar o JWT");
        }

        return base64UrlEncode(
            std::string{
                reinterpret_cast<const char *>(digest),
                digestLength});
    }

    std::string JwtService::issueAccessToken(
        const std::string &userId,
        const std::string &email,
        const std::string &role) const
    {
        const auto issuedAt = nowSeconds();

        Json::Value header(Json::objectValue);
        header["alg"] = "HS256";
        header["typ"] = "JWT";

        Json::Value payload(Json::objectValue);
        payload["sub"] = userId;
        payload["email"] = email;
        payload["role"] = role;
        payload["type"] = "access";
        payload["iat"] = static_cast<Json::Int64>(issuedAt);
        payload["exp"] =
            static_cast<Json::Int64>(issuedAt + accessTokenTtlSeconds_);

        const std::string signingInput =
            base64UrlEncode(toJson(header)) + "." +
            base64UrlEncode(toJson(payload));

        return signingInput + "." + sign(signingInput);
    }

    std::string JwtService::issueRefreshToken(
        const std::string &userId) const
    {
        const auto issuedAt = nowSeconds();

        Json::Value header(Json::objectValue);
        header["alg"] = "HS256";
        header["typ"] = "JWT";

        Json::Value payload(Json::objectValue);
        payload["sub"] = userId;
        payload["type"] = "refresh";
        payload["iat"] = static_cast<Json::Int64>(issuedAt);
        payload["exp"] =
            static_cast<Json::Int64>(issuedAt + refreshTokenTtlSeconds_);

        const std::string signingInput =
            base64UrlEncode(toJson(header)) + "." +
            base64UrlEncode(toJson(payload));

        return signingInput + "." + sign(signingInput);
    }

    std::optional<JwtClaims> JwtService::verify(
        const std::string &token) const
    {
        const auto firstDot = token.find('.');
        if (firstDot == std::string::npos || firstDot == 0)
            return std::nullopt;

        const auto secondDot = token.find('.', firstDot + 1);
        if (secondDot == std::string::npos ||
            secondDot == firstDot + 1 ||
            secondDot + 1 == token.size() ||
            token.find('.', secondDot + 1) != std::string::npos)
        {
            return std::nullopt;
        }

        const std::string signingInput = token.substr(0, secondDot);
        const std::string providedSignature = token.substr(secondDot + 1);
        const std::string expectedSignature = sign(signingInput);

        if (providedSignature.size() != expectedSignature.size() ||
            CRYPTO_memcmp(
                providedSignature.data(),
                expectedSignature.data(),
                expectedSignature.size()) != 0)
        {
            return std::nullopt;
        }

        Json::Value header;
        Json::Value payload;

        try
        {
            if (!parseJson(
                    base64UrlDecode(token.substr(0, firstDot)),
                    header) ||
                !parseJson(
                    base64UrlDecode(
                        token.substr(
                            firstDot + 1,
                            secondDot - firstDot - 1)),
                    payload))
            {
                return std::nullopt;
            }
        }
        catch (const std::invalid_argument &)
        {
            return std::nullopt;
        }

        if (!header.isObject() ||
            !header["alg"].isString() ||
            header["alg"].asString() != "HS256" ||
            !header["typ"].isString() ||
            header["typ"].asString() != "JWT" ||
            !payload.isObject() ||
            !payload["sub"].isString() ||
            payload["sub"].asString().empty() ||
            !payload["type"].isString() ||
            !payload["iat"].isInt64() ||
            !payload["exp"].isInt64())
        {
            return std::nullopt;
        }

        const auto type = payload["type"].asString();
        if (type != "access" && type != "refresh")
            return std::nullopt;

        if (type == "access" &&
            (!payload["email"].isString() ||
             !payload["role"].isString()))
        {
            return std::nullopt;
        }

        const auto issuedAt = payload["iat"].asInt64();
        const auto expiresAt = payload["exp"].asInt64();
        const auto now = nowSeconds();

        if (issuedAt > now || expiresAt <= now || expiresAt <= issuedAt)
            return std::nullopt;

        JwtClaims claims;
        claims.userId = payload["sub"].asString();
        claims.email = type == "access"
                           ? payload["email"].asString()
                           : "";
        claims.role = type == "access"
                          ? payload["role"].asString()
                          : "";
        claims.issuedAt = issuedAt;
        claims.expiresAt = expiresAt;

        return claims;
    }

} // namespace faststore::auth