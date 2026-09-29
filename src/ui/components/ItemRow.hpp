#pragma once

#include "ui/components/UiTheme.hpp"
#include "ui/components/Icon.hpp"
#include <imgui.h>
#include <string>
#include <vector>

// Составная интерактивная строка-карточка в стиле evo-machine-cs.
// Рендерит карточку списка с индивидуальной рамкой, скруглением, фоном, hover-подсветкой,
// заголовком с метаданными (символ, единицы), бейджами, кнопками действий справа,
// описанием с автопереносом текста, формулами и интерактивными чипами-тегами.
class ItemRow {
public:
    explicit ItemRow(const char* id);
    explicit ItemRow(const std::string& id);
    ~ItemRow();

    ItemRow(const ItemRow&) = delete;
    ItemRow& operator=(const ItemRow&) = delete;

    // Заголовок строки: название, символ в круглых скобках, единицы в квадратных скобках
    void Title(const std::string& name, const std::string& symbol = "", const std::string& unitDisplay = "");

    // Семантический бейдж на линии заголовка (автоматически размещается на той же строке)
    void Badge(const char* text, UiVariant variant = UiVariant::Default);

    // Подготовка слота действий справа (totalActions: количество кнопок, 1..N)
    void RightActions(int totalActions = 1);

    // Кнопка действия (ToolButton) справа.
    // Если RightActions не вызывался явно, автоматически рассчитывает слот под totalActions.
    bool Action(Icon icon, const char* tooltip = nullptr, UiVariant variant = UiVariant::Secondary,
                bool disabled = false, int totalActions = 1);

    // Описание элемента (серый текст с автопереносом по ширине карточки с учетом правых кнопок)
    void Description(const std::string& text);

    // Формульное выражение ("символ = выражение")
    void Formula(const std::string& symbol, const std::string& expression);

    // Теги в стиле evo-machine-cs (# прочность # растяжение ...)
    // Если пользователь кликнул по тегу, записывает его в outClickedTag и возвращает true
    bool Tags(const std::vector<std::string>& tags, std::string* outClickedTag = nullptr);

    // Текстовая метка статуса (например: «В методике»)
    void Status(const char* text, UiVariant variant = UiVariant::Success);

    // Завершение строки (вызывается автоматически в деструкторе)
    void End();

private:
    void DrawBadges();

    struct BadgeItem {
        std::string text;
        UiVariant variant;
    };

    std::string m_id;
    bool m_open = false;
    int m_actionCount = 0;
    int m_totalActionsExpected = 1;
    ImVec2 m_startPos = ImVec2(0.0f, 0.0f);
    float m_availW = 0.0f;
    float m_padX = 0.0f;
    float m_padY = 0.0f;
    float m_prevHeight = 0.0f;

    float m_titleRowTopY = 0.0f;
    float m_titleRowHeight = 0.0f;
    float m_titleEndX = 0.0f;
    std::vector<BadgeItem> m_badges;

    ImDrawListSplitter m_splitter;
};
