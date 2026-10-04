#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <algorithm>
#include <optional>
#include <cstddef>
#include <functional>
#include <type_traits>

#include "omnikit/Scope.hpp"
#include "omnikit/ColumnLayout.hpp"
#include "omnikit/UiTheme.hpp"
#include "omnikit/TextSuggest.hpp"
#include "omnikit/UiKey.hpp"
#include <source_location>
#include "omnikit/Icon.hpp"
#include "omnikit/PresetGrid.hpp"
#include "omnikit/Toggle.hpp"

namespace OmniKit {

// Field state in card: normal, modified (not yet written to device), no value
// (not read — renders dash placeholder instead of input)
enum class FieldState { Normal, Modified, NoValue };

// Card options (designated initializers):
//
//     if (Card card({.title = "Stream Actions"})) { ... }
//     if (Card card({.title = "Log", .stretchY = true})) { ... }
//
// Card identity (ImGui ID) is taken from key, or defaults to title. Two cards without
// a title and without a key in the same window will collide — provide an explicit key.
struct CardOptions {
    const char* title    = nullptr;
    UiKey       key      = {};          // identity; defaults to title
    float       width    = 0.0f;        // base px (scaled); 0 — width of column/layout cell
    float       height   = 0.0f;        // base px (scaled); 0 — fit content
    bool        stretchY = false;       // stretch vertically to fill remaining height
    const CardStyle* style = nullptr;   // style override; nullptr — from theme
};

// Options for any card item (input field, button, value display). Only fields applicable
// to the specific element are used; all others are ignored:
//
//     card.Float(rate,  {.label = "Rate", .unit = "Hz", .col = Col::Half()});
//     card.Toggle(on,   {.label = "Enable", .sublabel = "Verifies liveness"});
//     card.Button({.label = "Save", .variant = UiVariant::Primary, .icon = Icon::Check});
struct FieldOptions {
    const char* label    = nullptr;     // label (above field / on button)
    const char* sublabel = nullptr;     // Toggle: second line
    const char* unit     = nullptr;     // unit of measurement
    const char* format   = nullptr;     // printf number format; nullptr — "%.2f"
    const char* hint     = nullptr;     // Text: placeholder; nullptr — " — "
    UiVariant   variant  = UiVariant::Default;  // Button / Value: semantic style
    Icon        icon     = Icon::None;  // Button
    const char* tooltip  = nullptr;     // Button
    std::optional< OmniKit::Col> col;         // row fraction; defaults to Half (value displays and preset grids — Full)
    std::optional<UiSize>  size;        // height; defaults to current Row() height
    bool        disabled = false;
    UiKey       key      = {};          // identity; defaults to label
    std::optional<bool> alignBottom;    // Toggle: align to bottom baseline of adjacent fields
    int         columns  = 5;           // PresetGrid: number of preset grid columns
    const TextSuggest* suggest = nullptr;  // Text: suggestions below field (caller builds list)
};

class Card : public Scope {
public:
    // RAII row for card grid with uniform item height:  if (auto row = card.Row()) { ... }
    class RowScope : public Scope {
    public:
        RowScope(Card* parent, std::optional<UiSize> size);
        ~RowScope();
    private:
        Card* m_parent = nullptr;
    };

    // RAII grid cell for arbitrary widgets:  if (auto c = card.Col(Col::Third())) { ... }
    class ColumnScope : public Scope {
    public:
        ColumnScope(Card* parent, OmniKit::Col col);
        ~ColumnScope();
        float Width() const { return m_width; }
    private:
        Card* m_parent = nullptr;
        OmniKit::Col m_col = OmniKit::Col::Half();
        float m_width = 0.0f;
    };

    // RAII scrollable content list region (fills remaining card height)
    class ContentListScope : public Scope {
    public:
        ContentListScope(Card* parent, const char* key);
        ~ContentListScope();
    private:
        Card* m_parent = nullptr;
    };

    // Opens a styled card (style defaults to UiTheme::Get().card)
    Card(const CardOptions& options = {});
    explicit Card(const char* title) : Card(CardOptions{.title = title}) {}
    ~Card();

    // 2D grid: row and column management
    RowScope Row(std::optional<UiSize> size = UiSize::Medium);
    ColumnScope Col(OmniKit::Col col = OmniKit::Col::Half());

    // Explicitly advances to next grid row
    void NextRow();

    // Buttons with semantic styles; disabled — button is visible but non-interactive.
    // Col::Auto() sizes to content (unconstrained by 12-column grid)
    bool Button(const FieldOptions& options,
                std::source_location loc = std::source_location::current());

    // Hold button: true while held down (momentary action). Non-interactive/disabled returns false.
    bool HoldButton(const FieldOptions& options,
                    std::source_location loc = std::source_location::current());

    // Informational/computed value display (ValueDisplay); label — caption above display.
    // Without explicit size, height is 80 base px; std::nullopt renders a dash placeholder
    void Value(double value, const FieldOptions& options = {},
               std::source_location loc = std::source_location::current());
    void Value(const char* text, const FieldOptions& options = {},
               std::source_location loc = std::source_location::current());
    void Value(std::optional<double> value, const FieldOptions& options = {},
               std::source_location loc = std::source_location::current());

    // Preset selector with embedded display (PresetGrid); unit — display unit,
    // columns — preset grid column count, format — display value format
    bool PresetGrid(float& value, const std::vector<float>& presets, const FieldOptions& options = {},
                    std::source_location loc = std::source_location::current());
    bool PresetGrid(double& value, const std::vector<double>& presets, const FieldOptions& options = {},
                    std::source_location loc = std::source_location::current());

    // Card field state queried by field address (float/int/bool/enum): without provider,
    // all fields are Normal. Provider lifetime must not exceed the card
    using FieldStateProvider = std::function<FieldState(const void* field, std::size_t size)>;
    void SetFieldStates(FieldStateProvider provider) { m_fieldStates = std::move(provider); }

    // Input fields in 12-column grid
    bool Float(float& value, const FieldOptions& options,
               std::source_location loc = std::source_location::current());
    bool Float(std::optional<float>& value, const FieldOptions& options,
               std::source_location loc = std::source_location::current());
    bool Double(double& value, const FieldOptions& options,
                std::source_location loc = std::source_location::current());
    bool Double(std::optional<double>& value, const FieldOptions& options,
                std::source_location loc = std::source_location::current());
    bool Int(int& value, const FieldOptions& options,
             std::source_location loc = std::source_location::current());
    bool Text(std::string& value, const FieldOptions& options,
              std::source_location loc = std::source_location::current());
    bool Toggle(bool& value, const FieldOptions& options,
                std::source_location loc = std::source_location::current());

    // Read-only field: label above value
    void Display(const std::string& value, const FieldOptions& options,
                 std::source_location loc = std::source_location::current());

    bool Combo(int& currentItem, const char* const items[], int itemsCount, const FieldOptions& options,
               std::source_location loc = std::source_location::current());
    bool Combo(int& currentItem, const std::vector<std::string>& items, const FieldOptions& options,
               std::source_location loc = std::source_location::current());

    // Array with deduced size
    template<std::size_t N>
    bool Combo(int& currentItem, const char* const (&items)[N], const FieldOptions& options,
               std::source_location loc = std::source_location::current()) {
        return Combo(currentItem, items, static_cast<int>(N), options, loc);
    }

    // Strongly-typed Enum and arrays with deduced size
    template<typename EnumT, std::size_t N, std::enable_if_t<std::is_enum_v<EnumT>, int> = 0>
    bool Combo(EnumT& currentItem, const char* const (&items)[N], const FieldOptions& options,
               std::source_location loc = std::source_location::current())
    {
        int current = static_cast<int>(currentItem);
        if (ComboImpl(StateOf(&currentItem, sizeof(currentItem)), current, items,
                      static_cast<int>(N), options, loc)) {
            currentItem = static_cast<EnumT>(current);
            return true;
        }
        return false;
    }

    // 12-column grid geometry
    float GetFullWidth() const;
    float CalculateSpanWidth(int span) const;
    float GetColumnWidth(int span = 6) const { return CalculateSpanWidth(span); }
    float GetColumnSpacing() const { return m_columnSpacing; }
    int GetRemainingSpan() const { return 12 - m_rowSpanUsed; }

    void SetRowSpacing(float spacingPx) { m_rowSpacing = spacingPx; }
    float GetRowSpacing() const { return m_rowSpacing; }

    float CurrentRowHeight() const { return m_currentRowHeight; }

    // Available height inside card down to bottom boundary
    float GetRemainingHeight() const;
    bool IsStretchY() const { return m_stretchY; }

    // Scrollable content list region:  if (auto list = card.ContentList()) { ... }
    ContentListScope ContentList(const char* key = nullptr) {
        return ContentListScope(this, key);
    }

    // Empty state message inside card
    void Empty(const char* message, const char* detail = nullptr);

    // Helper layout methods inside card (ImGui encapsulation)
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
    // Dash placeholder in place of field: same label and width, no input
    void RenderNoValue(const FieldOptions& options, float width, float height,
                       std::source_location loc = std::source_location::current());
    bool ComboImpl(FieldState state, int& currentItem, const char* const items[], int itemsCount,
                   const FieldOptions& options, std::source_location loc = std::source_location::current());
    void ValueImpl(const char* text, const FieldOptions& options, std::source_location loc = std::source_location::current());
    template<typename T>
    bool PresetGridImpl(T& value, const std::vector<T>& presets, int columns, const FieldOptions& options,
                        std::source_location loc = std::source_location::current());
    void PrepareField(OmniKit::Col col, float& outWidth);
    void FinishField(OmniKit::Col col);
    // Item height in resolved px: explicit size, otherwise row height, otherwise fallback; 0 — auto
    float ResolveHeight(std::optional<UiSize> size, std::optional<UiSize> fallback = std::nullopt) const;

    int m_rowSpanUsed = 0; // Spans used in current row (0..12)
    bool m_ended = false;
    float m_columnSpacing = 12.0f;
    float m_rowSpacing = 12.0f;
    float m_rowStartX = 0.0f;
    float m_rowStartY = 0.0f;
    float m_rowMaxY = 0.0f;
    float m_currentRowHeight = 0.0f; // Current active frame height of row
    bool m_needsRowAdvance = false;
    bool m_inAutoRow = false;
    bool m_stretchY = false;
    CardStyle m_style;
    FieldStateProvider m_fieldStates;
};

} // namespace OmniKit
