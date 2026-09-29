#pragma once

#include "ui/components/UiTheme.hpp"
#include <imgui.h>

class Header {
public:
    class LeftScope {
    public:
        explicit LeftScope(Header* parent);
        ~LeftScope();
        LeftScope(const LeftScope&) = delete;
        LeftScope& operator=(const LeftScope&) = delete;
        LeftScope(LeftScope&& other) noexcept;
        LeftScope& operator=(LeftScope&& other) noexcept;
        explicit operator bool() const { return m_active; }
        float Height() const;
    private:
        Header* m_parent = nullptr;
        bool m_active = false;
    };

    class CenterScope {
    public:
        explicit CenterScope(Header* parent);
        ~CenterScope();
        CenterScope(const CenterScope&) = delete;
        CenterScope& operator=(const CenterScope&) = delete;
        CenterScope(CenterScope&& other) noexcept;
        CenterScope& operator=(CenterScope&& other) noexcept;
        explicit operator bool() const { return m_active; }
        float Width() const { return m_width; }
        float Height() const;
    private:
        Header* m_parent = nullptr;
        float m_width = 0.0f;
        bool m_active = false;
    };

    class RightScope {
    public:
        explicit RightScope(Header* parent);
        ~RightScope();
        RightScope(const RightScope&) = delete;
        RightScope& operator=(const RightScope&) = delete;
        RightScope(RightScope&& other) noexcept;
        RightScope& operator=(RightScope&& other) noexcept;
        explicit operator bool() const { return m_active; }
        float Height() const;
    private:
        Header* m_parent = nullptr;
        bool m_active = false;
    };

    Header(const HeaderStyle* customStyle = nullptr);
    ~Header();

    Header(const Header&) = delete;
    Header& operator=(const Header&) = delete;

    Header(Header&& other) noexcept;
    Header& operator=(Header&& other) noexcept;

    bool Begin(float height = 0.0f);
    void End();

    explicit operator bool() const { return m_open; }

    LeftScope Left();
    CenterScope Center();
    RightScope Right();

    float GetHeight() const;
    float GetContentHeight() const;

private:
    friend class LeftScope;
    friend class CenterScope;
    friend class RightScope;

    // Явно переданный стиль; nullptr — стиль текущей темы (смена темы видна сразу)
    const HeaderStyle* m_customStyle = nullptr;
    const HeaderStyle& Style() const { return m_customStyle ? *m_customStyle : UiTheme::Get().header; }
    bool m_open = false;
    bool m_ended = false;

    float m_height = 0.0f;
    float m_leftWidth = 0.0f;
    float m_rightWidth = 0.0f;
    ImGuiID m_storageId = 0;
};
