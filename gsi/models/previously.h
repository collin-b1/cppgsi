#pragma once
#include <nlohmann/json.hpp>
#include <optional>
#include "map.h"
#include "player.h"
#include "provider.h"
#include "round.h"

namespace cs2gsi {
    // Mirrors GameState but all top-level fields are optional — CS2 only sends
    // the sub-fields that actually changed since the previous update.
    struct Previously {
        std::optional<Provider> provider;
        std::optional<Map> map;
        std::optional<Round> round;
        std::optional<Player> player;

        bool operator==(const Previously &) const = default;
        static Previously from_json(const nlohmann::json &j);
    };
} // namespace cs2gsi
