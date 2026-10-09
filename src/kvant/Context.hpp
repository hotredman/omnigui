#pragma once

#include <chrono>
#include <functional>
#include <initializer_list>
#include <optional>
#include <span>
#include <source_location>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "kvant/backend/ICanvas.hpp"
#include "kvant/core/BumpAllocator.hpp"
#include "kvant/core/Id.hpp"
#include "kvant/core/Response.hpp"
#include "kvant/core/Tokens.hpp"
#include "kvant/core/Types.hpp"
#include "kvant/core/UiString.hpp"
#include "kvant/graphics/Icon.hpp"
#include "kvant/layout/LayoutSolver.hpp"
#include "kvant/layout/Node.hpp"
#include "kvant/layout/Scope.hpp"
#include "kvant/widgets/Button.hpp"
#include "kvant/widgets/Checkbox.hpp"
#include "kvant/widgets/InputField.hpp"
#include "kvant/widgets/Select.hpp"
#include "kvant/widgets/Switch.hpp"
#include "kvant/widgets/Text.hpp"

namespace kvant {

/// Primary immediate mode context coordinating input, layout, and rendering.
class Context {
public:
    Context();
    ~Context() = default;

    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;

    // --- Frame Lifecycle ---
    void BeginFrame(Vec2 viewportSize, float dpiScale = 1.0f);
    void EndFrame(ICanvas& canvas);

    // --- Input Handling ---
    void OnMouseMove(Vec2 pos);
    void OnMouseDown(int button);
    void OnMouseUp(int button);
    void OnTextInput(std::string_view utf8Text);
    void OnKeyDown(Key key, KeyModifiers mods = {});
    void OnMouseWheel(float x, float y);

    // --- Container Scopes ---
    RowScope Row(
        RowOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    ColumnScope Column(
        ColumnOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    BoxScope Box(
        BoxOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    WrapScope Wrap(
        WrapOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    void Spacer(
        float flex = 1.0f,
        std::source_location loc = std::source_location::current()
    );

    // --- Scale & State Scopes ---
    void PushSize(UiSize size);
    void PopSize();
    [[nodiscard]] UiSize GetCurrentSize() const;

    void PushDisabled(bool disabled = true);
    void PopDisabled();
    [[nodiscard]] bool IsDisabled() const;

    template<typename Fn>
    void Disabled(bool disabled, Fn&& fn) {
        PushDisabled(disabled);
        fn();
        PopDisabled();
    }

    // --- Widgets ---
    Response Button(
        ButtonOptions opts,
        std::source_location loc = std::source_location::current()
    );

    Response Button(
        std::string_view label,
        ButtonOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    Response Text(
        std::string_view label,
        TextOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    template <typename... Args>
    Response TextFormat(
        std::format_string<Args...> fmt,
        Args&&... args
    ) {
        return Text(Format(fmt, std::forward<Args>(args)...));
    }

    // Semantic typography shorthands
    Response Display(std::string_view label, UiVariant variant = UiVariant::Default, std::source_location loc = std::source_location::current());
    Response Headline(std::string_view label, UiVariant variant = UiVariant::Default, std::source_location loc = std::source_location::current());
    Response Title(std::string_view label, UiVariant variant = UiVariant::Default, std::source_location loc = std::source_location::current());
    Response Subtitle(std::string_view label, UiVariant variant = UiVariant::Default, std::source_location loc = std::source_location::current());
    Response Body(std::string_view label, UiVariant variant = UiVariant::Default, std::source_location loc = std::source_location::current());
    Response Caption(std::string_view label, UiVariant variant = UiVariant::Muted, std::source_location loc = std::source_location::current());
    Response Code(std::string_view label, UiVariant variant = UiVariant::Default, std::source_location loc = std::source_location::current());

    // --- Switch Widget ---
    Response Switch(
        std::string_view label,
        bool* value,
        SwitchOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    Response Switch(
        bool* value,
        SwitchOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    Response Switch(
        std::string_view label,
        bool checked,
        SwitchOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    Response Switch(
        bool checked,
        SwitchOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    // --- Checkbox Widget ---
    Response Checkbox(
        std::string_view label,
        bool* value,
        CheckboxOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    Response Checkbox(
        bool* value,
        CheckboxOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    Response Checkbox(
        std::string_view label,
        bool checked,
        CheckboxOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    Response Checkbox(
        bool checked,
        CheckboxOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    /// Convenience aliases for developers familiar with Toggle terminology.
    Response Toggle(
        std::string_view label,
        bool* value,
        SwitchOptions opts = {},
        std::source_location loc = std::source_location::current()
    ) {
        return Switch(label, value, opts, loc);
    }

    Response Toggle(
        bool* value,
        SwitchOptions opts = {},
        std::source_location loc = std::source_location::current()
    ) {
        return Switch(value, opts, loc);
    }

    Response Toggle(
        std::string_view label,
        bool checked,
        SwitchOptions opts = {},
        std::source_location loc = std::source_location::current()
    ) {
        return Switch(label, checked, opts, loc);
    }

    Response Toggle(
        bool checked,
        SwitchOptions opts = {},
        std::source_location loc = std::source_location::current()
    ) {
        return Switch(checked, opts, loc);
    }

    // --- InputField Widget ---
    Response InputField(
        std::string_view label,
        std::string& value,
        InputFieldOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    Response InputField(
        std::string& value,
        InputFieldOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    Response InputField(
        std::string_view label,
        float& value,
        InputFieldOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    Response InputField(
        float& value,
        InputFieldOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    Response InputField(
        std::string_view label,
        double& value,
        InputFieldOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    Response InputField(
        double& value,
        InputFieldOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    Response InputField(
        std::string_view label,
        int& value,
        InputFieldOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    Response InputField(
        int& value,
        InputFieldOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    Response InputField(
        std::string_view label,
        std::string_view value,
        InputFieldOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    template <typename T>
    Response InputNumber(
        std::string_view label,
        T& value,
        std::string_view unit = {},
        InputFieldOptions opts = {},
        std::source_location loc = std::source_location::current()
    ) {
        if (!unit.empty()) {
            opts.suffix = unit;
        }
        return InputField(label, value, opts, loc);
    }

    // --- Select (Dropdown) Widget ---
    Response Select(
        std::string_view label,
        size_t& selectedIndex,
        std::span<const std::string_view> items,
        SelectOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    Response Select(
        size_t& selectedIndex,
        std::span<const std::string_view> items,
        SelectOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    Response Select(
        std::string_view label,
        int& selectedIndex,
        std::span<const std::string_view> items,
        SelectOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    Response Select(
        int& selectedIndex,
        std::span<const std::string_view> items,
        SelectOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    Response Select(
        std::string_view label,
        std::string& selectedValue,
        std::span<const std::string_view> items,
        SelectOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    Response Select(
        std::string& selectedValue,
        std::span<const std::string_view> items,
        SelectOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    Response Select(
        std::string_view label,
        std::string_view& selectedValue,
        std::span<const std::string_view> items,
        SelectOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    Response Select(
        std::string_view& selectedValue,
        std::span<const std::string_view> items,
        SelectOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    Response Select(
        std::string_view label,
        size_t& selectedIndex,
        std::initializer_list<std::string_view> items,
        SelectOptions opts = {},
        std::source_location loc = std::source_location::current()
    ) {
        return Select(label, selectedIndex, std::span<const std::string_view>(items.begin(), items.size()), opts, loc);
    }

    Response Select(
        size_t& selectedIndex,
        std::initializer_list<std::string_view> items,
        SelectOptions opts = {},
        std::source_location loc = std::source_location::current()
    ) {
        return Select(selectedIndex, std::span<const std::string_view>(items.begin(), items.size()), opts, loc);
    }

    Response Select(
        std::string_view label,
        int& selectedIndex,
        std::initializer_list<std::string_view> items,
        SelectOptions opts = {},
        std::source_location loc = std::source_location::current()
    ) {
        return Select(label, selectedIndex, std::span<const std::string_view>(items.begin(), items.size()), opts, loc);
    }

    Response Select(
        int& selectedIndex,
        std::initializer_list<std::string_view> items,
        SelectOptions opts = {},
        std::source_location loc = std::source_location::current()
    ) {
        return Select(selectedIndex, std::span<const std::string_view>(items.begin(), items.size()), opts, loc);
    }

    Response Select(
        std::string_view label,
        std::string& selectedValue,
        std::initializer_list<std::string_view> items,
        SelectOptions opts = {},
        std::source_location loc = std::source_location::current()
    ) {
        return Select(label, selectedValue, std::span<const std::string_view>(items.begin(), items.size()), opts, loc);
    }

    Response Select(
        std::string& selectedValue,
        std::initializer_list<std::string_view> items,
        SelectOptions opts = {},
        std::source_location loc = std::source_location::current()
    ) {
        return Select(selectedValue, std::span<const std::string_view>(items.begin(), items.size()), opts, loc);
    }

    // --- String & Formatting Services ---
    [[nodiscard]] UiString String(std::string_view text);
    [[nodiscard]] std::string_view InternString(std::string_view text);

    template <typename... Args>
    [[nodiscard]] UiString Format(std::format_string<Args...> fmt, Args&&... args) {
        BasicUiString<128> temp;
        temp.Format(fmt, std::forward<Args>(args)...);
        return UiString(InternString(temp.View()));
    }

    // --- Core Engine Services for Widgets ---
    Id MakeChildId(std::source_location loc);
    Response Interact(Id id);
    Node* CreateNode(NodeType type, Id id);
    void AdaptChildToContainer(Node* node);
    [[nodiscard]] Rect GetPrevFrameBounds(Id id) const;

    // --- Internal Hierarchy & Scope Management ---
    void PushContainer(Node* node);
    void PopContainer(Node* node);

    // Imperative actions requested from Response
    void RequestFocus(Id id);
    [[nodiscard]] Id GetFocusedId() const noexcept { return m_focusedId; }
    [[nodiscard]] bool IsFocused(Id id) const noexcept { return m_focusedId == id && id.IsValid(); }
    [[nodiscard]] bool HasActiveTextEdit(Id id) const noexcept { return m_editState.id == id && id.IsValid(); }
    void ClearFocus() noexcept { m_focusedId = Id::Null(); }

    std::string_view GetOrStartTextEdit(
        Id id,
        std::string_view initialText,
        size_t& outCursor,
        bool& outChanged,
        bool& outSubmitted
    );

    // --- Cursor Configuration & State ---
    void SetCursorConfig(const CursorConfig& config) noexcept { m_cursorConfig = config; }
    [[nodiscard]] const CursorConfig& GetCursorConfig() const noexcept { return m_cursorConfig; }
    [[nodiscard]] bool IsCursorVisible(std::optional<CursorBlinkMode> modeOverride = std::nullopt) const noexcept;
    void ResetCursorBlink() noexcept;

    // --- Deterministic Time Control (for unit tests / animations) ---
    void SetCurrentTime(double timeSeconds) noexcept;
    void AdvanceTime(double deltaSeconds) noexcept;
    [[nodiscard]] double GetCurrentTime() const noexcept { return m_currentTimeSec; }

    // --- Popup & Overlay Management ---
    void OpenPopup(Id id, Rect bounds);
    void ClosePopup();
    [[nodiscard]] bool IsPopupOpen(Id id) const noexcept { return m_openPopupId == id && id.IsValid(); }
    [[nodiscard]] Id GetOpenPopupId() const noexcept { return m_openPopupId; }
    [[nodiscard]] Rect GetOpenPopupRect() const noexcept { return m_openPopupRect; }
    void RegisterOverlay(std::function<void(ICanvas&)> drawFn);

    // --- Input Queries & Consumption ---
    [[nodiscard]] Vec2 GetMousePos() const noexcept { return m_mousePos; }
    [[nodiscard]] bool IsMouseDown() const noexcept { return m_mouseDown; }
    [[nodiscard]] bool IsMouseClicked() const noexcept { return m_mouseClicked; }
    [[nodiscard]] bool IsMouseReleased() const noexcept { return m_mouseReleased; }
    void ConsumeMouseClick() noexcept { m_mouseClicked = false; m_mouseReleased = false; }
    [[nodiscard]] Vec2 GetViewportSize() const noexcept { return m_viewportSize; }
    [[nodiscard]] Vec2 GetMouseWheel() const noexcept { return m_mouseWheel; }
    bool ConsumeKey(Key key);

private:
    void RenderNodeRecursive(Node* node, ICanvas& canvas);

    BumpAllocator m_arena;
    std::vector<Node*> m_containerStack;
    std::vector<UiSize> m_sizeStack;
    std::vector<bool> m_disabledStack;
    Node* m_root{nullptr};

    Vec2 m_viewportSize{800.0f, 600.0f};
    float m_dpiScale{1.0f};

    // Time & Cursor animation state
    CursorConfig m_cursorConfig{};
    std::chrono::steady_clock::time_point m_startTime{std::chrono::steady_clock::now()};
    double m_currentTimeSec{0.0};
    double m_lastInputTimeSec{0.0};
    bool m_manualTime{false};

    // Input state
    Vec2 m_mousePos{0.0f, 0.0f};
    bool m_mouseDown{false};
    bool m_mouseClicked{false};
    bool m_mouseReleased{false};

    Id m_hoveredId{Id::Null()};
    Id m_activeId{Id::Null()};
    Id m_focusedId{Id::Null()};

    // Active text edit state
    struct ActiveEditState {
        Id id{Id::Null()};
        std::string text;
        size_t cursor{0};
        bool changed{false};
        bool submitted{false};
    };
    ActiveEditState m_editState;

    struct QueuedKeyEvent {
        Key key;
        KeyModifiers mods;
    };
    std::vector<std::string> m_pendingTextInput;
    std::vector<QueuedKeyEvent> m_pendingKeyEvents;

    // Previous frame widget bounds for instantaneous hit testing
    std::unordered_map<Id, Rect> m_prevFrameBounds;
    std::unordered_map<Id, Rect> m_currFrameBounds;

    // Floating popup & overlay state
    Id m_openPopupId{Id::Null()};
    Rect m_openPopupRect{0.0f, 0.0f, 0.0f, 0.0f};
    std::vector<std::function<void(ICanvas&)>> m_overlayPasses;
    Vec2 m_mouseWheel{0.0f, 0.0f};
};

// Inline implementations of Response imperative actions
inline void Response::Focus() {
    if (ctx && id.IsValid()) {
        ctx->RequestFocus(id);
    }
}

inline void Response::ScrollTo() {
    // Forward to scroll controller when ScrollArea is implemented
}

// Inline implementation of ContainerScope destructor
inline ContainerScope::~ContainerScope() {
    if (m_ctx && m_node) {
        m_ctx->PopContainer(m_node);
        m_ctx = nullptr;
        m_node = nullptr;
    }
}

} // namespace kvant
