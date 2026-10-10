#include "kvant/widgets/TabBar.hpp"
#include <algorithm>
#include "kvant/Context.hpp"
#include "kvant/backend/ICanvas.hpp"
#include "kvant/layout/Node.hpp"

namespace kvant {

Response TabBar::Run(
    Context& ctx,
    int& activeIndex,
    std::span<const TabItem> tabs,
    const TabBarOptions& opts,
    std::source_location loc
) {
    Response res;
    if (tabs.empty()) return res;

    const Id barId = ctx.MakeChildId(loc);
    const UiSize activeSize = opts.size;
    const ComponentMetrics m = GetComponentMetrics(activeSize);

    Node* barNode = ctx.CreateNode(NodeType::Container, barId);
    barNode->direction = FlexDirection::Row;
    barNode->align = Alignment::Center;
    barNode->gap = opts.gap;
    barNode->uiSize = activeSize;
    barNode->widthMode = opts.fullWidth ? SizeMode::Stretch : SizeMode::Fit;
    barNode->heightMode = SizeMode::Fixed;
    barNode->fixedHeight = m.defaultHeight;
    barNode->tabBarOpts = opts;

    ctx.PushContainer(barNode);

    for (size_t i = 0; i < tabs.size(); ++i) {
        const TabItem& item = tabs[i];
        const Id tabId = MakeId(static_cast<uint32_t>(i), loc, barId);
        const bool effectiveDisabled = item.disabled || ctx.IsDisabled();

        Response tabRes = ctx.Interact(tabId);
        if (effectiveDisabled) {
            tabRes.clicked = false;
            tabRes.hovered = false;
            tabRes.pressed = false;
        }

        if (tabRes.clicked && !effectiveDisabled && activeIndex != static_cast<int>(i)) {
            activeIndex = static_cast<int>(i);
            res.changed = true;
            res.clicked = true;
            ctx.RequestRepaint(3);
        }

        Node* tabNode = ctx.CreateNode(NodeType::TabItem, tabId);
        tabNode->text = ctx.InternString(item.label);
        tabNode->tabItemOpts = item;
        tabNode->tabItemOpts.label = tabNode->text;
        tabNode->tabBarOpts = opts;
        tabNode->tabActive = (activeIndex == static_cast<int>(i));
        tabNode->uiSize = activeSize;
        tabNode->disabled = effectiveDisabled;
        tabNode->hovered = tabRes.hovered;
        tabNode->pressed = tabRes.pressed;
        tabNode->widthMode = opts.fullWidth ? SizeMode::Stretch : SizeMode::Fit;
        tabNode->heightMode = SizeMode::Stretch;
    }

    ctx.PopContainer(barNode);
    ctx.AdaptChildToContainer(barNode);

    return res;
}

Vec2 TabBar::MeasureTab(
    const TabItem& item,
    const TabBarOptions& opts,
    UiSize size,
    ICanvas* canvas
) {
    const ComponentMetrics m = GetComponentMetrics(size);
    float contentW = 0.0f;

    if (item.icon.IsValid()) {
        contentW += m.iconSize;
    }

    if (!item.label.empty()) {
        if (contentW > 0.0f) {
            contentW += 6.0f; // Gap between icon and text
        }
        float textW = 0.0f;
        if (canvas) {
            textW = canvas->MeasureText(item.label, m.fontSize).x;
        } else {
            textW = static_cast<float>(item.label.length()) * (m.fontSize * 0.62f);
        }
        contentW += textW;
    }

    if (!item.badge.empty()) {
        contentW += 6.0f; // Gap between text and badge
        float badgeTextW = 0.0f;
        const float badgeFontSize = m.fontSize * 0.78f;
        if (canvas) {
            badgeTextW = canvas->MeasureText(item.badge, badgeFontSize).x;
        } else {
            badgeTextW = static_cast<float>(item.badge.length()) * (badgeFontSize * 0.55f);
        }
        contentW += badgeTextW + 12.0f; // Badge padding
    }

    const float padX = (opts.style == TabBarStyle::Underline) ? m.paddingX : (m.paddingX * 0.85f);
    const float totalW = contentW + padX * 2.0f;

    return {std::max(m.defaultHeight, totalW), m.defaultHeight};
}

void TabBar::RenderTab(
    ICanvas& canvas,
    Rect bounds,
    const TabItem& item,
    const TabBarOptions& opts,
    UiSize size,
    bool active,
    bool hovered,
    bool pressed,
    bool disabled
) {
    const ComponentMetrics m = GetComponentMetrics(size);

    if (opts.style == TabBarStyle::Underline) {
        // Horizontal baseline separator spanning across tabs
        if (opts.showSeparator) {
            canvas.DrawLine(
                Vec2(bounds.x, bounds.Bottom() - 0.5f),
                Vec2(bounds.Right(), bounds.Bottom() - 0.5f),
                Color::Hex(0x33415580),
                1.0f
            );
        }

        // Hover backdrop highlight
        if (hovered && !active && !disabled) {
            const Rect hoverR(bounds.x, bounds.y + 4.0f, bounds.w, bounds.h - 8.0f);
            canvas.DrawRoundRect(hoverR, 4.0f, Color::Hex(0x33415540));
        }

        // Bottom active underline accent
        if (active) {
            const float lineH = 3.0f;
            const float linePad = 2.0f;
            const Color lineCol = (opts.activeColor.a > 0)
                ? opts.activeColor
                : (item.variant != UiVariant::Default ? GetVariantColors(item.variant).bg : Color::Hex(0x38BDF8FF));
            const Rect lineR(bounds.x + linePad, bounds.Bottom() - lineH, std::max(0.0f, bounds.w - linePad * 2.0f), lineH);
            canvas.DrawRoundRect(lineR, 1.5f, lineCol);
        }
    } else {
        // Pill / Segmented style
        if (active) {
            const Color pillCol = (opts.activeColor.a > 0)
                ? opts.activeColor
                : (item.variant != UiVariant::Default ? GetVariantColors(item.variant).bgActive : Color::Hex(0x334155FF));
            canvas.DrawRoundRect(bounds, 6.0f, pillCol);
        } else if (hovered && !disabled) {
            canvas.DrawRoundRect(bounds, 6.0f, Color::Hex(0x1E293B80));
        }
    }

    // Determine foreground colors for text and icon
    Color fgCol;
    if (disabled) {
        fgCol = Color::Hex(0x64748B80);
    } else if (active) {
        fgCol = Color::Hex(0xF8FAFCFF);
    } else if (hovered) {
        fgCol = Color::Hex(0xE2E8F0FF);
    } else {
        fgCol = Color::Hex(0x94A3B8FF);
    }

    // Measure components to center content within tab bounds
    float contentW = 0.0f;
    if (item.icon.IsValid()) {
        contentW += m.iconSize;
    }
    float textW = 0.0f;
    if (!item.label.empty()) {
        if (contentW > 0.0f) contentW += 6.0f;
        textW = canvas.MeasureText(item.label, m.fontSize).x;
        contentW += textW;
    }
    float badgeW = 0.0f;
    const float badgeFontSize = m.fontSize * 0.78f;
    if (!item.badge.empty()) {
        contentW += 6.0f;
        badgeW = canvas.MeasureText(item.badge, badgeFontSize).x + 12.0f;
        contentW += badgeW;
    }

    float curX = bounds.x + std::max(0.0f, (bounds.w - contentW) * 0.5f);
    const float centerY = bounds.y + bounds.h * 0.5f;

    // Draw Icon
    if (item.icon.IsValid()) {
        const Rect iconR(curX, centerY - m.iconSize * 0.5f, m.iconSize, m.iconSize);
        item.icon.Draw(canvas, iconR, fgCol);
        curX += m.iconSize + 6.0f;
    }

    // Draw Label
    if (!item.label.empty()) {
        const Rect textR(curX, bounds.y, textW, bounds.h);
        canvas.DrawTextBox(item.label, textR, m.fontSize, fgCol, 0.0f, 0.5f);
        curX += textW;
    }

    // Draw Badge
    if (!item.badge.empty()) {
        curX += 6.0f;
        const float badgeH = badgeFontSize + 6.0f;
        const Rect badgeR(curX, centerY - badgeH * 0.5f, badgeW, badgeH);

        Color badgeBg = active ? Color::Hex(0x0284C740) : Color::Hex(0x33415580);
        Color badgeText = active ? Color::Hex(0x38BDF8FF) : Color::Hex(0x94A3B8FF);

        // Highlight "Live" badges in emerald green
        if (item.badge == "Live") {
            badgeBg = Color::Hex(0x065F4680);
            badgeText = Color::Hex(0x34D399FF);
        }

        canvas.DrawRoundRect(badgeR, 4.0f, badgeBg);
        canvas.DrawTextBox(item.badge, badgeR, badgeFontSize, badgeText, 0.5f, 0.5f);
    }
}

} // namespace kvant
