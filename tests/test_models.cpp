#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nlohmann/json.hpp>

#include "gsi/models/vec3.h"
#include "gsi/models/bomb.h"
#include "gsi/models/map.h"
#include "gsi/models/round.h"
#include "gsi/models/weapon.h"
#include "gsi/models/player.h"
#include "gsi/models/grenade.h"
#include "gsi/models/phase_countdowns.h"
#include "gsi/game_state.h"

using Catch::Approx;
using json = nlohmann::json;
namespace g = cs2gsi;

// ---- Vec3 -------------------------------------------------------------------

TEST_CASE("Vec3 equality")
{
    g::Vec3 a{1.0, 2.0, 3.0};
    g::Vec3 b{1.0, 2.0, 3.0};
    g::Vec3 c{1.0, 2.0, 4.0};
    REQUIRE(a == b);
    REQUIRE(a != c);
}

TEST_CASE("Vec3 stream output")
{
    std::ostringstream ss;
    ss << g::Vec3{1.5, -2.0, 3.25};
    REQUIRE(ss.str() == "1.5, -2, 3.25");
}

// ---- Bomb -------------------------------------------------------------------

TEST_CASE("Bomb::from_json parses all BombState values")
{
    auto parse_state = [](const std::string& s)
    {
        return g::Bomb::from_json(json{{"state", s}}).state;
    };
    REQUIRE(parse_state("carried") == g::BombState::Carried);
    REQUIRE(parse_state("dropped") == g::BombState::Dropped);
    REQUIRE(parse_state("planting") == g::BombState::Planting);
    REQUIRE(parse_state("planted") == g::BombState::Planted);
    REQUIRE(parse_state("defusing") == g::BombState::Defusing);
    REQUIRE(parse_state("defused") == g::BombState::Defused);
    REQUIRE(parse_state("exploded") == g::BombState::Exploded);
    REQUIRE(parse_state("unknown") == g::BombState::Unknown);
}

TEST_CASE("Bomb::from_json parses position and countdown")
{
    auto j = json{
        {"state", "planted"},
        {"position", "-1004.75, -2820.54, 223.94"},
        {"player", "521"},
        {"countdown", "35.5"}
    };
    auto bomb = g::Bomb::from_json(j);
    REQUIRE(bomb.state == g::BombState::Planted);
    REQUIRE(bomb.position.x == Approx(-1004.75));
    REQUIRE(bomb.position.y == Approx(-2820.54));
    REQUIRE(bomb.position.z == Approx(223.94));
    REQUIRE(bomb.player == "521");
    REQUIRE(bomb.countdown.has_value());
    REQUIRE(*bomb.countdown == Approx(35.5));
}

TEST_CASE("Bomb::from_json accepts countdown as number")
{
    auto j = json{{"state", "planted"}, {"countdown", 12.3}};
    auto bomb = g::Bomb::from_json(j);
    REQUIRE(bomb.countdown.has_value());
    REQUIRE(*bomb.countdown == Approx(12.3));
}

TEST_CASE("Bomb::from_json handles false for missing string fields (CS2 GSI quirk)")
{
    auto j = json{{"state", "carried"}, {"position", false}, {"player", false}};
    auto bomb = g::Bomb::from_json(j);
    REQUIRE(bomb.state == g::BombState::Carried);
    REQUIRE(bomb.player == "");
    REQUIRE(bomb.countdown == std::nullopt);
}

TEST_CASE("Bomb equality")
{
    auto j = json{{"state", "planted"}, {"position", "1.0, 2.0, 3.0"}, {"player", "42"}};
    REQUIRE(g::Bomb::from_json(j) == g::Bomb::from_json(j));

    auto j2 = json{{"state", "defused"}, {"position", "1.0, 2.0, 3.0"}, {"player", "42"}};
    REQUIRE(g::Bomb::from_json(j) != g::Bomb::from_json(j2));
}

// ---- Map --------------------------------------------------------------------

TEST_CASE("Map::from_json parses phase")
{
    auto parse_phase = [](const std::string& s)
    {
        return g::Map::from_json(json{{"phase", s}}).phase;
    };
    REQUIRE(parse_phase("warmup") == g::MapPhase::Warmup);
    REQUIRE(parse_phase("live") == g::MapPhase::Live);
    REQUIRE(parse_phase("intermission") == g::MapPhase::Intermission);
    REQUIRE(parse_phase("gameover") == g::MapPhase::GameOver);
    REQUIRE(parse_phase("other") == g::MapPhase::Unknown);
}

TEST_CASE("Map::from_json parses team scores and round_wins")
{
    auto j = json{
        {"mode", "casual"},
        {"name", "de_overpass"},
        {"phase", "live"},
        {"round", 9},
        {"num_matches_to_win_series", 0},
        {
            "team_ct",
            {{"score", 5}, {"consecutive_round_losses", 0}, {"timeouts_remaining", 1}, {"matches_won_this_series", 0}}
        },
        {
            "team_t",
            {{"score", 3}, {"consecutive_round_losses", 1}, {"timeouts_remaining", 1}, {"matches_won_this_series", 0}}
        },
        {"round_wins", {{"1", "ct_win_elimination"}, {"2", "t_win_bomb"}, {"3", "ct_win_defuse"}}}
    };
    auto map = g::Map::from_json(j);
    REQUIRE(map.mode == "casual");
    REQUIRE(map.name == "de_overpass");
    REQUIRE(map.phase == g::MapPhase::Live);
    REQUIRE(map.round == 9);
    REQUIRE(map.num_matches_to_win_series == 0);
    REQUIRE(map.team_ct.score == 5);
    REQUIRE(map.team_t.score == 3);
    REQUIRE(map.team_t.consecutive_round_losses == 1);
    REQUIRE(map.round_wins.size() == 3);
    REQUIRE(map.round_wins.at(1) == "ct_win_elimination");
    REQUIRE(map.round_wins.at(2) == "t_win_bomb");
    REQUIRE(map.round_wins.at(3) == "ct_win_defuse");
}

TEST_CASE("Map::from_json with missing optional sections")
{
    auto j = json{{"mode", "casual"}, {"name", "de_dust2"}, {"phase", "live"}, {"round", 1}};
    auto map = g::Map::from_json(j);
    REQUIRE(map.round_wins.empty());
    REQUIRE(map.team_ct.score == 0);
}

// ---- Round ------------------------------------------------------------------

TEST_CASE("Round::from_json parses phase and optional fields")
{
    auto parse_phase = [](const std::string& s)
    {
        return g::Round::from_json(json{{"phase", s}}).phase;
    };
    REQUIRE(parse_phase("freezetime") == g::RoundPhase::FreezeTime);
    REQUIRE(parse_phase("live") == g::RoundPhase::Live);
    REQUIRE(parse_phase("over") == g::RoundPhase::Over);
    REQUIRE(parse_phase("other") == g::RoundPhase::Unknown);
}

TEST_CASE("Round::from_json parses optional bomb and win_team")
{
    SECTION("round over with win info")
    {
        auto j = json{{"phase", "over"}, {"bomb", "exploded"}, {"win_team", "T"}};
        auto round = g::Round::from_json(j);
        REQUIRE(round.phase == g::RoundPhase::Over);
        REQUIRE(round.bomb.has_value());
        REQUIRE(*round.bomb == g::RoundBombState::Exploded);
        REQUIRE(round.win_team.has_value());
        REQUIRE(*round.win_team == "T");
    }
    SECTION("live round without bomb/win_team")
    {
        auto j = json{{"phase", "live"}};
        auto round = g::Round::from_json(j);
        REQUIRE(round.bomb == std::nullopt);
        REQUIRE(round.win_team == std::nullopt);
    }
    SECTION("false for missing string field")
    {
        auto j = json{{"phase", "live"}, {"bomb", false}};
        auto round = g::Round::from_json(j);
        REQUIRE(round.bomb == std::nullopt);
    }
}

// ---- Weapon -----------------------------------------------------------------

TEST_CASE("Weapon::from_json parses rifle with ammo")
{
    auto j = json{
        {"name", "weapon_ak47"},
        {"paintkit", "default"},
        {"type", "Rifle"},
        {"state", "active"},
        {"ammo_clip", 28},
        {"ammo_clip_max", 30},
        {"ammo_reserve", 90}
    };
    auto w = g::Weapon::from_json(j);
    REQUIRE(w.name == "weapon_ak47");
    REQUIRE(w.type == "Rifle");
    REQUIRE(w.state == g::WeaponState::Active);
    REQUIRE(w.ammo_clip.has_value());
    REQUIRE(*w.ammo_clip == 28);
    REQUIRE(*w.ammo_clip_max == 30);
    REQUIRE(*w.ammo_reserve == 90);
}

TEST_CASE("Weapon::from_json handles false ammo fields (melee/grenade weapons)")
{
    auto j = json{
        {"name", "weapon_knife"},
        {"paintkit", "default"},
        {"type", "Knife"},
        {"state", "holstered"},
        {"ammo_clip", false},
        {"ammo_clip_max", false},
        {"ammo_reserve", false}
    };
    auto w = g::Weapon::from_json(j);
    REQUIRE(w.name == "weapon_knife");
    REQUIRE(w.ammo_clip == std::nullopt);
    REQUIRE(w.ammo_clip_max == std::nullopt);
    REQUIRE(w.ammo_reserve == std::nullopt);
}

TEST_CASE("Weapon::from_json handles false string fields")
{
    auto j = json{{"name", false}, {"paintkit", false}, {"type", false}, {"state", false}};
    auto w = g::Weapon::from_json(j);
    REQUIRE(w.name == "");
    REQUIRE(w.paintkit == "");
    REQUIRE(w.type == "");
    REQUIRE(w.state == g::WeaponState::Unknown);
}

TEST_CASE("Weapon::from_json parses all WeaponState values")
{
    auto parse_state = [](const std::string& s)
    {
        return g::Weapon::from_json(json{{"state", s}}).state;
    };
    REQUIRE(parse_state("active") == g::WeaponState::Active);
    REQUIRE(parse_state("holstered") == g::WeaponState::Holstered);
    REQUIRE(parse_state("reloading") == g::WeaponState::Reloading);
    REQUIRE(parse_state("other") == g::WeaponState::Unknown);
}

// ---- Player -----------------------------------------------------------------

TEST_CASE("Player::from_json parses core fields")
{
    auto j = json{
        {"steamid", "76561198000000001"},
        {"name", "TestPlayer"},
        {"clan", "TestClan"},
        {"observer_slot", 3},
        {"team", "CT"},
        {"activity", "playing"},
        {"position", "-730.81, -2162.57, 244.03"},
        {"forward", "0.93, 0.37, -0.02"}
    };
    auto p = g::Player::from_json(j);
    REQUIRE(p.steamid == "76561198000000001");
    REQUIRE(p.name == "TestPlayer");
    REQUIRE(p.clan == "TestClan");
    REQUIRE(p.observer_slot == 3);
    REQUIRE(p.team == "CT");
    REQUIRE(p.position.x == Approx(-730.81));
    REQUIRE(p.position.z == Approx(244.03));
    REQUIRE(p.forward.x == Approx(0.93));
    REQUIRE(p.spectarget == std::nullopt);
}

TEST_CASE("Player::from_json parses spectarget when present")
{
    auto j = json{{"steamid", "500"}, {"spectarget", "521"}};
    auto p = g::Player::from_json(j);
    REQUIRE(p.spectarget.has_value());
    REQUIRE(*p.spectarget == "521");
}

TEST_CASE("Player::from_json parses state and weapons")
{
    auto j = json{
        {
            "state", {
                {"health", 85}, {"armor", 100}, {"helmet", true},
                {"defusekit", false}, {"money", 3200}, {"equip_value", 4700},
                {"flashed", 0}, {"burning", 0}, {"smoked", 0},
                {"round_kills", 1}, {"round_killhs", 0}, {"round_totaldmg", 100}
            }
        },
        {
            "weapons", {
                {
                    "weapon_0",
                    {{"name", "weapon_knife"}, {"paintkit", "default"}, {"type", "Knife"}, {"state", "holstered"}}
                },
                {
                    "weapon_1",
                    {
                        {"name", "weapon_ak47"}, {"paintkit", "default"}, {"type", "Rifle"}, {"state", "active"},
                        {"ammo_clip", 28}, {"ammo_clip_max", 30}, {"ammo_reserve", 90}
                    }
                }
            }
        },
        {"match_stats", {{"kills", 5}, {"assists", 1}, {"deaths", 2}, {"mvps", 1}, {"score", 20}}}
    };
    auto p = g::Player::from_json(j);
    REQUIRE(p.state.health == 85);
    REQUIRE(p.state.helmet == true);
    REQUIRE(p.state.money == 3200);
    REQUIRE(p.match_stats.kills == 5);
    REQUIRE(p.match_stats.deaths == 2);
    REQUIRE(p.weapons.size() == 2);
    REQUIRE(p.weapons.at("weapon_1").name == "weapon_ak47");
}

TEST_CASE("all_players_from_json injects steamid from map key")
{
    auto j = json{
        {"500", {{"name", "Alpha"}, {"team", "CT"}, {"observer_slot", 1}}},
        {"521", {{"name", "Bravo"}, {"team", "T"}, {"observer_slot", 2}}}
    };
    auto players = g::all_players_from_json(j);
    REQUIRE(players.size() == 2);
    REQUIRE(players.at("500").steamid == "500");
    REQUIRE(players.at("500").name == "Alpha");
    REQUIRE(players.at("521").steamid == "521");
    REQUIRE(players.at("521").team == "T");
}

// ---- Grenade ----------------------------------------------------------------

TEST_CASE("Grenade::from_json parses all GrenadeType values")
{
    auto parse_type = [](const std::string& s)
    {
        return g::Grenade::from_json(json{{"type", s}, {"owner", "500"}, {"lifetime", "1.0"}}).type;
    };
    REQUIRE(parse_type("frag") == g::GrenadeType::Frag);
    REQUIRE(parse_type("inferno") == g::GrenadeType::Inferno);
    REQUIRE(parse_type("smoke") == g::GrenadeType::Smoke);
    REQUIRE(parse_type("flashbang") == g::GrenadeType::Flashbang);
    REQUIRE(parse_type("decoy") == g::GrenadeType::Decoy);
    REQUIRE(parse_type("other") == g::GrenadeType::Unknown);
}

TEST_CASE("Grenade::from_json parses frag grenade with position and velocity")
{
    auto j = json{
        {"type", "frag"},
        {"owner", "509"},
        {"lifetime", "2.4"},
        {"position", "-1811.57, -1031.68, 104.72"},
        {"velocity", "168.80, -248.73, 24.17"}
    };
    auto g = cs2gsi::Grenade::from_json(j);
    REQUIRE(g.type == cs2gsi::GrenadeType::Frag);
    REQUIRE(g.owner == "509");
    REQUIRE(g.lifetime == Approx(2.4));
    REQUIRE(g.position.has_value());
    REQUIRE(g.position->x == Approx(-1811.57));
    REQUIRE(g.velocity.has_value());
    REQUIRE(g.velocity->x == Approx(168.80));
    REQUIRE(g.flames == std::nullopt);
}

TEST_CASE("Grenade::from_json parses inferno with flames")
{
    auto j = json{
        {"type", "inferno"},
        {"owner", "521"},
        {"lifetime", "3.6"},
        {
            "flames", {
                {"flame_n493_n2300_p149", "-493.00, -2300.00, 149.00"},
                {"flame_n497_n2373_p150", "-497.00, -2373.00, 150.00"}
            }
        }
    };
    auto g = cs2gsi::Grenade::from_json(j);
    REQUIRE(g.type == cs2gsi::GrenadeType::Inferno);
    REQUIRE(g.position == std::nullopt);
    REQUIRE(g.velocity == std::nullopt);
    REQUIRE(g.flames.has_value());
    REQUIRE(g.flames->size() == 2);
    REQUIRE(g.flames->at("flame_n493_n2300_p149").x == Approx(-493.0));
    REQUIRE(g.flames->at("flame_n497_n2373_p150").y == Approx(-2373.0));
}

TEST_CASE("grenades_from_json builds map keyed by entity id")
{
    auto j = json{
        {"729", {{"type", "inferno"}, {"owner", "521"}, {"lifetime", "1.5"}}},
        {
            "731",
            {
                {"type", "frag"}, {"owner", "509"}, {"lifetime", "0.3"}, {"position", "1.0, 2.0, 3.0"},
                {"velocity", "0.0, 0.0, 0.0"}
            }
        }
    };
    auto grenades = cs2gsi::grenades_from_json(j);
    REQUIRE(grenades.size() == 2);
    REQUIRE(grenades.at("729").type == cs2gsi::GrenadeType::Inferno);
    REQUIRE(grenades.at("731").type == cs2gsi::GrenadeType::Frag);
    REQUIRE(grenades.at("731").position.has_value());
}

// ---- PhaseCountdowns --------------------------------------------------------

TEST_CASE("PhaseCountdowns::from_json parses all phases")
{
    auto parse_phase = [](const std::string& s)
    {
        return g::PhaseCountdowns::from_json(json{{"phase", s}, {"phase_ends_in", "10.0"}}).phase;
    };
    REQUIRE(parse_phase("warmup") == g::PhaseCountdownPhase::Warmup);
    REQUIRE(parse_phase("freezetime") == g::PhaseCountdownPhase::FreezeTime);
    REQUIRE(parse_phase("live") == g::PhaseCountdownPhase::Live);
    REQUIRE(parse_phase("bomb") == g::PhaseCountdownPhase::Bomb);
    REQUIRE(parse_phase("over") == g::PhaseCountdownPhase::Over);
    REQUIRE(parse_phase("other") == g::PhaseCountdownPhase::Unknown);
}

TEST_CASE("PhaseCountdowns::from_json parses phase_ends_in from string")
{
    auto j = json{{"phase", "bomb"}, {"phase_ends_in", "35.4"}};
    auto pc = g::PhaseCountdowns::from_json(j);
    REQUIRE(pc.phase_ends_in == Approx(35.4));
}

// ---- GameState --------------------------------------------------------------

TEST_CASE("GameState::from_json populates present sections and leaves others nullopt")
{
    auto j = json{
        {
            "provider",
            {
                {"name", "CS2"}, {"appid", 730}, {"version", 1}, {"steamid", "76561198000000001"},
                {"timestamp", 1700000000}
            }
        },
        {"map", {{"mode", "casual"}, {"name", "de_overpass"}, {"phase", "live"}, {"round", 5}}},
        {"round", {{"phase", "live"}}},
        {"bomb", {{"state", "planted"}, {"position", "1.0, 2.0, 3.0"}, {"player", "521"}, {"countdown", "25.0"}}}
    };
    auto gs = g::GameState::from_json(j);
    REQUIRE(gs.provider.has_value());
    REQUIRE(gs.provider->appid == 730);
    REQUIRE(gs.map.has_value());
    REQUIRE(gs.map->name == "de_overpass");
    REQUIRE(gs.round.has_value());
    REQUIRE(gs.bomb.has_value());
    REQUIRE(gs.bomb->state == g::BombState::Planted);
    REQUIRE(gs.player == std::nullopt);
    REQUIRE(gs.all_players == std::nullopt);
    REQUIRE(gs.grenades == std::nullopt);
    REQUIRE(gs.phase_countdowns == std::nullopt);
}

TEST_CASE("GameState::from_json parses allplayers and phase_countdowns")
{
    auto j = json{
        {
            "allplayers", {
                {"500", {{"name", "Alpha"}, {"team", "CT"}, {"observer_slot", 1}}}
            }
        },
        {"phase_countdowns", {{"phase", "freezetime"}, {"phase_ends_in", "15.0"}}},
        {
            "grenades", {
                {
                    "731",
                    {
                        {"type", "frag"}, {"owner", "500"}, {"lifetime", "0.5"}, {"position", "1.0, 2.0, 3.0"},
                        {"velocity", "0.0, 0.0, 0.0"}
                    }
                }
            }
        }
    };
    auto gs = g::GameState::from_json(j);
    REQUIRE(gs.all_players.has_value());
    REQUIRE(gs.all_players->size() == 1);
    REQUIRE(gs.all_players->at("500").name == "Alpha");
    REQUIRE(gs.phase_countdowns.has_value());
    REQUIRE(gs.phase_countdowns->phase == g::PhaseCountdownPhase::FreezeTime);
    REQUIRE(gs.grenades.has_value());
    REQUIRE(gs.grenades->at("731").type == g::GrenadeType::Frag);
}
