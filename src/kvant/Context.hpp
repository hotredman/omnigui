#pragma once

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

namespace kvant {

/// Options for text widget.
struct TextOptions {
    Color color{Color::Hex(0xE2E8F0FF)};           // Slate-200
    float fontSize{14.0f};
};

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

    void Text(
        std::string_view label,
        TextOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    template <typename... Args>
    void TextFormat(
        std::format_string<Args...> fmt,
        Args&&... args
    ) {
        Text(Format(fmt, std::forward<Args>(args)...));
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

private:
    void RenderNodeRecursive(Node* node, ICanvas& canvas);

    BumpAllocator m_arena;
    std::vector<Node*> m_containerStack;
    std::vector<UiSize> m_sizeStack;
    std::vector<bool> m_disabledStack;
    Node* m_root{nullptr};

    Vec2 m_viewportSize{800.0f, 600.0f};
    float m_dpiScale{1.0f};

    // Input state
    Vec2 m_mousePos{0.0f, 0.0f};
    bool m_mouseDown{false};
    bool m_mouseClicked{false};
    bool m_mouseReleased{false};

    Id m_hoveredId{Id::Null()};
    Id m_activeId{Id::Null()};
    Id m_focusedId{Id::Null()};

    // Previous frame widget bounds for instantaneous hit testing
    std::unordered_map<Id, Rect> m_prevFrameBounds;
    std::unordered_map<Id, Rect> m_currFrameBounds;
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
