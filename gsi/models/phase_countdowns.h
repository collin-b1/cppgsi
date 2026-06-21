#pragma once
#include <nlohmann/json.hpp>

namespace cs2gsi
{
    enum class PhaseCountdownPhase { Warmup, FreezeTime, Live, Bomb, Over, Unknown };

    struct PhaseCountdowns
    {
        PhaseCountdownPhase phase{PhaseCountdownPhase::Unknown};
        double phase_ends_in{};

        bool operator==(const PhaseCountdowns&) const = default;
        static PhaseCountdowns from_json(const nlohmann::json& j);
    };
} // namespace cs2gsi
