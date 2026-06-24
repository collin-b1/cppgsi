#pragma once
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include "bomb.h"

namespace cs2gsi {
    enum class RoundPhase { FreezeTime, Live, Over, Unknown };

    struct Round {
        RoundPhase phase{RoundPhase::Unknown};
        std::optional<BombState> bomb;
        std::optional<std::string> win_team;

        bool operator==(const Round &) const = default;
        static Round from_json(const nlohmann::json &j);
    };
} // namespace cs2gsi
