#pragma once
#include <optional>
#include <nlohmann/json.hpp>

#include "models/added.h"
#include "models/bomb.h"
#include "models/provider.h"
#include "models/map.h"
#include "models/round.h"
#include "models/player.h"
#include "models/previously.h"

namespace cs2gsi
{
    struct GameState
    {
        std::optional<Added> added;
        std::optional<Bomb> bomb;
        std::optional<Map> map;
        std::optional<Player> player;
        std::optional<Previously> previously;
        std::optional<Provider> provider;
        std::optional<Round> round;

        static GameState from_json(const nlohmann::json& j);
    };
} // namespace cs2gsi
