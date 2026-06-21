#pragma once
#include <ostream>

namespace cs2gsi
{
    struct Vec3
    {
        double x{};
        double y{};
        double z{};

        bool operator==(const Vec3&) const = default;

        friend std::ostream& operator<<(std::ostream& os, const Vec3& v)
        {
            return os << v.x << ", " << v.y << ", " << v.z;
        }
    };
} // namespace cs2gsi
