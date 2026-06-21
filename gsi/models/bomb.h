#pragma once
#include <optional>
#include <string>
#include <nlohmann/json.hpp>

namespace cs2gsi
{
    enum class BombState { Carried, Dropped, Planting, Planted, Defusing, Defused, Unknown };

    struct Bomb
    {
        BombState state{};
        std::string position;
        std::string player;
        std::optional<double> countdown;

        static Bomb from_json(const nlohmann::json& j);
    };
} // namespace cs2gsi
