#pragma once

#include "ui/components/Scope.hpp"
#include "ui/components/UiTheme.hpp"
#include "ui/components/UiKey.hpp"
#include <imgui.h>
#include <string>
#include <source_location>

// Строка состояния внизу окна (StatusBar): полоса во всю ширину под
// рабочей областью. Содержимое — элементы в одну строку слева направо:
// тексты и вертикальные разделители. Последний текст может занять остаток
// ширины (maxWidth = 0): длинная строка обрезается многоточием, полный
// текст — в подсказке при наведении.
//
// Геометрия — из UiTheme (StatusBarHeight): Sidebar и ContentArea по
// умолчанию заканчиваются над полосой.
//
// RAII-область: конструктор открывает окно, деструктор закрывает.
//
//     if (auto status = StatusBar()) { status.Text("Ready"); }
struct StatusBarOptions {
    const StatusBarStyle* style = nullptr;   // оверрайд стиля; nullptr — из темы
};

// Параметры текста строки состояния:
//
//     status.Text("Uplink: active", {.variant = UiVariant::Success});
struct StatusBarTextOptions {
    UiVariant variant  = UiVariant::Default;   // цвет текста (Default — стандартный цвет статуса)
    float     maxWidth = 0.0f;                 // базовые px; 0 — по ширине текста
    UiKey     key      = {};
};

class StatusBar : public Scope {
public:
    explicit StatusBar(const StatusBarOptions& options = {});
    ~StatusBar();

    // Текст в одну строку. variant задаёт цвет (Default — вторичный текст
    // полосы); maxWidth = 0 — до правого края полосы
    void Text(const std::string& text, const StatusBarTextOptions& options = {},
              std::source_location loc = std::source_location::current());

    // Вертикальный разделитель между элементами
    void Separator();

    float GetHeight() const { return m_height; }

private:
    // Начало следующего элемента в строке (после предыдущего + зазор)
    void NextItem();

    // Явно переданный стиль; nullptr — стиль текущей темы (смена темы видна сразу)
    const StatusBarStyle* m_customStyle = nullptr;
    const StatusBarStyle& Style() const { return m_customStyle ? *m_customStyle : UiTheme::Get().statusBar; }
    bool m_hasItems = false;
    int m_textCount = 0;   // порядковый номер текста: уникальный ID для каждого Text()
    float m_height = 0.0f;
};
