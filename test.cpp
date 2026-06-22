#include <iostream>
#include "gsi/gsi_config.h"
#include "gsi/gsi_dispatcher.h"
#include "gsi/gsi_server.h"

int main() {
    const auto config_path = cs2gsi::generate_gsi_config("testgsi");
    std::cout << "GSI config written to: " << config_path << "\n";

    cs2gsi::GSIDispatcher dispatcher;

    dispatcher.player().on(&cs2gsi::Player::state,
                           [](const cs2gsi::PlayerState &old_s, const cs2gsi::PlayerState &new_s) {
                               if (new_s.health < old_s.health) {
                                   std::cout << "health: " << old_s.health << " -> " << new_s.health << "\n";
                               }
                           });

    dispatcher.on_grenade_thrown([&dispatcher](const std::string &id, const cs2gsi::Grenade &g) {
        std::cout << "Grenade " << id << " thrown by " << g.owner << "\n";

        dispatcher.grenade(id).on(&cs2gsi::Grenade::position, [id](const auto &old_pos, const auto &new_pos) {
            if (new_pos)
                std::cout << "Grenade " << id << " position: " << *new_pos << "\n";
        });

        dispatcher.on_grenade_detonated(
                id, [id](const cs2gsi::Grenade &) { std::cout << "Grenade " << id << " detonated\n"; });
    });

    cs2gsi::GSIServer server(dispatcher);
    server.start();
}
