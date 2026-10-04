#include "omnikit/Button.hpp"
#include "omnikit/DisabledScope.hpp"
#include <algorithm>

namespace OmniKit {

static const char* FindTextEnd(const char* text) {
    if (!text) return nullptr;
    const char* p = text;
    while (*p) {
        if (p[0] == '#' && p[1] == '#') return p;
        p++;
    }
    return p;
}

// Selects font and size based on button height (even scale 14..22pt):
// Large control buttons (START/STOP >= 44px) use 22pt Bold,
// Highlighted control buttons (Pause/Fault Reset 38..43px) use 20pt Medium,
// Standard buttons (34..37px) use 18pt Medium,
// Compact buttons (28..33px) use 16pt Medium,
// Miniature buttons (< 28px) use 14pt Medium
static ImFont* PickButtonFont(const UiTheme& theme, float h, float& fontPt) {
    if (h >= theme.Scale(44.0f)) {
        fontPt = 22.0f;
        return theme.fontBold ? theme.fontBold : theme.buttonFont;
    }
    if (h >= theme.Scale(38.0f)) fontPt = 20.0f;
    else if (h >= theme.Scale(34.0f)) fontPt = 18.0f;
    else if (h >= theme.Scale(28.0f)) fontPt = 16.0f;
    else fontPt = 14.0f;
    return theme.fontMedium ? theme.fontMedium : theme.buttonFont;
}

bool Button(const ButtonOptions& o, std::source_location loc) {
    return ButtonPx(ImVec2(0.0f, 0.0f), o, loc);
}

bool Button(const char* label, std::source_location loc) {
    return ButtonPx(ImVec2(0.0f, 0.0f), {.label = label}, loc);
}

bool ButtonPx(ImVec2 size, const ButtonOptions& o, std::source_location loc) {
    const char* label = o.label;
    {
        // Unspecified axes are taken from options: width — width, height — size
        const UiTheme& t = UiTheme::Get();
        if (size.x <= 0.0f) {
            if (o.width == Fill) size.x = ImGui::GetContentRegionAvail().x;
            else if (o.width > 0.0f)            size.x = t.Scale(o.width);
        }
        if (size.y <= 0.0f) size.y = t.GetMetrics(o.size).height;
    }

    const UiVariant variant = o.variant;
    const Icon icon = o.icon;

    DisabledScope disabledScope(o.disabled);
    const UiTheme& theme = UiTheme::Get();
    const SemanticStyle& style = theme.GetVariantStyle(variant);

    int pushedColors = 0;
    if (variant != UiVariant::Default) {
        ImGui::PushStyleColor(ImGuiCol_Button, style.colBg);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style.colBgHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, style.colBgActive);
        ImGui::PushStyleColor(ImGuiCol_Text, ImColor(style.colBtnText).Value);
        pushedColors = 4;
    }

    // Standard button height in the design system: Medium (36px, scaled for DPI)
    float h = (size.y > 0.0f) ? size.y : theme.GetMetrics(UiSize::Medium).height;

    float fontPt = 18.0f;
    ImFont* btnFont = PickButtonFont(theme, h, fontPt);

    theme.PushFont(btnFont, fontPt);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, theme.Scale(theme.input.frameRounding));

    const char* text_end = label ? FindTextEnd(label) : nullptr;
    bool hasLabel = (label && text_end > label);
    ImVec2 textSize = hasLabel ? ImGui::CalcTextSize(label, text_end) : ImVec2(0, 0);

    float iconSize = (icon != Icon::None) ? std::clamp(h * 0.44f, theme.Scale(12.0f), theme.Scale(24.0f)) : 0.0f;
    float gap = (hasLabel && icon != Icon::None) ? theme.Scale(8.0f) : 0.0f;
    float totalW = iconSize + gap + textSize.x;

    // Default horizontal padding: 14px on each side (for buttons with text)
    float padX = theme.Scale(14.0f);
    float minW = hasLabel ? (totalW + padX * 2.0f) : (size.x > 0.0f ? size.x : h);
    float w = (size.x > 0.0f) ? std::max(size.x, minW) : minW;

    ImVec2 screenPos = ImGui::GetCursorScreenPos();
    // Identity: explicit key, otherwise compile-time source_location + label/icon
    bool clicked = false;
    {
        AutoIdScope idScope(o.key, label, loc, static_cast<int>(icon.GetId()));
        clicked = ImGui::Button("##btn", ImVec2(w, h));
    }
    if (o.tooltip && o.tooltip[0] != '\0' && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("%s", o.tooltip);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImU32 textColor = (pushedColors > 0) ? ImGui::GetColorU32(style.colBtnText) : ImGui::GetColorU32(ImGuiCol_Text);

    float startX = screenPos.x + (w - totalW) * 0.5f;
    if (hasLabel && startX < screenPos.x + padX) {
        startX = screenPos.x + padX;
    }
    float centerY = screenPos.y + h * 0.5f;

    if (ImGui::IsItemActive()) {
        startX += 1.0f;
        centerY += 1.0f;
    }

    if (icon != Icon::None) {
        ImVec2 iconCenter(startX + iconSize * 0.5f, centerY);
        icon.Draw(dl, iconCenter, iconSize, textColor);
    }

    if (hasLabel) {
        float textX = startX + (icon != Icon::None ? (iconSize + gap) : 0.0f);
        float textY = centerY - textSize.y * 0.5f - 1.0f;
        dl->AddText(ImVec2(textX, textY), textColor, label, text_end);
    }

    ImGui::PopStyleVar();
    theme.PopFont();
    if (pushedColors > 0) ImGui::PopStyleColor(pushedColors);

    return clicked;
}

float ButtonWidth(const ButtonOptions& options) {
    return ButtonWidthPx(options, UiTheme::Get().GetMetrics(options.size).height);
}

float ButtonWidthPx(const ButtonOptions& options, float heightPx) {
    const char* label = options.label;
    const Icon icon = options.icon;
    const UiTheme& theme = UiTheme::Get();
    float h = (heightPx > 0.0f) ? heightPx : theme.GetMetrics(UiSize::Medium).height;

    float fontPt = 18.0f;
    ImFont* btnFont = PickButtonFont(theme, h, fontPt);

    theme.PushFont(btnFont, fontPt);
    const char* text_end = label ? FindTextEnd(label) : nullptr;
    bool hasLabel = (label && text_end > label);
    ImVec2 textSize = hasLabel ? ImGui::CalcTextSize(label, text_end) : ImVec2(0, 0);
    theme.PopFont();

    float iconSize = (icon != Icon::None) ? std::clamp(h * 0.44f, theme.Scale(12.0f), theme.Scale(24.0f)) : 0.0f;
    float gap = (hasLabel && icon != Icon::None) ? theme.Scale(8.0f) : 0.0f;
    float totalW = iconSize + gap + textSize.x;
    float padX = theme.Scale(14.0f);
    return hasLabel ? (totalW + padX * 2.0f) : h;
}

} // namespace OmniKit
