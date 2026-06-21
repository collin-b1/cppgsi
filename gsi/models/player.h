#pragma once
#include <optional>
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "vec3.h"
#include "weapon.h"

namespace cs2gsi
{
    struct PlayerState
    {
        int armor{};
        int burning{};
        int equip_value{};
        bool defusekit{};
        int flashed{};
        int health{};
        bool helmet{};
        int money{};
        int round_killhs{};
        int round_kills{};
        int round_totaldmg{};
        int smoked{};

        bool operator==(const PlayerState&) const = default;
        static PlayerState from_json(const nlohmann::json& j);
    };

    struct PlayerMatchStats
    {
        int kills{};
        int assists{};
        int deaths{};
        int mvps{};
        int score{};

        bool operator==(const PlayerMatchStats&) const = default;
        static PlayerMatchStats from_json(const nlohmann::json& j);
    };

    struct Player
    {
        std::string steamid;
        std::string name;
        std::string clan;
        int observer_slot{};
        std::string team;
        std::string activity;
        Vec3 position;
        Vec3 forward;
        std::optional<std::string> spectarget;
        PlayerState state;
        WeaponSlots weapons;
        PlayerMatchStats match_stats;

        bool operator==(const Player&) const = default;
        static Player from_json(const nlohmann::json& j);
    };

    using AllPlayersMap = std::unordered_map<std::string, Player>;

    AllPlayersMap all_players_from_json(const nlohmann::json& j);
} // namespace cs2gsi
