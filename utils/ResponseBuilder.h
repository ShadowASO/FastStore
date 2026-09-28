// utils/ResponseBuilder.h
#pragma once

#include <drogon/HttpResponse.h>
#include <jsoncpp/json/json.h>

#include <string>
#include <vector>

namespace faststore::utils
{
    struct ValidationIssue
    {
        std::string field;
        std::string issue;
    };

    class ResponseBuilder
    {
    public:
        static drogon::HttpResponsePtr success(
            const Json::Value &data,
            drogon::HttpStatusCode code = drogon::k200OK,
            const Json::Value &meta = Json::Value{})
        {
            Json::Value body;
            body["success"] = true;
            body["data"] = data;

            if (!meta.isNull())
            {
                body["meta"] = meta;
            }

            auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
            resp->setStatusCode(code);
            return resp;
        }

        // 201
        static drogon::HttpResponsePtr create(
            const Json::Value &data,
            const std::string &locationHeader)
        {
            auto resp = success(data, drogon::k201Created);
            resp->addHeader("Location", locationHeader);
            return resp;
        }

        // 204
        static drogon::HttpResponsePtr noContent()
        {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::k204NoContent);
            return resp;
        }

        // Generic error envelope
        static drogon::HttpResponsePtr error(
            const std::string &code,
            const std::string &message,
            drogon::HttpStatusCode statusCode,
            const std::vector<ValidationIssue> &details = {})
        {
            Json::Value body;
            body["success"] = false;

            Json::Value errObj;
            errObj["code"] = code;
            errObj["message"] = message;

            if (!details.empty())
            {

                Json::Value detailArr(Json::arrayValue);

                for (const auto &d : details)
                {
                    Json::Value item;
                    item["field"] = d.field;
                    item["issue"] = d.issue;
                    detailArr.append(item);
                }

                errObj["details"] = detailArr;
            }

            body["error"] = errObj;

            auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
            resp->setStatusCode(statusCode);
            return resp;
        };

        // Common named errors
        static drogon::HttpResponsePtr notFound(
            const std::string &resource,
            const std::string &id)
        {
            return error(
                "NOT FOUND",
                resource + " with id " + id + " was not found.",
                drogon::k404NotFound);
        };
        static drogon::HttpResponsePtr validationFailed(
            const std::vector<ValidationIssue> &issues)
        {
            return error(
                "VALIDATION_ERROR",
                "One or more fields failed validation.",
                drogon::k400BadRequest,
                issues);
        };
        static drogon::HttpResponsePtr unauthorized(
            const std::string &message = "Autentication required.")
        {
            return error(
                "UNAUTHORIZED",
                message,
                drogon::k401Unauthorized);
        };
        static drogon::HttpResponsePtr forbidden(
            const std::string &message = "You do not have permission to perform this action.")
        {
            return error(
                "FORBIDDEN",
                message,
                drogon::k403Forbidden);
        };
        static drogon::HttpResponsePtr internalError(
            const std::string &message = "An unexpected error occurred.")
        {
            return error(
                "INTERNAL_ERROR",
                message,
                drogon::k500InternalServerError);
        };
        static drogon::HttpResponsePtr conflict(
            const std::string &message = "conflito com valor pre-existente.")
        {
            return error(
                "CONFLICT",
                message,
                drogon::k409Conflict);
        };
    };
}