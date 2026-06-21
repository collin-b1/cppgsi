#pragma once
#include <optional>
#include <string>
#include <nlohmann/json.hpp>
#include "vec3.h"

namespace cs2gsi
{
    enum class BombState { Carried, Dropped, Planting, Planted, Defusing, Defused, Exploded, Unknown };

    struct Bomb
    {
        BombState state{BombState::Unknown};
        Vec3 position;
        std::string player;
        std::optional<double> countdown;

        bool operator==(const Bomb&) const = default;
        static Bomb from_json(const nlohmann::json& j);
    };
} // namespace cs2gsi
