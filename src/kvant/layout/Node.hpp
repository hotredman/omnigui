#pragma once

#include <cstdint>
#include <string_view>
#include "kvant/core/Id.hpp"
#include "kvant/core/Tokens.hpp"
#include "kvant/core/Types.hpp"
#include "kvant/layout/LayoutTypes.hpp"
#include "kvant/widgets/Button.hpp"
#include "kvant/widgets/Checkbox.hpp"
#include "kvant/widgets/Switch.hpp"
#include "kvant/widgets/Text.hpp"

namespace kvant {

/// Type of element represented by a frame graph node.
enum class NodeType : uint8_t {
    Container,  ///< General flex layout container (Row, Column, Box)
    Text,       ///< Text label
    Button,     ///< Clickable button widget
    Switch,     ///< Sliding pill toggle switch
    Checkbox,   ///< Box toggle with checkmark
    Spacer,     ///< Elastic spacer expanding to fill remaining room
    Custom      ///< User or custom renderer node
};

/// Lightweight, intrusive node in the per-frame layout graph allocated from BumpAllocator.
struct Node {
    // Intrusive tree pointers (O(1) hierarchy without dynamic heap vectors)
    Node* parent{nullptr};
    Node* firstChild{nullptr};
    Node* lastChild{nullptr};
    Node* nextSibling{nullptr};

    uint32_t childIndex{0};     ///< Index of this node within its parent container
    uint32_t childCount{0};     ///< Total number of direct children

    Id id{Id::Null()};          ///< Unique identity
    NodeType type{NodeType::Container};

    // Layout configuration
    FlexDirection direction{FlexDirection::Column};
    SizeMode widthMode{SizeMode::Stretch};
    SizeMode heightMode{SizeMode::Fit};
    float fixedWidth{0.0f};
    float fixedHeight{0.0f};
    float flexGrow{1.0f};       ///< Distribution factor for Stretch/Spacer elements
    float gap{0.0f};            ///< Gap between consecutive children on same line
    float runGap{0.0f};         ///< Vertical gap between consecutive wrapped lines
    Padding padding{};          ///< Internal padding inside container
    Align alignCross{Align::Stretch};
    Justify justifyMain{Justify::Start};
    bool isSpacer{false};
    bool clipChildren{false};

    // Measured and computed bounds
    Vec2 intrinsicSize{0.0f, 0.0f};  ///< Natural content size (bottom-up measure pass)
    Rect computedRect{0.0f, 0.0f, 0.0f, 0.0f}; ///< Final placed coordinates (top-down arrange pass)

    // Visual attributes (rendered by backend)
    Color backgroundColor{0, 0, 0, 0};
    Color borderColor{0, 0, 0, 0};
    float borderWidth{0.0f};
    float cornerRadius{0.0f};

    // Text / Content attributes
    std::string_view text{};
    Color textColor{255, 255, 255, 255};
    float fontSize{14.0f};

    // Ambient state and widget configuration
    UiSize uiSize{UiSize::Md};
    bool disabled{false};
    bool hovered{false};
    bool pressed{false};
    ButtonOptions buttonOpts{};
    TextOptions textOpts{};
    SwitchOptions switchOpts{};
    CheckboxOptions checkboxOpts{};
    bool checked{false};

    /// Appends a child node to this container in O(1).
    void AddChild(Node* child) noexcept {
        if (!child) return;
        child->parent = this;
        child->childIndex = childCount++;
        child->nextSibling = nullptr;

        if (!firstChild) {
            firstChild = child;
            lastChild = child;
        } else {
            lastChild->nextSibling = child;
            lastChild = child;
        }
    }
};

} // namespace kvant
