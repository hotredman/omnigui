#pragma once

#include <imgui.h>
#include "ui/components/UiTheme.hpp"

#include <string>
#include <vector>

// ============================================================================
// Подсказки под текстовым полем: список значений под рамкой поля, пока оно
// активно. Выбор — мышью или стрелками и Enter; выбранное значение подставляется
// в поле целиком. Список строит вызывающий, поле только показывает его и не
// знает, откуда значения.
//
//     TextSuggest suggest{.entries = {{"Alpha", "12 runs"}, {"Alpha-2", "3 runs"}},
//                         .headline = "New project will be created"};
//     InputField(name, {.label = "Project", .suggest = &suggest});
// ============================================================================
struct TextSuggestEntry {
    std::string value;   // подставляется в поле
    std::string note;    // справа, приглушённо («12 runs · 25.09.2026»)
};

struct TextSuggest {
    std::vector<TextSuggestEntry> entries;   // лучшие первыми
    std::string headline;                    // строка над списком; пусто — нет

    bool Empty() const { return entries.empty() && headline.empty(); }
};

// Состояние поля ввода в кадре (снимается сразу после ImGui::InputText)
struct TextSuggestFieldState {
    bool active = false;        // поле активно (идёт ввод)
    bool activated = false;     // стало активным в этом кадре
    bool deactivated = false;   // перестало быть активным в этом кадре (Enter, щелчок мимо)
    bool edited = false;        // текст изменён в этом кадре
};

// Рисует список под полем, только что отрисованным (rectMin/rectMax — рамка поля; size —
// размер поля: от него зависят шрифты и высота строк). Вызывается из InputField;
// напрямую нужна только при собственной обёртке над ImGui::InputText.
// Возвращает true, если выбрано значение: value уже заменено. Вызывать сразу после поля,
// до отрисовки следующего виджета.
bool DrawTextSuggest(ImVec2 rectMin, ImVec2 rectMax, UiSize size, const TextSuggestFieldState& field,
                     const TextSuggest& suggest, std::string& value);
