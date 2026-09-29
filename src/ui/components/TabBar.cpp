#include "ui/components/TabBar.hpp"
#include "ui/components/Carousel.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <cfloat>

bool TabBar::Render(const char* id,
                    const char* const items[],
                    int itemsCount,
                    int& selectedIndex,
                    bool showSeparator,
                    float itemWidth,
                    float itemHeight,
                    const TabBarStyle* customStyle,
                    const UiVariant* variants)
{
    const UiTheme& theme = UiTheme::Get();
    const TabBarStyle& style = customStyle ? *customStyle : theme.tab;
    bool changed = false;

    // Headless-режим без активного окна ImGui (для минималистичных юнит-тестов)
    if (ImGui::GetCurrentWindowRead() == nullptr) {
        for (int i = 0; i < itemsCount; ++i) {
            bool isSelected = (selectedIndex == i);
            char btnId[64];
            std::snprintf(btnId, sizeof(btnId), "%s_tab_%d", id, i);
            if (TabItem(btnId, items[i], isSelected, itemWidth, itemHeight, i > 0, customStyle,
                        variants ? variants[i] : UiVariant::Default)) {
                selectedIndex = i;
                changed = true;
            }
        }
        if (showSeparator) {
            AddSeparator(customStyle);
        }
        return changed;
    }

    // Полноценный адаптивный рендеринг через карусель со стрелками при переполнении
    float tabH = (itemHeight > 0.0f) ? itemHeight : theme.Scale(style.height);
    CarouselStyle cStyle = theme.carousel;
    cStyle.itemSpacing = theme.Scale(style.itemSpacing);
    cStyle.scrollStep = theme.Scale(160.0f);
    cStyle.arrowWidth = theme.Scale(20.0f);
    cStyle.hideDisabledArrows = true;
    cStyle.paddingY = 2.0f; // запас от субпиксельного клиппинга скруглений

    ImGuiID storageId = ImGui::GetID(id);
    ImGuiStorage* storage = ImGui::GetStateStorage();
    int* pPrevSelected = storage->GetIntRef(storageId + 10, -1);
    bool* pNeedsScroll = storage->GetBoolRef(storageId + 11, true);

    if (*pPrevSelected != selectedIndex) {
        *pPrevSelected = selectedIndex;
        *pNeedsScroll = true;
    }

    float selectedMinX = 0.0f;
    float selectedMaxX = 0.0f;
    bool hasSelected = false;

    if (Carousel carousel(id, ImVec2(0.0f, tabH), &cStyle); carousel) {
        for (int i = 0; i < itemsCount; ++i) {
            bool isSelected = (selectedIndex == i);
            char btnId[64];
            std::snprintf(btnId, sizeof(btnId), "%s_tab_%d", id, i);

            if (TabItem(btnId, items[i], isSelected, itemWidth, itemHeight, i > 0, customStyle,
                        variants ? variants[i] : UiVariant::Default)) {
                selectedIndex = i;
                changed = true;
                *pNeedsScroll = true;
            }

            if (selectedIndex == i) {
                selectedMinX = ImGui::GetItemRectMin().x - ImGui::GetWindowPos().x + ImGui::GetScrollX();
                selectedMaxX = ImGui::GetItemRectMax().x - ImGui::GetWindowPos().x + ImGui::GetScrollX();
                hasSelected = true;
            }
        }

        if (hasSelected && *pNeedsScroll) {
            carousel.EnsureVisible(selectedMinX, selectedMaxX);
            if (carousel.CanScroll()) {
                *pNeedsScroll = false;
            }
        }
    }

    if (showSeparator) {
        AddSeparator(customStyle);
    }

    return changed;
}

bool TabBar::Render(const char* id,
                    const std::vector<std::string>& items,
                    int& selectedIndex,
                    bool showSeparator,
                    float itemWidth,
                    float itemHeight,
                    const TabBarStyle* customStyle)
{
    std::vector<const char*> cItems;
    cItems.reserve(items.size());
    for (const auto& s : items) {
        cItems.push_back(s.c_str());
    }
    return Render(id, cItems.empty() ? nullptr : cItems.data(), static_cast<int>(cItems.size()),
                  selectedIndex, showSeparator, itemWidth, itemHeight, customStyle);
}

bool TabBar::RenderEx(const char* id,
                      const char* const items[],
                      int itemsCount,
                      int& selectedIndex,
                      const TabBarStyle& customStyle,
                      bool showSeparator,
                      float itemWidth,
                      float itemHeight)
{
    return Render(id, items, itemsCount, selectedIndex, showSeparator, itemWidth, itemHeight, &customStyle);
}

bool TabBar::TabItem(const char* id,
                     const char* label,
                     bool isSelected,
                     float width,
                     float height,
                     bool sameLine,
                     const TabBarStyle* customStyle,
                     UiVariant variant)
{
    const UiTheme& theme = UiTheme::Get();
    const TabBarStyle& style = customStyle ? *customStyle : theme.tab;

    if (sameLine) {
        // Настраиваемый зазор между вкладками по горизонтали
        ImGui::SameLine(0.0f, theme.Scale(style.itemSpacing));
    }

    // Расчет габаритов
    float tabH = (height > 0.0f) ? height : theme.Scale(style.height);
    float tabW = width;

    ImFont* font = style.font ? style.font : theme.defaultFont;
    float fontSize = theme.Scale(style.fontSize);

    if (tabW <= 0.0f) {
        ImVec2 textSize = font ? font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, label) : ImGui::CalcTextSize(label);
        tabW = textSize.x + theme.Scale(style.horizontalPadding * 2.0f);
    }

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 size(tabW, tabH);

    // Невидимая кнопка для обработки ввода
    bool pressed = ImGui::InvisibleButton(id, size);
    bool hovered = ImGui::IsItemHovered();

    // Отрисовка
    ImDrawList* dl = ImGui::GetWindowDrawList();
    float radius = theme.Scale(style.cornerRadius);

    ImU32 bgCol = isSelected ? style.colActiveBg : (hovered ? style.colHoverBg : IM_COL32(0, 0, 0, 0));
    if ((bgCol & IM_COL32_A_MASK) != 0) {
        dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), bgCol, radius);
    }

    ImU32 textCol = isSelected ? style.colActiveText : (hovered ? style.colHoverText : style.colInactiveText);
    const bool marked = variant != UiVariant::Default;
    if (marked)
        textCol = theme.GetVariantStyle(variant).colText;
    ImVec2 textSize = font ? font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, label) : ImGui::CalcTextSize(label);
    ImVec2 textPos(
        pos.x + (size.x - textSize.x) * 0.5f,
        pos.y + (size.y - textSize.y) * 0.5f
    );

    if (font) {
        dl->AddText(font, fontSize, textPos, textCol, label);
    } else {
        dl->AddText(textPos, textCol, label);
    }

    if (marked) {
        // Подчёркивание цвета варианта: вкладка выделена и на активной, и на неактивной
        const float thick = theme.Scale(3.0f);
        dl->AddRectFilled(ImVec2(pos.x + radius, pos.y + size.y - thick),
                          ImVec2(pos.x + size.x - radius, pos.y + size.y), textCol, thick * 0.5f);
    }

    return pressed;
}

void TabBar::AddSeparator(const TabBarStyle* customStyle) {
    const UiTheme& theme = UiTheme::Get();
    const TabBarStyle& style = customStyle ? *customStyle : theme.tab;

    float gapTop = theme.Scale(style.separatorGapTop);
    float gapBottom = theme.Scale(style.separatorGapBottom);

    ImVec2 cursor = ImGui::GetCursorScreenPos();
    float availW = ImGui::GetContentRegionAvail().x;
    ImDrawList* dl = ImGui::GetWindowDrawList();

    float lineY = cursor.y + gapTop;
    dl->AddLine(
        ImVec2(cursor.x, lineY),
        ImVec2(cursor.x + availW, lineY),
        style.colSeparator,
        1.0f
    );

    ImGui::SetCursorScreenPos(ImVec2(cursor.x, lineY + 1.0f + gapBottom));
    ImGui::Dummy(ImVec2(0.0f, 0.0f));
}
