#include "ui/components/TabBar.hpp"
#include "ui/components/Carousel.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <cfloat>

bool TabBar(const char* const items[], int itemsCount, int& selectedIndex, const TabBarOptions& options)
{
    const UiTheme& theme = UiTheme::Get();
    const TabBarStyle& style = options.style ? *options.style : theme.tab;
    const UiVariant* variants = options.variants;
    bool changed = false;

    // Идентичность панели: ключ либо подпись первой вкладки
    std::string id = std::string("##tabs_") +
        (options.key ? options.key : (itemsCount > 0 ? items[0] : ""));

    auto tabOptions = [&](int i) {
        TabItemOptions o;
        o.variant = variants ? variants[i] : UiVariant::Default;
        o.sameLine = i > 0;
        o.style = options.style;
        return o;
    };

    // Headless-режим без активного окна ImGui (для минималистичных юнит-тестов)
    if (ImGui::GetCurrentWindowRead() == nullptr) {
        for (int i = 0; i < itemsCount; ++i) {
            bool isSelected = (selectedIndex == i);
            ImGui::PushID(i);
            if (TabItem(items[i], isSelected, tabOptions(i))) {
                selectedIndex = i;
                changed = true;
            }
            ImGui::PopID();
        }
        if (options.separator) {
            TabSeparator(options.style);
        }
        return changed;
    }

    // Полноценный адаптивный рендеринг через карусель со стрелками при переполнении
    float tabH = theme.Scale(style.height);
    CarouselStyle cStyle = theme.carousel;
    cStyle.itemSpacing = theme.Scale(style.itemSpacing);
    cStyle.scrollStep = theme.Scale(160.0f);
    cStyle.arrowWidth = theme.Scale(20.0f);
    cStyle.hideDisabledArrows = true;
    cStyle.paddingY = 2.0f; // запас от субпиксельного клиппинга скруглений

    ImGuiID storageId = ImGui::GetID(id.c_str());
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

    if (Carousel carousel(id.c_str(), ImVec2(0.0f, tabH), &cStyle); carousel) {
        for (int i = 0; i < itemsCount; ++i) {
            bool isSelected = (selectedIndex == i);

            ImGui::PushID(i);
            if (TabItem(items[i], isSelected, tabOptions(i))) {
                selectedIndex = i;
                changed = true;
                *pNeedsScroll = true;
            }
            ImGui::PopID();

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

    if (options.separator) {
        TabSeparator(options.style);
    }

    return changed;
}

bool TabBar(const std::vector<std::string>& items, int& selectedIndex, const TabBarOptions& options)
{
    std::vector<const char*> cItems;
    cItems.reserve(items.size());
    for (const auto& s : items) {
        cItems.push_back(s.c_str());
    }
    return TabBar(cItems.empty() ? nullptr : cItems.data(), static_cast<int>(cItems.size()),
                  selectedIndex, options);
}

bool TabItem(const char* label, bool isSelected, const TabItemOptions& options)
{
    const UiTheme& theme = UiTheme::Get();
    const TabBarStyle& style = options.style ? *options.style : theme.tab;
    const UiVariant variant = options.variant;

    if (options.sameLine) {
        // Настраиваемый зазор между вкладками по горизонтали
        ImGui::SameLine(0.0f, theme.Scale(style.itemSpacing));
    }

    // Расчет габаритов
    float tabH = (options.sizePx.y > 0.0f) ? options.sizePx.y : theme.Scale(style.height);
    float tabW = options.sizePx.x;

    ImFont* font = style.font ? style.font : theme.defaultFont;
    float fontSize = theme.Scale(style.fontSize);

    if (tabW <= 0.0f) {
        ImVec2 textSize = font ? font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, label) : ImGui::CalcTextSize(label);
        tabW = textSize.x + theme.Scale(style.horizontalPadding * 2.0f);
    }

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 size(tabW, tabH);

    // Невидимая кнопка для обработки ввода
    ImGui::PushID(options.key ? options.key : label);
    bool pressed = ImGui::InvisibleButton("##tab", size);
    ImGui::PopID();
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

void TabSeparator(const TabBarStyle* customStyle) {
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
