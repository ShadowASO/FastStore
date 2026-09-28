// config/Config.h
#pragma once

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace faststore::config
{
    class Config
    {
    public:
        explicit Config(const std::filesystem::path &path = ".env")
        {
            std::ifstream file(path);

            if (!file)
            {
                throw std::runtime_error(
                    "Não foi possível abrir o arquivo: " + path.string());
            }

            std::string line;
            std::size_t lineNumber = 0;

            while (std::getline(file, line))
            {
                ++lineNumber;
                line = trim(std::move(line));

                if (line.empty() || line.front() == '#')
                    continue;

                const auto separator = line.find('=');

                if (separator == std::string::npos)
                {
                    throw std::runtime_error(
                        "Linha inválida no .env: " +
                        std::to_string(lineNumber));
                }

                std::string key = trim(line.substr(0, separator));
                std::string value = trim(line.substr(separator + 1));

                if (key.empty())
                {
                    throw std::runtime_error(
                        "Nome vazio no .env, linha " +
                        std::to_string(lineNumber));
                }

                if (!value.empty() &&
                    (value.front() == '\'' || value.front() == '"'))
                {
                    if (value.size() < 2 ||
                        value.back() != value.front())
                    {
                        throw std::runtime_error(
                            "Aspas não fechadas no .env, linha " +
                            std::to_string(lineNumber));
                    }

                    value = value.substr(1, value.size() - 2);
                }

                values_[std::move(key)] = std::move(value);
            }

            if (file.bad())
            {
                throw std::runtime_error(
                    "Erro ao ler o arquivo: " + path.string());
            }
        }

        // Exige que a chave exista e tenha valor.
        [[nodiscard]] std::string get(std::string_view key) const
        {
            const auto it = values_.find(std::string{key});

            if (it == values_.end() || it->second.empty())
            {
                throw std::runtime_error(
                    "Configuração ausente ou vazia no .env: " +
                    std::string{key});
            }

            return it->second;
        }

        // Usa fallback quando a chave não existe ou está vazia.
        [[nodiscard]] std::string getOr(
            std::string_view key,
            std::string fallback) const
        {
            const auto it = values_.find(std::string{key});

            if (it == values_.end() || it->second.empty())
                return fallback;

            return it->second;
        }

    private:
        static std::string trim(std::string text)
        {
            const auto first = text.find_first_not_of(" \t\r\n");

            if (first == std::string::npos)
                return {};

            const auto last = text.find_last_not_of(" \t\r\n");
            return text.substr(first, last - first + 1);
        }

        std::unordered_map<std::string, std::string> values_;
    };
}