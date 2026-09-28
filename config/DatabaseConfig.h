#pragma once

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

namespace faststore::config
{
    struct DatabaseConfig
    {
        std::string host;
        std::string port;
        std::string dbName;
        std::string user;
        std::string password;
        std::size_t connectionPoolSize = 10;
        std::string jwt_secret;

        static std::string getEnvOr(
            const std::unordered_map<std::string, std::string> &values,
            const std::string &key,
            std::string fallback)
        {
            const auto it = values.find(key);
            return it == values.end() || it->second.empty()
                       ? std::move(fallback)
                       : it->second;
        }

        static DatabaseConfig fromDotEnv(
            const std::filesystem::path &path = ".env")
        {
            std::ifstream file(path);
            if (!file)
                throw std::runtime_error(
                    "Não foi possível abrir o arquivo: " + path.string());

            std::unordered_map<std::string, std::string> values;
            std::string line;
            std::size_t lineNumber = 0;

            auto trim = [](std::string text)
            {
                const auto first = text.find_first_not_of(" \t\r\n");
                if (first == std::string::npos)
                    return std::string{};

                const auto last = text.find_last_not_of(" \t\r\n");
                return text.substr(first, last - first + 1);
            };

            while (std::getline(file, line))
            {
                ++lineNumber;
                line = trim(line);

                if (line.empty() || line.front() == '#')
                    continue;

                const auto separator = line.find('=');
                if (separator == std::string::npos)
                    throw std::runtime_error(
                        "Linha inválida no .env: " +
                        std::to_string(lineNumber));

                auto key = trim(line.substr(0, separator));
                auto value = trim(line.substr(separator + 1));

                if (key.empty())
                    throw std::runtime_error(
                        "Nome vazio no .env, linha " +
                        std::to_string(lineNumber));

                if (!value.empty() &&
                    (value.front() == '\'' || value.front() == '"'))
                {
                    if (value.size() < 2 || value.back() != value.front())
                        throw std::runtime_error(
                            "Aspas não fechadas no .env, linha " +
                            std::to_string(lineNumber));

                    value = value.substr(1, value.size() - 2);
                }

                values[std::move(key)] = std::move(value);
            }

            auto required = [&values](const std::string &key)
                -> std::string
            {
                const auto it = values.find(key);
                if (it == values.end() || it->second.empty())
                    throw std::runtime_error(
                        "Configuração ausente no .env: " + key);

                return it->second;
            };

            return {
                .host = getEnvOr(values, "DB_HOST", "localhost"),
                .port = getEnvOr(values, "DB_PORT", "5432"),
                .dbName = getEnvOr(values, "DB_NAME", "faststore"),
                .user = getEnvOr(values, "DB_USER", "faststore_dev"),
                .password = required("DB_PASSWORD"),
                .connectionPoolSize = 10};
        }
    };
}