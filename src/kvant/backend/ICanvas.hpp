#pragma once

#include <cstdint>
#include <string_view>
#include "kvant/core/Types.hpp"

namespace kvant {

/// Abstract interface for vector graphics rendering backends.
class ICanvas {
public:
    virtual ~ICanvas() = default;

    /// Prepares the canvas for a new frame.
    virtual void Begin(uint32_t width, uint32_t height) = 0;

    /// Finalizes and commits all recorded rendering commands.
    virtual void End() = 0;

    /// Pushes a rectangular clipping boundary.
    virtual void PushClip(Rect clipRect) = 0;

    /// Restores the previous clipping boundary.
    virtual void PopClip() = 0;

    /// Draws a solid axis-aligned rectangle.
    virtual void DrawRect(Rect rect, Color fill) = 0;

    /// Draws a rounded rectangle with optional fill, stroke, and corner radius.
    virtual void DrawRoundRect(
        Rect rect,
        float radius,
        Color fill,
        Color stroke = Color(0, 0, 0, 0),
        float strokeWidth = 0.0f
    ) = 0;

    /// Measures the dimensions of a single-line text string using backend font metrics.
    virtual Vec2 MeasureText(std::string_view text, float fontSize) = 0;

    /// Draws a single-line text string with specified font size and color.
    virtual void DrawText(std::string_view text, Vec2 pos, float fontSize, Color color) = 0;

    /// Draws text constrained within a bounding rectangle with horizontal and vertical alignment factors [0..1].
    virtual void DrawTextBox(
        std::string_view text,
        Rect bounds,
        float fontSize,
        Color color,
        float alignX = 0.0f,
        float alignY = 0.5f
    ) = 0;

    /// Draws a line segment between two points.
    virtual void DrawLine(Vec2 p1, Vec2 p2, Color color, float strokeWidth = 1.0f) = 0;
};

} // namespace kvant
