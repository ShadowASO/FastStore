// validation/Validator.h
#pragma once

#include <utils/ResponseBuilder.h>

#include <cmath>
#include <cstddef>
#include <regex>
#include <string>
#include <vector>

namespace faststore::validation
{
    class Validator
    {

    public:
        Validator() = default;

        Validator &required(const Json::Value &body, const std::string &field)
        {
            if (!body.isObject() ||
                !body.isMember(field) ||
                body[field].isNull() ||
                (body[field].isString() && body[field].asString().empty()))
            {
                issues_.push_back({field, "is required"});
            }

            return *this;
        }

        Validator &isEmail(const Json::Value &body, const std::string &field)
        {
            if (!hasValue(body, field))
            {
                return *this;
            }

            static const std::regex emailPattern(
                R"([^\s@]+@[^\s@]+\.[^\s@]+)");

            if (!body[field].isString() ||
                !std::regex_match(body[field].asString(), emailPattern))
            {
                issues_.push_back({field, "must be a valid email address"});
            }

            return *this;
        }

        Validator &minLength(
            const Json::Value &body,
            const std::string &field,
            std::size_t min)
        {
            if (!hasValue(body, field))
            {
                return *this;
            }

            if (!body[field].isString() ||
                body[field].asString().size() < min)
            {
                issues_.push_back({field,
                                   "must be a string with at least " +
                                       std::to_string(min) + " characters"});
            }

            return *this;
        }

        Validator &isPositiveNumber(
            const Json::Value &body,
            const std::string &field)
        {
            if (!hasValue(body, field))
            {
                return *this;
            }

            if (!body[field].isNumeric() ||
                !std::isfinite(body[field].asDouble()) ||
                body[field].asDouble() <= 0)
            {
                issues_.push_back({field, "must be a positive number"});
            }

            return *this;
        }

        [[nodiscard]] bool hasErrors() const
        {
            return !issues_.empty();
        }
        [[nodiscard]] const std::vector<faststore::utils::ValidationIssue> &issues() const
        {
            return issues_;
        }

    private:
        std::vector<faststore::utils::ValidationIssue> issues_;

        static bool hasValue(
            const Json::Value &body,
            const std::string &field)
        {
            return body.isObject() &&
                   body.isMember(field) &&
                   !body[field].isNull();
        }
    };
};