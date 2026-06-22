#pragma once
#include <nlohmann/json.hpp>
#include <optional>

#include "models/added.h"
#include "models/bomb.h"
#include "models/grenade.h"
#include "models/map.h"
#include "models/phase_countdowns.h"
#include "models/player.h"
#include "models/previously.h"
#include "models/provider.h"
#include "models/round.h"

namespace cs2gsi {
    struct GameState {
        std::optional<Added> added;
        std::optional<AllPlayersMap> all_players;
        std::optional<Bomb> bomb;
        std::optional<GrenadeMap> grenades;
        std::optional<Map> map;
        std::optional<PhaseCountdowns> phase_countdowns;
        std::optional<Player> player;
        std::optional<Previously> previously;
        std::optional<Provider> provider;
        std::optional<Round> round;

        bool operator==(const GameState &) const = default;
        static GameState from_json(const nlohmann::json &j);
    };
} // namespace cs2gsi
