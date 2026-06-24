// round_events.cpp
// Demonstrates round and map state tracking:
//   - GameMode / MapPhase enums
//   - RoundWinReason enum (per-round outcome history)
//   - BombState on Round (unified enum, was RoundBombState)
//   - TeamState::name / TeamState::flag (official matches)
//   - previously.bomb for tracking the transition that just occurred

#include <iostream>
#include "gsi/gsi_config.h"
#include "gsi/gsi_dispatcher.h"
#include "gsi/gsi_server.h"

using namespace cs2gsi;

static const char* round_win_str(RoundWinReason r) {
    switch (r) {
        case RoundWinReason::CTWinElimination: return "CT win — elimination";
        case RoundWinReason::CTWinTime:        return "CT win — time expired";
        case RoundWinReason::CTWinDefuse:      return "CT win — defuse";
        case RoundWinReason::TWinElimination:  return "T win — elimination";
        case RoundWinReason::TWinBomb:         return "T win — bomb";
        default:                               return "unknown";
    }
}

static const char* bomb_state_str(BombState s) {
    switch (s) {
        case BombState::Carried:  return "carried";
        case BombState::Dropped:  return "dropped";
        case BombState::Planting: return "planting";
        case BombState::Planted:  return "planted";
        case BombState::Defusing: return "defusing";
        case BombState::Defused:  return "defused";
        case BombState::Exploded: return "exploded";
        default:                  return "unknown";
    }
}

static const char* map_phase_str(MapPhase p) {
    switch (p) {
        case MapPhase::Warmup:       return "warmup";
        case MapPhase::Live:         return "live";
        case MapPhase::Intermission: return "intermission";
        case MapPhase::GameOver:     return "game over";
        default:                     return "unknown";
    }
}

int main() {
    const auto config_path = generate_gsi_config("cppgsi_round_events");
    std::cout << "GSI config written to: " << config_path << "\n\n";

    GSIDispatcher dispatcher;

    // Map phase transitions (warmup → live → intermission → game over).
    dispatcher.map().on(&Map::phase, [](MapPhase old_p, MapPhase new_p) {
        std::cout << "[map phase] " << map_phase_str(old_p)
                  << " -> " << map_phase_str(new_p) << "\n";
    });

    // Team names and flags — only populated in official/FACEIT matches.
    // In casual/competitive they will be empty strings.
    dispatcher.on_map([](const Map& old_m, const Map& new_m) {
        if (new_m.team_ct.name != old_m.team_ct.name && !new_m.team_ct.name.empty()) {
            std::cout << "[teams] CT: " << new_m.team_ct.name
                      << " [" << new_m.team_ct.flag << "]"
                      << "  T: " << new_m.team_t.name
                      << " [" << new_m.team_t.flag << "]\n";
        }
    });

    // Round outcome: fires at the end of each round when a new entry is appended.
    dispatcher.map().on(&Map::round_wins,
        [](const std::map<int, RoundWinReason>& old_wins,
           const std::map<int, RoundWinReason>& new_wins) {
            if (new_wins.size() > old_wins.size()) {
                auto it = new_wins.end();
                --it;
                std::cout << "[round " << it->first << "] "
                          << round_win_str(it->second) << "\n";
            }
        });

    // Bomb state on the Round struct uses the same BombState enum as the Bomb
    // struct — it reflects the round-level outcome (planted/defused/exploded).
    dispatcher.round().on(&Round::bomb,
        [](std::optional<BombState> old_b, std::optional<BombState> new_b) {
            if (new_b)
                std::cout << "[round.bomb] "
                          << (old_b ? bomb_state_str(*old_b) : "none")
                          << " -> " << bomb_state_str(*new_b) << "\n";
        });

    // Bomb object: real-time state during the round.
    dispatcher.bomb().on(&Bomb::state, [](BombState old_s, BombState new_s) {
        std::cout << "[bomb] " << bomb_state_str(old_s)
                  << " -> " << bomb_state_str(new_s) << "\n";
    });

    // previously.bomb: CS2's own delta field, carries the state just before
    // the current update. Lets you verify the transition without keeping your
    // own previous-state copy.
    dispatcher.on_game_state([](const GameState&, const GameState& new_gs) {
        if (new_gs.previously && new_gs.previously->bomb) {
            const BombState was = new_gs.previously->bomb->state;
            const BombState now = new_gs.bomb ? new_gs.bomb->state : BombState::Unknown;
            std::cout << "[previously.bomb] was=" << bomb_state_str(was)
                      << " now=" << bomb_state_str(now) << "\n";
        }
    });

    // Phase countdown: useful for displaying time-to-live on HUD overlays.
    dispatcher.phase_countdowns().on(&PhaseCountdowns::phase_ends_in,
        [](double, double new_t) {
            std::cout << "[phase] " << new_t << "s remaining\n";
        });

    GSIServer server(dispatcher);
    server.start();
}
