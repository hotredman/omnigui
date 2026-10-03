#pragma once

#include "ui/components/Scope.hpp"
#include "ui/components/UiTheme.hpp"
#include "ui/components/Icon.hpp"
#include <imgui.h>
#include <optional>
#include <string>
#include <vector>

// Параметры строки-карточки
struct ItemRowOptions {
    const char* key     = "item";   // идентичность строки (ID и кэш высоты)
    int         actions = 1;        // сколько кнопок действий справа (резервирует слот)
};

// Параметры заголовка строки (название передаётся позиционно)
struct ItemRowTitleOptions {
    const char* symbol = nullptr;   // символ в круглых скобках
    const char* unit   = nullptr;   // единицы в квадратных скобках
};

// Параметры бейджа на линии заголовка (текст передаётся позиционно)
struct ItemRowBadgeOptions {
    UiVariant variant = UiVariant::Default;
};

// Параметры кнопки действия справа
struct ItemRowActionOptions {
    Icon        icon     = Icon::None;
    const char* tooltip  = nullptr;
    UiVariant   variant  = UiVariant::Secondary;
    bool        disabled = false;
};

// Составная интерактивная строка-карточка в стиле evo-machine-cs.
// Рендерит карточку списка с индивидуальной рамкой, скруглением, фоном, hover-подсветкой,
// заголовком с метаданными (символ, единицы), бейджами, кнопками действий справа,
// описанием с автопереносом текста, формулами и интерактивными чипами-тегами.
// RAII-область: конструктор открывает карточку, деструктор рисует подложку и закрывает.
//
//     if (ItemRow row({.key = "density", .actions = 2}); row) {
//         row.Title("Density", {.symbol = "rho", .unit = "kg/m3"});
//         row.Badge("In method", {.variant = UiVariant::Success});
//         if (row.Action({.icon = Icon::Edit, .tooltip = "Edit"})) { ... }
//         row.Description("Mass per unit volume");
//     }
class ItemRow : public Scope {
public:
    explicit ItemRow(const ItemRowOptions& options = {});
    ~ItemRow();

    // Заголовок строки: название, символ в круглых скобках, единицы в квадратных скобках
    void Title(const std::string& name, const ItemRowTitleOptions& options = {});

    // Семантический бейдж на линии заголовка (автоматически размещается на той же строке)
    void Badge(const char* text, const ItemRowBadgeOptions& options = {});

    // Кнопка действия (ToolButton) справа; слоты резервируются через ItemRowOptions::actions
    bool Action(const ItemRowActionOptions& options);

    // Описание элемента (серый текст с автопереносом по ширине карточки с учетом правых кнопок)
    void Description(const std::string& text);

    // Формульное выражение ("символ = выражение")
    void Formula(const std::string& symbol, const std::string& expression);

    // Теги в стиле evo-machine-cs (# прочность # растяжение ...);
    // возвращает текст тега, по которому кликнули в этом кадре
    std::optional<std::string> Tags(const std::vector<std::string>& tags);

private:
    void DrawBadges();
    void Finish();

    struct BadgeItem {
        std::string text;
        UiVariant variant;
    };

    std::string m_id;
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
