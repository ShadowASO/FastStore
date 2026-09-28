// config/AppConfig.h
#pragma once

#include "config/Config.h"

using namespace faststore::config;

namespace faststore::config
{
    // Deve ser chamada uma vez, antes de drogon::app().run().
    void initialize(const std::filesystem::path &path = ".env");

    // Lança uma exceção se initialize() ainda não foi chamada.
    const Config &appConfig();
}