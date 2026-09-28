// repositories/UserRepository.h
#pragma once

#include <drogon/orm/Mapper.h>
#include <drogon/orm/DbClient.h>
#include "models/Users.h"
#include <functional>
#include <optional>
#include <string>

namespace faststore::repositories
{
    using UserModel = drogon_model::faststore::Users;
    class UserRepository
    {
    public:
        explicit UserRepository(const drogon::orm::DbClientPtr &dbClient)
            : mapper_(dbClient)
        {
        }
        void findById(const std::string &id,
                      std::function<void(std::optional<UserModel>)> onResult,
                      std::function<void(const drogon::orm::DrogonDbException &)> onError);
        void findByEmail(const std::string &email,
                         std::function<void(std::optional<UserModel>)> onResult,
                         std::function<void(const drogon::orm::DrogonDbException &)> onError);
        void create(UserModel user,
                    std::function<void(UserModel)> onResult,
                    std::function<void(const drogon::orm::DrogonDbException &)> onError);

    private:
        drogon::orm::Mapper<UserModel> mapper_;
    };
}