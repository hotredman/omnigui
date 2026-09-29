#include "ui/components/Indicator.hpp"
#include "ui/components/ContextMenu.hpp"
#include "ui/components/Utf8Text.hpp"
#include <imgui.h>
#include <algorithm>
#include <cstdio>

static const char* GetPrecisionLabel(int p) {
    switch (p) {
        case 0: return "0 decimals (0)";
        case 1: return "1 decimal (0.0)";
        case 2: return "2 decimals (0.00)";
        case 3: return "3 decimals (0.000)";
        case 4: return "4 decimals (0.0000)";
        case 5: return "5 decimals (0.00000)";
        default: return "";
    }
}

Indicator::Indicator(std::string title, std::string value, std::string unit, int digitsCount, bool showTare)
    : m_title(Utf8ToUpper(title))
    , m_value(std::move(value))
    , m_unit(std::move(unit))
    , m_digitsCount(digitsCount)
    , m_showTare(showTare)
{
    InitFromValueString();
}

Indicator::Indicator(std::string title, std::string value, std::string unit, bool showTare)
    : Indicator(std::move(title), std::move(value), std::move(unit), 0, showTare)
{
}

Indicator::Indicator(std::string title, double value, int precision, std::string unit, int digitsCount, bool showTare)
    : m_title(Utf8ToUpper(title))
    , m_unit(std::move(unit))
    , m_numericValue(value)
    , m_precision(std::clamp(precision, 0, 5))
    , m_digitsCount(digitsCount)
    , m_showTare(showTare)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "%.*f", m_precision, m_numericValue);
    m_value = buf;
    m_integerDigits = 2;
}

void Indicator::InitFromValueString() {
    size_t dotPos = m_value.find('.');
    if (dotPos != std::string::npos) {
        m_precision = static_cast<int>(m_value.length() - dotPos - 1);
        m_integerDigits = std::max(1, static_cast<int>(dotPos));
    } else {
        m_precision = 0;
        m_integerDigits = std::max(1, static_cast<int>(m_value.length()));
    }
    m_precision = std::clamp(m_precision, 0, 5);

    try {
        m_numericValue = std::stod(m_value);
    } catch (...) {
        m_numericValue = 0.0;
    }
}

void Indicator::SetValue(const std::string& value) {
    try {
        m_numericValue = std::stod(value);
        char buf[64];
        snprintf(buf, sizeof(buf), "%.*f", m_precision, m_numericValue);
        m_value = buf;
    } catch (...) {
        m_value = value;
    }
}

void Indicator::SetValue(double value) {
    m_numericValue = value;
    char buf[64];
    snprintf(buf, sizeof(buf), "%.*f", m_precision, m_numericValue);
    m_value = buf;
}

void Indicator::SetPrecision(int precision) {
    m_precision = std::clamp(precision, 0, 5);
    char buf[64];
    snprintf(buf, sizeof(buf), "%.*f", m_precision, m_numericValue);
    m_value = buf;

    int intPart = std::max(2, m_integerDigits);
    m_digitsCount = intPart + (m_precision > 0 ? (1 + m_precision) : 0);
}

void Indicator::SetTitle(std::string title) {
    std::string upper = Utf8ToUpper(title);
    if (m_title != upper) {
        m_title = std::move(upper);
        m_widthDirty = true;
    }
}

void Indicator::SetShowTare(bool show) {
    if (m_showTare != show) {
        m_showTare = show;
        m_widthDirty = true;
    }
}

void Indicator::SetOnRenderTitleMenu(MenuCallback callback) {
    m_onRenderTitleMenu = std::move(callback);
}

void Indicator::SetOnRenderUnitMenu(MenuCallback callback) {
    m_onRenderUnitMenu = std::move(callback);
}

void Indicator::SetOnPrecisionChanged(std::function<void(int)> callback) {
    m_onPrecisionChanged = std::move(callback);
}

void Indicator::SetUnit(const std::string& unit) {
    if (m_unit != unit) {
        m_unit = unit;
        m_widthDirty = true;
    }
}

void Indicator::SetDigitsCount(int digits) {
    if (m_digitsCount != digits) {
        m_digitsCount = digits;
        m_widthDirty = true;
    }
}

void Indicator::SetOnTare(std::function<void()> callback) {
    m_onTare = std::move(callback);
}

float Indicator::CalculateWidth() const {
    return CalculateWidth(UiTheme::Get().indicator);
}

float Indicator::CalculateWidth(const IndicatorStyle& style) const {
    const UiTheme& theme = UiTheme::Get();
    if (!style.autoWidth) {
        return theme.Scale(style.width);
    }

    float currentScale = theme.GetScale();
    size_t valLen = m_value.length();

    if (!m_widthDirty &&
        std::abs(m_lastScale - currentScale) < 0.001f &&
        m_lastPrecision == m_precision &&
        m_lastDigitsCount == m_digitsCount &&
        m_lastValueLen == valLen &&
        m_lastShowTare == m_showTare &&
        m_lastTitle == m_title &&
        m_lastUnit == m_unit)
    {
        return m_cachedWidth;
    }

    float padX = theme.Scale(style.padX);

    // 1. Верхняя строка: ширина заголовка
    ImFont* titleFont = style.titleFont ? style.titleFont : theme.smallFont;
    float titleW = titleFont ? 
        titleFont->CalcTextSizeA(theme.Scale(style.titleFontSize), FLT_MAX, 0.0f, m_title.c_str()).x :
        ImGui::CalcTextSize(m_title.c_str()).x;

    // 2. Нижняя строка: резервируемая ширина числа по N символам
    ImFont* valueFont = style.valueFont ? style.valueFont : theme.defaultFont;
    float valFontSize = theme.Scale(style.valueFontSize);

    int digits = (m_digitsCount > 0) ? m_digitsCount : style.defaultDigits;
    digits = std::max(digits, (int)valLen);

    char sampleBuf[64];
    int bufPos = 0;
    if (m_precision > 0) {
        int intDigits = std::max(2, digits - 1 - m_precision);
        if (intDigits > 30) intDigits = 30;
        for (int i = 0; i < intDigits && bufPos < 60; ++i) sampleBuf[bufPos++] = '0';
        sampleBuf[bufPos++] = '.';
        int prec = std::min(m_precision, 10);
        for (int i = 0; i < prec && bufPos < 60; ++i) sampleBuf[bufPos++] = '0';
    } else {
        int intDigits = std::max(2, digits);
        if (intDigits > 30) intDigits = 30;
        for (int i = 0; i < intDigits && bufPos < 60; ++i) sampleBuf[bufPos++] = '0';
    }
    sampleBuf[bufPos] = '\0';

    float valueW = valueFont ? 
        valueFont->CalcTextSizeA(valFontSize, FLT_MAX, 0.0f, sampleBuf).x :
        ImGui::CalcTextSize(sampleBuf).x;

    // 3. Нижняя строка: ширина единицы измерения
    ImFont* unitFont = style.unitFont ? style.unitFont : theme.defaultFont;
    float unitW = unitFont ?
        unitFont->CalcTextSizeA(theme.Scale(style.unitFontSize), FLT_MAX, 0.0f, m_unit.c_str()).x :
        ImGui::CalcTextSize(m_unit.c_str()).x;

    // 4. Опциональная кнопка тарирования
    float tareW = m_showTare ? theme.Scale(style.tareBtnSize + 8.0f) : 0.0f;

    // Сравнение ширины верхней и нижней строк
    float row1 = titleW;
    float row2 = valueW + theme.Scale(6.0f) + unitW + tareW;
    float contentW = std::max(row1, row2);

    float totalW = contentW + padX * 2.0f;
    m_cachedWidth = std::max(totalW, theme.Scale(style.minWidth));
    m_lastScale = currentScale;
    m_lastPrecision = m_precision;
    m_lastDigitsCount = m_digitsCount;
    m_lastValueLen = valLen;
    m_lastShowTare = m_showTare;
    m_lastTitle = m_title;
    m_lastUnit = m_unit;
    m_widthDirty = false;
    return m_cachedWidth;
}

void Indicator::Render() {
    Render(UiTheme::Get().indicator);
}

// Кликабельный текст с выпадающим меню: невидимая кнопка по размеру текста,
// пунктир под текстом при наведении и при открытом меню, подсказка; меню —
// под текстом. Идентификаторы уникальны для экземпляра индикатора
void Indicator::RenderMenuZone(const char* name, const MenuZone& zone, const char* tooltip,
                               const MenuCallback& fillMenu) {
    const UiTheme& theme = UiTheme::Get();
    char popupId[64];
    char buttonId[64];
    std::snprintf(popupId, sizeof(popupId), "##%s_popup_%p", name, static_cast<void*>(this));
    std::snprintf(buttonId, sizeof(buttonId), "##%s_click_%p", name, static_cast<void*>(this));

    const bool popupOpen = ImGui::IsPopupOpen(popupId);
    ImGui::SetCursorScreenPos(zone.pos);
    if (ImGui::InvisibleButton(buttonId, zone.size)) {
        ImGui::OpenPopup(popupId);
    }
    const bool hovered = ImGui::IsItemHovered();
    if (hovered) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        ImGui::SetTooltip("%s", tooltip);
    }

    if (hovered || popupOpen) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const float underlineY = zone.pos.y + zone.fontSize * 0.94f;
        const float dotStep = theme.Scale(4.5f);
        const float dotLen = theme.Scale(2.5f);
        for (float x = zone.pos.x; x + dotLen <= zone.pos.x + zone.size.x; x += dotStep) {
            drawList->AddLine(ImVec2(x, underlineY), ImVec2(x + dotLen, underlineY), zone.color,
                              zone.underlineThickness);
        }
    }

    const ImVec2 menuPos(zone.pos.x, zone.pos.y + zone.fontSize + theme.Scale(4.0f));
    if (ContextMenu menu(popupId, menuPos); menu) {
        fillMenu(menu);
    }
}

void Indicator::Render(const IndicatorStyle& style) {
    const UiTheme& theme = UiTheme::Get();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float width = CalculateWidth(style);
    float height = theme.Scale(style.height);
    float radius = theme.Scale(style.cornerRadius);

    // 1. Фон карточки и контур
    drawList->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height), style.colBg, radius);
    drawList->AddRect(pos, ImVec2(pos.x + width, pos.y + height), style.colBorder, radius, 0, 1.0f);

    // Внутренние отступы
    float padX = theme.Scale(style.padX);
    float padY = theme.Scale(style.padY);

    // 2. Верхняя строка: Заголовок капсом (меню выбора канала)
    ImVec2 labelPos(pos.x + padX, pos.y + padY);
    ImFont* titleFont = style.titleFont ? style.titleFont : theme.smallFont;
    float titleFontSize = theme.Scale(style.titleFontSize);
    ImVec2 titleSize = titleFont ?
        titleFont->CalcTextSizeA(titleFontSize, FLT_MAX, 0.0f, m_title.c_str()) :
        ImGui::CalcTextSize(m_title.c_str());

    if (m_onRenderTitleMenu) {
        RenderMenuZone("title", {labelPos, titleSize, titleFontSize, style.colTitle, 1.0f},
                       "Click to select channel", m_onRenderTitleMenu);
    }
    drawList->AddText(titleFont, titleFontSize, labelPos, style.colTitle, m_title.c_str());

    // 3. Bottom Row: Value (precision menu) + Unit (units menu)
    ImVec2 valPos(pos.x + padX, pos.y + padY + theme.Scale(14.0f));
    ImFont* valueFont = style.valueFont ? style.valueFont : theme.defaultFont;
    float valFontSize = theme.Scale(style.valueFontSize);

    ImVec2 valSize = valueFont ?
        valueFont->CalcTextSizeA(valFontSize, FLT_MAX, 0.0f, m_value.c_str()) :
        ImGui::CalcTextSize(m_value.c_str());

    if (m_allowPrecisionChange) {
        RenderMenuZone("prec", {valPos, valSize, valFontSize, style.colValue, 1.5f},
                       "Click to select precision", [this](ContextMenu& menu) {
            menu.Header("PRECISION (DECIMALS)");
            for (int p = 0; p <= 5; ++p) {
                if (menu.Item(GetPrecisionLabel(p), m_precision == p) && p != m_precision) {
                    SetPrecision(p);
                    if (m_onPrecisionChanged) m_onPrecisionChanged(m_precision);
                }
            }
        });
    }
    drawList->AddText(valueFont, valFontSize, valPos, style.colValue, m_value.c_str());

    // Units right after value
    ImVec2 unitPos(valPos.x + valSize.x + theme.Scale(6.0f), valPos.y + theme.Scale(16.0f));
    ImFont* unitFont = style.unitFont ? style.unitFont : theme.defaultFont;
    float unitFontSize = theme.Scale(style.unitFontSize);
    if (m_onRenderUnitMenu && !m_unit.empty()) {
        ImVec2 unitSize = unitFont ?
            unitFont->CalcTextSizeA(unitFontSize, FLT_MAX, 0.0f, m_unit.c_str()) :
            ImGui::CalcTextSize(m_unit.c_str());
        RenderMenuZone("unit", {unitPos, unitSize, unitFontSize, style.colUnit, 1.0f},
                       "Click to select measurement units", m_onRenderUnitMenu);
    }
    drawList->AddText(unitFont, unitFontSize, unitPos, style.colUnit, m_unit.c_str());

    // 4. Optional interactive tare/zero button
    if (m_showTare) {
        float btnSize = theme.Scale(style.tareBtnSize);
        float btnPosY = pos.y + height - padY - btnSize - theme.Scale(3.0f);
        ImVec2 btnPos(pos.x + width - padX - btnSize, btnPosY);

        ImGui::SetCursorScreenPos(btnPos);
        char btnId[64];
        std::snprintf(btnId, sizeof(btnId), "##tare_%p", static_cast<void*>(this));
        if (ImGui::InvisibleButton(btnId, ImVec2(btnSize, btnSize))) {
            if (m_onTare) {
                m_onTare();
            }
        }
        bool hovered = ImGui::IsItemHovered();
        bool active  = ImGui::IsItemActive();

        if (hovered) {
            ImGui::SetTooltip("Tare / Zero Reset");
        }

        // 4.1. Фон квадратной кнопки
        ImU32 bgCol = active  ? style.colTareBtnActive :
                      hovered ? style.colTareBtnHover : style.colTareBtn;
        float btnRadius = theme.Scale(style.tareBtnRadius);
        drawList->AddRectFilled(btnPos, ImVec2(btnPos.x + btnSize, btnPos.y + btnSize), bgCol, btnRadius);

        // 4.2. Контур квадратной кнопки
        ImU32 borderCol = (hovered || active) ? style.colTareBorderHover : style.colTareBorder;
        drawList->AddRect(btnPos, ImVec2(btnPos.x + btnSize, btnPos.y + btnSize), borderCol, btnRadius, 0, 1.0f);

        // 4.3. Символ / иконка внутри кнопки
        ImU32 iconColor = (hovered || active) ? style.colTareIconHover : style.colTareIcon;
        ImVec2 center(btnPos.x + btnSize * 0.5f, btnPos.y + btnSize * 0.5f);

        const std::string& label = !m_tareLabel.empty() ? m_tareLabel : style.tareLabel;
        if (!label.empty()) {
            ImFont* btnFont = theme.buttonFont ? theme.buttonFont : theme.defaultFont;
            float fontSz = theme.Scale(13.0f);
            ImVec2 txtSz = btnFont->CalcTextSizeA(fontSz, FLT_MAX, 0.0f, label.c_str());
            ImVec2 txtPos(center.x - txtSz.x * 0.5f, center.y - txtSz.y * 0.5f);
            drawList->AddText(btnFont, fontSz, txtPos, iconColor, label.c_str());
        } else {
            float ringRadius = btnSize * 0.26f;
            float dotRadius  = theme.Scale(1.8f);
            drawList->AddCircle(center, ringRadius, iconColor, 0, 1.5f);
            drawList->AddCircleFilled(center, dotRadius, iconColor);
        }
    }

    // Резервируем место в потоке компоновщика ImGui
    ImGui::SetCursorScreenPos(pos);
    ImGui::Dummy(ImVec2(width, height));
}
