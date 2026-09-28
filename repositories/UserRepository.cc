#include "UserRepository.h"

#include <drogon/orm/Criteria.h>
#include <drogon/orm/Exception.h>

#include <utility>

namespace faststore::repositories
{
    using drogon::orm::Criteria;
    using drogon::orm::CustomSql;
    using drogon::orm::DrogonDbException;
    using drogon::orm::UnexpectedRows;

    void UserRepository::findById(
        const std::string &id,
        std::function<void(std::optional<UserModel>)> onResult,
        std::function<void(const DrogonDbException &)> onError)
    {
        drogon::orm::Criteria criteria(
            "id",
            drogon::orm::CompareOperator::EQ,
            id);

        mapper_.findOne(
            criteria,
            [onResult](UserModel user)
            {
                onResult(std::move(user));
            },
            [onResult, onError = std::move(onError)](const DrogonDbException &e)
            {
                if (dynamic_cast<const UnexpectedRows *>(&e) != nullptr)
                {
                    onResult(std::nullopt);
                    return;
                }
                onError(e);
            });
    }

    void UserRepository::findByEmail(
        const std::string &email,
        std::function<void(std::optional<UserModel>)> onResult,
        std::function<void(const DrogonDbException &)> onError)
    {
        Criteria criteria(
            CustomSql("LOWER(email) = LOWER($?)"),
            email);

        mapper_.findOne(
            criteria,
            [onResult](UserModel user)
            {
                onResult(std::move(user));
            },
            [onResult, onError = std::move(onError)](const DrogonDbException &e)
            {
                if (dynamic_cast<const UnexpectedRows *>(&e) != nullptr)
                {
                    onResult(std::nullopt);
                    return;
                }

                onError(e);
            });
    }

    void UserRepository::create(
        UserModel user,
        std::function<void(UserModel)> onResult,
        std::function<void(const DrogonDbException &)> onError)
    {
        mapper_.insert(
            user,
            [onResult = std::move(onResult)](UserModel inserted)
            {
                onResult(std::move(inserted));
            },
            std::move(onError));
    }
}