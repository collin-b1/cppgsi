#include <cstdio>
#include "added.h"
#include "bomb.h"
#include "grenade.h"
#include "phase_countdowns.h"
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

    static Vec3 parse_vec3(const std::string& s)
    {
        Vec3 v{};
        std::sscanf(s.c_str(), "%lf, %lf, %lf", &v.x, &v.y, &v.z);
        return v;
    }

    // ---- Provider ---------------------------------------------------------------

    Provider Provider::from_json(const nlohmann::json& j)
    {
        return {
            .name = str_val(j, "name"),
            .appid = j.value("appid", 0),
            .version = j.value("version", 0),
            .steamid = str_val(j, "steamid"),
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

    static GameMode parse_game_mode(const std::string& s)
    {
        if (s == "competitive") return GameMode::Competitive;
        if (s == "casual") return GameMode::Casual;
        if (s == "deathmatch") return GameMode::Deathmatch;
        if (s == "wingman") return GameMode::Wingman;
        if (s == "armsrace") return GameMode::ArmsRace;
        if (s == "demolition") return GameMode::Demolition;
        if (s == "custom") return GameMode::Custom;
        return GameMode::Unknown;
    }

    static RoundWinReason parse_round_win_reason(const std::string& s)
    {
        if (s == "ct_win_elimination") return RoundWinReason::CTWinElimination;
        if (s == "ct_win_time") return RoundWinReason::CTWinTime;
        if (s == "ct_win_defuse") return RoundWinReason::CTWinDefuse;
        if (s == "t_win_elimination") return RoundWinReason::TWinElimination;
        if (s == "t_win_bomb") return RoundWinReason::TWinBomb;
        return RoundWinReason::Unknown;
    }

    TeamState TeamState::from_json(const nlohmann::json& j)
    {
        return {
            .score = j.value("score", 0),
            .consecutive_round_losses = j.value("consecutive_round_losses", 0),
            .timeouts_remaining = j.value("timeouts_remaining", 0),
            .matches_won_this_series = j.value("matches_won_this_series", 0),
            .name = str_val(j, "name"),
            .flag = str_val(j, "flag"),
        };
    }

    Map Map::from_json(const nlohmann::json& j)
    {
        Map m;
        m.mode = parse_game_mode(str_val(j, "mode"));
        m.name = str_val(j, "name");
        m.phase = parse_map_phase(str_val(j, "phase"));
        m.round = j.value("round", 0);
        m.num_matches_to_win_series = j.value("num_matches_to_win_series", 0);
        if (j.contains("team_ct") && j["team_ct"].is_object()) m.team_ct = TeamState::from_json(j["team_ct"]);
        if (j.contains("team_t") && j["team_t"].is_object()) m.team_t = TeamState::from_json(j["team_t"]);
        if (j.contains("round_wins") && j["round_wins"].is_object()) {
            for (auto& [k, v] : j["round_wins"].items()) {
                try {
                    if (v.is_string())
                        m.round_wins[std::stoi(k)] = parse_round_win_reason(v.get<std::string>());
                } catch (...) {}
            }
        }
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

    Round Round::from_json(const nlohmann::json& j)
    {
        Round r;
        r.phase = parse_round_phase(str_val(j, "phase"));
        const auto bomb_str = str_val(j, "bomb");
        if (!bomb_str.empty()) r.bomb = parse_bomb_state(bomb_str);
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

    static WeaponType parse_weapon_type(const std::string& s)
    {
        if (s == "Pistol") return WeaponType::Pistol;
        if (s == "Rifle") return WeaponType::Rifle;
        if (s == "SniperRifle") return WeaponType::SniperRifle;
        if (s == "Submachine") return WeaponType::Submachine;
        if (s == "Shotgun") return WeaponType::Shotgun;
        if (s == "Machine") return WeaponType::Machine;
        if (s == "Knife") return WeaponType::Knife;
        if (s == "Grenade") return WeaponType::Grenade;
        if (s == "C4") return WeaponType::C4;
        return WeaponType::Unknown;
    }

    Weapon Weapon::from_json(const nlohmann::json& j)
    {
        Weapon w;
        w.name = str_val(j, "name");
        w.paintkit = str_val(j, "paintkit");
        w.type = parse_weapon_type(str_val(j, "type"));
        w.state = parse_weapon_state(str_val(j, "state"));
        if (auto it = j.find("ammo_clip");     it != j.end() && it->is_number()) w.ammo_clip     = it->get<int>();
        if (auto it = j.find("ammo_clip_max"); it != j.end() && it->is_number()) w.ammo_clip_max = it->get<int>();
        if (auto it = j.find("ammo_reserve");  it != j.end() && it->is_number()) w.ammo_reserve  = it->get<int>();
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

    static PlayerTeam parse_player_team(const std::string& s)
    {
        if (s == "CT") return PlayerTeam::CT;
        if (s == "T") return PlayerTeam::T;
        if (s == "spectator" || s == "Spectator") return PlayerTeam::Spectator;
        return PlayerTeam::Unknown;
    }

    static PlayerActivity parse_player_activity(const std::string& s)
    {
        if (s == "playing") return PlayerActivity::Playing;
        if (s == "spectating") return PlayerActivity::Spectating;
        if (s == "menu") return PlayerActivity::Menu;
        return PlayerActivity::Unknown;
    }

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
        p.team          = parse_player_team(str_val(j, "team"));
        p.activity      = parse_player_activity(str_val(j, "activity"));
        const auto pos_str = str_val(j, "position");
        if (!pos_str.empty()) p.position = parse_vec3(pos_str);
        const auto fwd_str = str_val(j, "forward");
        if (!fwd_str.empty()) p.forward = parse_vec3(fwd_str);
        const auto spectarget = str_val(j, "spectarget");
        if (!spectarget.empty()) p.spectarget = spectarget;
        const auto crosshaircode = str_val(j, "crosshaircode");
        if (!crosshaircode.empty()) p.crosshaircode = crosshaircode;
        if (j.contains("state")       && j["state"].is_object())       p.state       = PlayerState::from_json(j["state"]);
        if (j.contains("weapons")     && j["weapons"].is_object())     p.weapons     = weapons_from_json(j["weapons"]);
        if (j.contains("match_stats") && j["match_stats"].is_object()) p.match_stats = PlayerMatchStats::from_json(j["match_stats"]);
        return p;
    }

    AllPlayersMap all_players_from_json(const nlohmann::json& j)
    {
        AllPlayersMap players;
        for (auto& [steamid, data] : j.items()) {
            if (data.is_object()) {
                auto p = Player::from_json(data);
                p.steamid = steamid;
                players[steamid] = std::move(p);
            }
        }
        return players;
    }

    // ---- Bomb -------------------------------------------------------------------

    BombState parse_bomb_state(const std::string& s)
    {
        if (s == "carried") return BombState::Carried;
        if (s == "dropped") return BombState::Dropped;
        if (s == "planting") return BombState::Planting;
        if (s == "planted") return BombState::Planted;
        if (s == "defusing") return BombState::Defusing;
        if (s == "defused") return BombState::Defused;
        if (s == "exploded") return BombState::Exploded;
        return BombState::Unknown;
    }

    Bomb Bomb::from_json(const nlohmann::json& j)
    {
        Bomb b;
        b.state = parse_bomb_state(str_val(j, "state"));
        const auto pos_str = str_val(j, "position");
        if (!pos_str.empty()) b.position = parse_vec3(pos_str);
        b.player = str_val(j, "player");
        if (j.contains("countdown")) {
            const auto& v = j["countdown"];
            b.countdown = v.is_string() ? std::stod(v.get<std::string>()) : v.get<double>();
        }
        return b;
    }

    // ---- Grenade ----------------------------------------------------------------

    static GrenadeType parse_grenade_type(const std::string& s)
    {
        if (s == "frag") return GrenadeType::Frag;
        if (s == "inferno") return GrenadeType::Inferno;
        if (s == "smoke") return GrenadeType::Smoke;
        if (s == "flashbang") return GrenadeType::Flashbang;
        if (s == "decoy") return GrenadeType::Decoy;
        return GrenadeType::Unknown;
    }

    Grenade Grenade::from_json(const nlohmann::json& j)
    {
        Grenade g;
        g.type = parse_grenade_type(str_val(j, "type"));
        g.owner = str_val(j, "owner");
        const auto lifetime_str = str_val(j, "lifetime");
        if (!lifetime_str.empty()) {
            try { g.lifetime = std::stod(lifetime_str); } catch (...) {}
        }
        const auto pos_str = str_val(j, "position");
        if (!pos_str.empty()) g.position = parse_vec3(pos_str);
        const auto vel_str = str_val(j, "velocity");
        if (!vel_str.empty()) g.velocity = parse_vec3(vel_str);
        if (j.contains("flames") && j["flames"].is_object()) {
            std::map<std::string, Vec3> flames;
            for (auto& [name, pos] : j["flames"].items()) {
                if (pos.is_string()) flames[name] = parse_vec3(pos.get<std::string>());
            }
            g.flames = std::move(flames);
        }
        return g;
    }

    GrenadeMap grenades_from_json(const nlohmann::json& j)
    {
        GrenadeMap grenades;
        for (auto& [id, data] : j.items()) {
            if (data.is_object()) grenades[id] = Grenade::from_json(data);
        }
        return grenades;
    }

    // ---- PhaseCountdowns --------------------------------------------------------

    static PhaseCountdownPhase parse_phase_countdown_phase(const std::string& s)
    {
        if (s == "warmup") return PhaseCountdownPhase::Warmup;
        if (s == "freezetime") return PhaseCountdownPhase::FreezeTime;
        if (s == "live") return PhaseCountdownPhase::Live;
        if (s == "bomb") return PhaseCountdownPhase::Bomb;
        if (s == "over") return PhaseCountdownPhase::Over;
        return PhaseCountdownPhase::Unknown;
    }

    PhaseCountdowns PhaseCountdowns::from_json(const nlohmann::json& j)
    {
        PhaseCountdowns pc;
        pc.phase = parse_phase_countdown_phase(str_val(j, "phase"));
        const auto ends_in = str_val(j, "phase_ends_in");
        if (!ends_in.empty()) {
            try { pc.phase_ends_in = std::stod(ends_in); } catch (...) {}
        }
        return pc;
    }

    // ---- Previously -------------------------------------------------------------

    Previously Previously::from_json(const nlohmann::json& j)
    {
        Previously p;
        if (j.contains("provider")         && j["provider"].is_object())          p.provider         = Provider::from_json(j["provider"]);
        if (j.contains("map")              && j["map"].is_object())               p.map              = Map::from_json(j["map"]);
        if (j.contains("round")            && j["round"].is_object())             p.round            = Round::from_json(j["round"]);
        if (j.contains("player")           && j["player"].is_object())            p.player           = Player::from_json(j["player"]);
        if (j.contains("bomb")             && j["bomb"].is_object())              p.bomb             = Bomb::from_json(j["bomb"]);
        if (j.contains("allplayers")       && j["allplayers"].is_object())        p.all_players      = all_players_from_json(j["allplayers"]);
        if (j.contains("grenades")         && j["grenades"].is_object())          p.grenades         = grenades_from_json(j["grenades"]);
        if (j.contains("phase_countdowns") && j["phase_countdowns"].is_object())  p.phase_countdowns = PhaseCountdowns::from_json(j["phase_countdowns"]);
        return p;
    }

    // ---- Added ------------------------------------------------------------------

    Added Added::from_json(const nlohmann::json& j)
    {
        Added a;
        if (j.contains("provider")         && j["provider"].is_object())          a.provider         = Provider::from_json(j["provider"]);
        if (j.contains("map")              && j["map"].is_object())               a.map              = Map::from_json(j["map"]);
        if (j.contains("round")            && j["round"].is_object())             a.round            = Round::from_json(j["round"]);
        if (j.contains("player")           && j["player"].is_object())            a.player           = Player::from_json(j["player"]);
        if (j.contains("bomb")             && j["bomb"].is_object())              a.bomb             = Bomb::from_json(j["bomb"]);
        if (j.contains("allplayers")       && j["allplayers"].is_object())        a.all_players      = all_players_from_json(j["allplayers"]);
        if (j.contains("grenades")         && j["grenades"].is_object())          a.grenades         = grenades_from_json(j["grenades"]);
        if (j.contains("phase_countdowns") && j["phase_countdowns"].is_object())  a.phase_countdowns = PhaseCountdowns::from_json(j["phase_countdowns"]);
        return a;
    }
} // namespace cs2gsi
