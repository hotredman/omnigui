#pragma once

#include <imgui.h>
#include <vector>
#include <algorithm>

// ============================================================================
// Доля строки в 12-колоночной сетке (Bootstrap / Grid style)
// ============================================================================
struct Col {
    int span = 6; // По умолчанию 6 из 12 (двухколоночный режим = 50% ширины)
    float reservedPx = 0.0f; // Для Col::Fill(): резервируемая ширина справа (например, под кнопки)

    constexpr Col() : span(6), reservedPx(0.0f) {}
    constexpr Col(int s) : span(s < -1 ? -1 : (s > 12 ? 12 : s)), reservedPx(0.0f) {}
    constexpr Col(int s, float reserved) : span(s < -1 ? -1 : (s > 12 ? 12 : s)), reservedPx(reserved) {}

    static constexpr Col Auto()                         { return Col{0, 0.0f};  } // Автоматическая контентная ширина (не привязана к сетке)
    static constexpr Col Fit()                          { return Col{0, 0.0f};  } // Синоним для Auto
    static constexpr Col Fill(float reservedRight = 0.0f){ return Col{-1, reservedRight}; } // Растягивание до конца строки (с опциональным резервом под кнопки справа)
    static constexpr Col Full()                         { return Col{12, 0.0f}; } // 100% строки
    static constexpr Col Half()                         { return Col{6, 0.0f};  } // 50% строки (2 колонки)
    static constexpr Col Third()                        { return Col{4, 0.0f};  } // 33.3% строки (3 колонки)
    static constexpr Col TwoThirds()                    { return Col{8, 0.0f};  } // 66.7% строки (2/3)
    static constexpr Col Quarter()                      { return Col{3, 0.0f};  } // 25% строки (4 колонки)
    static constexpr Col Sixth()                        { return Col{2, 0.0f};  } // 16.7% строки (6 колонок)
    static constexpr Col Custom(int s)                  { return Col{s, 0.0f};  }

    bool IsAuto() const { return span == 0; }
    bool IsFill() const { return span < 0; }
};

// ============================================================================
// Стандартные токены высоты интерактивного фрейма (RowHeight)
// ============================================================================
struct RowHeight {
    float baselinePx = 0.0f; // Базовая высота в пикселях до масштабирования (0 = Auto)

    constexpr RowHeight() : baselinePx(0.0f) {}
    constexpr RowHeight(float px) : baselinePx(px) {}

    static constexpr RowHeight Auto()       { return RowHeight{ 0.0f }; }   // Автоматическая высота по умолчанию
    static constexpr RowHeight Small()      { return RowHeight{ 28.0f }; }  // 28px: чипы, пресеты скоростей
    static constexpr RowHeight Default()    { return RowHeight{ 40.0f }; }  // 40px: стандартные кнопки и рамки ввода
    static constexpr RowHeight Medium()     { return RowHeight{ 46.0f }; }  // 46px: увеличенные кнопки
    static constexpr RowHeight Large()      { return RowHeight{ 56.0f }; }  // 56px: крупные кнопки (ПУСК, СТОП)
    static constexpr RowHeight Display()    { return RowHeight{ 80.0f }; }  // 80px: расчетные табло (площадь, нагрузка)
    static constexpr RowHeight Fill()       { return RowHeight{ -1.0f }; }  // -1.0f: 100% оставшейся высоты карточки/ячейки
    static constexpr RowHeight Custom(float px) { return RowHeight{ px }; }

    bool IsAuto() const    { return baselinePx == 0.0f; }
    bool IsFill() const    { return baselinePx < 0.0f; }
    bool IsCustom() const  { return baselinePx > 0.0f; }
};

// ============================================================================
// Потоковая 12-колоночная раскладка с автопереносом строк (FlowLayout)
// ============================================================================
class FlowLayout {
public:
    class ColumnScope {
    public:
        ColumnScope(FlowLayout* parent, float width, bool active)
            : m_parent(parent), m_width(width), m_active(active) {}

        ~ColumnScope();

        ColumnScope(const ColumnScope&) = delete;
        ColumnScope& operator=(const ColumnScope&) = delete;

        ColumnScope(ColumnScope&& other) noexcept
            : m_parent(other.m_parent), m_width(other.m_width), m_active(other.m_active) {
            other.m_parent = nullptr;
            other.m_active = false;
        }

        explicit operator bool() const { return m_active; }
        float Width() const { return m_width; }
        void AlignWithInput();
        void RightAlign(float itemWidth);

    private:
        FlowLayout* m_parent = nullptr;
        float m_width = 0.0f;
        bool m_active = false;
    };

    explicit FlowLayout(float availableWidth = 0.0f, float columnSpacing = -1.0f, float rowSpacing = -1.0f);
    ~FlowLayout();

    FlowLayout(const FlowLayout&) = delete;
    FlowLayout& operator=(const FlowLayout&) = delete;

    FlowLayout(FlowLayout&& other) noexcept;
    FlowLayout& operator=(FlowLayout&& other) noexcept;

    // 1. Потоковый RAII синтаксис
    ColumnScope Col(::Col col = ::Col::Half());

    // 2. Классический Begin / End
    bool BeginCol(::Col col = ::Col::Half());
    void EndCol();

    // Принудительный переход на новую строку
    void NextRow();

    // Геометрия
    float GetAvailableWidth() const { return m_rowAvailWidth; }
    float CalculateSpanWidth(int span) const;
    float GetColumnSpacing() const { return m_columnSpacing; }
    float GetRowSpacing() const { return m_rowSpacing; }
    int GetRemainingSpan() const { return 12 - m_rowSpanUsed; }

    // Активная ширина колонки в текущем потоке (опрашивается Card)
    static float CurrentColumnWidth();

    // Фабричные методы
    static FlowLayout Columns2(float spacing = -1.0f);
    static FlowLayout Columns3(float spacing = -1.0f);
    static FlowLayout Columns4(float spacing = -1.0f);
    static FlowLayout Split(int spanLeft, int spanRight, float spacing = -1.0f);

    // Совместимость со старым API
    bool BeginColumn(int columnIndex);
    void EndColumn() { EndCol(); }

private:
    float m_totalAvailWidth = 0.0f;
    float m_rowAvailWidth = 0.0f;
    float m_columnSpacing = 16.0f;
    float m_rowSpacing = 16.0f;

    int m_rowSpanUsed = 0;
    int m_colIndexInRow = 0;
    float m_accumulatedWidth = 0.0f;
    int m_activeSpan = 0;
    float m_activeWidth = 0.0f;
    bool m_colOpen = false;
    bool m_needsNewRowSpacing = false;

    std::vector<int> m_presetSpans;

    static thread_local float s_currentColumnWidth;
};

using ColumnLayout = FlowLayout;
