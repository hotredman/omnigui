#include "ui/components/DeviceStatus.hpp"
#include "ui/components/ToolButton.hpp"
#include "ui/components/DisabledScope.hpp"
#include <cstring>
#include <imgui.h>

const char* DeviceStatus::GetDefaultStateText(DeviceState state) {
    switch (state) {
        case DeviceState::Disconnected: return "DISCONNECTED";
        case DeviceState::Connecting:   return "CONNECTING";
        case DeviceState::Idle:         return "READY";
        case DeviceState::Running:      return "ONLINE";
        case DeviceState::Paused:       return "PAUSED";
        case DeviceState::Fault:        return "FAULT";
        default:                        return "UNKNOWN";
    }
}

ImU32 DeviceStatus::GetStateLedColor(DeviceState state, const DeviceStatusStyle& style) {
    switch (state) {
        case DeviceState::Disconnected: return style.colLedDisconnected;
        case DeviceState::Connecting:   return style.colLedConnecting;
        case DeviceState::Idle:         return style.colLedIdle;
        case DeviceState::Running:      return style.colLedRunning;
        case DeviceState::Paused:       return style.colLedPaused;
        case DeviceState::Fault:        return style.colLedFault;
        default:                        return style.colLedDisconnected;
    }
}

float DeviceStatus::CalculateWidth(const std::vector<std::string>& titleCandidates,
                                   const std::string& currentTitle,
                                   const DeviceStatusStyle* customStyle)
{
    const UiTheme& theme = UiTheme::Get();
    const DeviceStatusStyle& style = customStyle ? *customStyle : theme.deviceStatus;

    ImFont* titleFont = style.titleFont ? style.titleFont : theme.defaultFont;
    float titleSize = theme.Scale(style.titleFontSize);

    ImFont* statusFont = style.statusFont ? style.statusFont : theme.defaultFont;
    float statusSize = theme.Scale(style.statusFontSize);

    // 1. Измеряем максимальную ширину имени устройства среди кандидатов и текущего значения
    float maxTitleW = 0.0f;
    auto measureTitle = [&](const std::string& str) {
        if (str.empty()) return;
        float w = titleFont 
            ? titleFont->CalcTextSizeA(titleSize, FLT_MAX, 0.0f, str.c_str()).x
            : ImGui::CalcTextSize(str.c_str()).x;
        maxTitleW = std::max(maxTitleW, w);
    };

    for (const auto& candidate : titleCandidates) {
        measureTitle(candidate);
    }
    measureTitle(currentTitle);

    // 2. Измеряем ширину всех возможных состояний устройства
    float maxStatusW = 0.0f;
    const DeviceState allStates[] = {
        DeviceState::Disconnected,
        DeviceState::Connecting,
        DeviceState::Idle,
        DeviceState::Running,
        DeviceState::Paused,
        DeviceState::Fault
    };

    for (DeviceState st : allStates) {
        const char* text = GetDefaultStateText(st);
        if (!text) continue;
        float w = statusFont
            ? statusFont->CalcTextSizeA(statusSize, FLT_MAX, 0.0f, text).x
            : ImGui::CalcTextSize(text).x;
        maxStatusW = std::max(maxStatusW, w);
    }

    // 3. Выбираем максимальную ширину текстовой зоны
    float maxTextW = std::max(maxTitleW, maxStatusW);

    // 4. Геометрия плашки:
    // - левый отступ до текста: 28px (включает светодиод 16px c радиусом 5px + зазор)
    // - зазор после текста до кнопки: 14px
    // - квадратная кнопка действия: style.actionBtnSize
    // - отступ справа от кнопки до границы плашки: 8px
    float leftPad  = theme.Scale(28.0f);
    float textGap  = theme.Scale(14.0f);
    float btnW     = theme.Scale(style.actionBtnSize);
    float rightPad = theme.Scale(8.0f);

    float totalW = leftPad + maxTextW + textGap + btnW + rightPad;
    return std::max(totalW, theme.Scale(style.width));
}

bool DeviceStatus::RenderAutoWidth(const char* deviceName,
                                   DeviceState state,
                                   const std::vector<std::string>& titleCandidates,
                                   const char* statusText,
                                   const char* buttonLabel,
                                   std::function<void()> onAction,
                                   const DeviceStatusStyle* customStyle)
{
    float w = CalculateWidth(titleCandidates, deviceName ? deviceName : "", customStyle);
    return Render(deviceName, state, statusText, buttonLabel, onAction, w, customStyle);
}

bool DeviceStatus::Render(const char* deviceName, 
                          DeviceState state, 
                          const char* statusText, 
                          const char* buttonLabel,
                          std::function<void()> onAction,
                          float width,
                          const DeviceStatusStyle* customStyle)
{
    const UiTheme& theme = UiTheme::Get();
    const DeviceStatusStyle& style = customStyle ? *customStyle : theme.deviceStatus;

    float w = (width > 0.0f) ? width : theme.Scale(style.width);
    float h = theme.Scale(style.height);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float radius = theme.Scale(style.cornerRadius);

    // 1. Фон и рамка плашки
    dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h), style.colBg, radius);
    if (style.borderSize > 0.0f) {
        dl->AddRect(pos, ImVec2(pos.x + w, pos.y + h), style.colBorder, radius, 0, style.borderSize);
    }

    // 2. Светодиодный индикатор (LED)
    ImVec2 ledCenter(pos.x + theme.Scale(16.0f), pos.y + h * 0.5f);
    float ledR = theme.Scale(style.ledRadius);
    ImU32 ledColor = GetStateLedColor(state, style);
    dl->AddCircleFilled(ledCenter, ledR, ledColor);

    // 3. Текстовая информация: Имя устройства + Статус
    ImFont* nameFont = style.titleFont ? style.titleFont : theme.defaultFont;
    float nameSize = theme.Scale(style.titleFontSize);
    ImVec2 textPos1(pos.x + theme.Scale(28.0f), pos.y + theme.Scale(7.0f));
    dl->AddText(nameFont, nameSize, textPos1, style.colTitle, deviceName ? deviceName : "");

    const char* displayStatus = (statusText && statusText[0] != '\0') 
                                ? statusText 
                                : GetDefaultStateText(state);
    ImFont* statusFont = style.statusFont ? style.statusFont : theme.defaultFont;
    float statusSize = theme.Scale(style.statusFontSize);
    ImVec2 textPos2(pos.x + theme.Scale(28.0f), pos.y + theme.Scale(26.0f));
    dl->AddText(statusFont, statusSize, textPos2, style.colStatusText, displayStatus);

    // 4. Кнопка действия (опционально)
    bool actionClicked = false;
    if (buttonLabel && buttonLabel[0] != '\0') {
        DisabledScope actionScope(!onAction);
        float btnSize = theme.Scale(style.actionBtnSize);
        ImVec2 btnPos(pos.x + w - btnSize - theme.Scale(8.0f), pos.y + (h - btnSize) * 0.5f);
        ImGui::SetCursorScreenPos(btnPos);
        bool clicked = false;
        if (std::strcmp(buttonLabel, "[P]") == 0) {
            bool isConnected = (state != DeviceState::Disconnected);
            const char* tooltip = "Connect to device";
            ImU32 iconColor = theme.palette.accent;
            if (state == DeviceState::Connecting) {
                tooltip = "Connecting...";
                iconColor = style.colLedConnecting;
            } else if (isConnected) {
                tooltip = "Disconnect from device";
                iconColor = style.colLedFault;
            }
            clicked = ToolButton({.icon = Icon::Power, .tooltip = tooltip, .iconColor = iconColor, .key = "power", .sidePx = btnSize});
        } else {
            clicked = ToolButton({.glyph = buttonLabel, .key = "action", .sidePx = btnSize});
        }
        if (clicked) {
            actionClicked = true;
            if (onAction) {
                onAction();
            }
        }
    }

    // 5. Продвижение позиции курсора в потоке ImGui
    ImGui::SetCursorScreenPos(pos);
    ImGui::Dummy(ImVec2(w, h));

    return actionClicked;
}
