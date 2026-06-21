#include "gsi_dispatcher.h"

namespace cs2gsi
{
    SubId GSIDispatcher::on_game_state(std::function<void(const GameState&, const GameState&)> cb)
    {
        std::lock_guard lock(mutex_);
        auto id = next_id_++;
        game_state_subs_[id] = std::move(cb);
        return id;
    }

    SubId GSIDispatcher::on_player(std::function<void(const Player&, const Player&)> cb)
    {
        std::lock_guard lock(mutex_);
        auto id = next_id_++;
        player_subs_[id] = std::move(cb);
        return id;
    }

    SubId GSIDispatcher::on_map(std::function<void(const Map&, const Map&)> cb)
    {
        std::lock_guard lock(mutex_);
        auto id = next_id_++;
        map_subs_[id] = std::move(cb);
        return id;
    }

    SubId GSIDispatcher::on_round(std::function<void(const Round&, const Round&)> cb)
    {
        std::lock_guard lock(mutex_);
        auto id = next_id_++;
        round_subs_[id] = std::move(cb);
        return id;
    }

    SubId GSIDispatcher::on_bomb(std::function<void(const Bomb&, const Bomb&)> cb)
    {
        std::lock_guard lock(mutex_);
        auto id = next_id_++;
        bomb_subs_[id] = std::move(cb);
        return id;
    }

    SubId GSIDispatcher::on_all_players(std::function<void(const AllPlayersMap&, const AllPlayersMap&)> cb)
    {
        std::lock_guard lock(mutex_);
        auto id = next_id_++;
        all_players_subs_[id] = std::move(cb);
        return id;
    }

    SubId GSIDispatcher::on_grenades(std::function<void(const GrenadeMap&, const GrenadeMap&)> cb)
    {
        std::lock_guard lock(mutex_);
        auto id = next_id_++;
        grenades_subs_[id] = std::move(cb);
        return id;
    }

    SubId GSIDispatcher::on_phase_countdowns(std::function<void(const PhaseCountdowns&, const PhaseCountdowns&)> cb)
    {
        std::lock_guard lock(mutex_);
        auto id = next_id_++;
        phase_countdowns_subs_[id] = std::move(cb);
        return id;
    }

    ComponentObserver<Bomb> GSIDispatcher::bomb()
    {
        return {[this](SubId id) { unsubscribe(id); }, [this](auto cb) { return on_bomb(std::move(cb)); }};
    }

    ComponentObserver<Player> GSIDispatcher::player()
    {
        return {[this](SubId id) { unsubscribe(id); }, [this](auto cb) { return on_player(std::move(cb)); }};
    }

    ComponentObserver<Map> GSIDispatcher::map()
    {
        return {[this](SubId id) { unsubscribe(id); }, [this](auto cb) { return on_map(std::move(cb)); }};
    }

    ComponentObserver<Round> GSIDispatcher::round()
    {
        return {[this](SubId id) { unsubscribe(id); }, [this](auto cb) { return on_round(std::move(cb)); }};
    }

    ComponentObserver<AllPlayersMap> GSIDispatcher::all_players()
    {
        return {[this](SubId id) { unsubscribe(id); }, [this](auto cb) { return on_all_players(std::move(cb)); }};
    }

    ComponentObserver<GrenadeMap> GSIDispatcher::grenades()
    {
        return {[this](SubId id) { unsubscribe(id); }, [this](auto cb) { return on_grenades(std::move(cb)); }};
    }

    ComponentObserver<PhaseCountdowns> GSIDispatcher::phase_countdowns()
    {
        return {[this](SubId id) { unsubscribe(id); }, [this](auto cb) { return on_phase_countdowns(std::move(cb)); }};
    }

    void GSIDispatcher::unsubscribe(SubId id)
    {
        std::lock_guard lock(mutex_);
        game_state_subs_.erase(id);
        player_subs_.erase(id);
        map_subs_.erase(id);
        round_subs_.erase(id);
        bomb_subs_.erase(id);
        all_players_subs_.erase(id);
        grenades_subs_.erase(id);
        phase_countdowns_subs_.erase(id);
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
        auto ap_subs = all_players_subs_;
        auto gr_subs = grenades_subs_;
        auto pc_subs = phase_countdowns_subs_;

        GameState old_gs = prev_game_state_.value_or(GameState{});
        Player old_p = prev_player_.value_or(Player{});
        Map old_m = prev_map_.value_or(Map{});
        Round old_r = prev_round_.value_or(Round{});
        Bomb old_b = prev_bomb_.value_or(Bomb{});
        AllPlayersMap old_ap = prev_all_players_.value_or(AllPlayersMap{});
        GrenadeMap old_gr = prev_grenades_.value_or(GrenadeMap{});
        PhaseCountdowns old_pc = prev_phase_countdowns_.value_or(PhaseCountdowns{});

        prev_game_state_ = state;
        if (state.player) prev_player_ = state.player;
        if (state.map) prev_map_ = state.map;
        if (state.round) prev_round_ = state.round;
        if (state.bomb) prev_bomb_ = state.bomb;
        if (state.all_players) prev_all_players_ = state.all_players;
        if (state.grenades) prev_grenades_ = state.grenades;
        if (state.phase_countdowns) prev_phase_countdowns_ = state.phase_countdowns;

        lock.unlock();

        if (state != old_gs)
            for (auto& [id, cb] : gs_subs) cb(old_gs, state);

        if (state.player && *state.player != old_p)
            for (auto& [id, cb] : p_subs) cb(old_p, *state.player);

        if (state.map && *state.map != old_m)
            for (auto& [id, cb] : m_subs) cb(old_m, *state.map);

        if (state.round && *state.round != old_r)
            for (auto& [id, cb] : r_subs) cb(old_r, *state.round);

        if (state.bomb && *state.bomb != old_b)
            for (auto& [id, cb] : b_subs) cb(old_b, *state.bomb);

        if (state.all_players && *state.all_players != old_ap)
            for (auto& [id, cb] : ap_subs) cb(old_ap, *state.all_players);

        if (state.grenades && *state.grenades != old_gr)
            for (auto& [id, cb] : gr_subs) cb(old_gr, *state.grenades);

        if (state.phase_countdowns && *state.phase_countdowns != old_pc)
            for (auto& [id, cb] : pc_subs) cb(old_pc, *state.phase_countdowns);
    }
} // namespace cs2gsi
