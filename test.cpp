#include <iostream>
#include "gsi/gsi_config.h"
#include "gsi/gsi_dispatcher.h"
#include "gsi/gsi_server.h"

int main()
{
    const auto config_path = cs2gsi::generate_gsi_config("testgsi");
    std::cout << "GSI config written to: " << config_path << "\n";

    cs2gsi::GSIDispatcher dispatcher;

    dispatcher.on_bomb([](const cs2gsi::Bomb& bomb)
    {
        std::cout << bomb.position << std::endl;
    });

    dispatcher.on_round([](const cs2gsi::Round& round)
    {
        std::cout << static_cast<int>(round.phase) << std::endl;
    });

    cs2gsi::GSIServer server(dispatcher);
    server.start();
}
