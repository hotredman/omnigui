#include "kvant/widgets/Toolbar.hpp"
#include <algorithm>
#include "kvant/Context.hpp"
#include "kvant/backend/ICanvas.hpp"
#include "kvant/layout/Node.hpp"

namespace kvant {

ToolbarScope::ToolbarScope(Context* ctx, Node* node)
    : m_ctx(ctx)
    , m_node(node)
{
}

ToolbarScope::~ToolbarScope() {
    if (m_ctx && m_node) {
        m_ctx->PopContainer(m_node);
    }
}

ToolbarScope::ToolbarScope(ToolbarScope&& other) noexcept
    : m_ctx(other.m_ctx)
    , m_node(other.m_node)
{
    other.m_ctx = nullptr;
    other.m_node = nullptr;
}

ToolbarScope& ToolbarScope::operator=(ToolbarScope&& other) noexcept {
    if (this != &other) {
        if (m_ctx && m_node) {
            m_ctx->PopContainer(m_node);
        }
        m_ctx = other.m_ctx;
        m_node = other.m_node;
        other.m_ctx = nullptr;
        other.m_node = nullptr;
    }
    return *this;
}

void ToolbarScope::Separator(float heightRatio, std::source_location loc) {
    if (!m_ctx || !m_node) return;
    const Id id = m_ctx->MakeChildId(loc);
    Node* sep = m_ctx->CreateNode(NodeType::Separator, id);
    sep->widthMode = SizeMode::Fixed;
    sep->fixedWidth = 10.0f; // 10px slot with centered 1px line
    sep->heightMode = SizeMode::Fixed;
    const float parentH = (m_node->fixedHeight > 0.0f) ? m_node->fixedHeight : 32.0f;
    sep->fixedHeight = parentH * std::clamp(heightRatio, 0.1f, 1.0f);
    sep->borderColor = m_node->borderColor.a > 0 ? m_node->borderColor : Color::Hex(0x334155FF);
}

void ToolbarScope::Spacer(std::source_location loc) {
    if (!m_ctx) return;
    const Id id = m_ctx->MakeChildId(loc);
    Node* spacer = m_ctx->CreateNode(NodeType::Spacer, id);
    spacer->widthMode = SizeMode::Stretch;
    spacer->isSpacer = true;
    spacer->flexGrow = 1.0f;
}

Response ToolbarScope::ToolButton(
    std::string_view label,
    ToolButtonOptions opts,
    std::source_location loc
) {
    if (!m_ctx) return Response{};
    return Toolbar::ToolButton(*m_ctx, label, opts, loc);
}

ToolbarScope Toolbar::Open(
    Context& ctx,
    ToolbarOptions opts,
    std::source_location loc
) {
    const Id id = ctx.MakeChildId(loc);
    const UiSize activeSize = opts.size.value_or(ctx.GetCurrentSize());
    const ComponentMetrics m = GetComponentMetrics(activeSize);
    const float barH = (opts.fixedHeight > 0.0f) ? opts.fixedHeight : (m.defaultHeight + 8.0f);
    const bool effectiveDisabled = opts.disabled || ctx.IsDisabled();

    Node* node = ctx.CreateNode(NodeType::Container, id);
    node->direction = FlexDirection::Row;
    node->align = Alignment::Center;
    node->gap = opts.gap;
    node->padding = opts.padding;
    node->widthMode = SizeMode::Stretch;
    node->heightMode = SizeMode::Fixed;
    node->fixedHeight = barH;
    const auto pal = ctx.GetThemePalette();
    node->backgroundColor = opts.backgroundColor.a > 0 ? opts.backgroundColor : pal.surface;
    node->borderColor = opts.borderColor.a > 0 ? opts.borderColor : pal.border;
    node->borderWidth = opts.borderWidth;
    node->cornerRadius = opts.cornerRadius;
    node->uiSize = activeSize;
    node->disabled = effectiveDisabled;

    ctx.PushSize(activeSize);
    ctx.PushDisabled(effectiveDisabled);
    ctx.PushContainer(node);

    return ToolbarScope(&ctx, node);
}

Response Toolbar::ToolButton(
    Context& ctx,
    std::string_view label,
    ToolButtonOptions opts,
    std::source_location loc
) {
    const Id id = ctx.MakeChildId(loc);
    const bool effectiveDisabled = opts.disabled || ctx.IsDisabled();
    const UiSize activeSize = ctx.GetCurrentSize();

    Response res = ctx.Interact(id);
    if (effectiveDisabled) {
        res.clicked = false;
        res.hovered = false;
        res.pressed = false;
    }

    Node* node = ctx.CreateNode(NodeType::Button, id);
    node->text = ctx.InternString(label);

    ButtonOptions bOpts;
    bOpts.label = node->text;
    bOpts.icon = opts.icon;
    bOpts.variant = opts.variant;
    bOpts.iconPlacement = opts.iconPlacement;
    bOpts.labelVariants = opts.labelVariants;
    bOpts.disabled = effectiveDisabled;
    bOpts.active = opts.active;

    node->buttonOpts = bOpts;
    node->uiSize = activeSize;
    node->disabled = effectiveDisabled;
    node->hovered = res.hovered;
    node->pressed = res.pressed;
    node->widthMode = SizeMode::Fit;
    node->heightMode = SizeMode::Fit;

    ctx.AdaptChildToContainer(node);

    return res;
}

void Toolbar::RenderSeparator(
    ICanvas& canvas,
    Rect bounds,
    Color color,
    ThemeMode theme
) {
    const Color stroke = color.a > 0 ? color : GetThemePalette(theme).borderSubtle;
    const float midX = bounds.x + bounds.w * 0.5f;
    const float padY = std::max(2.0f, bounds.h * 0.20f);
    canvas.DrawLine(
        Vec2(midX, bounds.y + padY),
        Vec2(midX, bounds.Bottom() - padY),
        stroke,
        1.0f
    );
}

} // namespace kvant
