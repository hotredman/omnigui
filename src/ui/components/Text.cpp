#include "ui/components/Text.hpp"
#include <cmath>
#include <cstdarg>
#include <cstdio>

namespace {

void RenderText(const char* text, ImFont* font, ImU32 color, Icon icon, float fontPt) {
    const UiTheme& theme = UiTheme::Get();
    float pt = fontPt;
    if (fontPt == 18.0f && (font == theme.fontBold || font == theme.projectTitleFont)) {
        pt = 20.0f;
    }
    theme.PushFont(font, pt);

    float fontSize = ImGui::GetFontSize();
    float lineHeight = ImGui::GetTextLineHeight();

    if (icon != Icon::None) {
        float sz = std::round(fontSize * 0.95f);
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

} // namespace

void Text(const char* text, const TextOptions& o) {
    if (!text) return;

    const UiTheme& theme = UiTheme::Get();
    const SemanticStyle& style = theme.GetVariantStyle(o.variant);

    // Многострочный текст: фиксированный кегль 16pt, переносится по границе контейнера
    if (o.wrap) {
        theme.PushFont(theme.fontRegular, 16.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, style.colText);
        ImGui::TextWrapped("%s", text);
        ImGui::PopStyleColor();
        theme.PopFont();
        return;
    }

    ImFont* font = theme.fontRegular;
    ImU32 color = style.colText;
    switch (o.role) {
        case TextRole::Body:
            break;
        case TextRole::Title:
            font = theme.fontBold ? theme.fontBold : theme.defaultFont;
            break;
        case TextRole::Muted:
            font = theme.fontRegular ? theme.fontRegular : theme.defaultFont;
            color = theme.palette.textMuted;
            break;
    }

    const float pt = o.size ? theme.GetMetrics(*o.size).fontSize : 18.0f;
    RenderText(text, font, color, o.icon, pt);
}

void Text(const std::string& text, const TextOptions& options) {
    Text(text.c_str(), options);
}

std::string Format(const char* format, ...) {
    va_list args;
    va_start(args, format);
    va_list copy;
    va_copy(copy, args);
    const int len = std::vsnprintf(nullptr, 0, format, copy);
    va_end(copy);
    std::string out;
    if (len > 0) {
        out.resize(static_cast<size_t>(len));
        std::vsnprintf(out.data(), out.size() + 1, format, args);
    }
    va_end(args);
    return out;
}
