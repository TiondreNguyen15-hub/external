#pragma once
#include <array>
#include <cmath>
#include <optional>
namespace visualizer {
struct Vec3 { float x, y, z; };
struct Vec2 { float x, y; };
// Row-major storage; column-vector multiplication. Input must already be
// world coordinates. A game adapter must explicitly convert its own layout.
using Matrix = std::array<float, 16>;
inline std::optional<Vec2> project(Vec3 p, const Matrix& m, float width, float height) {
    if (!std::isfinite(width) || !std::isfinite(height) || width <= 0 || height <= 0) return {};
    const float x = m[0]*p.x + m[1]*p.y + m[2]*p.z + m[3];
    const float y = m[4]*p.x + m[5]*p.y + m[6]*p.z + m[7];
    const float w = m[12]*p.x + m[13]*p.y + m[14]*p.z + m[15];
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(w) || w <= 0.001f) return {};
    const float nx = x/w, ny = y/w;
    if (std::abs(nx) > 1 || std::abs(ny) > 1) return {};
    return Vec2{(nx+1)*width*0.5f, (1-ny)*height*0.5f};
}
struct Player {
    int health = 100;
    int max_health = 100;
    // Head, neck, chest, pelvis, left shoulder/elbow/hand,
    // right shoulder/elbow/hand, left knee/foot, right knee/foot.
    std::array<Vec3, 14> bones;
};
inline constexpr std::array<std::array<int,2>,13> links{{
    {0,1},{1,2},{2,3},{1,4},{4,5},{5,6},{1,7},{7,8},{8,9},
    {3,10},{10,11},{3,12},{12,13}
}};
}
