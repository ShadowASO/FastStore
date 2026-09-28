// config/AppConfig.cc
#include "config/AppConfig.h"

#include <memory>
#include <stdexcept>

namespace faststore::config
{
    namespace
    {
        std::unique_ptr<Config> instance;
    }

    void initialize(const std::filesystem::path &path)
    {
        if (instance)
            throw std::logic_error("Configuração já inicializada");

        instance = std::make_unique<Config>(path);
    }

    const Config &appConfig()
    {
        if (!instance)
            throw std::logic_error("Configuração não inicializada");

        return *instance;
    }
}