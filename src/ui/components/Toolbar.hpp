#pragma once

#include "ui/components/UiTheme.hpp"
#include "ui/components/Icon.hpp"
#include <imgui.h>

class Toolbar {
public:
    class LeftScope {
    public:
        explicit LeftScope(Toolbar* parent);
        ~LeftScope();
        LeftScope(const LeftScope&) = delete;
        LeftScope& operator=(const LeftScope&) = delete;
        LeftScope(LeftScope&& other) noexcept;
        LeftScope& operator=(LeftScope&& other) noexcept;
        explicit operator bool() const { return m_active; }
        float Height() const;
        void Label(const char* text, UiVariant variant = UiVariant::Secondary);
    private:
        Toolbar* m_parent = nullptr;
        bool m_active = false;
    };

    class CenterScope {
    public:
        explicit CenterScope(Toolbar* parent);
        ~CenterScope();
        CenterScope(const CenterScope&) = delete;
        CenterScope& operator=(const CenterScope&) = delete;
        CenterScope(CenterScope&& other) noexcept;
        CenterScope& operator=(CenterScope&& other) noexcept;
        explicit operator bool() const { return m_active; }
        float Width() const { return m_width; }
        float Height() const;
        void Label(const char* text, UiVariant variant = UiVariant::Secondary);
    private:
        Toolbar* m_parent = nullptr;
        float m_width = 0.0f;
        bool m_active = false;
    };

    class RightScope {
    public:
        explicit RightScope(Toolbar* parent);
        ~RightScope();
        RightScope(const RightScope&) = delete;
        RightScope& operator=(const RightScope&) = delete;
        RightScope(RightScope&& other) noexcept;
        RightScope& operator=(RightScope&& other) noexcept;
        explicit operator bool() const { return m_active; }
        float Height() const;
        void Label(const char* text, UiVariant variant = UiVariant::Secondary);
        bool Button(const char* label, UiVariant variant = UiVariant::Default, Icon icon = Icon::None, float baseWidth = 0.0f);
        bool ButtonPrimary(const char* label, Icon icon = Icon::None, float baseWidth = 0.0f);
    private:
        Toolbar* m_parent = nullptr;
        bool m_active = false;
    };

    Toolbar(const ToolbarStyle* customStyle = nullptr);
    ~Toolbar();

    Toolbar(const Toolbar&) = delete;
    Toolbar& operator=(const Toolbar&) = delete;

    Toolbar(Toolbar&& other) noexcept;
    Toolbar& operator=(Toolbar&& other) noexcept;

    // Begin toolbar. If posY < 0, posY defaults to theme.HeaderHeight().
    // If height <= 0, height defaults to theme.ToolbarHeight().
    bool Begin(float posY = -1.0f, float height = 0.0f);
    void End();

    explicit operator bool() const { return m_open; }

    LeftScope Left();
    CenterScope Center();
    RightScope Right();

    using FillScope = CenterScope;
    FillScope Fill() { return Center(); }

    float GetHeight() const;
    float GetContentHeight() const;

private:
    friend class LeftScope;
    friend class CenterScope;
    friend class RightScope;

    // Явно переданный стиль; nullptr — стиль текущей темы (смена темы видна сразу)
    const ToolbarStyle* m_customStyle = nullptr;
    const ToolbarStyle& Style() const { return m_customStyle ? *m_customStyle : UiTheme::Get().toolbar; }
    bool m_open = false;
    bool m_ended = false;

    float m_posY = 0.0f;
    float m_height = 0.0f;
    float m_leftWidth = 0.0f;
    float m_rightWidth = 0.0f;
    ImGuiID m_storageId = 0;
};
