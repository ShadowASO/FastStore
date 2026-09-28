// middleware/AuthFilter.cc


#include "AuthFilter.h"
#include "auth/JwtService.h"
#include "utils/ResponseBuilder.h"
#include "config/AppConfig.h"

using namespace faststore::auth;
using namespace faststore::utils;


void AuthFilter::doFilter(const HttpRequestPtr &req,
                          FilterCallback &&fcb,
                          FilterChainCallback &&fccb)
{
    //std::cout << "Entrei no filter" << std::endl;

    auto authHeader = req->getHeader("Authorization");
    static const std::string prefix = "Bearer ";

    if (authHeader.size() <= prefix.size() || authHeader.compare(0, prefix.size(), prefix) != 0)
    {
        fcb(ResponseBuilder::unauthorized("Missing or malformed Authorization header."));
        return;
    }

    std::string token = authHeader.substr(prefix.size());

    JwtService jwtService(faststore::config::appConfig().get("JWT_SECRET"));

    auto claims = jwtService.verify(token);
    if (!claims.has_value())
    {
        fcb(ResponseBuilder::unauthorized("Invalid or expired token."));
        return;
    }

    if (claims->role.empty())
    {
        // A refresh token was used where an access token was expected
        // refresh tokens carry no role claim by design (section 4.5).
        fcb(ResponseBuilder::unauthorized("This token cannot be used to authenticate requests."));
        return;
    }

    // Attach verified claims to the request's attributes, where downstream
    // handlers can read them without re-parsing the token themselves.
    req->attributes()->insert("userId", claims->userId);
    req->attributes()->insert("userEmail", claims->email);
    req->attributes()->insert("userRole", claims->role);

    fccb(); // continue the filter chain / reach the handler
}
