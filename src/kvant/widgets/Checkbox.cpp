#include "kvant/widgets/Checkbox.hpp"
#include <algorithm>
#include "kvant/Context.hpp"
#include "kvant/backend/ICanvas.hpp"
#include "kvant/layout/Node.hpp"

namespace kvant {

Response Checkbox::Run(
    Context& ctx,
    std::string_view label,
    bool* value,
    bool checkedState,
    const CheckboxOptions& opts,
    std::source_location loc
) {
    const Id id = ctx.MakeChildId(loc);
    const UiSize activeSize = ctx.GetCurrentSize();
    const bool effectiveDisabled = opts.disabled || ctx.IsDisabled();

    Response res = ctx.Interact(id);
    if (effectiveDisabled) {
        res.clicked = false;
        res.hovered = false;
        res.pressed = false;
        res.rightClicked = false;
        res.doubleClicked = false;
    }

    bool currentChecked = (value != nullptr) ? *value : checkedState;

    if (res.clicked && !effectiveDisabled) {
        currentChecked = !currentChecked;
        if (value != nullptr) {
            *value = currentChecked;
        }
        res.changed = true;
    }

    Node* node = ctx.CreateNode(NodeType::Checkbox, id);
    node->text = ctx.InternString(label);
    CheckboxOptions optsCopy = opts;
    if (!opts.description.empty()) {
        optsCopy.description = ctx.InternString(opts.description);
    }
    node->checkboxOpts = optsCopy;
    node->checked = currentChecked;
    node->uiSize = activeSize;
    node->disabled = effectiveDisabled;
    node->hovered = res.hovered;
    node->pressed = res.pressed;
    node->widthMode = SizeMode::Fit;
    node->heightMode = SizeMode::Fit;

    ctx.AdaptChildToContainer(node);

    return res;
}

Vec2 Checkbox::Measure(
    std::string_view label,
    const CheckboxOptions& opts,
    UiSize size,
    ICanvas* canvas
) {
    const CheckboxMetrics m = GetCheckboxMetrics(size);
    const bool hasLabel = !label.empty();
    const bool hasDesc = !opts.description.empty();

    if (!hasLabel && !hasDesc) {
        return {m.boxSize, m.boxSize};
    }

    Vec2 labelDims = {0.0f, 0.0f};
    if (hasLabel) {
        if (canvas) {
            labelDims = canvas->MeasureText(label, m.fontSize);
        } else {
            const float estW = static_cast<float>(label.length()) * (m.fontSize * 0.58f);
            labelDims = {estW, m.fontSize * 1.25f};
        }
    }

    Vec2 descDims = {0.0f, 0.0f};
    if (hasDesc) {
        if (canvas) {
            descDims = canvas->MeasureText(opts.description, m.descFontSize);
        } else {
            const float estW = static_cast<float>(opts.description.length()) * (m.descFontSize * 0.58f);
            descDims = {estW, m.descFontSize * 1.25f};
        }
    }

    if (opts.labelPlacement == Placement::Stacked) {
        // Label on top, checkbox in middle, description on bottom
        const float maxW = std::max({m.boxSize, labelDims.x, descDims.x});
        float totalH = m.boxSize;
        if (hasLabel) {
            totalH += labelDims.y + m.descGap;
        }
        if (hasDesc) {
            totalH += m.descGap + descDims.y;
        }
        return {maxW, totalH};
    }

    if (opts.labelPlacement == Placement::Top) {
        // Both label and description above checkbox
        const float maxW = std::max({m.boxSize, labelDims.x, descDims.x});
        float textH = 0.0f;
        if (hasLabel && hasDesc) {
            textH = labelDims.y + m.descGap + descDims.y;
        } else if (hasLabel) {
            textH = labelDims.y;
        } else if (hasDesc) {
            textH = descDims.y;
        }
        return {maxW, textH + m.descGap + m.boxSize};
    }

    if (opts.labelPlacement == Placement::Bottom) {
        // Checkbox above, both label and description below checkbox
        const float maxW = std::max({m.boxSize, labelDims.x, descDims.x});
        float textH = 0.0f;
        if (hasLabel && hasDesc) {
            textH = labelDims.y + m.descGap + descDims.y;
        } else if (hasLabel) {
            textH = labelDims.y;
        } else if (hasDesc) {
            textH = descDims.y;
        }
        return {maxW, m.boxSize + m.descGap + textH};
    }

    // Horizontal: Leading or Trailing
    const float textW = std::max(labelDims.x, descDims.x);
    float textH = 0.0f;
    if (hasLabel && hasDesc) {
        textH = labelDims.y + m.descGap + descDims.y;
    } else if (hasLabel) {
        textH = labelDims.y;
    } else if (hasDesc) {
        textH = descDims.y;
    }

    const float totalW = m.boxSize + m.gap + textW;
    const float totalH = std::max(m.boxSize, textH);
    return {totalW, totalH};
}

void Checkbox::Render(
    ICanvas& canvas,
    Rect bounds,
    std::string_view label,
    bool checked,
    const CheckboxOptions& opts,
    UiSize size,
    bool hovered,
    bool /*pressed*/,
    bool disabled
) {
    if (bounds.w <= 0.0f || bounds.h <= 0.0f) {
        return;
    }

    const CheckboxMetrics m = GetCheckboxMetrics(size);
    const bool hasLabel = !label.empty();
    const bool hasDesc = !opts.description.empty();
    const bool hasText = hasLabel || hasDesc;

    Rect boxRect;
    Rect labelRect;
    Rect descRect;
    float alignX = 0.0f;

    const float labelH = hasLabel ? std::max(m.fontSize * 1.2f, canvas.MeasureText(label, m.fontSize).y) : 0.0f;
    const float descH = hasDesc ? std::max(m.descFontSize * 1.2f, canvas.MeasureText(opts.description, m.descFontSize).y) : 0.0f;

    if (!hasText) {
        const float boxY = bounds.y + (bounds.h - m.boxSize) * 0.5f;
        boxRect = Rect(bounds.x, boxY, m.boxSize, m.boxSize);
    } else if (opts.labelPlacement == Placement::Stacked) {
        // Label on top, checkbox in middle, description on bottom
        float curY = bounds.y;
        if (hasLabel) {
            labelRect = Rect(bounds.x, curY, bounds.w, labelH);
            curY += labelH + m.descGap;
        }
        boxRect = Rect(bounds.x, curY, m.boxSize, m.boxSize);
        curY += m.boxSize + m.descGap;
        if (hasDesc) {
            descRect = Rect(bounds.x, curY, bounds.w, descH);
        }
        alignX = 0.0f;
    } else if (opts.labelPlacement == Placement::Top) {
        // Both label and description above checkbox
        float curY = bounds.y;
        if (hasLabel) {
            labelRect = Rect(bounds.x, curY, bounds.w, labelH);
            curY += labelH + (hasDesc ? m.descGap : 0.0f);
        }
        if (hasDesc) {
            descRect = Rect(bounds.x, curY, bounds.w, descH);
            curY += descH;
        }
        curY += m.descGap;
        boxRect = Rect(bounds.x, curY, m.boxSize, m.boxSize);
        alignX = 0.0f;
    } else if (opts.labelPlacement == Placement::Bottom) {
        // Checkbox on top, both label and description below checkbox
        float curY = bounds.y;
        boxRect = Rect(bounds.x, curY, m.boxSize, m.boxSize);
        curY += m.boxSize + m.descGap;
        if (hasLabel) {
            labelRect = Rect(bounds.x, curY, bounds.w, labelH);
            curY += labelH + (hasDesc ? m.descGap : 0.0f);
        }
        if (hasDesc) {
            descRect = Rect(bounds.x, curY, bounds.w, descH);
        }
        alignX = 0.0f;
    } else if (opts.labelPlacement == Placement::Leading) {
        // Horizontal: Text on left, checkbox on right
        const float boxY = bounds.y + (bounds.h - m.boxSize) * 0.5f;
        const float textW = std::max(0.0f, bounds.w - m.boxSize - m.gap);
        const float totalTextH = (hasLabel && hasDesc) ? (labelH + m.descGap + descH) : (hasLabel ? labelH : descH);
        const float startY = bounds.y + (bounds.h - totalTextH) * 0.5f;

        if (hasLabel && hasDesc) {
            labelRect = Rect(bounds.x, startY, textW, labelH);
            descRect = Rect(bounds.x, startY + labelH + m.descGap, textW, descH);
        } else if (hasLabel) {
            labelRect = Rect(bounds.x, startY, textW, labelH);
        } else if (hasDesc) {
            descRect = Rect(bounds.x, startY, textW, descH);
        }

        boxRect = Rect(bounds.x + bounds.w - m.boxSize, boxY, m.boxSize, m.boxSize);
        alignX = 1.0f;
    } else { // Trailing (default)
        // Horizontal: Checkbox on left, text on right
        const float boxY = bounds.y + (bounds.h - m.boxSize) * 0.5f;
        boxRect = Rect(bounds.x, boxY, m.boxSize, m.boxSize);

        const float textX = bounds.x + m.boxSize + m.gap;
        const float textW = std::max(0.0f, bounds.w - m.boxSize - m.gap);
        const float totalTextH = (hasLabel && hasDesc) ? (labelH + m.descGap + descH) : (hasLabel ? labelH : descH);
        const float startY = bounds.y + (bounds.h - totalTextH) * 0.5f;

        if (hasLabel && hasDesc) {
            labelRect = Rect(textX, startY, textW, labelH);
            descRect = Rect(textX, startY + labelH + m.descGap, textW, descH);
        } else if (hasLabel) {
            labelRect = Rect(textX, startY, textW, labelH);
        } else if (hasDesc) {
            descRect = Rect(textX, startY, textW, descH);
        }
        alignX = 0.0f;
    }

    // Determine colors
    Color boxFill;
    Color boxStroke;
    float strokeWidth = 1.5f;
    Color markColor = Color::Hex(0xFFFFFFFF);

    const bool isActive = checked || opts.indeterminate;

    if (isActive) {
        const VariantColors v = GetVariantColors(opts.variant);
        boxFill = hovered && !disabled ? v.bgHover : v.bg;
        boxStroke = boxFill;
    } else {
        boxFill = hovered && !disabled ? Color::Hex(0x334155FF) : Color::Hex(0x1E293BFF); // Slate-700 / Slate-800
        boxStroke = hovered && !disabled ? Color::Hex(0x64748BFF) : Color::Hex(0x475569FF); // Slate-500 / Slate-600
    }

    Color textCol = Color::Hex(0xE2E8F0FF); // Slate-200
    Color descCol = Color::Hex(0x94A3B8FF); // Slate-400

    if (disabled) {
        boxFill = boxFill.WithAlphaF(0.40f);
        boxStroke = boxStroke.WithAlphaF(0.40f);
        markColor = markColor.WithAlphaF(0.40f);
        textCol = textCol.WithAlphaF(0.40f);
        descCol = descCol.WithAlphaF(0.40f);
    }

    // Draw box
    canvas.DrawRoundRect(boxRect, m.cornerRadius, boxFill, boxStroke, strokeWidth);

    // Draw checkmark or indeterminate dash
    if (checked) {
        const Vec2 p1(boxRect.x + boxRect.w * 0.22f, boxRect.y + boxRect.h * 0.52f);
        const Vec2 p2(boxRect.x + boxRect.w * 0.42f, boxRect.y + boxRect.h * 0.74f);
        const Vec2 p3(boxRect.x + boxRect.w * 0.78f, boxRect.y + boxRect.h * 0.26f);

        canvas.DrawLine(p1, p2, markColor, m.strokeWidth);
        canvas.DrawLine(p2, p3, markColor, m.strokeWidth);
    } else if (opts.indeterminate) {
        const Vec2 p1(boxRect.x + boxRect.w * 0.25f, boxRect.y + boxRect.h * 0.50f);
        const Vec2 p2(boxRect.x + boxRect.w * 0.75f, boxRect.y + boxRect.h * 0.50f);

        canvas.DrawLine(p1, p2, markColor, m.strokeWidth);
    }

    // Draw text elements
    if (hasLabel) {
        canvas.DrawTextBox(label, labelRect, m.fontSize, textCol, alignX, 0.5f);
    }
    if (hasDesc) {
        canvas.DrawTextBox(opts.description, descRect, m.descFontSize, descCol, alignX, 0.5f);
    }
}

} // namespace kvant
