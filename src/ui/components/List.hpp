#pragma once

#include "ui/components/Scope.hpp"
#include "ui/components/UiTheme.hpp"
#include "ui/components/Icon.hpp"
#include <imgui.h>
#include <string>
#include <functional>

// Параметры списка (designated initializers). Размеры — базовые px, 0 — всё свободное место:
//
//     if (List list({.key = "runs", .height = 240}); list) { ... }
struct ListOptions {
    const char*      key    = "##List";   // идентичность контейнера
    float            width  = 0.0f;
    float            height = 0.0f;
    const ListStyle* style  = nullptr;    // оверрайд стиля; nullptr — из темы
};

// Параметры заголовка секции. Заголовок передаётся позиционно:
//
//     if (list.Header("RUNS", {.actionIcon = "+", .badge = "3"})) { /* нажата кнопка действия */ }
struct ListHeaderOptions {
    const char* actionIcon = nullptr;   // кнопка действия справа, например "+"
    const char* badge      = nullptr;   // счётчик справа, например "3"
};

// Параметры элемента списка. Идентичность — key, иначе label:
//
//     if (list.Item({.label = "Run #01", .sublabel = "10:14", .rightText = "1240 pts", .selected = true})) { ... }
struct ListItemOptions {
    const char* label     = nullptr;
    const char* sublabel  = nullptr;   // вторая строка; при её наличии элемент выше
    const char* rightText = nullptr;   // подпись справа
    Icon        icon      = Icon::None;
    ImU32       iconColor = 0;         // цвет иконки; без иконки — точка-индикатор (LED)
    bool        selected  = false;
    float       height    = 0.0f;      // базовые px; 0 — по стилю
    const char* key       = nullptr;
};

// Универсальный компонент вертикального списка (List / ListView)
// Поддерживает заголовки секций, одиночный выбор, иконки, статус-индикаторы,
// подписи справа и пустые состояния. Полная инкапсуляция ImGui.
// RAII-область: конструктор открывает скроллируемый контейнер, деструктор закрывает.
class List : public Scope {
public:
    explicit List(const ListOptions& options = {});
    ~List();

    // Заголовок секции списка с опциональным счетчиком и кнопкой действия справа;
    // true — в этом кадре нажата кнопка действия
    bool Header(const char* title, const ListHeaderOptions& options = {});

    // Элемент списка (одно- или двухстрочный) с поддержкой выбора, иконки/LED и текста справа
    bool Item(const ListItemOptions& options);

    // Empty state display
    void Empty(const char* message = "List is empty", const char* detail = nullptr);

    // Тонкий горизонтальный разделитель
    void Separator();

private:
    const ListStyle* m_style = nullptr;
};
