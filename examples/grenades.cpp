// grenades.cpp
// Demonstrates grenade lifecycle tracking:
//   - on_grenade_thrown      fires when any new grenade appears on the map
//   - on_grenade_detonated   two forms: any grenade, or a specific grenade by ID
//   - grenade(id).on(...)    field-level subscription for a single grenade's data

#include <iostream>
#include "gsi/gsi_config.h"
#include "gsi/gsi_dispatcher.h"
#include "gsi/gsi_server.h"

using namespace cs2gsi;

static const char *grenade_type_str(GrenadeType t) {
    switch (t) {
        case GrenadeType::Frag:
            return "HE";
        case GrenadeType::Inferno:
            return "Molotov/Incendiary";
        case GrenadeType::Smoke:
            return "Smoke";
        case GrenadeType::Flashbang:
            return "Flash";
        case GrenadeType::Decoy:
            return "Decoy";
        default:
            return "Unknown";
    }
}

int main() {
    const auto config_path = generate_gsi_config("cppgsi_grenades");
    std::cout << "GSI config written to: " << config_path << "\n\n";

    GSIDispatcher dispatcher;

    // Fires for every grenade that disappears from the map (detonation, expiry).
    // This is the any-grenade form — useful for aggregate stats or UI clearing.
    dispatcher.on_grenade_detonated([](const std::string &id, const Grenade &g) {
        std::cout << "[detonated] " << grenade_type_str(g.type) << " id=" << id << " owner=" << g.owner
                  << " lifetime=" << g.lifetime << "s\n";
    });

    // Fires when any new grenade appears. Use the id here to set up per-grenade
    // subscriptions that are automatically cleaned up after detonation.
    dispatcher.on_grenade_thrown([&dispatcher](const std::string &id, const Grenade &g) {
        std::cout << "[thrown] " << grenade_type_str(g.type) << " id=" << id << " owner=" << g.owner << "\n";

        // Track position updates for this specific grenade while it's in flight.
        dispatcher.grenade(id).on(&Grenade::position,
                                  [id](const std::optional<Vec3> &, const std::optional<Vec3> &new_pos) {
                                      if (new_pos)
                                          std::cout << "[grenade " << id << "] pos: " << *new_pos << "\n";
                                  });

        // For infernos, track each individual flame point as it spreads.
        if (g.type == GrenadeType::Inferno) {
            dispatcher.grenade(id).on(&Grenade::flames, [id](const auto &old_f, const auto &new_f) {
                int old_count = old_f ? static_cast<int>(old_f->size()) : 0;
                int new_count = new_f ? static_cast<int>(new_f->size()) : 0;
                if (new_count != old_count)
                    std::cout << "[inferno " << id << "] flame points: " << old_count << " -> " << new_count << "\n";
            });
        }

        // Per-grenade detonation: receives only this grenade's final state.
        // Cleaned up automatically after it fires.
        dispatcher.on_grenade_detonated(id, [id, type = g.type](const Grenade &nade) {
            std::cout << "[detonated:" << id << "] " << grenade_type_str(type) << " lasted " << nade.lifetime << "s\n";
        });
    });

    GSIServer server(dispatcher);
    server.start();
}
