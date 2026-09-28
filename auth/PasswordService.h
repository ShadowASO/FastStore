//auth/PasswordService.h

#pragma once

#include <string>

namespace faststore::auth
{
    class PasswordService
    {
    
    public:
        // Returns an encoded Argon2id hash string (includes algorithm,
        // version, and parameters   this is why we don't need a separate
        // "salt" column; Argon2's encoded output is self-describing).
        static std::string hash(const std::string &plaintextPassword);
        
        static bool verify(const std::string &plaintextPassword,
                           const std::string &encodedHash);
    };
}