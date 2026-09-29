#pragma once

#include "ui/components/UiTheme.hpp"
#include "ui/components/Icon.hpp"
#include <imgui.h>
#include <string>
#include <functional>

// Универсальный компонент вертикального списка (List / ListView)
// Поддерживает заголовки секций, одиночный выбор, иконки, статус-индикаторы,
// подписи справа и пустые состояния. Полная инкапсуляция ImGui.
class List {
public:
    List();
    ~List();

    // Открывает скроллируемый контейнер списка
    bool Begin(const char* id, float width = 0.0f, float height = 0.0f, const ListStyle* customStyle = nullptr);
    void End();

    // Заголовок секции списка с опциональным счетчиком и кнопкой действия справа
    void Header(const char* title, const char* actionIcon = nullptr, bool* outActionClicked = nullptr, const char* badgeText = nullptr);

    // Элемент списка с поддержкой выбора, иконки/LED и текста справа
    bool Item(const char* id,
              const char* label,
              bool isSelected = false,
              const char* rightText = nullptr,
              Icon icon = Icon::None,
              ImU32 iconColor = 0,
              float height = 0.0f);

    // Двухстрочный элемент списка (заголовок + подзаголовок)
    bool ItemEx(const char* id,
                const char* label,
                const char* sublabel,
                bool isSelected = false,
                const char* rightText = nullptr,
                Icon icon = Icon::None,
                ImU32 iconColor = 0,
                float height = 0.0f);

    // Empty state display
    void Empty(const char* message = "List is empty", const char* detail = nullptr);

    // Тонкий горизонтальный разделитель
    void Separator();

private:
    const ListStyle* m_style = nullptr;
    bool m_childActive = false;
};
