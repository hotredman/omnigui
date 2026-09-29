#include "ui/components/Header.hpp"
#include <algorithm>

Header::Header(const HeaderStyle* customStyle)
    : m_customStyle(customStyle)
{
}

Header::~Header() {
    if (!m_ended) {
        End();
    }
}

Header::Header(Header&& other) noexcept
    : m_customStyle(other.m_customStyle)
    , m_open(other.m_open)
    , m_ended(other.m_ended)
    , m_height(other.m_height)
    , m_leftWidth(other.m_leftWidth)
    , m_rightWidth(other.m_rightWidth)
    , m_storageId(other.m_storageId)
{
    other.m_open = false;
    other.m_ended = true;
}

Header& Header::operator=(Header&& other) noexcept {
    if (this != &other) {
        End();
        m_customStyle = other.m_customStyle;
        m_open = other.m_open;
        m_ended = other.m_ended;
        m_height = other.m_height;
        m_leftWidth = other.m_leftWidth;
        m_rightWidth = other.m_rightWidth;
        m_storageId = other.m_storageId;
        other.m_open = false;
        other.m_ended = true;
    }
    return *this;
}

bool Header::Begin(float height) {
    if (m_open) return m_open;
    m_ended = false;

    const UiTheme& theme = UiTheme::Get();
    ImGuiIO& io = ImGui::GetIO();

    m_height = (height > 0.0f) ? height : theme.Scale(Style().height);

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, m_height));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(theme.Scale(Style().paddingX), theme.Scale(Style().paddingY)));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(Style().colBg).Value);

    m_open = ImGui::Begin("##HeaderWindow", nullptr, flags);

    if (m_open) {
        // Отрисовка нижней разделительной линии
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddLine(
            ImVec2(0, m_height - Style().separatorSize),
            ImVec2(io.DisplaySize.x, m_height - Style().separatorSize),
            Style().colSeparator,
            Style().separatorSize
        );

        m_storageId = ImGui::GetID("##HeaderZones");
        ImGuiStorage* storage = ImGui::GetStateStorage();
        m_leftWidth = *storage->GetFloatRef(m_storageId, theme.Scale(Style().defaultLeftWidth));
        m_rightWidth = *storage->GetFloatRef(m_storageId + 1, theme.Scale(Style().defaultRightWidth));
    }

    return m_open;
}

void Header::End() {
    if (m_ended) return;

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);

    m_ended = true;
    m_open = false;
}

float Header::GetHeight() const {
    return m_height > 0.0f ? m_height : UiTheme::Get().Scale(Style().height);
}

float Header::GetContentHeight() const {
    const UiTheme& theme = UiTheme::Get();
    float padY = theme.Scale(Style().paddingY);
    float avail = GetHeight() - padY * 2.0f;
    return std::max(0.0f, avail);
}

// ----------------------------------------------------------------------------
// Left Zone
// ----------------------------------------------------------------------------

Header::LeftScope::LeftScope(Header* parent)
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

float Header::LeftScope::Height() const {
    return m_parent ? m_parent->GetContentHeight() : 0.0f;
}

Header::LeftScope::~LeftScope() {
    if (m_active && m_parent) {
        ImGui::EndGroup();
        float measuredW = ImGui::GetItemRectSize().x;
        m_parent->m_leftWidth = measuredW;
        ImGuiStorage* storage = ImGui::GetStateStorage();
        *storage->GetFloatRef(m_parent->m_storageId, 0.0f) = measuredW;
    }
}

Header::LeftScope::LeftScope(LeftScope&& other) noexcept
    : m_parent(other.m_parent)
    , m_active(other.m_active)
{
    other.m_parent = nullptr;
    other.m_active = false;
}

Header::LeftScope& Header::LeftScope::operator=(LeftScope&& other) noexcept {
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

Header::LeftScope Header::Left() {
    return LeftScope(this);
}

// ----------------------------------------------------------------------------
// Right Zone
// ----------------------------------------------------------------------------

Header::RightScope::RightScope(Header* parent)
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

float Header::RightScope::Height() const {
    return m_parent ? m_parent->GetContentHeight() : 0.0f;
}

Header::RightScope::~RightScope() {
    if (m_active && m_parent) {
        ImGui::EndGroup();
        float measuredW = ImGui::GetItemRectSize().x;
        m_parent->m_rightWidth = measuredW;
        ImGuiStorage* storage = ImGui::GetStateStorage();
        *storage->GetFloatRef(m_parent->m_storageId + 1, 0.0f) = measuredW;
    }
}

Header::RightScope::RightScope(RightScope&& other) noexcept
    : m_parent(other.m_parent)
    , m_active(other.m_active)
{
    other.m_parent = nullptr;
    other.m_active = false;
}

Header::RightScope& Header::RightScope::operator=(RightScope&& other) noexcept {
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

Header::RightScope Header::Right() {
    return RightScope(this);
}

// ----------------------------------------------------------------------------
// Center Zone
// ----------------------------------------------------------------------------

Header::CenterScope::CenterScope(Header* parent)
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

float Header::CenterScope::Height() const {
    return m_parent ? m_parent->GetContentHeight() : 0.0f;
}

Header::CenterScope::~CenterScope() {
    if (m_active && m_parent) {
        ImGui::EndGroup();
    }
}

Header::CenterScope::CenterScope(CenterScope&& other) noexcept
    : m_parent(other.m_parent)
    , m_width(other.m_width)
    , m_active(other.m_active)
{
    other.m_parent = nullptr;
    other.m_active = false;
}

Header::CenterScope& Header::CenterScope::operator=(CenterScope&& other) noexcept {
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

Header::CenterScope Header::Center() {
    return CenterScope(this);
}
