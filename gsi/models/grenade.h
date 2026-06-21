#pragma once
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "vec3.h"

namespace cs2gsi
{
    enum class GrenadeType { Frag, Inferno, Smoke, Flashbang, Decoy, Unknown };

    struct Grenade
    {
        GrenadeType type{GrenadeType::Unknown};
        std::string owner;
        double lifetime{};
        std::optional<Vec3> position;
        std::optional<Vec3> velocity;
        std::optional<std::map<std::string, Vec3>> flames;

        bool operator==(const Grenade&) const = default;
        static Grenade from_json(const nlohmann::json& j);
    };

    using GrenadeMap = std::unordered_map<std::string, Grenade>;

    GrenadeMap grenades_from_json(const nlohmann::json& j);
} // namespace cs2gsi
