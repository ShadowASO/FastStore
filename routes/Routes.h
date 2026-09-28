// routes/Routes.h

#pragma once

#include <drogon/HttpController.h>
#include <drogon/drogon.h>

#include "controllers/HealthController.h"
#include "controllers/EchoController.h"

#include <functional>

namespace faststore::routes
{
    void registerAll();
}