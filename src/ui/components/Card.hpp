#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <algorithm>
#include <optional>
#include <cstddef>
#include <functional>
#include <type_traits>

#include "ui/components/FlowLayout.hpp"
#include "ui/components/UiTheme.hpp"
#include "ui/components/Icon.hpp"
#include "ui/components/PresetGrid.hpp"
#include "ui/components/Toggle.hpp"

// Состояние поля в карточке: обычное, изменено (в машину не записано), нет значения
// (не считано — вместо ввода прочерк)
enum class FieldState { Normal, Modified, NoValue };

class Card {
public:
    // RAII RowScope для блочной настройки высоты строки и выравнивания
    class RowScope {
    public:
        RowScope(Card* parent, RowHeight height);
        ~RowScope();

        RowScope(const RowScope&) = delete;
        RowScope& operator=(const RowScope&) = delete;

        RowScope(RowScope&& other) noexcept;
        RowScope& operator=(RowScope&& other) noexcept;

        explicit operator bool() const { return m_parent != nullptr; }

    private:
        Card* m_parent = nullptr;
    };

    // RAII ColumnScope для произвольных виджетов в ячейке сетки карточки
    class ColumnScope {
    public:
        ColumnScope(Card* parent, ::Col col);
        ~ColumnScope();

        ColumnScope(const ColumnScope&) = delete;
        ColumnScope& operator=(const ColumnScope&) = delete;

        ColumnScope(ColumnScope&& other) noexcept;
        ColumnScope& operator=(ColumnScope&& other) noexcept;

        explicit operator bool() const { return m_parent != nullptr; }
        float Width() const { return m_width; }

    private:
        Card* m_parent = nullptr;
        ::Col m_col = ::Col::Half();
        float m_width = 0.0f;
    };

    // RAII конструктор: открывает стилизованную карточку (стиль берется из UiTheme::Get().card)
    Card(const char* id, const char* title = nullptr, ImVec2 size = ImVec2(0, 0));
    Card(const char* id, const char* title, float width);
    Card(const char* id, const char* title, bool stretchY);
    Card(const char* id, const char* title, RowHeight height);
    Card(const char* id, const char* title, const CardStyle& customStyle, ImVec2 size = ImVec2(0, 0), bool stretchY = false);
    ~Card();

    // Фабричный метод для карточки, растягивающейся по вертикали
    static Card StretchY(const char* id, const char* title = nullptr);

    // Запрет копирования (во избежание двойного закрытия ImGui Child)
    Card(const Card&) = delete;
    Card& operator=(const Card&) = delete;

    // Перемещение (Move)
    Card(Card&& other) noexcept;
    Card& operator=(Card&& other) noexcept;

    // Проверка открытия: if (Card card(...); card) или if (card)
    explicit operator bool() const { return m_open; }
    bool IsOpen() const { return m_open; }

    // Явное закрытие (деструктор не будет повторно вызывать EndChild)
    void End();

    // 2D-сетка: управление строками и колонками
    RowScope Row(RowHeight height = RowHeight::Default());
    ColumnScope Col(::Col col = ::Col::Half());

    void BeginRow(RowHeight height = RowHeight::Default());
    void EndRow();

    // Кнопки с семантическими стилями (UiVariant) и высотой строки;
    // disabled — кнопка видна, но неактивна (клик не проходит)
    bool Button(const char* label, UiVariant variant = UiVariant::Default, 
                RowHeight height = RowHeight::Auto(), ::Col col = ::Col::Half(),
                bool disabled = false);

    bool Button(const char* label, Icon icon, UiVariant variant = UiVariant::Default, 
                RowHeight height = RowHeight::Auto(), ::Col col = ::Col::Half(),
                bool disabled = false);

    // Автоматическая ширина кнопки по содержимому (не привязана к 12-колоночной сетке)
    bool ButtonAuto(const char* label, UiVariant variant = UiVariant::Default, 
                    RowHeight height = RowHeight::Auto(), bool disabled = false);

    bool ButtonAuto(const char* label, Icon icon, UiVariant variant = UiVariant::Default, 
                    RowHeight height = RowHeight::Auto(), bool disabled = false);

    // Информационное/расчетное табло (ValueDisplay)
    void AddValueDisplay(double value, const char* unit = nullptr, const char* format = "%.2f", 
                         RowHeight height = RowHeight::Display(), ::Col col = ::Col::Full());

    void AddValueDisplay(const char* text, const char* unit = nullptr, 
                         RowHeight height = RowHeight::Display(), ::Col col = ::Col::Full());

    void AddValueDisplay(const char* label, std::optional<double> value, const char* unit = nullptr,
                         const char* format = "%.2f", RowHeight height = RowHeight::Display(),
                         ::Col col = ::Col::Full(), UiVariant variant = UiVariant::Default);

    // Селектор пресетов со встроенным табло (PresetGrid)
    bool AddPresetGrid(float& value, const std::vector<float>& presets, 
                       const char* unit = nullptr, int columns = 5,
                       RowHeight displayHeight = RowHeight(44.0f), ::Col col = ::Col::Full());

    bool AddPresetGrid(double& value, const std::vector<double>& presets, 
                       const char* unit = nullptr, int columns = 5,
                       RowHeight displayHeight = RowHeight(44.0f), ::Col col = ::Col::Full());

    // Состояние полей карточки по адресу поля (float/int/bool/enum): без провайдера
    // все поля обычные. Провайдер живёт не дольше карточки
    using FieldStateProvider = std::function<FieldState(const void* field, std::size_t size)>;
    void SetFieldStates(FieldStateProvider provider) { m_fieldStates = std::move(provider); }

    // Методы добавления полей ввода в 12-колоночную сетку
    bool AddFloat(const char* id, const char* label, float& value, 
                  const char* unit = nullptr, const char* format = "%.2f", 
                  ::Col col = ::Col::Half(), RowHeight height = RowHeight::Auto(),
                  bool disabled = false);

    bool AddFloat(const char* id, const char* label, std::optional<float>& value, 
                  const char* unit = nullptr, const char* format = "%.2f", 
                  ::Col col = ::Col::Half(), RowHeight height = RowHeight::Auto(),
                  bool disabled = false);

    bool AddDouble(const char* id, const char* label, double& value, 
                   const char* unit = nullptr, const char* format = "%.2f", 
                   ::Col col = ::Col::Half(), RowHeight height = RowHeight::Auto(),
                   bool disabled = false);

    bool AddDouble(const char* id, const char* label, std::optional<double>& value, 
                   const char* unit = nullptr, const char* format = "%.2f", 
                   ::Col col = ::Col::Half(), RowHeight height = RowHeight::Auto(),
                   bool disabled = false);

    bool AddToggle(const char* id, bool& value,
                   const char* label = nullptr,
                   const char* sublabel = nullptr,
                   bool disabled = false,
                   ::Col col = ::Col::Half(),
                   std::optional<bool> alignBottom = std::nullopt);

    bool AddInt(const char* id, const char* label, int& value, 
                const char* unit = nullptr, 
                ::Col col = ::Col::Half(), RowHeight height = RowHeight::Auto());

    bool AddText(const char* id, const char* label, std::string& value, 
                 ::Col col = ::Col::Half(), RowHeight height = RowHeight::Auto(),
                 const char* hint = " — ");

    void AddDisplay(const char* id, const char* label, const std::string& value, 
                    ::Col col = ::Col::Half(), RowHeight height = RowHeight::Auto());

    bool AddText(const char* id, const char* label, std::string& value, 
                 const char* hint, ::Col col = ::Col::Half(), 
                 RowHeight height = RowHeight::Auto());

    bool AddCombo(const char* id, const char* label, int& currentItem, 
                  const char* const items[], int itemsCount, 
                  ::Col col = ::Col::Half(), RowHeight height = RowHeight::Auto());

    bool AddCombo(const char* id, const char* label, int& currentItem, 
                  const std::vector<std::string>& items, 
                  ::Col col = ::Col::Half(), RowHeight height = RowHeight::Auto());

    // Шаблонный AddCombo для строго типизированных Enum и массивов с автовыводом размера
    template<typename EnumT, size_t N, std::enable_if_t<std::is_enum_v<EnumT>, int> = 0>
    bool AddCombo(const char* id, const char* label, EnumT& currentItem, 
                  const char* const (&items)[N], 
                  ::Col col = ::Col::Half(), RowHeight height = RowHeight::Auto())
    {
        int current = static_cast<int>(currentItem);
        if (AddComboImpl(StateOf(&currentItem, sizeof(currentItem)), id, label, current, items,
                         static_cast<int>(N), col, height)) {
            currentItem = static_cast<EnumT>(current);
            return true;
        }
        return false;
    }

    // Принудительный переход на следующую строку сетки
    void NextRow();

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

    // Скроллируемая область списка контента (занимает оставшуюся высоту карточки)
    bool BeginContentList(const char* id);
    void EndContentList();

    // RAII scope для скроллируемой области списка контента
    class ContentListScope {
    public:
        ContentListScope(Card* parent, const char* id);
        ~ContentListScope();
        ContentListScope(const ContentListScope&) = delete;
        ContentListScope& operator=(const ContentListScope&) = delete;
        ContentListScope(ContentListScope&& other) noexcept;
        ContentListScope& operator=(ContentListScope&& other) noexcept;
        explicit operator bool() const { return m_open; }
    private:
        Card* m_parent = nullptr;
        bool m_open = false;
    };

    ContentListScope ContentList(const char* id) {
        return ContentListScope(this, id);
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
    FieldState StateOf(const void* field, std::size_t size) const;
    // Прочерк на месте поля: та же подпись и ширина, ввода нет
    void RenderNoValue(const char* id, const char* label, float width, float height);
    bool AddComboImpl(FieldState state, const char* id, const char* label, int& currentItem,
                      const char* const items[], int itemsCount, ::Col col, RowHeight height);
    void PrepareField(::Col col, float& outWidth);
    void FinishField(::Col col);
    float ResolveHeight(RowHeight height, RowHeight defaultFallback) const;

    int m_rowSpanUsed = 0; // Использовано долей в текущей строке (0..12)
    bool m_open = false;
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
