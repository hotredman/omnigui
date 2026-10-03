#include "ui/components/ContentArea.hpp"
#include <algorithm>

ContentArea::ContentArea(const ContentAreaOptions& options) {
    Open(options.posXPx, options.posYPx, options.widthPx, options.heightPx, options.style);
}

ContentArea::~ContentArea() {
    // ImGui::End() обязателен независимо от результата Begin
    ImGui::End();
}

void ContentArea::Open(float customPosX, float customPosY, float customWidth, float customHeight,
                       const ContentAreaStyle* customStyle)
{
    const UiTheme& theme = UiTheme::Get();
    m_style = customStyle ? customStyle : &theme.contentArea;

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
    m_open = ImGui::Begin("##ContentAreaWindow", nullptr, flags);
    ImGui::PopStyleVar();
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
