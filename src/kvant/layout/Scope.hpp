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

using RowScope = ContainerScope;
using ColumnScope = ContainerScope;
using BoxScope = ContainerScope;
using WrapScope = ContainerScope;

} // namespace kvant
