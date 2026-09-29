#include "ui/components/Carousel.hpp"
#include "ui/components/DisabledScope.hpp"
#include "ui/components/Icon.hpp"
#include <imgui_internal.h>
#include <algorithm>

Carousel::Carousel(const char* id, ImVec2 size, const CarouselStyle* customStyle)
    : m_id(id)
    , m_requestedSize(size)
    , m_style(customStyle ? *customStyle : UiTheme::Get().carousel)
{
    Begin();
}

Carousel::~Carousel() {
    End();
}

Carousel::Carousel(Carousel&& other) noexcept
    : m_id(other.m_id)
    , m_requestedSize(other.m_requestedSize)
    , m_style(other.m_style)
    , m_open(other.m_open)
    , m_ended(other.m_ended)
    , m_groupStarted(other.m_groupStarted)
    , m_childStarted(other.m_childStarted)
    , m_canScroll(other.m_canScroll)
    , m_showLeftArrow(other.m_showLeftArrow)
    , m_showRightArrow(other.m_showRightArrow)
    , m_arrowW(other.m_arrowW)
    , m_cardH(other.m_cardH)
    , m_spacing(other.m_spacing)
    , m_scrollAreaW(other.m_scrollAreaW)
    , m_maxScrollX(other.m_maxScrollX)
    , m_scrollX(other.m_scrollX)
    , m_targetScrollX(other.m_targetScrollX)
    , m_storageId(other.m_storageId)
{
    other.m_open = false;
    other.m_ended = true;
    other.m_groupStarted = false;
    other.m_childStarted = false;
}

Carousel& Carousel::operator=(Carousel&& other) noexcept {
    if (this != &other) {
        End();
        m_id = other.m_id;
        m_requestedSize = other.m_requestedSize;
        m_style = other.m_style;
        m_open = other.m_open;
        m_ended = other.m_ended;
        m_groupStarted = other.m_groupStarted;
        m_childStarted = other.m_childStarted;
        m_canScroll = other.m_canScroll;
        m_showLeftArrow = other.m_showLeftArrow;
        m_showRightArrow = other.m_showRightArrow;
        m_arrowW = other.m_arrowW;
        m_cardH = other.m_cardH;
        m_spacing = other.m_spacing;
        m_scrollAreaW = other.m_scrollAreaW;
        m_maxScrollX = other.m_maxScrollX;
        m_scrollX = other.m_scrollX;
        m_targetScrollX = other.m_targetScrollX;
        m_storageId = other.m_storageId;
        other.m_open = false;
        other.m_ended = true;
        other.m_groupStarted = false;
        other.m_childStarted = false;
    }
    return *this;
}

bool Carousel::Begin() {
    if (m_ended) return false;
    if (ImGui::GetCurrentWindowRead() == nullptr) {
        m_open = false;
        return false;
    }

    const UiTheme& theme = UiTheme::Get();
    ImGuiIO& io = ImGui::GetIO();

    float availW = (m_requestedSize.x > 0.0f) ? m_requestedSize.x : ImGui::GetContentRegionAvail().x;
    if (availW < theme.Scale(40.0f)) {
        m_open = false;
        return false;
    }

    m_cardH = (m_requestedSize.y > 0.0f) ? m_requestedSize.y : theme.IndicatorHeight();
    m_spacing = theme.Scale(m_style.itemSpacing);
    m_arrowW = theme.Scale(m_style.arrowWidth);

    m_storageId = ImGui::GetID(m_id);
    ImGuiStorage* storage = ImGui::GetStateStorage();
    float* pScrollX = storage->GetFloatRef(m_storageId, 0.0f);
    float* pTargetScrollX = storage->GetFloatRef(m_storageId + 1, 0.0f);
    float* pPrevMaxScrollX = storage->GetFloatRef(m_storageId + 2, 0.0f);

    m_scrollX = *pScrollX;
    m_targetScrollX = *pTargetScrollX;
    m_maxScrollX = *pPrevMaxScrollX;

    m_canScroll = (m_maxScrollX > 1.0f);
    m_showLeftArrow = m_canScroll && (!m_style.hideDisabledArrows || m_scrollX > 1.0f);
    m_showRightArrow = m_canScroll && (!m_style.hideDisabledArrows || m_scrollX < m_maxScrollX - 2.0f);

    float leftSpace = m_showLeftArrow ? (m_arrowW + m_spacing) : 0.0f;
    float rightSpace = m_showRightArrow ? (m_arrowW + m_spacing) : 0.0f;

    m_scrollAreaW = m_canScroll ? (availW - leftSpace - rightSpace) : availW;
    float scrollAreaW = m_scrollAreaW;

    ImGui::BeginGroup();
    m_groupStarted = true;

    // 1. Стрелка влево < (если содержимое превышает доступную ширину и не в самом начале ленты)
    if (m_showLeftArrow) {
        bool atStart = (m_scrollX <= 1.0f);
        DisabledScope arrowScope(atStart);

        char prevBtnId[128];
        std::snprintf(prevBtnId, sizeof(prevBtnId), "%s_prev_btn", m_id);

        ImVec2 screenPos = ImGui::GetCursorScreenPos();
        bool clicked = ImGui::InvisibleButton(prevBtnId, ImVec2(m_arrowW, m_cardH));
        bool hovered = ImGui::IsItemHovered();
        bool active = ImGui::IsItemActive();

        if (clicked) {
            m_targetScrollX -= (theme.Scale(m_style.scrollStep) + m_spacing);
        }

        ImDrawList* dl = ImGui::GetWindowDrawList();
        float radius = theme.Scale(4.0f);
        ImU32 bgCol = active ? theme.palette.bgControlActive :
                      (hovered ? theme.palette.bgControlHover : m_style.colArrowBg);
        ImU32 iconCol = hovered ? theme.palette.textPrimary : m_style.colArrowText;

        dl->AddRectFilled(screenPos, ImVec2(screenPos.x + m_arrowW, screenPos.y + m_cardH), ImGui::GetColorU32(bgCol), radius);

        float pressOffset = active ? 1.0f : 0.0f;
        ImVec2 iconCenter(screenPos.x + m_arrowW * 0.5f + pressOffset, screenPos.y + m_cardH * 0.5f + pressOffset);
        float iconSz = theme.Scale(12.0f);
        Icon(Icon::ChevronLeft).Draw(dl, iconCenter, iconSz, ImGui::GetColorU32(iconCol));

        ImGui::SameLine(0.0f, m_spacing);
    }

    // 2. Дочернее окно горизонтальной прокрутки
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(m_spacing, 0.0f));
    float childH = m_cardH + theme.Scale(m_style.paddingY);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, m_style.colBg);  // подложка — только окну ленты
    m_open = ImGui::BeginChild(m_id, ImVec2(scrollAreaW, childH), false,
                               ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleColor();
    m_childStarted = true;

    if (m_open) {
        // Прокрутка колесом мыши при наведении (без Shift)
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && io.MouseWheel != 0.0f) {
            float step = theme.Scale(m_style.scrollStep) + m_spacing;
            m_targetScrollX -= io.MouseWheel * step;
        }

        float currentMaxScroll = ImGui::GetScrollMaxX();
        *pPrevMaxScrollX = currentMaxScroll;
        m_maxScrollX = currentMaxScroll;

        m_targetScrollX = std::clamp(m_targetScrollX, 0.0f, std::max(0.0f, m_maxScrollX));

        // Плавная интерполяция (Lerp)
        float lerpFactor = std::clamp(m_style.scrollSpeed * io.DeltaTime, 0.01f, 1.0f);
        m_scrollX += (m_targetScrollX - m_scrollX) * lerpFactor;
        ImGui::SetScrollX(m_scrollX);

        *pScrollX = m_scrollX;
        *pTargetScrollX = m_targetScrollX;
    }

    return m_open;
}

void Carousel::End() {
    if (m_ended) return;
    m_ended = true;

    if (m_childStarted) {
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        m_childStarted = false;
    }

    if (m_groupStarted) {
        // 3. Стрелка вправо > (если содержимое превышает ширину и не в самом конце ленты)
        if (m_showRightArrow) {
            ImGui::SameLine(0.0f, m_spacing);
            bool atEnd = (m_scrollX >= m_maxScrollX - 2.0f);
            DisabledScope arrowScope(atEnd);

            const UiTheme& theme = UiTheme::Get();
            char nextBtnId[128];
            std::snprintf(nextBtnId, sizeof(nextBtnId), "%s_next_btn", m_id);

            ImVec2 screenPos = ImGui::GetCursorScreenPos();
            bool clicked = ImGui::InvisibleButton(nextBtnId, ImVec2(m_arrowW, m_cardH));
            bool hovered = ImGui::IsItemHovered();
            bool active = ImGui::IsItemActive();

            if (clicked) {
                m_targetScrollX += (theme.Scale(m_style.scrollStep) + m_spacing);
                ImGuiStorage* storage = ImGui::GetStateStorage();
                *storage->GetFloatRef(m_storageId + 1, 0.0f) = m_targetScrollX;
            }

            ImDrawList* dl = ImGui::GetWindowDrawList();
            float radius = theme.Scale(4.0f);
            ImU32 bgCol = active ? theme.palette.bgControlActive :
                          (hovered ? theme.palette.bgControlHover : m_style.colArrowBg);
            ImU32 iconCol = hovered ? theme.palette.textPrimary : m_style.colArrowText;

            dl->AddRectFilled(screenPos, ImVec2(screenPos.x + m_arrowW, screenPos.y + m_cardH), ImGui::GetColorU32(bgCol), radius);

            float pressOffset = active ? 1.0f : 0.0f;
            ImVec2 iconCenter(screenPos.x + m_arrowW * 0.5f + pressOffset, screenPos.y + m_cardH * 0.5f + pressOffset);
            float iconSz = theme.Scale(12.0f);
            Icon(Icon::ChevronRight).Draw(dl, iconCenter, iconSz, ImGui::GetColorU32(iconCol));

        }

        ImGui::EndGroup();
        m_groupStarted = false;
    }

    m_open = false;
}

void Carousel::EnsureVisible(float itemLeft, float itemRight) {
    if (!m_canScroll || m_scrollAreaW <= 0.0f) return;
    const UiTheme& theme = UiTheme::Get();
    float margin = theme.Scale(12.0f);
    if (itemLeft < m_targetScrollX + margin) {
        m_targetScrollX = std::max(0.0f, itemLeft - margin);
    } else if (itemRight > m_targetScrollX + m_scrollAreaW - margin) {
        m_targetScrollX = std::min(m_maxScrollX, itemRight - m_scrollAreaW + margin);
    }
    ImGuiStorage* storage = ImGui::GetStateStorage();
    *storage->GetFloatRef(m_storageId + 1, 0.0f) = m_targetScrollX;
}
