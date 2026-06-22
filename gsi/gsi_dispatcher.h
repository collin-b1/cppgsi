#pragma once
#include <functional>
#include <mutex>
#include <nlohmann/json.hpp>
#include <optional>
#include <unordered_map>
#include "component_observer.h"
#include "game_state.h"
#include "models/bomb.h"

namespace cs2gsi {
    class GSIDispatcher {
    public:
        // Component-level subscriptions. Fired whenever any field in the component changes.
        SubId on_game_state(std::function<void(const GameState &, const GameState &)> cb);
        SubId on_player(std::function<void(const Player &, const Player &)> cb);
        SubId on_map(std::function<void(const Map &, const Map &)> cb);
        SubId on_round(std::function<void(const Round &, const Round &)> cb);
        SubId on_bomb(std::function<void(const Bomb &, const Bomb &)> cb);
        SubId on_all_players(std::function<void(const AllPlayersMap &, const AllPlayersMap &)> cb);
        SubId on_grenades(std::function<void(const GrenadeMap &, const GrenadeMap &)> cb);
        SubId on_phase_countdowns(std::function<void(const PhaseCountdowns &, const PhaseCountdowns &)> cb);
        SubId on_grenade_thrown(std::function<void(const std::string &, const Grenade &)> cb);
        SubId on_grenade_detonated(const std::string &id, std::function<void(const Grenade &)> cb);

        // Field-level proxies. Use proxy.on(&Type::field, cb) to subscribe to a
        // single field — the callback fires only when that specific field changes.
        ComponentObserver<Bomb> bomb();
        ComponentObserver<Player> player();
        ComponentObserver<Map> map();
        ComponentObserver<Round> round();
        ComponentObserver<AllPlayersMap> all_players();
        ComponentObserver<GrenadeMap> grenades();
        ComponentObserver<PhaseCountdowns> phase_countdowns();
        ComponentObserver<Grenade> grenade(const std::string &id);

        // Remove a subscription by its ID. Safe to call with an unknown ID.
        void unsubscribe(SubId id);

        // Parse the raw JSON payload and notify subscribers whose component changed.
        // Thread-safe relative to concurrent subscribe/unsubscribe calls.
        void dispatch(const nlohmann::json &json);

    private:
        SubId next_id_{0};
        std::mutex mutex_;

        std::unordered_map<SubId, std::function<void(const GameState &, const GameState &)>> game_state_subs_;
        std::unordered_map<SubId, std::function<void(const Player &, const Player &)>> player_subs_;
        std::unordered_map<SubId, std::function<void(const Map &, const Map &)>> map_subs_;
        std::unordered_map<SubId, std::function<void(const Round &, const Round &)>> round_subs_;
        std::unordered_map<SubId, std::function<void(const Bomb &, const Bomb &)>> bomb_subs_;
        std::unordered_map<SubId, std::function<void(const AllPlayersMap &, const AllPlayersMap &)>> all_players_subs_;
        std::unordered_map<SubId, std::function<void(const GrenadeMap &, const GrenadeMap &)>> grenades_subs_;
        std::unordered_map<SubId, std::function<void(const PhaseCountdowns &, const PhaseCountdowns &)>>
                phase_countdowns_subs_;
        std::unordered_map<SubId, std::function<void(const std::string &, const Grenade &)>> grenade_thrown_subs_;
        std::unordered_map<std::string,
                           std::unordered_map<SubId, std::function<void(const Grenade &, const Grenade &)>>>
                grenade_field_subs_;
        std::unordered_map<std::string, std::unordered_map<SubId, std::function<void(const Grenade &)>>>
                grenade_detonated_subs_;

        std::optional<GameState> prev_game_state_;
        std::optional<Player> prev_player_;
        std::optional<Map> prev_map_;
        std::optional<Round> prev_round_;
        std::optional<Bomb> prev_bomb_;
        std::optional<AllPlayersMap> prev_all_players_;
        std::optional<GrenadeMap> prev_grenades_;
        std::optional<PhaseCountdowns> prev_phase_countdowns_;
    };
} // namespace cs2gsi
