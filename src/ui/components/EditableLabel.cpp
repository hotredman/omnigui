#include "ui/components/EditableLabel.hpp"
#include <imgui_internal.h>
#include <cstring>
#include <cmath>
#include <algorithm>

EditableLabel::EditableLabel(const EditableLabelStyle* customStyle)
    : m_customStyle(customStyle)
{
}

void EditableLabel::StartEdit(const std::string& currentValue) {
    m_editing = true;
    m_originalValue = currentValue;
    strncpy(m_buffer, currentValue.c_str(), sizeof(m_buffer) - 1);
    m_buffer[sizeof(m_buffer) - 1] = '\0';
    m_focusRequested = true;
    m_justOpened = true;
}

void EditableLabel::CancelEdit() {
    m_editing = false;
    m_focusRequested = false;
    m_justOpened = false;
}

void EditableLabel::CommitEdit(std::string& targetValue) {
    m_editing = false;
    m_focusRequested = false;
    m_justOpened = false;
    targetValue = m_buffer;
}

bool EditableLabel::Render(const char* id, std::string& value, float width, ImFont* font,
                           const std::function<std::string()>& onDefaultValue, bool editable) {
    const UiTheme& theme = UiTheme::Get();
    ImFont* currentFont = font ? font : (theme.fontBold ? theme.fontBold : theme.defaultFont);
    float pt = (font == theme.projectTitleFont || font == theme.fontBold) ? 20.0f : 18.0f;
    theme.PushFont(currentFont, pt);

    if (width < 0.0f) {
        width = ImGui::GetContentRegionAvail().x;
    }

    float fontH = ImGui::GetFontSize();
    float targetBoxH = theme.Scale(Style().boxHeight);
    float framePadY = std::max(2.0f, (targetBoxH - fontH) * 0.5f);
    float actualHeight = targetBoxH; // Гарантированная высота для кнопки и поля ввода, равная targetBoxH
    float iconSz = theme.Scale(Style().iconSize);
    float spacing = theme.Scale(Style().spacing);
    float padX = theme.Scale(Style().paddingX);
    float frameRounding = theme.Scale(Style().frameRounding);

    // Вертикальное центрирование только в узком контейнере (например, тулбаре)
    float winH = ImGui::GetWindowHeight();
    float targetCursorY = (winH < 60.0f) ? std::max(0.0f, std::floor((winH - actualHeight) * 0.5f))
                                         : ImGui::GetCursorPosY();
    if (winH < 60.0f) {
        ImGui::SetCursorPosY(targetCursorY);
    }

    bool valueChanged = false;
    ImGui::PushID(id);

    if (!editable) {
        // --- 0. Неактивный режим: статичный текст без карандаша и клика ---
        const char* displayStr = value.empty() ? " " : value.c_str();
        ImVec2 textSize = ImGui::CalcTextSize(displayStr);
        float textY = ImGui::GetCursorPosY() + std::floor((actualHeight - textSize.y) * 0.5f);
        ImGui::SetCursorPosY(textY);
        ImGui::TextColored(ImColor(theme.palette.textPrimary).Value, "%s", displayStr);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + std::max(2.0f, (actualHeight - textSize.y) * 0.5f));
        ImGui::PopID();
        theme.PopFont();
        return false;
    }

    if (!m_editing) {
        // --- 1. Режим просмотра (Read-Only) ---
        const char* displayStr = value.empty() ? " " : value.c_str();
        ImVec2 textSize = ImGui::CalcTextSize(displayStr);

        float totalW = iconSz + spacing + textSize.x + padX * 2.0f;
        if (width > 0.0f) totalW = std::max(totalW, width);

        ImVec2 screenPos = ImGui::GetCursorScreenPos();

        // Интерактивная невидимая кнопка для обработки наведения и клика
        bool clicked = ImGui::InvisibleButton("##LabelHitbox", ImVec2(totalW, actualHeight));
        bool isHovered = ImGui::IsItemHovered();

        if (isHovered) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);
        }

        if (clicked) {
            StartEdit(value);
        }

        // Отрисовка
        ImDrawList* dl = ImGui::GetWindowDrawList();

        // 1. Иконка карандаша (центрирована строго по высоте actualHeight)
        ImU32 iconCol = isHovered ? Style().colIconHover : Style().colIcon;
        float iconY = screenPos.y + std::floor((actualHeight - iconSz) * 0.5f);
        Icon::DrawAt(Icon::Pencil, dl, ImVec2(screenPos.x + padX, iconY), iconSz, iconCol);

        // 2. Текст (центрирован строго по высоте actualHeight)
        float textX = screenPos.x + padX + iconSz + spacing;
        float textY = screenPos.y + std::floor((actualHeight - textSize.y) * 0.5f);
        dl->AddText(ImVec2(textX, textY), theme.palette.textPrimary, displayStr);

    } else {
        // --- 2. Режим редактирования (Editing) ---
        ImVec2 screenPos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();

        // 1. Кнопка карандаша слева: строго квадратная (actualHeight x actualHeight)
        ImVec2 btnMin = screenPos;
        ImVec2 btnMax = ImVec2(screenPos.x + actualHeight, screenPos.y + actualHeight);

        // Хит-тест кнопки (работает мгновенно по первому клику, даже когда InputText активен)
        bool btnHovered = ImGui::IsMouseHoveringRect(btnMin, btnMax);
        if (btnHovered) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            ImGui::SetTooltip(onDefaultValue ? "Reset to default name" : "Save");
        }

        bool btnClicked = btnHovered && ImGui::IsMouseClicked(0);

        // Фон кнопки и контур (с визуальным откликом при наведении)
        ImU32 borderCol = btnHovered ? Style().colInputBorder : Style().colBtnBorder;
        ImU32 iconColor = btnHovered ? Style().colIconHover : Style().colIconActive;
        dl->AddRectFilled(btnMin, btnMax, Style().colBtnBg, frameRounding);
        dl->AddRect(btnMin, btnMax, borderCol, frameRounding, 0, 1.0f);

        // Иконка карандаша строго в центре кнопки
        Icon::Draw(Icon::Pencil, dl, ImVec2((btnMin.x + btnMax.x) * 0.5f, (btnMin.y + btnMax.y) * 0.5f), iconSz, iconColor);

        // Невидимая кнопка для позиционирования курсора
        ImGui::InvisibleButton("##IconBtn", ImVec2(actualHeight, actualHeight));

        if (btnClicked) {
            if (onDefaultValue) {
                std::string def = onDefaultValue();
                strncpy(m_buffer, def.c_str(), sizeof(m_buffer) - 1);
                m_buffer[sizeof(m_buffer) - 1] = '\0';
                ImGui::ClearActiveID(); // Освобождаем внутреннее владение текстом InputText для мгновенного обновления
                m_focusRequested = true;
            } else {
                if (value != m_buffer) {
                    valueChanged = true;
                }
                CommitEdit(value);
            }
        }

        // Переход на ту же строку и фиксация той же вертикальной координаты
        ImGui::SameLine(0, spacing);
        ImGui::SetCursorPosY(targetCursorY);

        // 2. Поле ввода (InputText)
        float inputW = width > 0.0f ? (width - actualHeight - spacing) : 0.0f;
        if (inputW <= 0.0f) {
            // Динамический размер поля ввода
            ImVec2 bufSize = ImGui::CalcTextSize(m_buffer);
            inputW = std::max(theme.Scale(Style().minInputWidth), bufSize.x + theme.Scale(40.0f));
        }

        ImGui::SetNextItemWidth(inputW);

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, frameRounding);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(padX, framePadY));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, Style().colInputBg);
        ImGui::PushStyleColor(ImGuiCol_Border, Style().colInputBorder);
        ImGui::PushStyleColor(ImGuiCol_Text, theme.palette.textPrimary);
        ImGui::PushStyleColor(ImGuiCol_NavHighlight, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        if (m_focusRequested) {
            ImGui::SetKeyboardFocusHere();
            m_focusRequested = false;
        }

        ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;
        bool enterPressed = ImGui::InputText("##EditInput", m_buffer, sizeof(m_buffer), inputFlags);
        bool isActive = ImGui::IsItemActive();

        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar(3);

        if (enterPressed) {
            if (value != m_buffer) {
                valueChanged = true;
            }
            CommitEdit(value);
        } else if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            CancelEdit();
        } else if (!m_justOpened && !isActive && !btnHovered && !btnClicked && ImGui::IsMouseClicked(0)) {
            // Клик вне поля ввода и вне кнопки -> сохранение и выход
            if (value != m_buffer) {
                valueChanged = true;
            }
            CommitEdit(value);
        }

        m_justOpened = false;
    }

    ImGui::PopID();
    theme.PopFont();
    return valueChanged;
}
