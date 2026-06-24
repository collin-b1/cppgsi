// quickstart.cpp
// Demonstrates core library concepts: config generation, field-level
// subscriptions, enum types for team/activity/weapon.
//
// Run this, launch CS2, and you should immediately see output as you move
// between the menu, a match, and switch weapons.

#include <iostream>
#include "gsi/gsi_config.h"
#include "gsi/gsi_dispatcher.h"
#include "gsi/gsi_server.h"

using namespace cs2gsi;

static const char* team_str(PlayerTeam t) {
    switch (t) {
        case PlayerTeam::CT:        return "CT";
        case PlayerTeam::T:         return "T";
        case PlayerTeam::Spectator: return "Spectator";
        default:                    return "Unknown";
    }
}

static const char* activity_str(PlayerActivity a) {
    switch (a) {
        case PlayerActivity::Playing:    return "Playing";
        case PlayerActivity::Spectating: return "Spectating";
        case PlayerActivity::Menu:       return "Menu";
        default:                         return "Unknown";
    }
}

static const char* weapon_type_str(WeaponType t) {
    switch (t) {
        case WeaponType::Pistol:      return "Pistol";
        case WeaponType::Rifle:       return "Rifle";
        case WeaponType::SniperRifle: return "SniperRifle";
        case WeaponType::Submachine:  return "Submachine";
        case WeaponType::Shotgun:     return "Shotgun";
        case WeaponType::Machine:     return "Machine";
        case WeaponType::Knife:       return "Knife";
        case WeaponType::Grenade:     return "Grenade";
        case WeaponType::C4:          return "C4";
        default:                      return "Unknown";
    }
}

int main() {
    // Write the GSI config to CS2's cfg directory so the game sends payloads.
    const auto config_path = generate_gsi_config("cppgsi_quickstart");
    std::cout << "GSI config written to: " << config_path << "\n\n";

    GSIDispatcher dispatcher;

    // Activity: fires when entering/leaving the main menu or spectator mode.
    dispatcher.player().on(&Player::activity, [](PlayerActivity old_a, PlayerActivity new_a) {
        std::cout << "[activity] " << activity_str(old_a)
                  << " -> " << activity_str(new_a) << "\n";
    });

    // Team: fires when the local player joins CT, T, or spectator.
    dispatcher.player().on(&Player::team, [](PlayerTeam old_t, PlayerTeam new_t) {
        std::cout << "[team] " << team_str(old_t)
                  << " -> " << team_str(new_t) << "\n";
    });

    // Health: fires on every damage tick or heal.
    dispatcher.player().on(&Player::state, [](const PlayerState& old_s, const PlayerState& new_s) {
        if (new_s.health != old_s.health)
            std::cout << "[health] " << old_s.health << " -> " << new_s.health << "\n";
    });

    // Crosshair code: arrives once on connect.
    dispatcher.player().on(&Player::crosshaircode,
        [](const std::optional<std::string>&, const std::optional<std::string>& new_c) {
            if (new_c)
                std::cout << "[crosshair] " << *new_c << "\n";
        });

    // Weapon switch: WeaponType enum lets you match on category, not raw string.
    // Detects any slot transitioning to Active (gun, knife, grenade, C4).
    dispatcher.on_player([](const Player& old_p, const Player& new_p) {
        for (auto& [slot, w] : new_p.weapons) {
            if (w.state != WeaponState::Active) continue;
            auto it = old_p.weapons.find(slot);
            bool was_active = (it != old_p.weapons.end() && it->second.state == WeaponState::Active);
            if (!was_active)
                std::cout << "[weapon] " << w.name
                          << " (" << weapon_type_str(w.type) << ")\n";
        }
    });

    GSIServer server(dispatcher);
    server.start();
}
