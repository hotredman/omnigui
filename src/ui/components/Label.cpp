#include "ui/components/Label.hpp"
#include <cmath>

void Label::RenderCustom(const char* text, ImFont* font, ImU32 color, Icon icon, float iconSize, float fontPt) {
    if (!text) return;

    const UiTheme& theme = UiTheme::Get();
    float pt = fontPt;
    if (fontPt == 18.0f && (font == theme.fontBold || font == theme.projectTitleFont)) {
        pt = 20.0f;
    }
    theme.PushFont(font, pt);

    float fontSize = ImGui::GetFontSize();
    float lineHeight = ImGui::GetTextLineHeight();

    if (icon != Icon::None) {
        float sz = (iconSize > 0.0f) ? iconSize : std::round(fontSize * 0.95f);
        ImVec2 cursor = ImGui::GetCursorScreenPos();
        float offsetY = std::round((lineHeight - sz) * 0.5f);
        icon.DrawAt(ImGui::GetWindowDrawList(), ImVec2(cursor.x, cursor.y + offsetY), sz, color);
        ImGui::SetCursorScreenPos(ImVec2(cursor.x + sz + theme.SpacingSmall(), cursor.y));
    }

    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();

    theme.PopFont();
}

void Label::Render(const char* text, UiVariant variant, Icon icon, float iconSize) {
    const UiTheme& theme = UiTheme::Get();
    const SemanticStyle& style = theme.GetVariantStyle(variant);
    RenderCustom(text, theme.fontRegular, style.colText, icon, iconSize);
}

void Label::Title(const char* text, UiVariant variant, Icon icon) {
    const UiTheme& theme = UiTheme::Get();
    ImFont* font = theme.fontBold ? theme.fontBold : theme.defaultFont;
    const SemanticStyle& style = theme.GetVariantStyle(variant);
    RenderCustom(text, font, style.colText, icon);
}

void Label::Muted(const char* text, Icon icon) {
    const UiTheme& theme = UiTheme::Get();
    ImFont* font = theme.fontRegular ? theme.fontRegular : theme.defaultFont;
    RenderCustom(text, font, theme.palette.textMuted, icon);
}

void Label::Muted(const char* text, UiSize size, Icon icon) {
    const UiTheme& theme = UiTheme::Get();
    ControlMetrics m = theme.GetMetrics(size);
    ImFont* font = theme.fontRegular ? theme.fontRegular : theme.defaultFont;
    RenderCustom(text, font, theme.palette.textMuted, icon, 0.0f, m.fontSize);
}

void Label::Wrapped(const char* text, UiVariant variant) {
    if (!text) return;
    const UiTheme& theme = UiTheme::Get();
    theme.PushFont(theme.fontRegular, 16.0f);
    const SemanticStyle& style = theme.GetVariantStyle(variant);
    ImGui::PushStyleColor(ImGuiCol_Text, style.colText);
    ImGui::TextWrapped("%s", text);
    ImGui::PopStyleColor();
    theme.PopFont();
}
