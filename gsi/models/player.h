#pragma once
#include <string>
#include <nlohmann/json.hpp>
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

        static PlayerState from_json(const nlohmann::json& j);
    };

    struct PlayerMatchStats
    {
        int kills{};
        int assists{};
        int deaths{};
        int mvps{};
        int score{};

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
        PlayerState state;
        WeaponSlots weapons;
        PlayerMatchStats match_stats;

        static Player from_json(const nlohmann::json& j);
    };
} // namespace cs2gsi
