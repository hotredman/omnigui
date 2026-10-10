#pragma once

#include <optional>
#include <source_location>
#include <string_view>
#include "kvant/core/Response.hpp"
#include "kvant/core/Tokens.hpp"
#include "kvant/core/Types.hpp"
#include "kvant/graphics/Icon.hpp"
#include "kvant/layout/LayoutTypes.hpp"

namespace kvant {

class Context;
class ICanvas;
class Node;

/// Configuration options for a ToolButton.
struct ToolButtonOptions {
    Icon icon{};
    UiVariant variant{UiVariant::Ghost};
    Placement iconPlacement{Placement::Leading};
    LabelVariants labelVariants{};
    bool disabled{false};
    bool active{false};         ///< Toggle active/selected highlight state
};

/// Configuration options for Toolbar container.
struct ToolbarOptions {
    std::optional<UiSize> size{};
    float fixedHeight{0.0f};
    Padding padding{Padding::Symmetric(12.0f, 6.0f)};
    float gap{8.0f};
    Color backgroundColor{Color::Transparent()};
    Color borderColor{Color::Transparent()};
    float borderWidth{1.0f};
    float cornerRadius{6.0f};
    bool disabled{false};
};

/// RAII scope for Toolbar container.
class ToolbarScope {
public:
    ToolbarScope(Context* ctx, Node* node);
    ~ToolbarScope();

    ToolbarScope(const ToolbarScope&) = delete;
    ToolbarScope& operator=(const ToolbarScope&) = delete;

    ToolbarScope(ToolbarScope&& other) noexcept;
    ToolbarScope& operator=(ToolbarScope&& other) noexcept;

    explicit operator bool() const noexcept { return m_node != nullptr; }

    /// Adds a vertical separator bar between tool groups.
    void Separator(float heightRatio = 0.55f, std::source_location loc = std::source_location::current());

    /// Adds an elastic spacer pushing subsequent tools to the far right.
    void Spacer(std::source_location loc = std::source_location::current());

    /// Adds a compact tool button with icon and optional label.
    Response ToolButton(
        std::string_view label = {},
        ToolButtonOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    /// Returns the underlying Node.
    [[nodiscard]] Node* GetNode() const noexcept { return m_node; }

    /// Direct access to underlying Context.
    [[nodiscard]] Context& Ctx() noexcept { return *m_ctx; }

private:
    Context* m_ctx{nullptr};
    Node* m_node{nullptr};
};

/// Autonomous Toolbar widget logic (measuring, rendering, and tool button utilities).
class Toolbar {
public:
    static ToolbarScope Open(
        Context& ctx,
        ToolbarOptions opts = {},
        std::source_location loc = std::source_location::current()
    );

    static Response ToolButton(
        Context& ctx,
        std::string_view label,
        ToolButtonOptions opts,
        std::source_location loc = std::source_location::current()
    );

    static Vec2 MeasureToolButton(
        std::string_view label,
        const ToolButtonOptions& opts,
        UiSize size,
        ICanvas* canvas = nullptr
    );

    static void RenderToolButton(
        ICanvas& canvas,
        Rect bounds,
        std::string_view label,
        const ToolButtonOptions& opts,
        UiSize size,
        bool hovered,
        bool pressed,
        bool disabled
    );

    static void RenderSeparator(
        ICanvas& canvas,
        Rect bounds,
        Color color = Color::Transparent(),
        ThemeMode theme = ThemeMode::Dark
    );
};

} // namespace kvant
