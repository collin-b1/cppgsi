#include "gsi/gsi_dispatcher.h"
#include "gsi/gsi_server.h"
#include "gsi/gsi_config.h"

// Verifies that the installed headers and library are self-consistent:
// the key public types can be constructed and the API compiles cleanly.
int main() {
    cs2gsi::GSIDispatcher dispatcher;

    dispatcher.on_player([](const cs2gsi::Player&, const cs2gsi::Player&) {});
    dispatcher.on_bomb([](const cs2gsi::Bomb&, const cs2gsi::Bomb&) {});
    dispatcher.on_map([](const cs2gsi::Map&, const cs2gsi::Map&) {});
    dispatcher.on_round([](const cs2gsi::Round&, const cs2gsi::Round&) {});
    dispatcher.on_grenades([](const cs2gsi::GrenadeMap&, const cs2gsi::GrenadeMap&) {});
    dispatcher.on_phase_countdowns([](const cs2gsi::PhaseCountdowns&, const cs2gsi::PhaseCountdowns&) {});
    dispatcher.on_grenade_thrown([](const std::string&, const cs2gsi::Grenade&) {});
    dispatcher.on_grenade_detonated([](const std::string&, const cs2gsi::Grenade&) {});

    dispatcher.player().on(&cs2gsi::Player::team, [](cs2gsi::PlayerTeam, cs2gsi::PlayerTeam) {});
    dispatcher.bomb().on(&cs2gsi::Bomb::state, [](cs2gsi::BombState, cs2gsi::BombState) {});
    dispatcher.map().on(&cs2gsi::Map::mode, [](cs2gsi::GameMode, cs2gsi::GameMode) {});
    dispatcher.round().on(&cs2gsi::Round::phase, [](cs2gsi::RoundPhase, cs2gsi::RoundPhase) {});
    dispatcher.phase_countdowns().on(&cs2gsi::PhaseCountdowns::phase_ends_in, [](double, double) {});
}
