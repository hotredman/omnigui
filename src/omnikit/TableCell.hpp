#pragma once

#include "omnikit/UiTheme.hpp"
#include "omnikit/Badge.hpp"
#include <imgui.h>
#include <string>

namespace OmniKit {

// Semantic typographic blocks for table cells (full ImGui encapsulation)
class TableCell {
public:
    // Two-line block: title + muted subtitle (description/file)
    static void TitleWithSubtitle(const std::string& title, const std::string& subtitle) {
        UiTheme& theme = UiTheme::Get();

        // 1. Primary title (20pt Medium cellFontSize)
        theme.PushFont(theme.fontMedium, theme.table.cellFontSize);
        ImGui::TextColored(ImColor(theme.palette.textPrimary).Value, "%s", title.c_str());
        theme.PopFont();

        // 2. Subtitle (18pt Regular subFontSize)
        if (!subtitle.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImColor(theme.palette.textMuted).Value);
            theme.PushFont(theme.fontRegular, theme.table.subFontSize);
            ImGui::TextUnformatted(subtitle.c_str());
            theme.PopFont();
            ImGui::PopStyleColor();
        }
    }

    // Centered secondary color date/time
    static void DateTime(const std::string& formattedDate, float topOffset = -1.0f) {
        UiTheme& theme = UiTheme::Get();
        float offset = (topOffset >= 0.0f) ? topOffset : std::max(0.0f, (theme.table.rowHeight - theme.table.cellFontSize) * 0.5f - theme.table.cellPaddingY);
        if (offset > 0.0f) {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + theme.Scale(offset));
        }
        theme.PushFont(theme.fontRegular, theme.table.cellFontSize);
        ImGui::TextColored(ImColor(theme.palette.textMuted).Value, "%s", formattedDate.c_str());
        theme.PopFont();
    }

    // Vertically centered text in table row based on font line height
    static void Centered(const std::string& text, float fontSize, ImU32 color = 0) {
        UiTheme& theme = UiTheme::Get();
        theme.PushFont(theme.fontRegular, fontSize);
        const float lineHeight = ImGui::GetTextLineHeight();
        const float offset = (theme.Scale(theme.table.rowHeight) - lineHeight) * 0.5f -
                             theme.Scale(theme.table.cellPaddingY);
        if (offset > 0.0f) {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offset);
        }
        ImGui::TextColored(ImColor(color != 0 ? color : theme.palette.textPrimary).Value, "%s", text.c_str());
        theme.PopFont();
    }

    // Text cell with standard vertical padding
    static void Text(const std::string& text, float topOffset = -1.0f, ImU32 color = 0) {
        UiTheme& theme = UiTheme::Get();
        float offset = (topOffset >= 0.0f) ? topOffset : std::max(0.0f, (theme.table.rowHeight - theme.table.cellFontSize) * 0.5f - theme.table.cellPaddingY);
        if (offset > 0.0f) {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + theme.Scale(offset));
        }
        ImU32 c = (color != 0) ? color : theme.palette.textPrimary;
        theme.PushFont(theme.fontRegular, theme.table.cellFontSize);
        ImGui::TextColored(ImColor(c).Value, "%s", text.c_str());
        theme.PopFont();
    }

    // Metric symbol (18pt Bold, Primary / accent color)
    static void Symbol(const std::string& symbol, float topOffset = 0.0f) {
        UiTheme& theme = UiTheme::Get();
        if (topOffset > 0.0f) {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + theme.Scale(topOffset));
        }
        theme.PushFont(theme.fontBold, 18.0f);
        ImGui::TextColored(ImColor(theme.GetVariantStyle(UiVariant::Primary).colText).Value, "%s", symbol.c_str());
        theme.PopFont();
    }

    // Numeric value (18pt Medium, textPrimary) or disabled dash (if empty or "—")
    static void Value(const std::string& displayValue, float topOffset = 0.0f) {
        UiTheme& theme = UiTheme::Get();
        if (topOffset > 0.0f) {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + theme.Scale(topOffset));
        }
        theme.PushFont(theme.fontMedium, 18.0f);
        if (displayValue.empty() || displayValue == "—") {
            ImGui::TextDisabled("—");
        } else {
            ImGui::TextColored(ImColor(theme.palette.textPrimary).Value, "%s", displayValue.c_str());
        }
        theme.PopFont();
    }

    // Measurement unit (16pt Regular, textMuted)
    static void Unit(const std::string& unitLabel, float topOffset = 0.0f) {
        UiTheme& theme = UiTheme::Get();
        if (topOffset > 0.0f) {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + theme.Scale(topOffset));
        }
        theme.PushFont(theme.fontRegular, 16.0f);
        ImGui::TextColored(ImColor(theme.palette.textMuted).Value, "%s", unitLabel.c_str());
        theme.PopFont();
    }

    // Metric name / description (16pt Regular, textPrimary)
    static void Name(const std::string& name, float topOffset = 0.0f, float fontSize = 16.0f) {
        UiTheme& theme = UiTheme::Get();
        if (topOffset > 0.0f) {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + theme.Scale(topOffset));
        }
        theme.PushFont(theme.fontRegular, fontSize);
        ImGui::TextUnformatted(name.c_str());
        theme.PopFont();
    }

    // Semantic chip/badge (e.g., 5 runs)
    static void Badge(int value, const char* unit = nullptr, UiVariant variant = UiVariant::Default, float topOffset = -1.0f) {
        float offset = (topOffset >= 0.0f) ? topOffset : TableRowOffset();
        OmniKit::BadgeNumber(value, unit, {.variant = variant, .topOffset = offset});
    }

    // Text semantic chip/badge (e.g., "Custom", "Normal")
    static void BadgeText(const char* text, UiVariant variant = UiVariant::Default, float topOffset = -1.0f, const char* tooltip = nullptr) {
        float offset = (topOffset >= 0.0f) ? topOffset : TableRowOffset();
        OmniKit::Badge(text, {.variant = variant, .tooltip = tooltip, .topOffset = offset});
    }

    // Semantic metric status badge (status badge with tooltip + optional "Custom" badge)
    static void MetricStatus(const std::string& status, const std::string& errorTooltip = "", bool isOverridden = false) {
        const char* label = "Normal";
        UiVariant variant = UiVariant::Success;
        if (status == "ok") {
            label = "Normal";
            variant = UiVariant::Success;
        } else if (status == "manual_needed") {
            label = "No Data";
            variant = UiVariant::Warning;
        } else if (status == "cyclic_dependency") {
            label = "Cyclic Reference";
            variant = UiVariant::Danger;
        } else if (status == "empty") {
            label = "No Formula";
            variant = UiVariant::Warning;
        } else {
            label = "Error";
            variant = UiVariant::Danger;
        }

        BadgeText(label, variant, 0.0f, errorTooltip.empty() ? nullptr : errorTooltip.c_str());

        if (isOverridden) {
            ImGui::SameLine(0.0f, 4.0f);
            BadgeText("Custom", UiVariant::Info, 0.0f, "Calculation parameters configured for this sample");
        }
    }

private:
    static float TableRowOffset() {
        const UiTheme& theme = UiTheme::Get();
        return std::max(0.0f, (theme.table.rowHeight - 25.0f) * 0.5f - theme.table.cellPaddingY);
    }
};

} // namespace OmniKit
