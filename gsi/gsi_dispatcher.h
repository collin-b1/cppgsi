#pragma once
#include <cstdint>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "game_state.h"
#include "models/bomb.h"

namespace cs2gsi
{
    // Opaque handle returned by subscribe calls. Pass to unsubscribe() to remove.
    using SubId = uint32_t;

    class GSIDispatcher
    {
    public:
        // Subscribe to every incoming game state update.
        SubId on_game_state(std::function<void(const GameState&)> cb);

        // Subscribe to player updates (only fired when the payload contains "player").
        SubId on_player(std::function<void(const Player&)> cb);

        // Subscribe to map updates (only fired when the payload contains "map").
        SubId on_map(std::function<void(const Map&)> cb);

        // Subscribe to round updates (only fired when the payload contains "round").
        SubId on_round(std::function<void(const Round&)> cb);

        // Subscribe to round updates (only fired when the payload contains "bomb").
        SubId on_bomb(std::function<void(const Bomb&)> cb);

        // Remove a subscription by its ID. Safe to call with an unknown ID.
        void unsubscribe(SubId id);

        // Parse the raw JSON payload and notify all relevant subscribers.
        // Thread-safe relative to concurrent subscribe/unsubscribe calls.
        void dispatch(const nlohmann::json& json);

    private:
        SubId next_id_{0};
        std::mutex mutex_;

        std::unordered_map<SubId, std::function<void(const GameState&)>> game_state_subs_;
        std::unordered_map<SubId, std::function<void(const Player&)>> player_subs_;
        std::unordered_map<SubId, std::function<void(const Map&)>> map_subs_;
        std::unordered_map<SubId, std::function<void(const Round&)>> round_subs_;
        std::unordered_map<SubId, std::function<void(const Bomb&)>> bomb_subs_;
    };
} // namespace cs2gsi
