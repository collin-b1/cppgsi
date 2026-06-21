#pragma once
#include <cstdint>
#include <string>
#include <nlohmann/json.hpp>

namespace cs2gsi
{
    struct Provider
    {
        std::string name;
        int appid{};
        int version{};
        std::string steamid;
        std::int64_t timestamp{};

        bool operator==(const Provider&) const = default;
        static Provider from_json(const nlohmann::json& j);
    };
} // namespace cs2gsi
