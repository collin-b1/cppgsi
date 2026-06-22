#pragma once
#include <map>
#include <nlohmann/json.hpp>
#include <string>

namespace cs2gsi {
    enum class MapPhase { Warmup, Live, Intermission, GameOver, Unknown };

    struct TeamState {
        int score{};
        int consecutive_round_losses{};
        int timeouts_remaining{};
        int matches_won_this_series{};

        bool operator==(const TeamState &) const = default;
        static TeamState from_json(const nlohmann::json &j);
    };

    struct Map {
        std::string mode;
        std::string name;
        MapPhase phase{MapPhase::Unknown};
        int round{};
        int num_matches_to_win_series{};
        TeamState team_ct;
        TeamState team_t;
        std::map<int, std::string> round_wins;

        bool operator==(const Map &) const = default;
        static Map from_json(const nlohmann::json &j);
    };
} // namespace cs2gsi
