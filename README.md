# cppgsi

A C++ library for consuming CS2 Game State Integration (GSI) payloads. Provides a typed model layer over the raw JSON and an event-driven dispatcher for subscribing to state changes.

## Requirements

- C++23
- CMake 3.14+

Dependencies (bundled as submodules):
- [cpp-httplib](https://github.com/yhirose/cpp-httplib)
- [nlohmann/json](https://github.com/nlohmann/json)

## Integration

### FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(cppgsi
    GIT_REPOSITORY https://github.com/collin-b1/cppgsi
    GIT_TAG main
)
FetchContent_MakeAvailable(cppgsi)

target_link_libraries(your_target PRIVATE cppgsi::cppgsi)
```

### Submodule

```sh
git submodule add https://github.com/collin-b1/cppgsi third_party/cppgsi
git submodule update --init --recursive
```

```cmake
add_subdirectory(third_party/cppgsi)
target_link_libraries(your_target PRIVATE cppgsi::cppgsi)
```

### Installed

```sh
git clone --recurse-submodules https://github.com/collin-b1/cppgsi
cmake -S cppgsi -B cppgsi/build
cmake --install cppgsi/build --prefix /usr/local
```

```cmake
find_package(cppgsi REQUIRED)
target_link_libraries(your_target PRIVATE cppgsi::cppgsi)
```

## Usage

```cpp
#include "gsi/gsi_server.h"
#include "gsi/gsi_dispatcher.h"
#include "gsi/gsi_config.h"

int main() {
    cs2gsi::GSIConfig config;
    config.uri = "/";
    config.timeout = 1.0;
    config.buffer = 0.1;
    config.throttle = 0.0;
    config.heartbeat = 5.0;

    cs2gsi::GSIDispatcher dispatcher;

    dispatcher.on_player([](const cs2gsi::Player &prev, const cs2gsi::Player &curr) {
        // fires whenever any player field changes
    });

    dispatcher.player().on(&cs2gsi::Player::team, [](cs2gsi::PlayerTeam prev, cs2gsi::PlayerTeam curr) {
        // fires only when the team field changes
    });

    dispatcher.on_grenade_thrown([](const std::string &id, const cs2gsi::Grenade &grenade) {
        // fires when a grenade appears on the map
    });

    dispatcher.on_grenade_detonated([](const std::string &id, const cs2gsi::Grenade &grenade) {
        // fires when a grenade disappears from the map
    });

    cs2gsi::GSIServer server("127.0.0.1", 3000, config, dispatcher);
    server.start(); // blocks
}
```

See `examples/` for more complete usage covering grenades, round events, and bomb state.

### Spectator-only data

The following are only populated when the client is spectating (GOTV, coach, or in-eye observer):

- `on_all_players` / `dispatcher.all_players()` — the full player list
- `on_grenades` / `dispatcher.grenades()` — live grenade positions
- `on_grenade_thrown` / `on_grenade_detonated` — grenade lifecycle events
- `Player::position`, `Player::forward` — world-space position and facing direction

When playing, `all_players` and `grenades` will be absent from every payload and their callbacks will never fire.

## Building

```sh
git clone --recurse-submodules https://github.com/collin-b1/cppgsi
cmake -S cppgsi -B cppgsi/build
cmake --build cppgsi/build
ctest --test-dir cppgsi/build
```

To build examples:

```sh
cmake -S cppgsi -B cppgsi/build -DCPPGSI_BUILD_EXAMPLES=ON
cmake --build cppgsi/build
```

## Game configuration

CS2 must be configured to send GSI payloads to your server. Create a config file at:

```
<CS2 install>/game/csgo/cfg/gamestate_integration_<name>.cfg
```

```
"cppgsi Example"
{
    "uri"           "http://127.0.0.1:3000/"
    "timeout"       "1.0"
    "buffer"        "0.1"
    "throttle"      "0.0"
    "heartbeat"     "5.0"
    "data"
    {
        "provider"          "1"
        "map"               "1"
        "map_round_wins"    "1"
        "round"             "1"
        "player_id"         "1"
        "player_state"      "1"
        "player_weapons"    "1"
        "player_match_stats" "1"
        "allplayers_id"     "1"
        "allplayers_state"  "1"
        "allplayers_match_stats" "1"
        "allplayers_weapons" "1"
        "allplayers_position" "1"
        "phase_countdowns"  "1"
        "allgrenades"       "1"
        "bomb"              "1"
    }
}
```
