#include <iostream>
#include "gsi/gsi_config.h"
#include "gsi/gsi_dispatcher.h"
#include "gsi/gsi_server.h"

int main()
{
    const auto config_path = cs2gsi::generate_gsi_config("testgsi");
    std::cout << "GSI config written to: " << config_path << "\n";

    cs2gsi::GSIDispatcher dispatcher;

    dispatcher.player().on(&cs2gsi::Player::state,
                           [](const auto& old_s, const auto& new_s)
                           {
                               if (old_s.health > new_s.health)
                               {
                                   std::cout << "health: " << old_s.health << " -> " << new_s.health << "\n";
                               }
                           });

    cs2gsi::GSIServer server(dispatcher);
    server.start();
}
