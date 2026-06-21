#include "gsi_dispatcher.h"

namespace cs2gsi
{
    SubId GSIDispatcher::on_game_state(std::function<void(const GameState&)> cb)
    {
        std::lock_guard lock(mutex_);
        auto id = next_id_++;
        game_state_subs_[id] = std::move(cb);
        return id;
    }

    SubId GSIDispatcher::on_player(std::function<void(const Player&)> cb)
    {
        std::lock_guard lock(mutex_);
        auto id = next_id_++;
        player_subs_[id] = std::move(cb);
        return id;
    }

    SubId GSIDispatcher::on_map(std::function<void(const Map&)> cb)
    {
        std::lock_guard lock(mutex_);
        auto id = next_id_++;
        map_subs_[id] = std::move(cb);
        return id;
    }

    SubId GSIDispatcher::on_round(std::function<void(const Round&)> cb)
    {
        std::lock_guard lock(mutex_);
        auto id = next_id_++;
        round_subs_[id] = std::move(cb);
        return id;
    }

    SubId GSIDispatcher::on_bomb(std::function<void(const Bomb&)> cb)
    {
        std::lock_guard lock(mutex_);
        auto id = next_id_++;
        bomb_subs_[id] = std::move(cb);
        return id;
    }

    void GSIDispatcher::unsubscribe(SubId id)
    {
        std::lock_guard lock(mutex_);
        game_state_subs_.erase(id);
        player_subs_.erase(id);
        map_subs_.erase(id);
        round_subs_.erase(id);
        bomb_subs_.erase(id);
    }

    void GSIDispatcher::dispatch(const nlohmann::json& json)
    {
        GameState state = GameState::from_json(json);

        std::unique_lock lock(mutex_);
        auto gs_subs = game_state_subs_;
        auto p_subs = player_subs_;
        auto m_subs = map_subs_;
        auto r_subs = round_subs_;
        auto b_subs = bomb_subs_;
        lock.unlock();

        for (auto& [id, cb] : gs_subs) cb(state);
        if (state.player) for (auto& [id, cb] : p_subs) cb(*state.player);
        if (state.map) for (auto& [id, cb] : m_subs) cb(*state.map);
        if (state.round) for (auto& [id, cb] : r_subs) cb(*state.round);
        if (state.bomb) for (auto& [id, cb] : b_subs) cb(*state.bomb);
    }
} // namespace cs2gsi
