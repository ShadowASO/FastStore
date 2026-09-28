// middleware/RequireRoleFilter.cc
#include "RequireRoleFilter.h"
#include <algorithm>

bool faststore::middleware::hasRequiredRole(
    const HttpRequestPtr &req,
    const std::vector<std::string> &allowedRoles)
{
    auto role = req->attributes()->get<std::string>("userRole");
    return std::find(allowedRoles.begin(), allowedRoles.end(), role) != allowedRoles.end();
}