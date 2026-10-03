#include "ui/components/TextSuggest.hpp"

#include <imgui_internal.h>

#include <algorithm>
#include <cstdio>

namespace {

// Ключи состояния поля в хранилище окна (ID поля + смещение — не пересекаются с чужими)
constexpr ImGuiID kKeepOpen = 0x51;
constexpr ImGuiID kHighlight = 0xA3;
constexpr ImGuiID kShown = 0x7C;

bool EnterPressed() {
    return ImGui::IsKeyPressed(ImGuiKey_Enter, false) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter, false);
}

}  // namespace

bool DrawTextSuggest(ImVec2 rectMin, ImVec2 rectMax, UiSize size, const TextSuggestFieldState& field,
                     const TextSuggest& suggest, std::string& value) {
    ImGuiStorage* storage = ImGui::GetStateStorage();
    const ImGuiID key = ImGui::GetID("##suggest");   // уникален внутри PushID поля
    int highlight = storage->GetInt(key ^ kHighlight, -1);
    const bool wasShown = storage->GetBool(key ^ kShown, false);
    const bool keepOpen = storage->GetBool(key ^ kKeepOpen, false);
    const int count = static_cast<int>(suggest.entries.size());

    bool picked = false;
    const auto pick = [&](int index) {
        value = suggest.entries[static_cast<size_t>(index)].value;
        picked = true;
        highlight = -1;
    };

    if (field.activated || field.edited)
        highlight = -1;

    // Enter обрабатывает само поле (оно теряет активность): выбираем подсвеченное
    if (field.deactivated && wasShown && EnterPressed() && highlight >= 0 && highlight < count)
        pick(highlight);

    const bool show = !suggest.Empty() && (field.active || keepOpen) && !picked;
    if (show && field.active && count > 0) {
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
            highlight = std::min(count - 1, highlight + 1);
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
            highlight = std::max(-1, highlight - 1);
    }

    bool hoveredPopup = false;
    if (show) {
        const UiTheme& theme = UiTheme::Get();
        const ComboStyle& combo = theme.combo;
        const InputFieldStyle& input = theme.input;
        const ControlMetrics m = theme.GetMetrics(size);

        const float rowHeight = theme.Scale(m.height);
        const float padX = theme.Scale(m.paddingX);
        const float padY = theme.Scale(input.suggestPaddingY);
        const float headHeight = theme.Scale(input.suggestHeadHeight);
        const float width = rectMax.x - rectMin.x;
        const float estimated = padY * 2.0f + count * rowHeight + (suggest.headline.empty() ? 0.0f : headHeight);
        const float captionSize = input.labelFontSize * theme.SizeFactor(size);
        ImFont* valueFont = m.font ? m.font : theme.fontMedium;

        ImVec2 pos(rectMin.x, rectMax.y + theme.Scale(input.suggestGap));
        ImVec2 pivot(0.0f, 0.0f);
        if (pos.y + estimated > ImGui::GetIO().DisplaySize.y && rectMin.y - estimated > 0.0f) {
            pos = ImVec2(rectMin.x, rectMin.y - theme.Scale(input.suggestGap));   // не помещается снизу — над полем
            pivot = ImVec2(0.0f, 1.0f);
        }
        ImGui::SetNextWindowPos(pos, ImGuiCond_Always, pivot);
        ImGui::SetNextWindowSize(ImVec2(width, 0.0f));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, padY));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, theme.Scale(combo.popupRounding));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, combo.popupBorderSize);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(combo.colPopupBg).Value);
        ImGui::PushStyleColor(ImGuiCol_Border, ImColor(combo.colPopupBorder).Value);

        char windowName[32];
        std::snprintf(windowName, sizeof(windowName), "##suggest_%08x", key);
        const ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                       ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                                       ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
                                       ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                                       ImGuiWindowFlags_NoCollapse;
        if (ImGui::Begin(windowName, nullptr, flags)) {
            ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());   // над полями под списком
            // Нажатая строка — активный элемент, а без этого флага ImGui считает окно «не наведённым»
            // и список закрылся бы до отпускания кнопки (клик не дошёл бы до выбора)
            hoveredPopup = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
            ImDrawList* dl = ImGui::GetWindowDrawList();

            if (!suggest.headline.empty()) {
                theme.PushFont(theme.fontRegular, captionSize);
                const ImVec2 p = ImGui::GetCursorScreenPos();
                const float textHeight = ImGui::GetTextLineHeight();
                dl->AddText(ImVec2(p.x + padX, p.y + (headHeight - textHeight) * 0.5f),
                            ImGui::GetColorU32(input.colLabel), suggest.headline.c_str());
                ImGui::Dummy(ImVec2(width, headHeight));
                theme.PopFont();
            }

            for (int i = 0; i < count; ++i) {
                const TextSuggestEntry& entry = suggest.entries[static_cast<size_t>(i)];
                ImGui::PushID(i);
                const ImVec2 p = ImGui::GetCursorScreenPos();
                if (ImGui::Selectable("##row", highlight == i, ImGuiSelectableFlags_None, ImVec2(width, rowHeight)))
                    pick(i);
                if (ImGui::IsItemHovered() && ImGui::GetIO().MouseDelta.x != 0.0f)
                    highlight = i;
                ImGui::PopID();

                float noteWidth = 0.0f;
                if (!entry.note.empty()) {
                    theme.PushFont(theme.fontRegular, captionSize);
                    const ImVec2 noteSize = ImGui::CalcTextSize(entry.note.c_str());
                    noteWidth = noteSize.x;
                    dl->AddText(ImVec2(p.x + width - padX - noteSize.x, p.y + (rowHeight - noteSize.y) * 0.5f),
                                ImGui::GetColorU32(input.colLabel), entry.note.c_str());
                    theme.PopFont();
                }
                theme.PushFont(valueFont, m.fontSize);
                const float textHeight = ImGui::GetTextLineHeight();
                dl->PushClipRect(p, ImVec2(p.x + width - padX - noteWidth - (noteWidth > 0.0f ? padX : 0.0f),
                                           p.y + rowHeight), true);
                dl->AddText(ImVec2(p.x + padX, p.y + (rowHeight - textHeight) * 0.5f),
                            ImGui::GetColorU32(input.colText), entry.value.c_str());
                dl->PopClipRect();
                theme.PopFont();
            }
        }
        ImGui::End();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(4);
    }

    // Пока мышь над списком (нажатие на строку уводит активность с поля), список не закрывается
    storage->SetBool(key ^ kKeepOpen, show && hoveredPopup && !picked);
    storage->SetBool(key ^ kShown, show);
    storage->SetInt(key ^ kHighlight, show ? highlight : -1);
    return picked;
}
