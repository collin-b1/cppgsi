#pragma once
#include <optional>
#include <nlohmann/json.hpp>
#include "provider.h"
#include "map.h"
#include "round.h"
#include "player.h"

namespace cs2gsi
{
    // Mirrors GameState but all top-level fields are optional — CS2 only sends
    // the sub-fields that actually changed since the previous update.
    struct Added
    {
        std::optional<Provider> provider;
        std::optional<Map> map;
        std::optional<Round> round;
        std::optional<Player> player;

        static Added from_json(const nlohmann::json& j);
    };
} // namespace cs2gsi
