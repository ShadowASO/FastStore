//middleware/RequireRoleFilter.h

#pragma once

#include <drogon/HttpFilter.h>
#include <string>
#include <vector>
using namespace drogon;
// Not a Drogon-registered filter itself (Drogon filters need a default
// constructor for its registration machinery)   instead, a small
// reusable helper called explicitly inside a handler, right after
// AuthFilter has already attached role claims. This keeps role checks
// flexible per-route without needing a combinatorial explosion of
// registered filter classes for every role combination.
namespace faststore::middleware
{
    bool hasRequiredRole(const HttpRequestPtr &req,
                         const std::vector<std::string> &allowedRoles);
}