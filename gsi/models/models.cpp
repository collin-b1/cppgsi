#include "added.h"
#include "bomb.h"
#include "provider.h"
#include "map.h"
#include "round.h"
#include "weapon.h"
#include "player.h"
#include "previously.h"

namespace cs2gsi
{
    // Returns the string value for key, or "" if missing or not a string type.
    // CS2 GSI sends false for optional string fields that have no value.
    static std::string str_val(const nlohmann::json& j, const std::string& key)
    {
        auto it = j.find(key);
        if (it != j.end() && it->is_string()) return it->get<std::string>();
        return "";
    }

    // ---- Provider ---------------------------------------------------------------

    Provider Provider::from_json(const nlohmann::json& j)
    {
        return {
            .name = j.value("name", ""),
            .appid = j.value("appid", 0),
            .version = j.value("version", 0),
            .steamid = j.value("steamid", ""),
            .timestamp = j.value("timestamp", int64_t{0}),
        };
    }

    // ---- Map --------------------------------------------------------------------

    static MapPhase parse_map_phase(const std::string& s)
    {
        if (s == "warmup") return MapPhase::Warmup;
        if (s == "live") return MapPhase::Live;
        if (s == "intermission") return MapPhase::Intermission;
        if (s == "gameover") return MapPhase::GameOver;
        return MapPhase::Unknown;
    }

    TeamState TeamState::from_json(const nlohmann::json& j)
    {
        return {
            .score = j.value("score", 0),
            .consecutive_round_losses = j.value("consecutive_round_losses", 0),
            .timeouts_remaining = j.value("timeouts_remaining", 0),
            .matches_won_this_series = j.value("matches_won_this_series", 0),
        };
    }

    Map Map::from_json(const nlohmann::json& j)
    {
        Map m;
        m.mode = str_val(j, "mode");
        m.name = str_val(j, "name");
        m.phase = parse_map_phase(str_val(j, "phase"));
        m.round = j.value("round", 0);
        if (j.contains("team_ct") && j["team_ct"].is_object()) m.team_ct = TeamState::from_json(j["team_ct"]);
        if (j.contains("team_t")  && j["team_t"].is_object())  m.team_t  = TeamState::from_json(j["team_t"]);
        return m;
    }

    // ---- Round ------------------------------------------------------------------

    static RoundPhase parse_round_phase(const std::string& s)
    {
        if (s == "freezetime") return RoundPhase::FreezeTime;
        if (s == "live") return RoundPhase::Live;
        if (s == "over") return RoundPhase::Over;
        return RoundPhase::Unknown;
    }

    static RoundBombState parse_round_bomb_state(const std::string& s)
    {
        if (s == "planted") return RoundBombState::Planted;
        if (s == "exploded") return RoundBombState::Exploded;
        if (s == "defused") return RoundBombState::Defused;
        return RoundBombState::Unknown;
    }

    Round Round::from_json(const nlohmann::json& j)
    {
        Round r;
        r.phase = parse_round_phase(str_val(j, "phase"));
        const auto bomb_str = str_val(j, "bomb");
        if (!bomb_str.empty()) r.bomb = parse_round_bomb_state(bomb_str);
        const auto win_team = str_val(j, "win_team");
        if (!win_team.empty()) r.win_team = win_team;
        return r;
    }

    // ---- Weapon -----------------------------------------------------------------

    static WeaponState parse_weapon_state(const std::string& s)
    {
        if (s == "active") return WeaponState::Active;
        if (s == "holstered") return WeaponState::Holstered;
        if (s == "reloading") return WeaponState::Reloading;
        return WeaponState::Unknown;
    }

    Weapon Weapon::from_json(const nlohmann::json& j)
    {
        Weapon w;
        w.name = j.value("name", "");
        w.paintkit = j.value("paintkit", "");
        w.type = j.value("type", "");
        w.state = parse_weapon_state(j.value("state", ""));
        if (j.contains("ammo_clip")) w.ammo_clip = j["ammo_clip"].get<int>();
        if (j.contains("ammo_clip_max")) w.ammo_clip_max = j["ammo_clip_max"].get<int>();
        if (j.contains("ammo_reserve")) w.ammo_reserve = j["ammo_reserve"].get<int>();
        return w;
    }

    WeaponSlots weapons_from_json(const nlohmann::json& j)
    {
        WeaponSlots slots;
        for (auto& [slot, data] : j.items())
            slots[slot] = Weapon::from_json(data);
        return slots;
    }

    // ---- Player -----------------------------------------------------------------

    PlayerState PlayerState::from_json(const nlohmann::json& j)
    {
        return {
            .armor = j.value("armor", 0),
            .burning = j.value("burning", 0),
            .equip_value = j.value("equip_value", 0),
            .defusekit = j.value("defusekit", false),
            .flashed = j.value("flashed", 0),
            .health = j.value("health", 0),
            .helmet = j.value("helmet", false),
            .money = j.value("money", 0),
            .round_killhs = j.value("round_killhs", 0),
            .round_kills = j.value("round_kills", 0),
            .round_totaldmg = j.value("round_totaldmg", 0),
            .smoked = j.value("smoked", 0),
        };
    }

    PlayerMatchStats PlayerMatchStats::from_json(const nlohmann::json& j)
    {
        return {
            .kills = j.value("kills", 0),
            .assists = j.value("assists", 0),
            .deaths = j.value("deaths", 0),
            .mvps = j.value("mvps", 0),
            .score = j.value("score", 0),
        };
    }

    Player Player::from_json(const nlohmann::json& j)
    {
        Player p;
        p.steamid       = str_val(j, "steamid");
        p.name          = str_val(j, "name");
        p.clan          = str_val(j, "clan");
        p.observer_slot = j.value("observer_slot", 0);
        p.team          = str_val(j, "team");
        p.activity      = str_val(j, "activity");
        if (j.contains("state")       && j["state"].is_object())       p.state       = PlayerState::from_json(j["state"]);
        if (j.contains("weapons")     && j["weapons"].is_object())     p.weapons     = weapons_from_json(j["weapons"]);
        if (j.contains("match_stats") && j["match_stats"].is_object()) p.match_stats = PlayerMatchStats::from_json(j["match_stats"]);
        return p;
    }

    // ---- Bomb -------------------------------------------------------------------

    static BombState parse_bomb_state(const std::string& s)
    {
        if (s == "carried") return BombState::Carried;
        if (s == "dropped") return BombState::Dropped;
        if (s == "planting") return BombState::Planting;
        if (s == "planted") return BombState::Planted;
        if (s == "defusing") return BombState::Defusing;
        if (s == "defused") return BombState::Defused;
        return BombState::Unknown;
    }

    Bomb Bomb::from_json(const nlohmann::json& j)
    {
        Bomb b;
        b.state    = parse_bomb_state(str_val(j, "state"));
        b.position = str_val(j, "position");
        b.player   = str_val(j, "player");
        if (j.contains("countdown")) {
            const auto& v = j["countdown"];
            b.countdown = v.is_string() ? std::stod(v.get<std::string>()) : v.get<double>();
        }
        return b;
    }

    // ---- Previously -------------------------------------------------------------

    Previously Previously::from_json(const nlohmann::json& j)
    {
        Previously p;
        if (j.contains("provider") && j["provider"].is_object()) p.provider = Provider::from_json(j["provider"]);
        if (j.contains("map")      && j["map"].is_object())      p.map      = Map::from_json(j["map"]);
        if (j.contains("round")    && j["round"].is_object())    p.round    = Round::from_json(j["round"]);
        if (j.contains("player")   && j["player"].is_object())   p.player   = Player::from_json(j["player"]);
        return p;
    }

    // ---- Added ------------------------------------------------------------------

    Added Added::from_json(const nlohmann::json& j)
    {
        Added a;
        if (j.contains("provider") && j["provider"].is_object()) a.provider = Provider::from_json(j["provider"]);
        if (j.contains("map")      && j["map"].is_object())      a.map      = Map::from_json(j["map"]);
        if (j.contains("round")    && j["round"].is_object())    a.round    = Round::from_json(j["round"]);
        if (j.contains("player")   && j["player"].is_object())   a.player   = Player::from_json(j["player"]);
        return a;
    }
} // namespace cs2gsi
