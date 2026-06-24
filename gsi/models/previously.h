#pragma once
#include <nlohmann/json.hpp>
#include <optional>
#include "bomb.h"
#include "grenade.h"
#include "map.h"
#include "phase_countdowns.h"
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
        std::optional<Bomb> bomb;
        std::optional<AllPlayersMap> all_players;
        std::optional<GrenadeMap> grenades;
        std::optional<PhaseCountdowns> phase_countdowns;

        bool operator==(const Previously &) const = default;
        static Previously from_json(const nlohmann::json &j);
    };
} // namespace cs2gsi
