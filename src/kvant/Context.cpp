#include "kvant/Context.hpp"
#include <algorithm>

namespace kvant {

Context::Context() {
    m_containerStack.reserve(16);
    m_sizeStack.reserve(16);
    m_disabledStack.reserve(16);
    m_sizeStack.push_back(UiSize::Md);
    m_disabledStack.push_back(false);
}

void Context::BeginFrame(Vec2 viewportSize, float dpiScale) {
    m_viewportSize = viewportSize;
    m_dpiScale = dpiScale;

    m_arena.Reset();
    m_containerStack.clear();
    m_sizeStack.clear();
    m_disabledStack.clear();
    m_currFrameBounds.clear();

    m_sizeStack.push_back(UiSize::Md);
    m_disabledStack.push_back(false);

    // Create root container spanning the entire viewport
    m_root = m_arena.Allocate<Node>();
    m_root->type = NodeType::Container;
    m_root->direction = FlexDirection::Column;
    m_root->widthMode = SizeMode::Stretch;
    m_root->heightMode = SizeMode::Stretch;
    m_root->id = Id{0xFFFF'FFFF'FFFF'FFFF};
    m_root->uiSize = UiSize::Md;
    m_root->disabled = false;

    m_containerStack.push_back(m_root);
}

Node* Context::CreateNode(NodeType type, Id id) {
    Node* node = m_arena.Allocate<Node>();
    node->type = type;
    node->id = id;

    if (!m_containerStack.empty()) {
        m_containerStack.back()->AddChild(node);
    }
    return node;
}

Id Context::MakeChildId(std::source_location loc) {
    const Id parentId = m_containerStack.empty() ? Id::Null() : m_containerStack.back()->id;
    const uint32_t childIndex = m_containerStack.empty() ? 0 : m_containerStack.back()->childCount;
    return MakeId(childIndex, loc, parentId);
}

Response Context::Interact(Id id) {
    Response res;
    res.id = id;
    res.ctx = this;

    auto it = m_prevFrameBounds.find(id);
    if (it != m_prevFrameBounds.end()) {
        res.rect = it->second;
        res.hovered = res.rect.Contains(m_mousePos);
    }

    if (res.hovered) {
        m_hoveredId = id;
        if (m_mouseClicked) {
            m_activeId = id;
        }
    }

    res.pressed = (m_activeId == id && m_mouseDown);
    res.clicked = (m_activeId == id && m_mouseReleased && res.hovered);

    if (m_mouseReleased && m_activeId == id) {
        m_activeId = Id::Null();
    }

    return res;
}

void Context::AdaptChildToContainer(Node* node) {
    if (!node || m_containerStack.empty()) return;
    Node* parent = m_containerStack.back();
    if (parent->direction == FlexDirection::Column) {
        node->widthMode = SizeMode::Stretch;
        if (parent->heightMode == SizeMode::Fixed) {
            node->heightMode = SizeMode::Stretch;
        }
    }
}

Rect Context::GetPrevFrameBounds(Id id) const {
    auto it = m_prevFrameBounds.find(id);
    if (it != m_prevFrameBounds.end()) {
        return it->second;
    }
    return Rect{};
}

void Context::PushContainer(Node* node) {
    m_containerStack.push_back(node);
}

void Context::PopContainer(Node* node) {
    if (!m_containerStack.empty() && m_containerStack.back() == node) {
        m_containerStack.pop_back();
        PopSize();
        PopDisabled();
    }
}

void Context::PushSize(UiSize size) {
    m_sizeStack.push_back(size);
}

void Context::PopSize() {
    if (m_sizeStack.size() > 1) {
        m_sizeStack.pop_back();
    }
}

UiSize Context::GetCurrentSize() const {
    return m_sizeStack.empty() ? UiSize::Md : m_sizeStack.back();
}

void Context::PushDisabled(bool disabled) {
    const bool current = IsDisabled();
    m_disabledStack.push_back(current || disabled);
}

void Context::PopDisabled() {
    if (m_disabledStack.size() > 1) {
        m_disabledStack.pop_back();
    }
}

bool Context::IsDisabled() const {
    return !m_disabledStack.empty() && m_disabledStack.back();
}

RowScope Context::Row(RowOptions opts, std::source_location loc) {
    const Id id = MakeChildId(loc);
    const UiSize activeSize = opts.size.value_or(GetCurrentSize());
    const bool activeDisabled = opts.disabled || IsDisabled();

    Node* node = CreateNode(NodeType::Container, id);
    node->direction = FlexDirection::Row;
    node->gap = opts.gap;
    node->padding = opts.padding;
    node->alignCross = opts.alignCross;
    node->justifyMain = opts.justifyMain;
    node->heightMode = opts.heightMode;
    node->widthMode = SizeMode::Stretch;
    node->uiSize = activeSize;
    node->disabled = activeDisabled;

    PushSize(activeSize);
    PushDisabled(activeDisabled);
    PushContainer(node);
    return RowScope(this, node);
}

ColumnScope Context::Column(ColumnOptions opts, std::source_location loc) {
    const Id id = MakeChildId(loc);
    const UiSize activeSize = opts.size.value_or(GetCurrentSize());
    const bool activeDisabled = opts.disabled || IsDisabled();

    Node* node = CreateNode(NodeType::Container, id);
    node->direction = FlexDirection::Column;
    node->gap = opts.gap;
    node->padding = opts.padding;
    node->alignCross = opts.alignCross;
    node->justifyMain = opts.justifyMain;
    node->widthMode = opts.widthMode;
    node->heightMode = opts.heightMode;
    node->uiSize = activeSize;
    node->disabled = activeDisabled;

    PushSize(activeSize);
    PushDisabled(activeDisabled);
    PushContainer(node);
    return ColumnScope(this, node);
}

BoxScope Context::Box(BoxOptions opts, std::source_location loc) {
    const Id id = MakeChildId(loc);
    const UiSize activeSize = opts.size.value_or(GetCurrentSize());
    const bool activeDisabled = opts.disabled || IsDisabled();

    Node* node = CreateNode(NodeType::Container, id);
    node->direction = FlexDirection::Column;
    node->gap = opts.gap;
    node->widthMode = opts.widthMode;
    node->heightMode = opts.heightMode;

    if (opts.fixedWidth > 0.0f) {
        node->widthMode = SizeMode::Fixed;
        node->fixedWidth = opts.fixedWidth;
    }
    if (opts.fixedHeight > 0.0f) {
        node->heightMode = SizeMode::Fixed;
        node->fixedHeight = opts.fixedHeight;
    }

    // If explicit size token is set and no fixedHeight was provided, use token height
    if (opts.size.has_value() && opts.fixedHeight <= 0.0f) {
        node->fixedHeight = GetComponentMetrics(*opts.size).defaultHeight;
        node->heightMode = SizeMode::Fixed;
    }

    node->padding = opts.padding;
    node->backgroundColor = opts.backgroundColor;
    node->borderColor = opts.borderColor;
    node->borderWidth = opts.borderWidth;
    node->cornerRadius = opts.cornerRadius;
    node->clipChildren = opts.clipChildren;
    node->uiSize = activeSize;
    node->disabled = activeDisabled;

    PushSize(activeSize);
    PushDisabled(activeDisabled);
    PushContainer(node);
    return BoxScope(this, node);
}

WrapScope Context::Wrap(WrapOptions opts, std::source_location loc) {
    const Id id = MakeChildId(loc);
    const UiSize activeSize = opts.size.value_or(GetCurrentSize());
    const bool activeDisabled = opts.disabled || IsDisabled();

    Node* node = CreateNode(NodeType::Container, id);
    node->direction = FlexDirection::Wrap;
    node->gap = opts.gap;
    node->runGap = (opts.runGap > 0.0f) ? opts.runGap : opts.gap;
    node->padding = opts.padding;
    node->alignCross = opts.alignCross;
    node->justifyMain = opts.justifyMain;
    node->widthMode = opts.widthMode;
    node->heightMode = opts.heightMode;
    node->uiSize = activeSize;
    node->disabled = activeDisabled;

    PushSize(activeSize);
    PushDisabled(activeDisabled);
    PushContainer(node);
    return WrapScope(this, node);
}

void Context::Spacer(float flex, std::source_location loc) {
    const Id id = MakeChildId(loc);
    Node* node = CreateNode(NodeType::Spacer, id);
    node->isSpacer = true;
    node->widthMode = SizeMode::Stretch;
    node->heightMode = SizeMode::Stretch;
    node->flexGrow = std::max(0.01f, flex);
}

Response Context::Button(ButtonOptions opts, std::source_location loc) {
    return Button::Run(*this, opts, loc);
}

Response Context::Button(std::string_view label, ButtonOptions opts, std::source_location loc) {
    opts.label = label;
    return Button::Run(*this, opts, loc);
}

std::string_view Context::InternString(std::string_view text) {
    if (text.empty()) return {};
    char* buf = static_cast<char*>(m_arena.AllocateRaw(text.size() + 1, alignof(char)));
    std::memcpy(buf, text.data(), text.size());
    buf[text.size()] = '\0';
    return std::string_view(buf, text.size());
}

UiString Context::String(std::string_view text) {
    return UiString(InternString(text));
}

void Context::Text(std::string_view label, TextOptions opts, std::source_location loc) {
    const Id id = MakeChildId(loc);
    Node* node = CreateNode(NodeType::Text, id);
    node->text = InternString(label);
    node->textColor = opts.color;
    node->fontSize = opts.fontSize;
    node->widthMode = SizeMode::Fit;
    node->heightMode = SizeMode::Fit;
}

void Context::RequestFocus(Id id) {
    m_focusedId = id;
}

void Context::OnMouseMove(Vec2 pos) {
    m_mousePos = pos;
}

void Context::OnMouseDown(int button) {
    if (button == 1) { // Left mouse button
        m_mouseDown = true;
        m_mouseClicked = true;
    }
}

void Context::OnMouseUp(int button) {
    if (button == 1) { // Left mouse button
        m_mouseDown = false;
        m_mouseReleased = true;
    }
}

static void CollectBoundsRecursive(Node* node, std::unordered_map<Id, Rect>& outBounds) {
    if (!node) return;
    if (node->id.IsValid()) {
        outBounds[node->id] = node->computedRect;
    }
    for (Node* child = node->firstChild; child != nullptr; child = child->nextSibling) {
        CollectBoundsRecursive(child, outBounds);
    }
}

void Context::EndFrame(ICanvas& canvas) {
    // Balance any unclosed container scopes
    while (m_containerStack.size() > 1) {
        m_containerStack.pop_back();
    }

    // Resolve geometry using canvas font metrics
    LayoutSolver::Measure(m_root, &canvas);
    LayoutSolver::Arrange(m_root, Rect(0.0f, 0.0f, m_viewportSize.x, m_viewportSize.y));

    // Cache computed bounds for instantaneous interaction feedback next frame
    CollectBoundsRecursive(m_root, m_currFrameBounds);
    m_prevFrameBounds = std::move(m_currFrameBounds);

    // Reset single-frame mouse impulse states
    m_mouseClicked = false;
    m_mouseReleased = false;

    // Render entire frame graph
    canvas.Begin(static_cast<uint32_t>(m_viewportSize.x), static_cast<uint32_t>(m_viewportSize.y));
    canvas.PushClip(Rect(0.0f, 0.0f, m_viewportSize.x, m_viewportSize.y));
    RenderNodeRecursive(m_root, canvas);
    canvas.PopClip();
    canvas.End();
}

void Context::RenderNodeRecursive(Node* node, ICanvas& canvas) {
    if (!node) return;
    if (node->computedRect.w <= 0.0f || node->computedRect.h <= 0.0f) return;

    if (node->clipChildren) {
        canvas.PushClip(node->computedRect);
    }

    // Render background / border if present (skip for Button as Button::Render handles it)
    if (node->type != NodeType::Button) {
        if (node->backgroundColor.a > 0 || (node->borderColor.a > 0 && node->borderWidth > 0.0f)) {
            if (node->cornerRadius > 0.0f) {
                canvas.DrawRoundRect(
                    node->computedRect,
                    node->cornerRadius,
                    node->backgroundColor,
                    node->borderColor,
                    node->borderWidth
                );
            } else {
                canvas.DrawRect(node->computedRect, node->backgroundColor);
            }
        }
    }

    // Render specific widget contents
    if (node->type == NodeType::Text && !node->text.empty()) {
        canvas.DrawTextBox(node->text, node->computedRect, node->fontSize, node->textColor, 0.0f, 0.5f);
    } else if (node->type == NodeType::Button) {
        Button::Render(
            canvas,
            node->computedRect,
            node->buttonOpts,
            node->uiSize,
            node->hovered,
            node->pressed,
            node->disabled
        );
    }

    // Recurse on children
    for (Node* child = node->firstChild; child != nullptr; child = child->nextSibling) {
        RenderNodeRecursive(child, canvas);
    }

    if (node->clipChildren) {
        canvas.PopClip();
    }
}

} // namespace kvant
