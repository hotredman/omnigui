#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace kvant {

/// 2D floating-point vector for layout coordinates, sizes, and offsets.
struct Vec2 {
    float x{0.0f};
    float y{0.0f};

    constexpr Vec2() noexcept = default;
    constexpr Vec2(float inX, float inY) noexcept : x(inX), y(inY) {}

    [[nodiscard]] constexpr bool operator==(const Vec2& other) const noexcept {
        return x == other.x && y == other.y;
    }

    [[nodiscard]] constexpr bool operator!=(const Vec2& other) const noexcept {
        return !(*this == other);
    }

    [[nodiscard]] constexpr Vec2 operator+(const Vec2& other) const noexcept {
        return {x + other.x, y + other.y};
    }

    [[nodiscard]] constexpr Vec2 operator-(const Vec2& other) const noexcept {
        return {x - other.x, y - other.y};
    }

    [[nodiscard]] constexpr Vec2 operator*(float scalar) const noexcept {
        return {x * scalar, y * scalar};
    }

    [[nodiscard]] constexpr Vec2 operator/(float scalar) const noexcept {
        return {x / scalar, y / scalar};
    }

    constexpr Vec2& operator+=(const Vec2& other) noexcept {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr Vec2& operator-=(const Vec2& other) noexcept {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    constexpr Vec2& operator*=(float scalar) noexcept {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    [[nodiscard]] float Length() const noexcept {
        return std::sqrt(x * x + y * y);
    }
};

/// 2D rectangle defined by top-left position and dimensions.
struct Rect {
    float x{0.0f};
    float y{0.0f};
    float w{0.0f};
    float h{0.0f};

    constexpr Rect() noexcept = default;
    constexpr Rect(float inX, float inY, float inW, float inH) noexcept
        : x(inX), y(inY), w(inW), h(inH) {}
    constexpr Rect(Vec2 pos, Vec2 size) noexcept
        : x(pos.x), y(pos.y), w(size.x), h(size.y) {}

    [[nodiscard]] constexpr Vec2 Pos() const noexcept { return {x, y}; }
    [[nodiscard]] constexpr Vec2 Size() const noexcept { return {w, h}; }

    [[nodiscard]] constexpr float Left() const noexcept { return x; }
    [[nodiscard]] constexpr float Top() const noexcept { return y; }
    [[nodiscard]] constexpr float Right() const noexcept { return x + w; }
    [[nodiscard]] constexpr float Bottom() const noexcept { return y + h; }

    [[nodiscard]] constexpr Vec2 Center() const noexcept {
        return {x + w * 0.5f, y + h * 0.5f};
    }

    [[nodiscard]] constexpr bool Contains(Vec2 pt) const noexcept {
        return pt.x >= x && pt.x <= (x + w) && pt.y >= y && pt.y <= (y + h);
    }

    [[nodiscard]] constexpr bool Intersects(const Rect& other) const noexcept {
        return x < other.Right() && Right() > other.x &&
               y < other.Bottom() && Bottom() > other.y;
    }

    [[nodiscard]] constexpr Rect Inset(float amount) const noexcept {
        return {x + amount, y + amount, std::max(0.0f, w - 2.0f * amount), std::max(0.0f, h - 2.0f * amount)};
    }

    [[nodiscard]] constexpr Rect Inset(float dx, float dy) const noexcept {
        return {x + dx, y + dy, std::max(0.0f, w - 2.0f * dx), std::max(0.0f, h - 2.0f * dy)};
    }
};

/// 32-bit RGBA color with linear blending utilities.
struct Color {
    uint8_t r{0};
    uint8_t g{0};
    uint8_t b{0};
    uint8_t a{255};

    constexpr Color() noexcept = default;
    constexpr Color(uint8_t inR, uint8_t inG, uint8_t inB, uint8_t inA = 255) noexcept
        : r(inR), g(inG), b(inB), a(inA) {}

    /// Construct from hexadecimal value (0xRRGGBBAA or 0xRRGGBB).
    [[nodiscard]] static constexpr Color Hex(uint32_t hex, bool hasAlpha = true) noexcept {
        if (hasAlpha) {
            return {
                static_cast<uint8_t>((hex >> 24) & 0xFF),
                static_cast<uint8_t>((hex >> 16) & 0xFF),
                static_cast<uint8_t>((hex >> 8) & 0xFF),
                static_cast<uint8_t>(hex & 0xFF)
            };
        }
        return {
            static_cast<uint8_t>((hex >> 16) & 0xFF),
            static_cast<uint8_t>((hex >> 8) & 0xFF),
            static_cast<uint8_t>(hex & 0xFF),
            255
        };
    }

    [[nodiscard]] static constexpr Color Rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) noexcept {
        return {r, g, b, a};
    }

    [[nodiscard]] constexpr Color WithAlpha(uint8_t newAlpha) const noexcept {
        return {r, g, b, newAlpha};
    }

    [[nodiscard]] constexpr Color WithAlphaF(float factor) const noexcept {
        const float clamped = std::clamp(factor, 0.0f, 1.0f);
        return {r, g, b, static_cast<uint8_t>(static_cast<float>(a) * clamped)};
    }

    [[nodiscard]] static Color Lerp(Color a, Color b, float t) noexcept {
        const float f = std::clamp(t, 0.0f, 1.0f);
        return {
            static_cast<uint8_t>(std::lerp(static_cast<float>(a.r), static_cast<float>(b.r), f)),
            static_cast<uint8_t>(std::lerp(static_cast<float>(a.g), static_cast<float>(b.g), f)),
            static_cast<uint8_t>(std::lerp(static_cast<float>(a.b), static_cast<float>(b.b), f)),
            static_cast<uint8_t>(std::lerp(static_cast<float>(a.a), static_cast<float>(b.a), f))
        };
    }
};

/// Semantic spacing tokens in density-independent points.
enum class Spacing : uint8_t {
    None    = 0,
    XSmall  = 4,
    Small   = 8,
    Medium  = 12,
    Large   = 16,
    XLarge  = 24,
    XXLarge = 32
};

/// Helper to convert semantic spacing enum to floating-point value.
[[nodiscard]] constexpr float ToPoints(Spacing spacing) noexcept {
    return static_cast<float>(static_cast<uint8_t>(spacing));
}

/// Density-independent point unit (dp) wrapper.
struct Dp {
    float value{0.0f};

    constexpr explicit Dp(float v) noexcept : value(v) {}

    [[nodiscard]] inline float ToPixels(float scaleFactor) const noexcept {
        return std::round(value * scaleFactor);
    }
};

namespace literals {
    constexpr Dp operator""_dp(long double v) noexcept {
        return Dp(static_cast<float>(v));
    }
    constexpr Dp operator""_dp(unsigned long long v) noexcept {
        return Dp(static_cast<float>(v));
    }
} // namespace literals

} // namespace kvant
