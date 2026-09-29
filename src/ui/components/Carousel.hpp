#pragma once

#include "ui/components/UiTheme.hpp"
#include <imgui.h>

class Carousel {
public:
    Carousel(const char* id, ImVec2 size = ImVec2(0.0f, 0.0f), const CarouselStyle* customStyle = nullptr);
    ~Carousel();

    Carousel(const Carousel&) = delete;
    Carousel& operator=(const Carousel&) = delete;

    Carousel(Carousel&& other) noexcept;
    Carousel& operator=(Carousel&& other) noexcept;

    bool Begin();
    void End();

    void EnsureVisible(float itemLeft, float itemRight);
    bool CanScroll() const { return m_canScroll; }
    float GetScrollX() const { return m_scrollX; }
    float GetMaxScrollX() const { return m_maxScrollX; }

    explicit operator bool() const { return m_open; }

private:
    const char* m_id = "##Carousel";
    ImVec2 m_requestedSize = ImVec2(0.0f, 0.0f);
    CarouselStyle m_style;
    bool m_open = false;
    bool m_ended = false;
    bool m_groupStarted = false;
    bool m_childStarted = false;

    bool m_canScroll = false;
    bool m_showLeftArrow = false;
    bool m_showRightArrow = false;
    float m_arrowW = 0.0f;
    float m_cardH = 0.0f;
    float m_spacing = 0.0f;
    float m_scrollAreaW = 0.0f;
    float m_maxScrollX = 0.0f;
    float m_scrollX = 0.0f;
    float m_targetScrollX = 0.0f;
    ImGuiID m_storageId = 0;
};
