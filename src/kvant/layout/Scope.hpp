#pragma once

#include <optional>
#include "kvant/core/Tokens.hpp"
#include "kvant/core/Types.hpp"
#include "kvant/layout/LayoutTypes.hpp"
#include "kvant/layout/Node.hpp"

namespace kvant {

class Context;

/// Options for configuring a horizontal Row container.
struct RowOptions {
    float gap{0.0f};
    Padding padding{};
    Alignment align{Alignment::Stretch};
    Justify justify{Justify::Start};
    SizeMode heightMode{SizeMode::Fit};
    std::optional<UiSize> size{std::nullopt};
    bool disabled{false};
};

/// Options for configuring a vertical Column container.
struct ColumnOptions {
    float gap{0.0f};
    Padding padding{};
    Alignment align{Alignment::Stretch};
    Justify justify{Justify::Start};
    SizeMode widthMode{SizeMode::Stretch};
    SizeMode heightMode{SizeMode::Stretch};
    std::optional<UiSize> size{std::nullopt};
    bool disabled{false};
};

/// Options for configuring a visual Box container.
struct BoxOptions {
    float gap{0.0f};
    SizeMode widthMode{SizeMode::Stretch};
    SizeMode heightMode{SizeMode::Fit};
    float fixedWidth{0.0f};
    float fixedHeight{0.0f};
    Padding padding{};
    Color backgroundColor{0, 0, 0, 0};
    Color borderColor{0, 0, 0, 0};
    float borderWidth{0.0f};
    float cornerRadius{0.0f};
    bool clipChildren{false};
    std::optional<UiSize> size{std::nullopt};
    bool disabled{false};
};

/// Options for configuring an auto-wrapping flow container.
struct WrapOptions {
    float gap{0.0f};        ///< Horizontal gap between items on same line
    float runGap{0.0f};     ///< Vertical gap between consecutive wrapped lines
    Padding padding{};
    Alignment align{Alignment::Center};
    Justify justify{Justify::Start};
    SizeMode widthMode{SizeMode::Stretch};
    SizeMode heightMode{SizeMode::Fit};
    std::optional<UiSize> size{std::nullopt};
    bool disabled{false};
};

/// Options for configuring a 12-column responsive grid container.
struct GridOptions {
    int columns{12};            ///< Total number of columns in the grid (default: 12)
    float minColWidth{0.0f};    ///< Minimum column width before wrapping items
    float gap{12.0f};           ///< Horizontal gap between columns
    float runGap{12.0f};        ///< Vertical gap between wrapped rows
    Padding padding{};          ///< Container padding
    Alignment align{Alignment::Stretch};
    Justify justify{Justify::Start};
    SizeMode widthMode{SizeMode::Stretch};
    SizeMode heightMode{SizeMode::Fit};
    std::optional<UiSize> size{std::nullopt};
    bool disabled{false};
};

/// Options for configuring an individual grid cell slot.
struct CellOptions {
    int span{12};               ///< Number of columns occupied by this cell (1..12)
    float minWidth{0.0f};       ///< Minimum width in points before wrapping to full width / next line
    float gap{8.0f};            ///< Gap between stacked children inside this cell
    Padding padding{};          ///< Internal cell padding
    Alignment align{Alignment::Stretch};
    Justify justify{Justify::Start};
    SizeMode heightMode{SizeMode::Fit};
    std::optional<UiSize> size{std::nullopt};
    bool disabled{false};
};

/// Options for configuring a high-level Card container.
struct CardOptions {
    std::string_view subtitle{};
    UiVariant variant{UiVariant::Default};
    float minWidth{0.0f};       ///< Minimum card width constraint
    float minCellWidth{0.0f};   ///< Minimum width for inner grid cells before wrapping
    float gap{12.0f};           ///< Horizontal gap between cells in inner grid
    float runGap{12.0f};        ///< Vertical gap between wrapped cell rows in inner grid
    Padding padding{Padding::All(16.0f)};
    Color backgroundColor{Color::Hex(0x1E293BFF)}; // Slate-800
    Color borderColor{Color::Hex(0x334155FF)};     // Slate-700
    float borderWidth{1.0f};
    float cornerRadius{8.0f};
    bool clipChildren{true};    ///< Default true: clip children to prevent content from bleeding outside card
    bool gridLayout{true};      ///< When true, children are placed in a 12-column grid automatically
    int gridColumns{12};        ///< Number of columns in inner grid
    SizeMode widthMode{SizeMode::Stretch};
    SizeMode heightMode{SizeMode::Fit};
    std::optional<UiSize> size{std::nullopt};
    bool disabled{false};
};

/// Options for configuring a ScrollArea viewport.
struct ScrollAreaOptions {
    Orientation orientation{Orientation::Vertical};
    VisibilityPolicy scrollbar{VisibilityPolicy::Auto};
    Padding padding{};
    std::optional<UiSize> size{std::nullopt};
    bool disabled{false};
};

/// RAII Scope guard for hierarchical layout containers.
class ContainerScope {
public:
    ContainerScope(Context* ctx, Node* node) noexcept
        : m_ctx(ctx), m_node(node) {}

    ~ContainerScope();

    ContainerScope(const ContainerScope&) = delete;
    ContainerScope& operator=(const ContainerScope&) = delete;

    ContainerScope(ContainerScope&& other) noexcept
        : m_ctx(other.m_ctx), m_node(other.m_node) {
        other.m_ctx = nullptr;
        other.m_node = nullptr;
    }

    ContainerScope& operator=(ContainerScope&& other) noexcept {
        if (this != &other) {
            m_ctx = other.m_ctx;
            m_node = other.m_node;
            other.m_ctx = nullptr;
            other.m_node = nullptr;
        }
        return *this;
    }

    /// Evaluates to true allowing `if (auto row = ui.Row()) { ... }` syntax.
    [[nodiscard]] constexpr operator bool() const noexcept {
        return m_node != nullptr;
    }

    [[nodiscard]] Node* GetNode() const noexcept {
        return m_node;
    }

private:
    Context* m_ctx{nullptr};
    Node* m_node{nullptr};
};

/// RAII Scope guard for Card containers managing both frame and inner layout.
class CardScope {
public:
    CardScope(Context* ctx, Node* outerNode, Node* innerNode = nullptr) noexcept
        : m_ctx(ctx), m_outerNode(outerNode), m_innerNode(innerNode) {}

    ~CardScope();

    CardScope(const CardScope&) = delete;
    CardScope& operator=(const CardScope&) = delete;

    CardScope(CardScope&& other) noexcept
        : m_ctx(other.m_ctx), m_outerNode(other.m_outerNode), m_innerNode(other.m_innerNode) {
        other.m_ctx = nullptr;
        other.m_outerNode = nullptr;
        other.m_innerNode = nullptr;
    }

    CardScope& operator=(CardScope&& other) noexcept {
        if (this != &other) {
            m_ctx = other.m_ctx;
            m_outerNode = other.m_outerNode;
            m_innerNode = other.m_innerNode;
            other.m_ctx = nullptr;
            other.m_outerNode = nullptr;
            other.m_innerNode = nullptr;
        }
        return *this;
    }

    [[nodiscard]] constexpr operator bool() const noexcept {
        return m_outerNode != nullptr;
    }

    [[nodiscard]] Node* GetNode() const noexcept {
        return m_innerNode ? m_innerNode : m_outerNode;
    }

private:
    Context* m_ctx{nullptr};
    Node* m_outerNode{nullptr};
    Node* m_innerNode{nullptr};
};

using RowScope = ContainerScope;
using ColumnScope = ContainerScope;
using BoxScope = ContainerScope;
using WrapScope = ContainerScope;
using GridScope = ContainerScope;
using CellScope = ContainerScope;
using ScrollAreaScope = ContainerScope;

} // namespace kvant
