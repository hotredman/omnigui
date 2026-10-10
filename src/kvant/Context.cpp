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

    if (!m_manualTime) {
        const auto now = std::chrono::steady_clock::now();
        m_currentTimeSec = std::chrono::duration<double>(now - m_startTime).count();
    }

    m_overlayPasses.clear();

    if (m_mouseClicked && m_openPopupId.IsValid()) {
        auto it = m_prevFrameBounds.find(m_openPopupId);
        const bool overSelect = (it != m_prevFrameBounds.end()) && it->second.Contains(m_mousePos);
        const bool overPopup = m_openPopupRect.Contains(m_mousePos);
        if (!overSelect && !overPopup) {
            ClosePopup();
            ConsumeMouseClick();
        }
    }

    if (m_mouseClicked && m_focusedId.IsValid()) {
        auto it = m_prevFrameBounds.find(m_focusedId);
        if (it != m_prevFrameBounds.end()) {
            if (!it->second.Contains(m_mousePos)) {
                m_focusedId = Id::Null();
            }
        } else {
            m_focusedId = Id::Null();
        }
    }

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

    // Suppress interaction on background widgets occluded by a floating overlay popup
    if (m_openPopupId.IsValid() && id != m_openPopupId && m_openPopupRect.Contains(m_mousePos)) {
        return res;
    }

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
    if (parent->direction == FlexDirection::Column || parent->direction == FlexDirection::Grid) {
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
    node->align = opts.align;
    node->justify = opts.justify;
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
    node->align = opts.align;
    node->justify = opts.justify;
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
    node->align = opts.align;
    node->justify = opts.justify;
    node->widthMode = opts.widthMode;
    node->heightMode = opts.heightMode;
    node->uiSize = activeSize;
    node->disabled = activeDisabled;

    PushSize(activeSize);
    PushDisabled(activeDisabled);
    PushContainer(node);
    return WrapScope(this, node);
}

GridScope Context::Grid(GridOptions opts, std::source_location loc) {
    const Id id = MakeChildId(loc);
    const UiSize activeSize = opts.size.value_or(GetCurrentSize());
    const bool activeDisabled = opts.disabled || IsDisabled();

    Node* node = CreateNode(NodeType::Container, id);
    node->direction = FlexDirection::Grid;
    node->gridColumns = (opts.columns > 0) ? opts.columns : 12;
    node->minColWidth = opts.minColWidth;
    node->gap = opts.gap;
    node->runGap = (opts.runGap > 0.0f) ? opts.runGap : opts.gap;
    node->padding = opts.padding;
    node->align = opts.align;
    node->justify = opts.justify;
    node->widthMode = opts.widthMode;
    node->heightMode = opts.heightMode;
    node->uiSize = activeSize;
    node->disabled = activeDisabled;

    PushSize(activeSize);
    PushDisabled(activeDisabled);
    PushContainer(node);
    return GridScope(this, node);
}

ScrollAreaScope Context::ScrollArea(ScrollAreaOptions opts, std::source_location loc) {
    const Id id = MakeChildId(loc);
    const UiSize activeSize = opts.size.value_or(GetCurrentSize());
    const bool activeDisabled = opts.disabled || IsDisabled();

    Node* node = CreateNode(NodeType::ScrollArea, id);
    node->direction = FlexDirection::Column;
    node->clipChildren = true;
    node->widthMode = SizeMode::Stretch;
    node->heightMode = SizeMode::Stretch;
    node->flexGrow = 1.0f;
    node->padding = opts.padding;
    node->scrollOrientation = opts.orientation;
    node->scrollbarPolicy = opts.scrollbar;
    node->uiSize = activeSize;
    node->disabled = activeDisabled;

    // Retrieve persistent scroll state
    auto& state = m_scrollStates[id];

    // Handle immediate mouse wheel scroll if hovered on previous frame bounds
    auto itPrev = m_prevFrameBounds.find(id);
    if (itPrev != m_prevFrameBounds.end() && !node->disabled) {
        if (itPrev->second.Contains(m_mousePos) && (m_mouseWheel.x != 0.0f || m_mouseWheel.y != 0.0f)) {
            const ScrollbarMetrics m = GetScrollbarMetrics(activeSize);
            state.offset.y -= m_mouseWheel.y * m.scrollStep;
            state.offset.x -= m_mouseWheel.x * m.scrollStep;
            if (state.offset.y < 0.0f) state.offset.y = 0.0f;
            if (state.offset.x < 0.0f) state.offset.x = 0.0f;
        }
    }

    node->scrollOffset = state.offset;

    PushSize(activeSize);
    PushDisabled(activeDisabled);
    PushContainer(node);
    return ScrollAreaScope(this, node);
}

CardScope Context::Card(std::string_view title, CardOptions opts, std::source_location loc) {
    const Id idOuter = MakeChildId(loc);
    const UiSize activeSize = opts.size.value_or(GetCurrentSize());
    const bool activeDisabled = opts.disabled || IsDisabled();

    // 1. Outer styled container frame
    Node* outerNode = CreateNode(NodeType::Container, idOuter);
    outerNode->direction = FlexDirection::Column;
    outerNode->gap = 12.0f;
    outerNode->minWidth = opts.minWidth;
    outerNode->widthMode = opts.widthMode;
    outerNode->heightMode = opts.heightMode;
    outerNode->padding = opts.padding;
    outerNode->backgroundColor = opts.backgroundColor;
    outerNode->borderColor = opts.borderColor;
    outerNode->borderWidth = opts.borderWidth;
    outerNode->cornerRadius = opts.cornerRadius;
    outerNode->clipChildren = opts.clipChildren;
    outerNode->uiSize = activeSize;
    outerNode->disabled = activeDisabled;

    PushSize(activeSize);
    PushDisabled(activeDisabled);
    PushContainer(outerNode);

    // 2. Optional Header block
    if (!title.empty() || !opts.subtitle.empty()) {
        const Id idHeader = Id{idOuter.value ^ 0x517CC1B727220A95ULL};
        Node* headerNode = CreateNode(NodeType::Container, idHeader);
        headerNode->direction = FlexDirection::Column;
        headerNode->gap = 2.0f;
        headerNode->widthMode = SizeMode::Stretch;
        headerNode->heightMode = SizeMode::Fit;
        headerNode->uiSize = activeSize;
        headerNode->disabled = activeDisabled;

        PushSize(activeSize);
        PushDisabled(activeDisabled);
        PushContainer(headerNode);

        if (!title.empty()) {
            Text(title, {.role = TextRole::TitleMedium, .variant = opts.variant});
        }
        if (!opts.subtitle.empty()) {
            Caption(opts.subtitle, UiVariant::Muted);
        }

        PopContainer(headerNode);
    }

    // 3. Inner layout container (Grid if gridLayout == true)
    Node* innerNode = nullptr;
    if (opts.gridLayout) {
        const Id idInner = Id{idOuter.value ^ 0x9E3779B97F4A7C15ULL};
        innerNode = CreateNode(NodeType::Container, idInner);
        innerNode->direction = FlexDirection::Grid;
        innerNode->gridColumns = (opts.gridColumns > 0) ? opts.gridColumns : 12;
        innerNode->minColWidth = opts.minCellWidth;
        innerNode->gap = opts.gap;
        innerNode->runGap = (opts.runGap > 0.0f) ? opts.runGap : opts.gap;
        innerNode->widthMode = SizeMode::Stretch;
        innerNode->heightMode = SizeMode::Fit;
        innerNode->uiSize = activeSize;
        innerNode->disabled = activeDisabled;

        PushSize(activeSize);
        PushDisabled(activeDisabled);
        PushContainer(innerNode);
    }

    return CardScope(this, outerNode, innerNode);
}

CardScope Context::Card(CardOptions opts, std::source_location loc) {
    return Card({}, opts, loc);
}

CellScope Context::Cell(int span, CellOptions opts, std::source_location loc) {
    const Id id = MakeChildId(loc);
    const UiSize activeSize = opts.size.value_or(GetCurrentSize());
    const bool activeDisabled = opts.disabled || IsDisabled();

    Node* node = CreateNode(NodeType::Container, id);
    node->direction = FlexDirection::Column;
    node->colSpan = std::clamp(span, 1, 12);
    node->minWidth = opts.minWidth;
    node->clipChildren = true;
    node->gap = opts.gap;
    node->padding = opts.padding;
    node->align = opts.align;
    node->justify = opts.justify;
    node->widthMode = SizeMode::Stretch;
    node->heightMode = opts.heightMode;
    node->uiSize = activeSize;
    node->disabled = activeDisabled;

    PushSize(activeSize);
    PushDisabled(activeDisabled);
    PushContainer(node);
    return CellScope(this, node);
}

CellScope Context::Cell(CellOptions opts, std::source_location loc) {
    return Cell(opts.span, opts, loc);
}

CellScope Context::Cell12(CellOptions opts, std::source_location loc) {
    return Cell(12, opts, loc);
}

CellScope Context::Cell8(CellOptions opts, std::source_location loc) {
    return Cell(8, opts, loc);
}

CellScope Context::Cell6(CellOptions opts, std::source_location loc) {
    return Cell(6, opts, loc);
}

CellScope Context::Cell4(CellOptions opts, std::source_location loc) {
    return Cell(4, opts, loc);
}

CellScope Context::Cell3(CellOptions opts, std::source_location loc) {
    return Cell(3, opts, loc);
}

CellScope Context::Cell2(CellOptions opts, std::source_location loc) {
    return Cell(2, opts, loc);
}

CellScope Context::Cell1(CellOptions opts, std::source_location loc) {
    return Cell(1, opts, loc);
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

UiString Context::String(std::string_view text) {
    return UiString(InternString(text));
}

std::string_view Context::InternString(std::string_view text) {
    if (text.empty()) return {};
    char* buf = static_cast<char*>(m_arena.AllocateRaw(text.size() + 1, alignof(char)));
    std::memcpy(buf, text.data(), text.size());
    buf[text.size()] = '\0';
    return std::string_view(buf, text.size());
}

Response Context::Text(std::string_view label, TextOptions opts, std::source_location loc) {
    return Text::Run(*this, label, opts, loc);
}

Response Context::Display(std::string_view label, UiVariant variant, std::source_location loc) {
    return Text(label, {.role = TextRole::DisplayLarge, .variant = variant}, loc);
}

Response Context::Headline(std::string_view label, UiVariant variant, std::source_location loc) {
    return Text(label, {.role = TextRole::HeadlineMedium, .variant = variant}, loc);
}

Response Context::Title(std::string_view label, UiVariant variant, std::source_location loc) {
    return Text(label, {.role = TextRole::TitleLarge, .variant = variant}, loc);
}

Response Context::Subtitle(std::string_view label, UiVariant variant, std::source_location loc) {
    return Text(label, {.role = TextRole::TitleMedium, .variant = variant}, loc);
}

Response Context::Body(std::string_view label, UiVariant variant, std::source_location loc) {
    return Text(label, {.role = TextRole::BodyMedium, .variant = variant}, loc);
}

Response Context::Caption(std::string_view label, UiVariant variant, std::source_location loc) {
    return Text(label, {.role = TextRole::LabelMedium, .variant = variant}, loc);
}

Response Context::Code(std::string_view label, UiVariant variant, std::source_location loc) {
    return Text(label, {.role = TextRole::CodeMedium, .variant = variant}, loc);
}

Response Context::Switch(std::string_view label, bool* value, SwitchOptions opts, std::source_location loc) {
    return Switch::Run(*this, label, value, value ? *value : false, opts, loc);
}

Response Context::Switch(bool* value, SwitchOptions opts, std::source_location loc) {
    return Switch::Run(*this, {}, value, value ? *value : false, opts, loc);
}

Response Context::Switch(std::string_view label, bool checked, SwitchOptions opts, std::source_location loc) {
    return Switch::Run(*this, label, nullptr, checked, opts, loc);
}

Response Context::Switch(bool checked, SwitchOptions opts, std::source_location loc) {
    return Switch::Run(*this, {}, nullptr, checked, opts, loc);
}

Response Context::Checkbox(std::string_view label, bool* value, CheckboxOptions opts, std::source_location loc) {
    return Checkbox::Run(*this, label, value, value ? *value : false, opts, loc);
}

Response Context::Checkbox(bool* value, CheckboxOptions opts, std::source_location loc) {
    return Checkbox::Run(*this, {}, value, value ? *value : false, opts, loc);
}

Response Context::Checkbox(std::string_view label, bool checked, CheckboxOptions opts, std::source_location loc) {
    return Checkbox::Run(*this, label, nullptr, checked, opts, loc);
}

Response Context::Checkbox(bool checked, CheckboxOptions opts, std::source_location loc) {
    return Checkbox::Run(*this, {}, nullptr, checked, opts, loc);
}

Response Context::InputField(std::string_view label, std::string& value, InputFieldOptions opts, std::source_location loc) {
    return InputField::Run(*this, label, &value, nullptr, nullptr, nullptr, {}, opts, loc);
}

Response Context::InputField(std::string& value, InputFieldOptions opts, std::source_location loc) {
    return InputField::Run(*this, {}, &value, nullptr, nullptr, nullptr, {}, opts, loc);
}

Response Context::InputField(std::string_view label, float& value, InputFieldOptions opts, std::source_location loc) {
    return InputField::Run(*this, label, nullptr, &value, nullptr, nullptr, {}, opts, loc);
}

Response Context::InputField(float& value, InputFieldOptions opts, std::source_location loc) {
    return InputField::Run(*this, {}, nullptr, &value, nullptr, nullptr, {}, opts, loc);
}

Response Context::InputField(std::string_view label, double& value, InputFieldOptions opts, std::source_location loc) {
    return InputField::Run(*this, label, nullptr, nullptr, &value, nullptr, {}, opts, loc);
}

Response Context::InputField(double& value, InputFieldOptions opts, std::source_location loc) {
    return InputField::Run(*this, {}, nullptr, nullptr, &value, nullptr, {}, opts, loc);
}

Response Context::InputField(std::string_view label, int& value, InputFieldOptions opts, std::source_location loc) {
    return InputField::Run(*this, label, nullptr, nullptr, nullptr, &value, {}, opts, loc);
}

Response Context::InputField(int& value, InputFieldOptions opts, std::source_location loc) {
    return InputField::Run(*this, {}, nullptr, nullptr, nullptr, &value, {}, opts, loc);
}

Response Context::InputField(std::string_view label, std::string_view value, InputFieldOptions opts, std::source_location loc) {
    return InputField::Run(*this, label, nullptr, nullptr, nullptr, nullptr, value, opts, loc);
}

Response Context::Select(std::string_view label, size_t& selectedIndex, std::span<const std::string_view> items, SelectOptions opts, std::source_location loc) {
    return Select::Run(*this, label, &selectedIndex, nullptr, nullptr, nullptr, items, opts, loc);
}

Response Context::Select(size_t& selectedIndex, std::span<const std::string_view> items, SelectOptions opts, std::source_location loc) {
    return Select::Run(*this, {}, &selectedIndex, nullptr, nullptr, nullptr, items, opts, loc);
}

Response Context::Select(std::string_view label, int& selectedIndex, std::span<const std::string_view> items, SelectOptions opts, std::source_location loc) {
    return Select::Run(*this, label, nullptr, &selectedIndex, nullptr, nullptr, items, opts, loc);
}

Response Context::Select(int& selectedIndex, std::span<const std::string_view> items, SelectOptions opts, std::source_location loc) {
    return Select::Run(*this, {}, nullptr, &selectedIndex, nullptr, nullptr, items, opts, loc);
}

Response Context::Select(std::string_view label, std::string& selectedValue, std::span<const std::string_view> items, SelectOptions opts, std::source_location loc) {
    return Select::Run(*this, label, nullptr, nullptr, &selectedValue, nullptr, items, opts, loc);
}

Response Context::Select(std::string& selectedValue, std::span<const std::string_view> items, SelectOptions opts, std::source_location loc) {
    return Select::Run(*this, {}, nullptr, nullptr, &selectedValue, nullptr, items, opts, loc);
}

Response Context::Select(std::string_view label, std::string_view& selectedValue, std::span<const std::string_view> items, SelectOptions opts, std::source_location loc) {
    return Select::Run(*this, label, nullptr, nullptr, nullptr, &selectedValue, items, opts, loc);
}

Response Context::Select(std::string_view& selectedValue, std::span<const std::string_view> items, SelectOptions opts, std::source_location loc) {
    return Select::Run(*this, {}, nullptr, nullptr, nullptr, &selectedValue, items, opts, loc);
}

void Context::RequestFocus(Id id) {
    m_focusedId = id;
    ResetCursorBlink();
}

void Context::RequestRepaint(int frames) noexcept {
    m_repaintFrames = std::max(m_repaintFrames, frames);
}

bool Context::NeedsRepaint() const noexcept {
    if (m_repaintFrames > 0) return true;
    if (m_mouseDown) return true;
    if (m_activeId.IsValid()) return true;
    if (m_openPopupId.IsValid()) return true;
    return false;
}

int Context::GetRecommendedSleepTimeoutMs() const noexcept {
    if (NeedsRepaint()) {
        return 0;
    }
    if (m_editState.id.IsValid() && m_cursorConfig.mode == CursorBlinkMode::Blinking) {
        return 120; // Periodically wake up for smooth cursor blinking
    }
    return m_idleHeartbeatMs; // User-configurable heartbeat interval (or <= 0 for deep sleep)
}

void Context::OnMouseMove(Vec2 pos) {
    m_mousePos = pos;
    RequestRepaint(3);
}

void Context::OnMouseDown(int button) {
    if (button == 1) { // Left mouse button
        m_mouseDown = true;
        m_mouseClicked = true;
        ResetCursorBlink();
        RequestRepaint(3);
    }
}

void Context::OnMouseUp(int button) {
    if (button == 1) { // Left mouse button
        m_mouseDown = false;
        m_mouseReleased = true;
        RequestRepaint(3);
    }
}

void Context::OnMouseWheel(float x, float y) {
    m_mouseWheel = {x, y};
    RequestRepaint(3);
}

void Context::OnTextInput(std::string_view utf8Text) {
    ResetCursorBlink();
    RequestRepaint(3);
    if (m_focusedId.IsValid() && m_editState.id == m_focusedId) {
        if (m_editState.cursor <= m_editState.text.size()) {
            m_editState.text.insert(m_editState.cursor, utf8Text);
            m_editState.cursor += utf8Text.size();
            m_editState.changed = true;
        }
    } else {
        m_pendingTextInput.emplace_back(utf8Text);
    }
}

void Context::OnKeyDown(Key key, KeyModifiers mods) {
    ResetCursorBlink();
    RequestRepaint(3);

    if (key == Key::Escape && m_openPopupId.IsValid()) {
        ClosePopup();
    }

    if (m_focusedId.IsValid() && m_editState.id == m_focusedId) {
        switch (key) {
            case Key::Backspace:
                if (m_editState.cursor > 0 && !m_editState.text.empty()) {
                    m_editState.text.erase(m_editState.cursor - 1, 1);
                    m_editState.cursor--;
                    m_editState.changed = true;
                }
                break;
            case Key::Delete:
                if (m_editState.cursor < m_editState.text.size()) {
                    m_editState.text.erase(m_editState.cursor, 1);
                    m_editState.changed = true;
                }
                break;
            case Key::Left:
                if (m_editState.cursor > 0) {
                    m_editState.cursor--;
                }
                break;
            case Key::Right:
                if (m_editState.cursor < m_editState.text.size()) {
                    m_editState.cursor++;
                }
                break;
            case Key::Home:
                m_editState.cursor = 0;
                break;
            case Key::End:
                m_editState.cursor = m_editState.text.size();
                break;
            case Key::Enter:
                m_editState.submitted = true;
                m_focusedId = Id::Null();
                break;
            case Key::Escape:
                m_focusedId = Id::Null();
                m_editState.id = Id::Null();
                break;
            default:
                break;
        }
    } else {
        m_pendingKeyEvents.push_back({key, mods});
    }
}

void Context::OpenPopup(Id id, Rect bounds) {
    m_openPopupId = id;
    m_openPopupRect = bounds;
}

void Context::ClosePopup() {
    m_openPopupId = Id::Null();
    m_openPopupRect = Rect{};
}

void Context::RegisterOverlay(std::function<void(ICanvas&)> drawFn) {
    m_overlayPasses.push_back(std::move(drawFn));
}

bool Context::ConsumeKey(Key key) {
    for (auto it = m_pendingKeyEvents.begin(); it != m_pendingKeyEvents.end(); ++it) {
        if (it->key == key) {
            m_pendingKeyEvents.erase(it);
            return true;
        }
    }
    return false;
}

std::string_view Context::GetOrStartTextEdit(
    Id id,
    std::string_view initialText,
    size_t& outCursor,
    bool& outChanged,
    bool& outSubmitted
) {
    if (m_editState.id != id) {
        m_editState.id = id;
        m_editState.text = initialText;
        m_editState.cursor = m_editState.text.size();
        m_editState.changed = false;
        m_editState.submitted = false;
    }

    for (const auto& txt : m_pendingTextInput) {
        if (m_editState.cursor <= m_editState.text.size()) {
            m_editState.text.insert(m_editState.cursor, txt);
            m_editState.cursor += txt.size();
            m_editState.changed = true;
        }
    }
    m_pendingTextInput.clear();

    for (const auto& ev : m_pendingKeyEvents) {
        switch (ev.key) {
            case Key::Backspace:
                if (m_editState.cursor > 0 && !m_editState.text.empty()) {
                    m_editState.text.erase(m_editState.cursor - 1, 1);
                    m_editState.cursor--;
                    m_editState.changed = true;
                }
                break;
            case Key::Delete:
                if (m_editState.cursor < m_editState.text.size()) {
                    m_editState.text.erase(m_editState.cursor, 1);
                    m_editState.changed = true;
                }
                break;
            case Key::Left:
                if (m_editState.cursor > 0) m_editState.cursor--;
                break;
            case Key::Right:
                if (m_editState.cursor < m_editState.text.size()) m_editState.cursor++;
                break;
            case Key::Home:
                m_editState.cursor = 0;
                break;
            case Key::End:
                m_editState.cursor = m_editState.text.size();
                break;
            case Key::Enter:
                m_editState.submitted = true;
                m_focusedId = Id::Null();
                break;
            case Key::Escape:
                m_focusedId = Id::Null();
                m_editState.id = Id::Null();
                break;
            default:
                break;
        }
    }
    m_pendingKeyEvents.clear();

    outCursor = m_editState.cursor;
    outChanged = m_editState.changed;
    outSubmitted = m_editState.submitted;

    m_editState.changed = false;
    m_editState.submitted = false;
    if (outSubmitted) {
        m_editState.id = Id::Null();
    }

    return m_editState.text;
}

void Context::ResetCursorBlink() noexcept {
    m_lastInputTimeSec = m_currentTimeSec;
}

void Context::SetCurrentTime(double timeSeconds) noexcept {
    m_manualTime = true;
    m_currentTimeSec = timeSeconds;
}

void Context::AdvanceTime(double deltaSeconds) noexcept {
    m_manualTime = true;
    m_currentTimeSec += deltaSeconds;
}

bool Context::IsCursorVisible(std::optional<CursorBlinkMode> modeOverride) const noexcept {
    const CursorBlinkMode mode = modeOverride.value_or(m_cursorConfig.mode);
    switch (mode) {
        case CursorBlinkMode::Hidden:
            return false;
        case CursorBlinkMode::Solid:
            return true;
        case CursorBlinkMode::Blinking: {
            const double elapsedSinceInput = m_currentTimeSec - m_lastInputTimeSec;
            // Typing reset phase: cursor remains solid/visible for 0.5s after user interaction
            if (elapsedSinceInput >= 0.0 && elapsedSinceInput < 0.5) {
                return true;
            }
            if (m_cursorConfig.blinkPeriod <= 0.0f) {
                return true;
            }
            const double cycleTime = elapsedSinceInput - 0.5;
            const double phase = std::fmod(cycleTime, static_cast<double>(m_cursorConfig.blinkPeriod));
            return phase < (static_cast<double>(m_cursorConfig.blinkPeriod) * 0.5);
        }
    }
    return true;
}

static void CollectBoundsRecursive(Node* node, std::unordered_map<Id, Rect>& outBounds, Rect parentClip) {
    if (!node) return;

    Rect effectiveClip = parentClip;
    if (node->clipChildren) {
        if (node->type == NodeType::ScrollArea) {
            const ScrollbarMetrics sm = GetScrollbarMetrics(node->uiSize);
            const bool showScrollbar = (node->scrollbarPolicy == VisibilityPolicy::Always) ||
                (node->scrollbarPolicy == VisibilityPolicy::Auto && node->scrollContentSize.y > node->computedRect.h);
            const float gutter = (showScrollbar && node->scrollOrientation != Orientation::Horizontal) ? sm.gutter : 0.0f;
            const Rect viewRect(
                node->computedRect.x + node->padding.left,
                node->computedRect.y + node->padding.top,
                std::max(0.0f, node->computedRect.w - node->padding.Horizontal() - gutter),
                std::max(0.0f, node->computedRect.h - node->padding.Vertical())
            );
            effectiveClip = parentClip.Intersect(viewRect);
        } else {
            effectiveClip = parentClip.Intersect(node->computedRect);
        }
    }

    if (node->id.IsValid()) {
        const Rect clipped = node->computedRect.Intersect(effectiveClip);
        if (clipped.w > 0.0f && clipped.h > 0.0f) {
            outBounds[node->id] = clipped;
        }
    }

    for (Node* child = node->firstChild; child != nullptr; child = child->nextSibling) {
        CollectBoundsRecursive(child, outBounds, effectiveClip);
    }
}

void Context::UpdateScrollAreasRecursive(Node* node) {
    if (!node) return;

    if (node->type == NodeType::ScrollArea && node->id.IsValid()) {
        auto& state = m_scrollStates[node->id];
        const ScrollbarMetrics m = GetScrollbarMetrics(node->uiSize);

        const float viewportW = node->computedRect.w;
        const float viewportH = node->computedRect.h;
        const float contentW = node->scrollContentSize.x;
        const float contentH = node->scrollContentSize.y;

        const float maxScrollY = std::max(0.0f, contentH - viewportH);
        const float maxScrollX = std::max(0.0f, contentW - viewportW);

        state.offset.y = std::clamp(state.offset.y, 0.0f, maxScrollY);
        state.offset.x = std::clamp(state.offset.x, 0.0f, maxScrollX);
        node->scrollOffset = state.offset;

        // Thumb geometry for vertical scroll
        if (viewportH > 10.0f && maxScrollY > 0.0f) {
            const float trackX = node->computedRect.Right() - m.trackWidth - m.margin;
            const float trackY = node->computedRect.y + m.margin;
            const float trackH = std::max(0.0f, viewportH - 2.0f * m.margin);

            const float thumbH = std::clamp((viewportH / contentH) * trackH, m.minThumbLength, trackH);
            const float travelH = trackH - thumbH;
            const float scrollRatio = (maxScrollY > 0.0f) ? (state.offset.y / maxScrollY) : 0.0f;
            const float thumbY = trackY + scrollRatio * travelH;
            const Rect trackRect(trackX, trackY, m.trackWidth, trackH);
            const Rect thumbRect(trackX, thumbY, m.trackWidth, thumbH);

            const bool hoverThumb = thumbRect.Contains(m_mousePos);
            const bool hoverTrack = trackRect.Contains(m_mousePos);
            node->scrollbarHovered = hoverThumb || hoverTrack;

            if (hoverThumb && m_mouseClicked) {
                state.draggingY = true;
                state.dragStartMouse = m_mousePos;
                state.dragStartOffset = state.offset.y;
                m_activeId = node->id;
            } else if (hoverTrack && m_mouseClicked && !hoverThumb) {
                // Page jump towards click position on track
                if (m_mousePos.y < thumbY) {
                    state.offset.y = std::max(0.0f, state.offset.y - viewportH * 0.8f);
                } else {
                    state.offset.y = std::min(maxScrollY, state.offset.y + viewportH * 0.8f);
                }
                node->scrollOffset = state.offset;
            }

            if (state.draggingY) {
                node->scrollbarActive = true;
                if (m_mouseDown) {
                    const float deltaY = m_mousePos.y - state.dragStartMouse.y;
                    if (travelH > 0.0f) {
                        state.offset.y = std::clamp(state.dragStartOffset + (deltaY / travelH) * maxScrollY, 0.0f, maxScrollY);
                        node->scrollOffset = state.offset;
                    }
                } else {
                    state.draggingY = false;
                    if (m_activeId == node->id) {
                        m_activeId = Id::Null();
                    }
                }
            }
        } else {
            state.draggingY = false;
        }
    }

    for (Node* child = node->firstChild; child != nullptr; child = child->nextSibling) {
        UpdateScrollAreasRecursive(child);
    }
}

void Context::ScrollToId(Id id) {
    if (!id.IsValid() || !m_root) return;

    Node* target = nullptr;
    std::function<void(Node*)> findNode = [&](Node* n) {
        if (!n || target) return;
        if (n->id == id) {
            target = n;
            return;
        }
        for (Node* c = n->firstChild; c != nullptr; c = c->nextSibling) {
            findNode(c);
        }
    };
    findNode(m_root);

    if (!target) return;

    for (Node* p = target->parent; p != nullptr; p = p->parent) {
        if (p->type == NodeType::ScrollArea && p->id.IsValid()) {
            auto& state = m_scrollStates[p->id];
            const float targetTop = target->computedRect.y;
            const float targetBottom = target->computedRect.Bottom();
            const float viewTop = p->computedRect.y;
            const float viewBottom = p->computedRect.Bottom();

            if (targetTop < viewTop) {
                state.offset.y -= (viewTop - targetTop);
            } else if (targetBottom > viewBottom) {
                state.offset.y += (targetBottom - viewBottom);
            }
            if (state.offset.y < 0.0f) state.offset.y = 0.0f;
            p->scrollOffset = state.offset;
            break;
        }
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

    // Update interactive scroll areas
    UpdateScrollAreasRecursive(m_root);

    // Cache computed bounds for instantaneous interaction feedback next frame
    CollectBoundsRecursive(m_root, m_currFrameBounds, Rect(0.0f, 0.0f, m_viewportSize.x, m_viewportSize.y));
    m_prevFrameBounds = std::move(m_currFrameBounds);

    // Reset single-frame mouse impulse states
    m_mouseClicked = false;
    m_mouseReleased = false;
    m_mouseWheel = {0.0f, 0.0f};

    // Render entire frame graph
    canvas.Begin(static_cast<uint32_t>(m_viewportSize.x), static_cast<uint32_t>(m_viewportSize.y));
    canvas.PushClip(Rect(0.0f, 0.0f, m_viewportSize.x, m_viewportSize.y));
    RenderNodeRecursive(m_root, canvas);

    // Render floating overlay passes (e.g. dropdown popups, tooltips)
    for (const auto& overlayFn : m_overlayPasses) {
        overlayFn(canvas);
    }

    canvas.PopClip();
    canvas.End();

    m_pendingKeyEvents.clear();
    m_pendingTextInput.clear();

    if (m_repaintFrames > 0) {
        --m_repaintFrames;
    }
}

void Context::RenderNodeRecursive(Node* node, ICanvas& canvas) {
    if (!node) return;
    if (node->computedRect.w <= 0.0f || node->computedRect.h <= 0.0f) return;

    if (node->clipChildren) {
        if (node->type == NodeType::ScrollArea) {
            const ScrollbarMetrics sm = GetScrollbarMetrics(node->uiSize);
            const bool showScrollbar = (node->scrollbarPolicy == VisibilityPolicy::Always) ||
                (node->scrollbarPolicy == VisibilityPolicy::Auto && node->scrollContentSize.y > node->computedRect.h);
            const float gutter = (showScrollbar && node->scrollOrientation != Orientation::Horizontal) ? sm.gutter : 0.0f;
            const Rect clipRect(
                node->computedRect.x + node->padding.left,
                node->computedRect.y + node->padding.top,
                std::max(0.0f, node->computedRect.w - node->padding.Horizontal() - gutter),
                std::max(0.0f, node->computedRect.h - node->padding.Vertical())
            );
            canvas.PushClip(clipRect);
        } else {
            canvas.PushClip(node->computedRect);
        }
    }

    // Render background / border if present (skip for self-rendering widgets)
    if (node->type != NodeType::Button && node->type != NodeType::Text &&
        node->type != NodeType::Switch && node->type != NodeType::Checkbox &&
        node->type != NodeType::InputField && node->type != NodeType::Select) {
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
        Text::Render(
            canvas,
            node->computedRect,
            node->text,
            node->textOpts,
            node->hovered,
            node->disabled
        );
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
    } else if (node->type == NodeType::Switch) {
        Switch::Render(
            canvas,
            node->computedRect,
            node->text,
            node->checked,
            node->switchOpts,
            node->uiSize,
            node->hovered,
            node->pressed,
            node->disabled
        );
    } else if (node->type == NodeType::Checkbox) {
        Checkbox::Render(
            canvas,
            node->computedRect,
            node->text,
            node->checked,
            node->checkboxOpts,
            node->uiSize,
            node->hovered,
            node->pressed,
            node->disabled
        );
    } else if (node->type == NodeType::InputField) {
        InputField::Render(
            canvas,
            node->computedRect,
            node->text,
            node->displayText,
            node->inputOpts,
            node->uiSize,
            node->hovered,
            node->focused,
            node->cursor,
            node->disabled,
            node->cursorVisible,
            node->cursorWidth
        );
    } else if (node->type == NodeType::Select) {
        Select::Render(
            canvas,
            node->computedRect,
            node->text,
            node->displayText,
            node->selectOpts,
            node->uiSize,
            node->hovered,
            node->focused,
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

    // Render scrollbar for ScrollArea
    if (node->type == NodeType::ScrollArea) {
        bool showScrollbar = false;
        if (node->scrollbarPolicy == VisibilityPolicy::Always) {
            showScrollbar = true;
        } else if (node->scrollbarPolicy == VisibilityPolicy::Auto) {
            showScrollbar = (node->scrollContentSize.y > node->computedRect.h);
        }

        if (showScrollbar && node->computedRect.h > 10.0f) {
            const ScrollbarMetrics m = GetScrollbarMetrics(node->uiSize);
            const float trackX = node->computedRect.Right() - m.trackWidth - m.margin;
            const float trackY = node->computedRect.y + m.margin;
            const float trackH = std::max(0.0f, node->computedRect.h - 2.0f * m.margin);

            const float contentH = std::max(node->computedRect.h, node->scrollContentSize.y);
            const float maxScrollY = contentH - node->computedRect.h;
            const float thumbH = std::clamp((node->computedRect.h / contentH) * trackH, m.minThumbLength, trackH);
            const float travelH = trackH - thumbH;
            const float scrollRatio = (maxScrollY > 0.0f) ? (node->scrollOffset.y / maxScrollY) : 0.0f;
            const float thumbY = trackY + scrollRatio * travelH;

            const Rect trackRect(trackX, trackY, m.trackWidth, trackH);
            const Rect thumbRect(trackX, thumbY, m.trackWidth, thumbH);

            // Subtle track background
            const Color trackCol = Color::Hex(0x1E293B33); // Slate-800 20%
            canvas.DrawRoundRect(trackRect, m.cornerRadius, trackCol);

            // Thumb pill
            Color thumbCol = Color::Hex(0x64748B80); // Slate-500 50%
            if (node->scrollbarActive) {
                thumbCol = Color::Hex(0xCBD5E1FF); // Slate-300 100%
            } else if (node->scrollbarHovered) {
                thumbCol = Color::Hex(0x94A3B8B3); // Slate-400 70%
            }

            canvas.DrawRoundRect(thumbRect, m.cornerRadius, thumbCol);
        }
    }
}

} // namespace kvant
