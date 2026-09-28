// auth/PasswordService.cc

#include "PasswordService.h"
#include <argon2.h>
#include <vector>
#include <stdexcept>
#include <random>

using namespace faststore::auth;

namespace
{
    // Argon2id parameters. These numbers are a deliberate balance
    // increasing them raises the cost of brute-force attacks but also
    // raises the cost (in latency and memory) of every legitimate login.
    // OWASP's current guidance for Argon2id is a reasonable floor of
    // 19 MiB memory / 2 iterations / 1 parallelism for constrained
    // environments, or higher when you can afford it. We're targeting a
    // server-side workload, so we go higher than the mobile-constrained
    // floor.

    constexpr uint32_t T_COST = 3;     // iterations
    constexpr uint32_t M_COST = 65536; // memory cost in KiB (64 MiB)
    constexpr uint32_t PARALLELISM = 1;
    constexpr size_t SALT_LENGTH = 16;
    constexpr size_t HASH_LENGTH = 32;

    std::vector<uint8_t> generateSalt()
    {
        std::vector<uint8_t> salt(SALT_LENGTH);
        std::random_device rd;
        for (auto &byte : salt)
        {
            byte = static_cast<uint8_t>(rd() & 0xFF);
        }
        return salt;
    }
}
std::string PasswordService::hash(const std::string &plaintextPassword)
{
    auto salt = generateSalt();

    // Argon2's encoded output format includes the salt, parameters, and
    // hash all in one self-describing string   this is the string we
    // store directly in password_hash, nothing else needed.

    size_t encodedLen = argon2_encodedlen(
        T_COST, M_COST, PARALLELISM, SALT_LENGTH, HASH_LENGTH, Argon2_id);


    std::vector<char> encoded(encodedLen);

    int result = argon2id_hash_encoded(
        T_COST, M_COST, PARALLELISM,
        plaintextPassword.data(), plaintextPassword.size(),
        salt.data(), salt.size(),
        HASH_LENGTH,
        encoded.data(), encoded.size());


    if (result != ARGON2_OK)
    {
        throw std::runtime_error(
            std::string("Password hashing failed: ") + argon2_error_message(result));
    }

    return std::string(encoded.data());
}

bool PasswordService::verify(const std::string &plaintextPassword,
                             const std::string &encodedHash)
{
    int result = argon2id_verify(
        encodedHash.c_str(),
        plaintextPassword.data(), 
        plaintextPassword.size());

    // ARGON2_OK means match. Every other result   including
    // ARGON2_VERIFY_MISMATCH   means "not a match" and must be treated
    // as a simple false, never as an exception. Distinguishing "wrong
    // password" from "corrupted hash" to the caller creates a timing
    // and information-disclosure surface we don't want. 
    
    return result == ARGON2_OK;
}
