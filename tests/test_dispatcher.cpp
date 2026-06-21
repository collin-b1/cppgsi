#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nlohmann/json.hpp>

#include "gsi/gsi_dispatcher.h"

using json = nlohmann::json;
namespace g = cs2gsi;

// ---- Helpers ----------------------------------------------------------------

static json bomb_payload(const std::string& state, const std::string& pos = "1.0, 2.0, 3.0",
                         const std::string& player = "521")
{
    return json{{"bomb", {{"state", state}, {"position", pos}, {"player", player}}}};
}

static json map_payload(const std::string& phase, int round = 1)
{
    return json{{"map", {{"mode", "casual"}, {"name", "de_overpass"}, {"phase", phase}, {"round", round}}}};
}

static json phase_payload(const std::string& phase, const std::string& ends_in = "30.0")
{
    return json{{"phase_countdowns", {{"phase", phase}, {"phase_ends_in", ends_in}}}};
}

static json round_payload(const std::string& phase)
{
    return json{{"round", {{"phase", phase}}}};
}

static json player_payload(int health, const std::string& team = "CT")
{
    return json{
        {
            "player", {
                {"steamid", "500"}, {"name", "Test"}, {"team", team},
                {
                    "state", {
                        {"health", health}, {"armor", 100}, {"helmet", true}, {"defusekit", false},
                        {"money", 800}, {"equip_value", 0}, {"flashed", 0}, {"burning", 0},
                        {"smoked", 0}, {"round_kills", 0}, {"round_killhs", 0}, {"round_totaldmg", 0}
                    }
                }
            }
        }
    };
}

// ---- Bomb change detection --------------------------------------------------

TEST_CASE(
    "on_bomb fires only when bomb state changes"
)
{
    g::GSIDispatcher d;
    int calls = 0;
    d.on_bomb([&](const g::Bomb&, const g::Bomb&) { ++calls; });

    d.dispatch(bomb_payload("planted"));
    REQUIRE(calls == 1);

    d.dispatch(bomb_payload("planted")); // same — no fire
    REQUIRE(calls == 1);

    d.dispatch(bomb_payload("defused")); // changed
    REQUIRE(calls == 2);
}

TEST_CASE(
    "on_bomb passes correct old and new values"
)
{
    g::GSIDispatcher d;
    g::Bomb captured_old, captured_new;
    d.on_bomb([&](const g::Bomb& o, const g::Bomb& n)
    {
        captured_old = o;
        captured_new = n;
    });

    d.dispatch(bomb_payload("planted", "1.0, 2.0, 3.0", "521"));
    REQUIRE(captured_old.state == g::BombState::Unknown); // default-constructed
    REQUIRE(captured_new.state == g::BombState::Planted);
    REQUIRE(captured_new.player == "521");

    d.dispatch(bomb_payload("defused", "1.0, 2.0, 3.0", "521"));
    REQUIRE(captured_old.state == g::BombState::Planted);
    REQUIRE(captured_new.state == g::BombState::Defused);
}

TEST_CASE(
    "on_bomb fires when only position changes"
)
{
    g::GSIDispatcher d;
    int calls = 0;
    d.on_bomb([&](const g::Bomb&, const g::Bomb&) { ++calls; });

    d.dispatch(bomb_payload("dropped", "1.0, 2.0, 3.0"));
    d.dispatch(bomb_payload("dropped", "4.0, 5.0, 6.0")); // same state, different position
    REQUIRE(calls == 2);
}

// ---- Map change detection ---------------------------------------------------

TEST_CASE(
    "on_map fires only when map state changes"
)
{
    g::GSIDispatcher d;
    int calls = 0;
    d.on_map([&](const g::Map&, const g::Map&) { ++calls; });

    d.dispatch(map_payload("live", 1));
    REQUIRE(calls == 1);

    d.dispatch(map_payload("live", 1)); // identical
    REQUIRE(calls == 1);

    d.dispatch(map_payload("live", 2)); // round number changed
    REQUIRE(calls == 2);

    d.dispatch(map_payload("intermission", 2));
    REQUIRE(calls == 3);
}

TEST_CASE(
    "on_map passes correct old and new values"
)
{
    g::GSIDispatcher d;
    g::Map old_map, new_map;
    d.on_map([&](const g::Map& o, const g::Map& n)
    {
        old_map = o;
        new_map = n;
    });

    d.dispatch(map_payload("live", 3));
    REQUIRE(old_map.phase == g::MapPhase::Unknown);
    REQUIRE(new_map.phase == g::MapPhase::Live);
    REQUIRE(new_map.round == 3);
}

// ---- Round change detection -------------------------------------------------

TEST_CASE(
    "on_round fires only on round state change"
)
{
    g::GSIDispatcher d;
    int calls = 0;
    d.on_round([&](const g::Round&, const g::Round&) { ++calls; });

    d.dispatch(round_payload("freezetime"));
    REQUIRE(calls == 1);
    d.dispatch(round_payload("freezetime"));
    REQUIRE(calls == 1);
    d.dispatch(round_payload("live"));
    REQUIRE(calls == 2);
}

// ---- Player change detection ------------------------------------------------

TEST_CASE(
    "on_player fires only when player state changes"
)
{
    g::GSIDispatcher d;
    int calls = 0;
    d.on_player([&](const g::Player&, const g::Player&) { ++calls; });

    d.dispatch(player_payload(100));
    REQUIRE(calls == 1);
    d.dispatch(player_payload(100)); // same
    REQUIRE(calls == 1);
    d.dispatch(player_payload(75)); // took damage
    REQUIRE(calls == 2);
}

TEST_CASE(
    "on_player passes correct old and new health"
)
{
    g::GSIDispatcher d;
    int old_hp = -1, new_hp = -1;
    d.on_player([&](const g::Player& o, const g::Player& n)
    {
        old_hp = o.state.health;
        new_hp = n.state.health;
    });

    d.dispatch(player_payload(100));
    REQUIRE(old_hp == 0); // default-constructed
    REQUIRE(new_hp == 100);

    d.dispatch(player_payload(50));
    REQUIRE(old_hp == 100);
    REQUIRE(new_hp == 50);
}

// ---- PhaseCountdowns change detection ---------------------------------------

TEST_CASE(
    "on_phase_countdowns fires on phase change"
)
{
    g::GSIDispatcher d;
    int calls = 0;
    d.on_phase_countdowns([&](const g::PhaseCountdowns&, const g::PhaseCountdowns&) { ++calls; });

    d.dispatch(phase_payload("freezetime", "15.0"));
    REQUIRE(calls == 1);
    d.dispatch(phase_payload("freezetime", "15.0")); // same
    REQUIRE(calls == 1);
    d.dispatch(phase_payload("live", "115.0"));
    REQUIRE(calls == 2);
}

TEST_CASE(
    "on_phase_countdowns fires when only countdown changes"
)
{
    g::GSIDispatcher d;
    int calls = 0;
    d.on_phase_countdowns([&](const g::PhaseCountdowns&, const g::PhaseCountdowns&) { ++calls; });

    d.dispatch(phase_payload("bomb", "35.0"));
    d.dispatch(phase_payload("bomb", "34.3")); // same phase, different time
    REQUIRE(calls == 2);
}

// ---- Grenades change detection ----------------------------------------------

TEST_CASE(
    "on_grenades fires when grenade set changes"
)
{
    g::GSIDispatcher d;
    int calls = 0;
    d.on_grenades([&](const g::GrenadeMap&, const g::GrenadeMap&) { ++calls; });

    auto j1 = json{
        {
            "grenades",
            {
                {
                    "731",
                    {
                        {"type", "frag"}, {"owner", "509"}, {"lifetime", "0.5"}, {"position", "1.0, 2.0, 3.0"},
                        {"velocity", "0.0, 0.0, 0.0"}
                    }
                }
            }
        }
    };
    d.dispatch(j1);
    REQUIRE(calls == 1);

    d.dispatch(j1); // identical
    REQUIRE(calls == 1);

    auto j2 = json{
        {
            "grenades",
            {
                {
                    "731",
                    {
                        {"type", "frag"}, {"owner", "509"}, {"lifetime", "1.2"}, {"position", "4.0, 5.0, 6.0"},
                        {"velocity", "0.0, 0.0, 0.0"}
                    }
                }
            }
        }
    };
    d.dispatch(j2); // grenade moved
    REQUIRE(calls == 2);
}

// ---- AllPlayers change detection --------------------------------------------

TEST_CASE(
    "on_all_players fires when any player changes"
)
{
    g::GSIDispatcher d;
    int calls = 0;
    d.on_all_players([&](const g::AllPlayersMap&, const g::AllPlayersMap&) { ++calls; });

    auto make = [](int health)
    {
        return json{
            {
                "allplayers", {
                    {
                        "500", {
                            {"name", "Alpha"}, {"team", "CT"}, {"observer_slot", 1},
                            {
                                "state", {
                                    {"health", health}, {"armor", 100}, {"helmet", false},
                                    {"money", 800}, {"equip_value", 0}, {"flashed", 0},
                                    {"burning", 0}, {"smoked", 0}, {"round_kills", 0},
                                    {"round_killhs", 0}, {"round_totaldmg", 0}
                                }
                            }
                        }
                    }
                }
            }
        };
    };

    d.dispatch(make(100));
    REQUIRE(calls == 1);
    d.dispatch(make(100));
    REQUIRE(calls == 1);
    d.dispatch(make(75));
    REQUIRE(calls == 2);
}

// ---- Multiple subscribers ---------------------------------------------------

TEST_CASE(
    "Multiple on_bomb subscribers all receive the event"
)
{
    g::GSIDispatcher d;
    int a = 0, b = 0;
    d.on_bomb([&](const g::Bomb&, const g::Bomb&) { ++a; });
    d.on_bomb([&](const g::Bomb&, const g::Bomb&) { ++b; });

    d.dispatch(bomb_payload("planted"));
    REQUIRE(a == 1);
    REQUIRE(b == 1);
}

// ---- Unsubscribe ------------------------------------------------------------

TEST_CASE(
    "Unsubscribed handler no longer receives events"
)
{
    g::GSIDispatcher d;
    int calls = 0;
    auto id = d.on_bomb([&](const g::Bomb&, const g::Bomb&) { ++calls; });

    d.dispatch(bomb_payload("planted"));
    REQUIRE(calls == 1);

    d.unsubscribe(id);
    d.dispatch(bomb_payload("defused"));
    REQUIRE(calls == 1); // not incremented
}

TEST_CASE(
    "Unsubscribing an unknown id is safe"
)
{
    g::GSIDispatcher d;
    REQUIRE_NOTHROW(d.unsubscribe(999));
}

// ---- Payload without the section does not update prev state -----------------

TEST_CASE(
    "Missing section in payload does not reset previous state"
)
{
    g::GSIDispatcher d;
    int calls = 0;
    d.on_bomb([&](const g::Bomb&, const g::Bomb&) { ++calls; });

    d.dispatch(bomb_payload("planted"));
    REQUIRE(calls == 1);

    // Payload with no bomb section — prev_bomb_ should stay as "planted"
    d.dispatch(json{{"map", {{"mode", "casual"}, {"name", "de_overpass"}, {"phase", "live"}, {"round", 1}}}});

    // Same bomb state again — should NOT fire since prev is still "planted"
    d.dispatch(bomb_payload("planted"));
    REQUIRE(calls == 1);
}

// ---- Field-level proxy: bomb() ----------------------------------------------

TEST_CASE("bomb().on(&Bomb::state) fires only when the state field changes") {
    g::GSIDispatcher d;
    int calls = 0;
    g::BombState old_s{g::BombState::Unknown}, new_s{g::BombState::Unknown};

    d.bomb().on(&g::Bomb::state, [&](g::BombState o, g::BombState n) {
        ++calls; old_s = o; new_s = n;
    });

    d.dispatch(bomb_payload("planted", "1.0, 2.0, 3.0"));
    REQUIRE(calls == 1);
    REQUIRE(old_s == g::BombState::Unknown);
    REQUIRE(new_s == g::BombState::Planted);

    // Position changed but state didn't — proxy must NOT fire
    d.dispatch(bomb_payload("planted", "4.0, 5.0, 6.0"));
    REQUIRE(calls == 1);

    d.dispatch(bomb_payload("defused", "4.0, 5.0, 6.0"));
    REQUIRE(calls == 2);
    REQUIRE(old_s == g::BombState::Planted);
    REQUIRE(new_s == g::BombState::Defused);
}

TEST_CASE("bomb().on(&Bomb::player) fires only when carrier changes") {
    g::GSIDispatcher d;
    int calls = 0;
    d.bomb().on(&g::Bomb::player, [&](const std::string&, const std::string&) { ++calls; });

    d.dispatch(bomb_payload("planted", "1.0, 2.0, 3.0", "521"));
    REQUIRE(calls == 1);

    // Same player, bomb position moved
    d.dispatch(bomb_payload("planted", "4.0, 5.0, 6.0", "521"));
    REQUIRE(calls == 1);

    d.dispatch(bomb_payload("planted", "4.0, 5.0, 6.0", "500"));
    REQUIRE(calls == 2);
}

TEST_CASE("proxy unsubscribe stops field-level handler") {
    g::GSIDispatcher d;
    int calls = 0;
    auto proxy = d.bomb();
    auto id = proxy.on(&g::Bomb::state, [&](g::BombState, g::BombState) { ++calls; });

    d.dispatch(bomb_payload("planted"));
    REQUIRE(calls == 1);

    proxy.unsubscribe(id);
    d.dispatch(bomb_payload("defused"));
    REQUIRE(calls == 1);
}

// ---- Field-level proxy: map() -----------------------------------------------

TEST_CASE("map().on(&Map::phase) fires only when phase field changes") {
    g::GSIDispatcher d;
    int calls = 0;
    d.map().on(&g::Map::phase, [&](g::MapPhase, g::MapPhase) { ++calls; });

    d.dispatch(map_payload("live", 1));
    REQUIRE(calls == 1);

    // Round incremented but phase unchanged
    d.dispatch(map_payload("live", 2));
    REQUIRE(calls == 1);

    d.dispatch(map_payload("intermission", 2));
    REQUIRE(calls == 2);
}

TEST_CASE("map().on(&Map::round) fires only when round number changes") {
    g::GSIDispatcher d;
    int calls = 0;
    d.map().on(&g::Map::round, [&](int, int) { ++calls; });

    d.dispatch(map_payload("live", 1));
    REQUIRE(calls == 1);
    d.dispatch(map_payload("live", 1));
    REQUIRE(calls == 1);
    d.dispatch(map_payload("live", 2));
    REQUIRE(calls == 2);
}

// ---- Field-level proxy: phase_countdowns() ----------------------------------

TEST_CASE("phase_countdowns().on(&PhaseCountdowns::phase_ends_in) fires on countdown tick") {
    g::GSIDispatcher d;
    int calls = 0;
    d.phase_countdowns().on(&g::PhaseCountdowns::phase_ends_in, [&](double, double) { ++calls; });

    d.dispatch(phase_payload("bomb", "35.0"));
    REQUIRE(calls == 1);
    d.dispatch(phase_payload("bomb", "35.0"));
    REQUIRE(calls == 1);
    d.dispatch(phase_payload("bomb", "34.3"));
    REQUIRE(calls == 2);
}

TEST_CASE("phase_countdowns().on(&PhaseCountdowns::phase) ignores timer ticks") {
    g::GSIDispatcher d;
    int calls = 0;
    d.phase_countdowns().on(&g::PhaseCountdowns::phase, [&](g::PhaseCountdownPhase, g::PhaseCountdownPhase) { ++calls; });

    d.dispatch(phase_payload("bomb", "35.0"));
    REQUIRE(calls == 1);
    // Timer ticked but phase is still bomb
    d.dispatch(phase_payload("bomb", "34.3"));
    REQUIRE(calls == 1);
    d.dispatch(phase_payload("over", "0.0"));
    REQUIRE(calls == 2);
}

// ---- Field-level proxy: round() ---------------------------------------------

TEST_CASE("round().on(&Round::phase) fires only on phase transition") {
    g::GSIDispatcher d;
    int calls = 0;
    d.round().on(&g::Round::phase, [&](g::RoundPhase, g::RoundPhase) { ++calls; });

    d.dispatch(round_payload("freezetime"));
    REQUIRE(calls == 1);
    d.dispatch(round_payload("freezetime"));
    REQUIRE(calls == 1);
    d.dispatch(round_payload("live"));
    REQUIRE(calls == 2);
}

// ---- Field-level proxy: player() --------------------------------------------

TEST_CASE("player().on(&Player::state) fires only when player state changes") {
    g::GSIDispatcher d;
    int calls = 0;
    d.player().on(&g::Player::state, [&](const g::PlayerState&, const g::PlayerState&) { ++calls; });

    d.dispatch(player_payload(100));
    REQUIRE(calls == 1);
    d.dispatch(player_payload(100));
    REQUIRE(calls == 1);
    d.dispatch(player_payload(75));
    REQUIRE(calls == 2);
}

// ---- on_game_state ----------------------------------------------------------

TEST_CASE(
    "on_game_state fires when anything in the payload changes"
)
{
    g::GSIDispatcher d;
    int calls = 0;
    d.on_game_state([&](const g::GameState&, const g::GameState&) { ++calls; });

    d.dispatch(bomb_payload("planted"));
    REQUIRE(calls == 1);

    d.dispatch(bomb_payload("planted")); // identical payload
    REQUIRE(calls == 1);

    d.dispatch(bomb_payload("defused"));
    REQUIRE(calls == 2);
}
