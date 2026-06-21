#pragma once
#include <optional>
#include <string>
#include <nlohmann/json.hpp>

namespace cs2gsi
{
    enum class RoundPhase { FreezeTime, Live, Over, Unknown };

    enum class RoundBombState { Planted, Exploded, Defused, Unknown };

    struct Round
    {
        RoundPhase phase{RoundPhase::Unknown};
        std::optional<RoundBombState> bomb;
        std::optional<std::string> win_team;

        static Round from_json(const nlohmann::json& j);
    };
} // namespace cs2gsi
