#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "kvant/backend/ICanvas.hpp"

namespace tvg {
class SwCanvas;
class Paint;
class Text;
} // namespace tvg

namespace kvant {

/// ThorVG-based vector graphics canvas implementation.
class ThorCanvas : public ICanvas {
public:
    ThorCanvas();
    ~ThorCanvas() override;

    ThorCanvas(const ThorCanvas&) = delete;
    ThorCanvas& operator=(const ThorCanvas&) = delete;

    /// Configures the render target buffer (ARGB8888 / ABGR8888).
    void SetTarget(uint32_t* buffer, uint32_t width, uint32_t height, uint32_t stride);

    /// Loads a TrueType/OpenType font from file for text rendering.
    bool LoadFontFile(const std::string& path);

    // ICanvas implementation
    void Begin(uint32_t width, uint32_t height) override;
    void End() override;

    void PushClip(Rect clipRect) override;
    void PopClip() override;

    void DrawRect(Rect rect, Color fill) override;
    void DrawRoundRect(
        Rect rect,
        float radius,
        Color fill,
        Color stroke = Color(0, 0, 0, 0),
        float strokeWidth = 0.0f
    ) override;

    Vec2 MeasureText(std::string_view text, float fontSize) override;
    void DrawText(std::string_view text, Vec2 pos, float fontSize, Color color) override;
    void DrawTextBox(
        std::string_view text,
        Rect bounds,
        float fontSize,
        Color color,
        float alignX = 0.0f,
        float alignY = 0.5f
    ) override;
    void DrawLine(Vec2 p1, Vec2 p2, Color color, float strokeWidth = 1.0f) override;

private:
    void ApplyClip(tvg::Paint* paint);

    tvg::SwCanvas* m_canvas{nullptr};
    tvg::Text* m_measureText{nullptr};
    uint32_t* m_targetBuffer{nullptr};
    uint32_t m_width{0};
    uint32_t m_height{0};
    uint32_t m_stride{0};

    std::vector<Rect> m_clipStack;
    std::string m_defaultFontPath;
    bool m_fontLoaded{false};
};

} // namespace kvant
