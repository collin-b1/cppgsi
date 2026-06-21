#pragma once
#include <string>
#include <nlohmann/json.hpp>

namespace cs2gsi
{
    enum class MapPhase { Warmup, Live, Intermission, GameOver, Unknown };

    struct TeamState
    {
        int score{};
        int consecutive_round_losses{};
        int timeouts_remaining{};
        int matches_won_this_series{};

        static TeamState from_json(const nlohmann::json& j);
    };

    struct Map
    {
        std::string mode;
        std::string name;
        MapPhase phase{MapPhase::Unknown};
        int round{};
        TeamState team_ct;
        TeamState team_t;

        static Map from_json(const nlohmann::json& j);
    };
} // namespace cs2gsi
