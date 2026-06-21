#pragma once
#include <map>
#include <optional>
#include <string>
#include <nlohmann/json.hpp>

namespace cs2gsi
{
    enum class WeaponState { Active, Holstered, Reloading, Unknown };

    struct Weapon
    {
        std::string name;
        std::string paintkit;
        std::string type;
        WeaponState state{WeaponState::Unknown};
        std::optional<int> ammo_clip;
        std::optional<int> ammo_clip_max;
        std::optional<int> ammo_reserve;

        bool operator==(const Weapon&) const = default;
        static Weapon from_json(const nlohmann::json& j);
    };

    // Keyed by slot name, e.g. "weapon_0", "weapon_1"
    using WeaponSlots = std::map<std::string, Weapon>;

    WeaponSlots weapons_from_json(const nlohmann::json& j);
} // namespace cs2gsi
