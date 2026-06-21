#include "game_state.h"

namespace cs2gsi
{
    GameState GameState::from_json(const nlohmann::json& j)
    {
        GameState gs;
        if (j.contains("added")            && j["added"].is_object())            gs.added            = Added::from_json(j["added"]);
        if (j.contains("allplayers")       && j["allplayers"].is_object())       gs.all_players      = all_players_from_json(j["allplayers"]);
        if (j.contains("bomb")             && j["bomb"].is_object())             gs.bomb             = Bomb::from_json(j["bomb"]);
        if (j.contains("grenades")         && j["grenades"].is_object())         gs.grenades         = grenades_from_json(j["grenades"]);
        if (j.contains("map")              && j["map"].is_object())              gs.map              = Map::from_json(j["map"]);
        if (j.contains("phase_countdowns") && j["phase_countdowns"].is_object()) gs.phase_countdowns = PhaseCountdowns::from_json(j["phase_countdowns"]);
        if (j.contains("player")           && j["player"].is_object())           gs.player           = Player::from_json(j["player"]);
        if (j.contains("previously")       && j["previously"].is_object())       gs.previously       = Previously::from_json(j["previously"]);
        if (j.contains("provider")         && j["provider"].is_object())         gs.provider         = Provider::from_json(j["provider"]);
        if (j.contains("round")            && j["round"].is_object())            gs.round            = Round::from_json(j["round"]);
        return gs;
    }
} // namespace cs2gsi
