#include "ui/components/ContentArea.hpp"
#include <algorithm>

ContentArea::ContentArea(const ContentAreaStyle* customStyle)
    : m_style(customStyle)
{
}

ContentArea::~ContentArea() {
    if (m_beginCalled) {
        End();
    }
}

bool ContentArea::Begin(float customPosX,
                        float customPosY,
                        float customWidth,
                        float customHeight,
                        const ContentAreaStyle* customStyle)
{
    const UiTheme& theme = UiTheme::Get();
    m_style = customStyle ? customStyle : (m_style ? m_style : &theme.contentArea);

    ImGuiIO& io = ImGui::GetIO();

    m_posX = (customPosX >= 0.0f) ? customPosX : theme.SidebarWidth();
    m_posY = (customPosY >= 0.0f) ? customPosY : (theme.HeaderHeight() + theme.TopBarHeight());
    m_width = (customWidth >= 0.0f) ? customWidth : (io.DisplaySize.x - m_posX);
    // По умолчанию — до строки состояния внизу окна
    m_height = (customHeight >= 0.0f)
        ? customHeight
        : (io.DisplaySize.y - m_posY - theme.StatusBarHeight());

    ImGui::SetNextWindowPos(ImVec2(m_posX, m_posY));
    ImGui::SetNextWindowSize(ImVec2(m_width, m_height));

    // Холст: фон — очистка кадра, окно его не перерисовывает
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoBackground;

    if (!m_style->enableScrollX && !m_style->enableScrollY) {
        flags |= ImGuiWindowFlags_NoScrollbar;
    }
    if (m_style->enableScrollX) {
        flags |= ImGuiWindowFlags_HorizontalScrollbar;
    }

    // Отступы — только окну рабочей области (в содержимое не протекают)
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(theme.Scale(m_style->paddingX), theme.Scale(m_style->paddingY)));
    m_beginCalled = true;
    m_open = ImGui::Begin("##ContentAreaWindow", nullptr, flags);
    ImGui::PopStyleVar();
    return m_open;
}

void ContentArea::End() {
    if (m_beginCalled) {
        ImGui::End();
        m_beginCalled = false;
        m_open = false;
    }
}

ImVec2 ContentArea::GetAvailableSize() const {
    if (m_open) {
        return ImGui::GetContentRegionAvail();
    }
    const UiTheme& theme = UiTheme::Get();
    float padX = m_style ? theme.Scale(m_style->paddingX) : theme.Scale(16.0f);
    float padY = m_style ? theme.Scale(m_style->paddingY) : theme.Scale(16.0f);
    return ImVec2(std::max(0.0f, m_width - padX * 2.0f), std::max(0.0f, m_height - padY * 2.0f));
}
