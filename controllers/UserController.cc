// controllers/UserController.cc

#include "UserController.h"

#include "repositories/UserRepository.h"
#include "utils/ResponseBuilder.h"
#include "validation/Validator.h"

#include <drogon/drogon.h>

#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>

using faststore::repositories::UserRepository;
using faststore::utils::ResponseBuilder;
using faststore::validation::Validator;

using drogon::HttpRequestPtr;
using drogon::HttpResponsePtr;

using UserModel = faststore::repositories::UserModel;

namespace
{
    Json::Value userToJson(const UserModel &user)
    {
        Json::Value data(Json::objectValue);
        data["id"] = user.getValueOfId();
        data["email"] = user.getValueOfEmail();
        data["firstName"] = user.getValueOfFirstName();
        data["lastName"] = user.getValueOfLastName();
        data["role"] = user.getValueOfRole();
        return data;
    }
}

void UserController::getById(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback,
    std::string id)
{
    (void)req;

    if (id.empty())
    {
        callback(ResponseBuilder::notFound("User", id));
        return;
    }

    auto repo = std::make_shared<UserRepository>(
        drogon::app().getDbClient("default"));

    repo->findById(
        id,
        [callback, repo, id](std::optional<UserModel> user)
        {
            if (!user || !user->getValueOfIsActive())
            {
                callback(ResponseBuilder::notFound("User", id));
                return;
            }

            callback(ResponseBuilder::success(userToJson(*user)));
        },
        [callback, repo](const drogon::orm::DrogonDbException &e)
        {
            LOG_ERROR << "Database error in getById: "
                      << e.base().what();
            callback(ResponseBuilder::internalError());
        });
}

void UserController::getCurrentUser(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback)
{
    std::string userId;

    try
    {
        userId = req->attributes()->get<std::string>("userId");
    }
    catch (const std::exception &e)
    {
        LOG_ERROR << "Missing or invalid userId attribute: " << e.what();
        callback(ResponseBuilder::unauthorized(
            "Authentication required."));
        return;
    }

    if (userId.empty())
    {
        callback(ResponseBuilder::unauthorized(
            "Authentication required."));
        return;
    }

    auto repo = std::make_shared<UserRepository>(
        drogon::app().getDbClient("default"));

    repo->findById(
        userId,
        [callback, repo](std::optional<UserModel> user)
        {
            if (!user || !user->getValueOfIsActive())
            {
                callback(ResponseBuilder::unauthorized(
                    "User account no longer exists."));
                return;
            }

            callback(ResponseBuilder::success(userToJson(*user)));
        },
        [callback, repo](
            const drogon::orm::DrogonDbException &e)
        {
            LOG_ERROR << "Database error in getCurrentUser: "
                      << e.base().what();
            callback(ResponseBuilder::internalError());
        });
}