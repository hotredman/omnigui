#include "ui/components/Toolbar.hpp"
#include "ui/components/Button.hpp"
#include "ui/components/Label.hpp"
#include <algorithm>
#include <cmath>

Toolbar::Toolbar(const ToolbarStyle* customStyle)
    : m_customStyle(customStyle)
{
}

Toolbar::~Toolbar() {
    if (!m_ended) {
        End();
    }
}

Toolbar::Toolbar(Toolbar&& other) noexcept
    : m_customStyle(other.m_customStyle)
    , m_open(other.m_open)
    , m_ended(other.m_ended)
    , m_posY(other.m_posY)
    , m_height(other.m_height)
    , m_leftWidth(other.m_leftWidth)
    , m_rightWidth(other.m_rightWidth)
    , m_storageId(other.m_storageId)
{
    other.m_open = false;
    other.m_ended = true;
}

Toolbar& Toolbar::operator=(Toolbar&& other) noexcept {
    if (this != &other) {
        End();
        m_customStyle = other.m_customStyle;
        m_open = other.m_open;
        m_ended = other.m_ended;
        m_posY = other.m_posY;
        m_height = other.m_height;
        m_leftWidth = other.m_leftWidth;
        m_rightWidth = other.m_rightWidth;
        m_storageId = other.m_storageId;
        other.m_open = false;
        other.m_ended = true;
    }
    return *this;
}

bool Toolbar::Begin(float posY, float height) {
    if (m_open) return m_open;
    m_ended = false;

    const UiTheme& theme = UiTheme::Get();
    ImGuiIO& io = ImGui::GetIO();

    m_posY = (posY >= 0.0f) ? posY : theme.HeaderHeight();
    m_height = (height > 0.0f) ? height : theme.Scale(Style().height);

    ImGui::SetNextWindowPos(ImVec2(0, m_posY));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, m_height));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(theme.Scale(Style().paddingX), theme.Scale(Style().paddingY)));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(Style().colBg).Value);

    m_open = ImGui::Begin("##ToolbarWindow", nullptr, flags);

    if (m_open) {
        // Отрисовка нижней разделительной линии
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddLine(
            ImVec2(0, m_posY + m_height - Style().separatorSize),
            ImVec2(io.DisplaySize.x, m_posY + m_height - Style().separatorSize),
            Style().colSeparator,
            Style().separatorSize
        );

        m_storageId = ImGui::GetID("##ToolbarZones");
        ImGuiStorage* storage = ImGui::GetStateStorage();
        m_leftWidth = *storage->GetFloatRef(m_storageId, theme.Scale(Style().defaultLeftWidth));
        m_rightWidth = *storage->GetFloatRef(m_storageId + 1, theme.Scale(Style().defaultRightWidth));
    }

    return m_open;
}

void Toolbar::End() {
    if (m_ended) return;

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);

    m_ended = true;
    m_open = false;
}

float Toolbar::GetHeight() const {
    return m_height > 0.0f ? m_height : UiTheme::Get().Scale(Style().height);
}

float Toolbar::GetContentHeight() const {
    const UiTheme& theme = UiTheme::Get();
    float padY = theme.Scale(Style().paddingY);
    float avail = GetHeight() - padY * 2.0f;
    return std::max(0.0f, avail);
}

// ----------------------------------------------------------------------------
// Left Zone
// ----------------------------------------------------------------------------

Toolbar::LeftScope::LeftScope(Toolbar* parent)
    : m_parent(parent)
    , m_active(parent && parent->m_open)
{
    if (m_active) {
        const UiTheme& theme = UiTheme::Get();
        float padX = theme.Scale(m_parent->Style().paddingX);
        float padY = theme.Scale(m_parent->Style().paddingY);
        ImGui::SetCursorPos(ImVec2(padX, padY));
        ImGui::BeginGroup();
    }
}

float Toolbar::LeftScope::Height() const {
    return m_parent ? m_parent->GetContentHeight() : 0.0f;
}

void Toolbar::LeftScope::Label(const char* text, UiVariant variant) {
    if (!m_active || !m_parent || !text || text[0] == '\0') return;

    const UiTheme& theme = UiTheme::Get();
    ImFont* font = theme.fontMedium ? theme.fontMedium : theme.defaultFont;
    theme.PushFont(font, 16.0f);

    float padY = theme.Scale(m_parent->Style().paddingY);
    float contentH = Height();
    float lineH = ImGui::GetTextLineHeight();
    float textY = padY + std::floor((contentH - lineH) * 0.5f);

    ImGui::SetCursorPosY(textY);
    const SemanticStyle& style = theme.GetVariantStyle(variant);
    ImGui::PushStyleColor(ImGuiCol_Text, style.colText);
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();

    theme.PopFont();
}

Toolbar::LeftScope::~LeftScope() {
    if (m_active && m_parent) {
        ImGui::EndGroup();
        float measuredW = ImGui::GetItemRectSize().x;
        m_parent->m_leftWidth = measuredW;
        ImGuiStorage* storage = ImGui::GetStateStorage();
        *storage->GetFloatRef(m_parent->m_storageId, 0.0f) = measuredW;
    }
}

Toolbar::LeftScope::LeftScope(LeftScope&& other) noexcept
    : m_parent(other.m_parent)
    , m_active(other.m_active)
{
    other.m_parent = nullptr;
    other.m_active = false;
}

Toolbar::LeftScope& Toolbar::LeftScope::operator=(LeftScope&& other) noexcept {
    if (this != &other) {
        if (m_active && m_parent) {
            ImGui::EndGroup();
        }
        m_parent = other.m_parent;
        m_active = other.m_active;
        other.m_parent = nullptr;
        other.m_active = false;
    }
    return *this;
}

Toolbar::LeftScope Toolbar::Left() {
    return LeftScope(this);
}

// ----------------------------------------------------------------------------
// Right Zone
// ----------------------------------------------------------------------------

Toolbar::RightScope::RightScope(Toolbar* parent)
    : m_parent(parent)
    , m_active(parent && parent->m_open)
{
    if (m_active) {
        const UiTheme& theme = UiTheme::Get();
        float padX = theme.Scale(m_parent->Style().paddingX);
        float padY = theme.Scale(m_parent->Style().paddingY);
        float rightStartX = ImGui::GetWindowWidth() - m_parent->m_rightWidth - padX;
        ImGui::SetCursorPos(ImVec2(rightStartX, padY));
        ImGui::BeginGroup();
    }
}

float Toolbar::RightScope::Height() const {
    return m_parent ? m_parent->GetContentHeight() : 0.0f;
}

void Toolbar::RightScope::Label(const char* text, UiVariant variant) {
    if (!m_active || !m_parent || !text || text[0] == '\0') return;

    const UiTheme& theme = UiTheme::Get();
    ImFont* font = theme.fontMedium ? theme.fontMedium : theme.defaultFont;
    theme.PushFont(font, 16.0f);

    float padY = theme.Scale(m_parent->Style().paddingY);
    float contentH = Height();
    float lineH = ImGui::GetTextLineHeight();
    float textY = padY + std::floor((contentH - lineH) * 0.5f);

    ImGui::SetCursorPosY(textY);
    const SemanticStyle& style = theme.GetVariantStyle(variant);
    ImGui::PushStyleColor(ImGuiCol_Text, style.colText);
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();

    theme.PopFont();
}

bool Toolbar::RightScope::Button(const char* label, UiVariant variant, Icon icon, float baseWidth) {
    if (!m_active || !m_parent) return false;
    return ::Button::Render(label, variant, icon, baseWidth, m_parent->Style().ContentHeight());
}

bool Toolbar::RightScope::ButtonPrimary(const char* label, Icon icon, float baseWidth) {
    return Button(label, UiVariant::Primary, icon, baseWidth);
}

Toolbar::RightScope::~RightScope() {
    if (m_active && m_parent) {
        ImGui::EndGroup();
        float measuredW = ImGui::GetItemRectSize().x;
        m_parent->m_rightWidth = measuredW;
        ImGuiStorage* storage = ImGui::GetStateStorage();
        *storage->GetFloatRef(m_parent->m_storageId + 1, 0.0f) = measuredW;
    }
}

Toolbar::RightScope::RightScope(RightScope&& other) noexcept
    : m_parent(other.m_parent)
    , m_active(other.m_active)
{
    other.m_parent = nullptr;
    other.m_active = false;
}

Toolbar::RightScope& Toolbar::RightScope::operator=(RightScope&& other) noexcept {
    if (this != &other) {
        if (m_active && m_parent) {
            ImGui::EndGroup();
        }
        m_parent = other.m_parent;
        m_active = other.m_active;
        other.m_parent = nullptr;
        other.m_active = false;
    }
    return *this;
}

Toolbar::RightScope Toolbar::Right() {
    return RightScope(this);
}

// ----------------------------------------------------------------------------
// Center Zone
// ----------------------------------------------------------------------------

Toolbar::CenterScope::CenterScope(Toolbar* parent)
    : m_parent(parent)
    , m_width(0.0f)
    , m_active(parent && parent->m_open)
{
    if (m_active) {
        const UiTheme& theme = UiTheme::Get();
        float padX = theme.Scale(m_parent->Style().paddingX);
        float padY = theme.Scale(m_parent->Style().paddingY);
        float spacing = theme.Scale(m_parent->Style().zoneSpacing);

        float leftEnd = padX + m_parent->m_leftWidth + spacing;
        float rightStart = ImGui::GetWindowWidth() - m_parent->m_rightWidth - padX - spacing;

        m_width = std::max(20.0f, rightStart - leftEnd);

        ImGui::SetCursorPos(ImVec2(leftEnd, padY));
        ImGui::BeginGroup();
    }
}

float Toolbar::CenterScope::Height() const {
    return m_parent ? m_parent->GetContentHeight() : 0.0f;
}

void Toolbar::CenterScope::Label(const char* text, UiVariant variant) {
    if (!m_active || !m_parent || !text || text[0] == '\0') return;

    const UiTheme& theme = UiTheme::Get();
    ImFont* font = theme.fontMedium ? theme.fontMedium : theme.defaultFont;
    theme.PushFont(font, 16.0f);

    float padY = theme.Scale(m_parent->Style().paddingY);
    float contentH = Height();
    float lineH = ImGui::GetTextLineHeight();
    float textY = padY + std::floor((contentH - lineH) * 0.5f);

    ImGui::SetCursorPosY(textY);
    const SemanticStyle& style = theme.GetVariantStyle(variant);
    ImGui::PushStyleColor(ImGuiCol_Text, style.colText);
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();

    theme.PopFont();
}

Toolbar::CenterScope::~CenterScope() {
    if (m_active && m_parent) {
        ImGui::EndGroup();
    }
}

Toolbar::CenterScope::CenterScope(CenterScope&& other) noexcept
    : m_parent(other.m_parent)
    , m_width(other.m_width)
    , m_active(other.m_active)
{
    other.m_parent = nullptr;
    other.m_active = false;
}

Toolbar::CenterScope& Toolbar::CenterScope::operator=(CenterScope&& other) noexcept {
    if (this != &other) {
        if (m_active && m_parent) {
            ImGui::EndGroup();
        }
        m_parent = other.m_parent;
        m_width = other.m_width;
        m_active = other.m_active;
        other.m_parent = nullptr;
        other.m_active = false;
    }
    return *this;
}

Toolbar::CenterScope Toolbar::Center() {
    return CenterScope(this);
}
