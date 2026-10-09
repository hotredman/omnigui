#include "kvant/widgets/Button.hpp"
#include <algorithm>
#include "kvant/backend/ICanvas.hpp"

namespace kvant {

Vec2 Button::Measure(const ButtonOptions& opts, UiSize size, ICanvas* canvas) {
    const ComponentMetrics m = GetComponentMetrics(size);
    const bool hasIcon = opts.icon.IsValid();
    const bool hasLabel = !opts.label.empty();

    // Icon-only button defaults to square
    if (hasIcon && !hasLabel) {
        return {m.defaultHeight, m.defaultHeight};
    }

    float contentW = 0.0f;
    if (hasIcon && hasLabel) {
        float textW = 0.0f;
        if (canvas) {
            textW = canvas->MeasureText(opts.label, m.fontSize).x;
        } else {
            textW = static_cast<float>(opts.label.length()) * (m.fontSize * 0.62f);
        }
        const float gap = 8.0f;
        contentW = m.iconSize + gap + textW;
    } else if (hasLabel) {
        if (canvas) {
            contentW = canvas->MeasureText(opts.label, m.fontSize).x;
        } else {
            contentW = static_cast<float>(opts.label.length()) * (m.fontSize * 0.62f);
        }
    }

    const float totalW = contentW + m.paddingX * 2.0f;
    return {std::max(m.defaultHeight, totalW), m.defaultHeight};
}

void Button::Render(
    ICanvas& canvas,
    Rect bounds,
    const ButtonOptions& opts,
    UiSize size,
    bool hovered,
    bool pressed,
    bool disabled
) {
    const ComponentMetrics m = GetComponentMetrics(size);
    const VariantColors colors = GetVariantColors(opts.variant);

    Color bg = colors.bg;
    Color textCol = colors.text;
    Color borderCol = colors.border;
    float borderW = colors.borderWidth;

    if (disabled) {
        bg = bg.WithAlphaF(0.40f);
        textCol = textCol.WithAlphaF(0.40f);
        borderCol = borderCol.WithAlphaF(0.40f);
    } else if (pressed) {
        bg = colors.bgActive;
    } else if (hovered) {
        bg = colors.bgHover;
    }

    // Background & border
    canvas.DrawRoundRect(bounds, m.cornerRadius, bg, borderCol, borderW);

    const bool hasIcon = opts.icon.IsValid();
    const bool hasLabel = !opts.label.empty();

    if (hasIcon && !hasLabel) {
        // Centered icon
        const float isz = m.iconSize;
        const Rect iconBounds(
            bounds.x + (bounds.w - isz) * 0.5f,
            bounds.y + (bounds.h - isz) * 0.5f,
            isz, isz
        );
        opts.icon.Draw(canvas, iconBounds, textCol);
    } else if (!hasIcon && hasLabel) {
        // Centered label
        canvas.DrawTextBox(opts.label, bounds, m.fontSize, textCol, 0.5f, 0.5f);
    } else if (hasIcon && hasLabel) {
        // Icon + Label
        const float gap = 8.0f;
        float textW = 0.0f;
        textW = canvas.MeasureText(opts.label, m.fontSize).x;
        const float totalContentW = m.iconSize + gap + textW;
        const float startX = bounds.x + (bounds.w - totalContentW) * 0.5f;
        const float iconY = bounds.y + (bounds.h - m.iconSize) * 0.5f;

        if (opts.iconPlacement == IconPlacement::Leading) {
            opts.icon.Draw(canvas, Rect(startX, iconY, m.iconSize, m.iconSize), textCol);
            const Rect textRect(startX + m.iconSize + gap, bounds.y, textW, bounds.h);
            canvas.DrawTextBox(opts.label, textRect, m.fontSize, textCol, 0.0f, 0.5f);
        } else {
            const Rect textRect(startX, bounds.y, textW, bounds.h);
            canvas.DrawTextBox(opts.label, textRect, m.fontSize, textCol, 0.0f, 0.5f);
            opts.icon.Draw(canvas, Rect(startX + textW + gap, iconY, m.iconSize, m.iconSize), textCol);
        }
    }
}

} // namespace kvant
