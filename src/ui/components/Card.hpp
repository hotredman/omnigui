#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <algorithm>
#include <optional>
#include <cstddef>
#include <functional>
#include <type_traits>

#include "ui/components/Scope.hpp"
#include "ui/components/ColumnLayout.hpp"
#include "ui/components/UiTheme.hpp"
#include "ui/components/Icon.hpp"
#include "ui/components/PresetGrid.hpp"
#include "ui/components/Toggle.hpp"

// Состояние поля в карточке: обычное, изменено (в машину не записано), нет значения
// (не считано — вместо ввода прочерк)
enum class FieldState { Normal, Modified, NoValue };

// Параметры самой карточки (designated initializers):
//
//     if (Card card({.title = "Stream Actions"})) { ... }
//     if (Card card({.title = "Log", .stretchY = true})) { ... }
//
// Идентичность карточки (ImGui ID) берётся из key, иначе из title. Две карточки без
// title и без key в одном окне конфликтуют — задайте key.
struct CardOptions {
    const char* title    = nullptr;
    const char* key      = nullptr;     // идентичность; по умолчанию — title
    float       width    = 0.0f;        // базовые px (масштабируются); 0 — ширина колонки/ячейки раскладки
    float       height   = 0.0f;        // базовые px (масштабируются); 0 — по содержимому
    bool        stretchY = false;       // растянуть по вертикали на всю оставшуюся высоту
    const CardStyle* style = nullptr;   // оверрайд стиля; nullptr — из темы
};

// Параметры любого элемента карточки (поля, кнопки, табло). Применяются только те поля,
// которые имеют смысл для конкретного элемента; остальные игнорируются:
//
//     card.Float(rate,  {.label = "Rate", .unit = "Hz", .col = Col::Half()});
//     card.Toggle(on,   {.label = "Enable", .sublabel = "Verifies liveness"});
//     card.Button({.label = "Save", .variant = UiVariant::Primary, .icon = Icon::Check});
struct FieldOptions {
    const char* label    = nullptr;     // подпись (над полем / на кнопке)
    const char* sublabel = nullptr;     // Toggle: вторая строка
    const char* unit     = nullptr;     // единица измерения
    const char* format   = nullptr;     // printf-формат числа; nullptr — "%.2f"
    const char* hint     = nullptr;     // Text: плейсхолдер; nullptr — " — "
    UiVariant   variant  = UiVariant::Default;  // Button / Value: семантический стиль
    Icon        icon     = Icon::None;  // Button
    const char* tooltip  = nullptr;     // Button
    std::optional< ::Col> col;         // доля строки; по умолчанию Half (табло и пресеты — Full)
    std::optional<UiSize>  size;        // высота; по умолчанию — высота текущей Row()
    bool        disabled = false;
    const char* key      = nullptr;     // идентичность; по умолчанию — label
    std::optional<bool> alignBottom;    // Toggle: выровнять по нижней кромке соседних полей
    int         columns  = 5;           // PresetGrid: число колонок сетки пресетов
};

class Card : public Scope {
public:
    // RAII-строка сетки карточки с общей высотой элементов:  if (auto row = card.Row()) { ... }
    class RowScope : public Scope {
    public:
        RowScope(Card* parent, std::optional<UiSize> size);
        ~RowScope();
    private:
        Card* m_parent = nullptr;
    };

    // RAII-ячейка сетки для произвольных виджетов:  if (auto c = card.Col(Col::Third())) { ... }
    class ColumnScope : public Scope {
    public:
        ColumnScope(Card* parent, ::Col col);
        ~ColumnScope();
        float Width() const { return m_width; }
    private:
        Card* m_parent = nullptr;
        ::Col m_col = ::Col::Half();
        float m_width = 0.0f;
    };

    // RAII-область скроллируемого списка контента (занимает оставшуюся высоту карточки)
    class ContentListScope : public Scope {
    public:
        ContentListScope(Card* parent, const char* key);
        ~ContentListScope();
    private:
        Card* m_parent = nullptr;
    };

    // Открывает стилизованную карточку (стиль — из UiTheme::Get().card)
    Card(const CardOptions& options = {});
    explicit Card(const char* title) : Card(CardOptions{.title = title}) {}
    ~Card();

    // 2D-сетка: управление строками и колонками
    RowScope Row(std::optional<UiSize> size = UiSize::Medium);
    ColumnScope Col(::Col col = ::Col::Half());

    // Принудительный переход на следующую строку сетки
    void NextRow();

    // Кнопки с семантическими стилями; disabled — кнопка видна, но неактивна.
    // Col::Auto() даёт ширину по содержимому (не привязана к 12-колоночной сетке)
    bool Button(const FieldOptions& options);

    // Кнопка удержания: true, пока её держат нажатой (движение «пока держат»). Неактивная — всегда false
    bool HoldButton(const FieldOptions& options);

    // Информационное/расчётное табло (ValueDisplay); label — подпись над табло.
    // Без размера высота — 80 базовых px; std::nullopt вместо числа — прочерк
    void Value(double value, const FieldOptions& options = {});
    void Value(const char* text, const FieldOptions& options = {});
    void Value(std::optional<double> value, const FieldOptions& options = {});

    // Селектор пресетов со встроенным табло (PresetGrid); unit — единица на табло,
    // columns — число колонок сетки пресетов, format — формат значения на табло
    bool PresetGrid(float& value, const std::vector<float>& presets, const FieldOptions& options = {});
    bool PresetGrid(double& value, const std::vector<double>& presets, const FieldOptions& options = {});

    // Состояние полей карточки по адресу поля (float/int/bool/enum): без провайдера
    // все поля обычные. Провайдер живёт не дольше карточки
    using FieldStateProvider = std::function<FieldState(const void* field, std::size_t size)>;
    void SetFieldStates(FieldStateProvider provider) { m_fieldStates = std::move(provider); }

    // Поля ввода в 12-колоночной сетке
    bool Float(float& value, const FieldOptions& options);
    bool Float(std::optional<float>& value, const FieldOptions& options);
    bool Double(double& value, const FieldOptions& options);
    bool Double(std::optional<double>& value, const FieldOptions& options);
    bool Int(int& value, const FieldOptions& options);
    bool Text(std::string& value, const FieldOptions& options);
    bool Toggle(bool& value, const FieldOptions& options);

    // Поле только для чтения: подпись над значением
    void Display(const std::string& value, const FieldOptions& options);

    bool Combo(int& currentItem, const char* const items[], int itemsCount, const FieldOptions& options);
    bool Combo(int& currentItem, const std::vector<std::string>& items, const FieldOptions& options);

    // Массив с автовыводом размера
    template<std::size_t N>
    bool Combo(int& currentItem, const char* const (&items)[N], const FieldOptions& options) {
        return Combo(currentItem, items, static_cast<int>(N), options);
    }

    // Строго типизированные Enum и массивы с автовыводом размера
    template<typename EnumT, std::size_t N, std::enable_if_t<std::is_enum_v<EnumT>, int> = 0>
    bool Combo(EnumT& currentItem, const char* const (&items)[N], const FieldOptions& options)
    {
        int current = static_cast<int>(currentItem);
        if (ComboImpl(StateOf(&currentItem, sizeof(currentItem)), current, items,
                      static_cast<int>(N), options)) {
            currentItem = static_cast<EnumT>(current);
            return true;
        }
        return false;
    }

    // Геометрия 12-колоночной сетки
    float GetFullWidth() const;
    float CalculateSpanWidth(int span) const;
    float GetColumnWidth(int span = 6) const { return CalculateSpanWidth(span); }
    float GetColumnSpacing() const { return m_columnSpacing; }
    int GetRemainingSpan() const { return 12 - m_rowSpanUsed; }

    void SetRowSpacing(float spacingPx) { m_rowSpacing = spacingPx; }
    float GetRowSpacing() const { return m_rowSpacing; }

    float CurrentRowHeight() const { return m_currentRowHeight; }

    // Доступная высота внутри карточки до нижней границы
    float GetRemainingHeight() const;
    bool IsStretchY() const { return m_stretchY; }

    // Скроллируемая область списка контента:  if (auto list = card.ContentList()) { ... }
    ContentListScope ContentList(const char* key = nullptr) {
        return ContentListScope(this, key);
    }

    // Сообщение о пустом состоянии внутри карточки
    void Empty(const char* message, const char* detail = nullptr);

    // Вспомогательные методы компоновки внутри карточки (инкапсуляция ImGui)
    void SameLine(float spacing = -1.0f);
    void RightAlign(float itemWidth);
    void Spacing();
    void Separator();
    void AlignTextToFrame();
    void AlignTextToButton(UiSize size = UiSize::Small);

private:
    void BeginRow(std::optional<UiSize> size);
    void EndRow();
    void End();
    bool BeginContentList(const char* key);
    void EndContentList();

    FieldState StateOf(const void* field, std::size_t size) const;
    // Прочерк на месте поля: та же подпись и ширина, ввода нет
    void RenderNoValue(const FieldOptions& options, float width, float height);
    bool ComboImpl(FieldState state, int& currentItem, const char* const items[], int itemsCount,
                   const FieldOptions& options);
    void ValueImpl(const char* text, const FieldOptions& options);
    template<typename T>
    bool PresetGridImpl(T& value, const std::vector<T>& presets, int columns, const FieldOptions& options);
    void PrepareField(::Col col, float& outWidth);
    void FinishField(::Col col);
    // Высота элемента в финальных px: явный size, иначе высота строки, иначе fallback; 0 — авто
    float ResolveHeight(std::optional<UiSize> size, std::optional<UiSize> fallback = std::nullopt) const;

    int m_rowSpanUsed = 0; // Использовано долей в текущей строке (0..12)
    bool m_ended = false;
    float m_columnSpacing = 12.0f;
    float m_rowSpacing = 12.0f;
    float m_rowStartX = 0.0f;
    float m_rowStartY = 0.0f;
    float m_rowMaxY = 0.0f;
    float m_currentRowHeight = 0.0f; // Текущая активная высота фреймов строки
    bool m_needsRowAdvance = false;
    bool m_inAutoRow = false;
    bool m_stretchY = false;
    CardStyle m_style;
    FieldStateProvider m_fieldStates;
};
