#pragma once
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include "vec3.h"
#include "weapon.h"

namespace cs2gsi {
    enum class PlayerTeam { CT, T, Spectator, Unknown };
    enum class PlayerActivity { Playing, Spectating, Menu, Unknown };

    struct PlayerState {
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

        bool operator==(const PlayerState &) const = default;
        static PlayerState from_json(const nlohmann::json &j);
    };

    struct PlayerMatchStats {
        int kills{};
        int assists{};
        int deaths{};
        int mvps{};
        int score{};

        bool operator==(const PlayerMatchStats &) const = default;
        static PlayerMatchStats from_json(const nlohmann::json &j);
    };

    struct Player {
        std::string steamid;
        std::string name;
        std::string clan;
        int observer_slot{};
        PlayerTeam team{PlayerTeam::Unknown};
        PlayerActivity activity{PlayerActivity::Unknown};
        Vec3 position;
        Vec3 forward;
        std::optional<std::string> spectarget;
        std::optional<std::string> crosshaircode;
        PlayerState state;
        WeaponSlots weapons;
        PlayerMatchStats match_stats;

        bool operator==(const Player &) const = default;
        static Player from_json(const nlohmann::json &j);
    };

    using AllPlayersMap = std::unordered_map<std::string, Player>;

    AllPlayersMap all_players_from_json(const nlohmann::json &j);
} // namespace cs2gsi
