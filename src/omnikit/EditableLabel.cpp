#include "omnikit/EditableLabel.hpp"
#include <imgui_internal.h>
#include <cstring>
#include <cmath>
#include <algorithm>

namespace OmniKit {

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

bool EditableLabel::Render(std::string& value, const EditableLabelOptions& options) {
    float width = options.widthPx;
    const std::function<std::string()>& onDefaultValue = options.defaultValue;
    const bool editable = options.editable;

    const UiTheme& theme = UiTheme::Get();
    const bool title = (options.role == TextRole::Title);
    ImFont* currentFont = title ? (theme.fontBold ? theme.fontBold : theme.defaultFont)
                                : (theme.fontRegular ? theme.fontRegular : theme.defaultFont);
    float pt = title ? 20.0f : 18.0f;
    theme.PushFont(currentFont, pt);

    if (width < 0.0f) {
        width = ImGui::GetContentRegionAvail().x;
    }

    float fontH = ImGui::GetFontSize();
    float targetBoxH = theme.Scale(Style().boxHeight);
    float framePadY = std::max(2.0f, (targetBoxH - fontH) * 0.5f);
    float actualHeight = targetBoxH; // Guaranteed height for button and input field equal to targetBoxH
    float iconSz = theme.Scale(Style().iconSize);
    float spacing = theme.Scale(Style().spacing);
    float padX = theme.Scale(Style().paddingX);
    float frameRounding = theme.Scale(Style().frameRounding);

    // Vertical centering only in narrow containers (e.g., toolbar)
    float winH = ImGui::GetWindowHeight();
    float targetCursorY = (winH < 60.0f) ? std::max(0.0f, std::floor((winH - actualHeight) * 0.5f))
                                         : ImGui::GetCursorPosY();
    if (winH < 60.0f) {
        ImGui::SetCursorPosY(targetCursorY);
    }

    bool valueChanged = false;
    ImGui::PushID(this);

    if (!editable) {
        // --- 0. Non-interactive mode: static text without pencil or click ---
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
        // --- 1. Read-Only mode ---
        const char* displayStr = value.empty() ? " " : value.c_str();
        ImVec2 textSize = ImGui::CalcTextSize(displayStr);

        float totalW = iconSz + spacing + textSize.x + padX * 2.0f;
        if (width > 0.0f) totalW = std::max(totalW, width);

        ImVec2 screenPos = ImGui::GetCursorScreenPos();

        // Interactive invisible button for handling hover and click
        bool clicked = ImGui::InvisibleButton("##LabelHitbox", ImVec2(totalW, actualHeight));
        bool isHovered = ImGui::IsItemHovered();

        if (isHovered) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);
        }

        if (clicked) {
            StartEdit(value);
        }

        // Rendering
        ImDrawList* dl = ImGui::GetWindowDrawList();

        // 1. Pencil icon (strictly centered across actualHeight)
        ImU32 iconCol = isHovered ? Style().colIconHover : Style().colIcon;
        float iconY = screenPos.y + std::floor((actualHeight - iconSz) * 0.5f);
        Icon::DrawAt(Icon::Pencil, dl, ImVec2(screenPos.x + padX, iconY), iconSz, iconCol);

        // 2. Text (strictly centered across actualHeight)
        float textX = screenPos.x + padX + iconSz + spacing;
        float textY = screenPos.y + std::floor((actualHeight - textSize.y) * 0.5f);
        dl->AddText(ImVec2(textX, textY), theme.palette.textPrimary, displayStr);

    } else {
        // --- 2. Editing mode ---
        ImVec2 screenPos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();

        // 1. Pencil button on the left: strictly square (actualHeight x actualHeight)
        ImVec2 btnMin = screenPos;
        ImVec2 btnMax = ImVec2(screenPos.x + actualHeight, screenPos.y + actualHeight);

        // Button hit test (triggers immediately on first click even while InputText is active)
        bool btnHovered = ImGui::IsMouseHoveringRect(btnMin, btnMax);
        if (btnHovered) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            ImGui::SetTooltip(onDefaultValue ? "Reset to default name" : "Save");
        }

        bool btnClicked = btnHovered && ImGui::IsMouseClicked(0);

        // Button background and border (with hover feedback)
        ImU32 borderCol = btnHovered ? Style().colInputBorder : Style().colBtnBorder;
        ImU32 iconColor = btnHovered ? Style().colIconHover : Style().colIconActive;
        dl->AddRectFilled(btnMin, btnMax, Style().colBtnBg, frameRounding);
        dl->AddRect(btnMin, btnMax, borderCol, frameRounding, 0, 1.0f);

        // Pencil icon centered strictly within the button
        Icon::Draw(Icon::Pencil, dl, ImVec2((btnMin.x + btnMax.x) * 0.5f, (btnMin.y + btnMax.y) * 0.5f), iconSz, iconColor);

        // Invisible button for cursor layout advancement
        ImGui::InvisibleButton("##IconBtn", ImVec2(actualHeight, actualHeight));

        if (btnClicked) {
            if (onDefaultValue) {
                std::string def = onDefaultValue();
                strncpy(m_buffer, def.c_str(), sizeof(m_buffer) - 1);
                m_buffer[sizeof(m_buffer) - 1] = '\0';
                ImGui::ClearActiveID(); // Release internal InputText ownership for immediate text update
                m_focusRequested = true;
            } else {
                if (value != m_buffer) {
                    valueChanged = true;
                }
                CommitEdit(value);
            }
        }

        // Advance along the same line and preserve identical vertical coordinate
        ImGui::SameLine(0, spacing);
        ImGui::SetCursorPosY(targetCursorY);

        // 2. Input field (InputText)
        float inputW = width > 0.0f ? (width - actualHeight - spacing) : 0.0f;
        if (inputW <= 0.0f) {
            // Dynamic input field sizing
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
            // Click outside input field and button -> commit and exit
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

} // namespace OmniKit
