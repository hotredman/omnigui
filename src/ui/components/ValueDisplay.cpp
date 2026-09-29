#include "ui/components/ValueDisplay.hpp"
#include <imgui.h>
#include <cstdio>
#include <algorithm>
#include <cfloat>

void ValueDisplay::Render(double value, const char* unit, float width, float height, 
                          const char* format, const ValueDisplayStyle* customStyle)
{
    char buf[64];
    snprintf(buf, sizeof(buf), format ? format : "%.2f", value);
    Render(buf, unit, width, height, customStyle);
}

void ValueDisplay::Render(const char* text, const char* unit, float width, float height, 
                          const ValueDisplayStyle* customStyle)
{
    const UiTheme& theme = UiTheme::Get();
    const ValueDisplayStyle& style = customStyle ? *customStyle : theme.valueDisplay;

    float w = width;
    if (w <= 0.0f) {
        float avail = ImGui::GetContentRegionAvail().x;
        w = (avail > 0.0f) ? avail : theme.Scale(200.0f);
    }

    float h = height;
    if (h <= 0.0f) {
        h = theme.Scale(80.0f);
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 innerPos = ImGui::GetCursorScreenPos();

    // Стилизованная подложка с рамкой
    dl->AddRectFilled(innerPos, ImVec2(innerPos.x + w, innerPos.y + h), 
                      style.colBg, theme.Scale(style.cornerRadius));
    if (style.borderSize > 0.0f) {
        dl->AddRect(innerPos, ImVec2(innerPos.x + w, innerPos.y + h), 
                    style.colBorder, theme.Scale(style.cornerRadius), 0, style.borderSize);
    }

    const char* valStr = text ? text : "";
    bool isCompact = (h < theme.Scale(60.0f));

    ImFont* valFont = style.valueFont ? style.valueFont : (theme.displayBigFont ? theme.displayBigFont : theme.defaultFont);
    float valFontSize = theme.Scale(isCompact ? style.valueCompactFontSize : style.valueFontSize);
    ImVec2 valTextSize = valFont ? valFont->CalcTextSizeA(valFontSize, FLT_MAX, 0.0f, valStr) : ImGui::CalcTextSize(valStr);

    ImFont* unitFont = style.unitFont ? style.unitFont : theme.defaultFont;
    float unitFontSize = theme.Scale(isCompact ? style.unitCompactFontSize : style.unitFontSize);
    const char* unitStr = unit ? unit : "";
    ImVec2 unitTextSize = (unitStr[0] != '\0') 
        ? (unitFont ? unitFont->CalcTextSizeA(unitFontSize, FLT_MAX, 0.0f, unitStr) : ImGui::CalcTextSize(unitStr)) 
        : ImVec2(0.0f, 0.0f);

    float spacing = (unitStr[0] != '\0') ? theme.Scale(6.0f) : 0.0f;
    float totalWidth = valTextSize.x + spacing + unitTextSize.x;
    float startX = innerPos.x + (w - totalWidth) * 0.5f;
    if (startX < innerPos.x + theme.Scale(4.0f)) {
        startX = innerPos.x + theme.Scale(4.0f);
    }
    float centerY = innerPos.y + (h - valTextSize.y) * 0.5f;

    dl->AddText(valFont, valFontSize, ImVec2(startX, centerY), style.colValue, valStr);
    if (unitStr[0] != '\0') {
        float unitY = centerY + valTextSize.y - unitTextSize.y - theme.Scale(2.0f);
        dl->AddText(unitFont, unitFontSize, ImVec2(startX + valTextSize.x + spacing, unitY), style.colUnit, unitStr);
    }

    ImGui::Dummy(ImVec2(w, h));
}
