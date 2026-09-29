#pragma once

#include "ui/components/UiTheme.hpp"
#include <functional>
#include <string>
#include <vector>

enum class DeviceState {
    Disconnected, // Серый: прибор обесточен / оффлайн
    Connecting,   // Жёлтый: опрос, подключение, рукопожатие
    Idle,         // Зелёный: связь есть, прибор в покое и готов к работе
    Running,      // Синий: активный рабочий процесс
    Paused,       // Янтарный/оранжевый: пауза
    Fault         // Красный: авария, перегрузка, сбой
};

class DeviceStatus {
public:
    // Предварительный расчет требуемой ширины плашки с учетом полной геометрии:
    // замеряет список кандидатов на имя устройства (titleCandidates), текущее имя (currentTitle),
    // все возможные состояния ("ОТКЛЮЧЕН", "ПОДКЛЮЧЕНИЕ" и др.), отступы, светодиод и кнопку действия.
    static float CalculateWidth(const std::vector<std::string>& titleCandidates = {},
                                const std::string& currentTitle = "",
                                const DeviceStatusStyle* customStyle = nullptr);

    // Отрисовка бейджа статуса устройства с кнопкой действия (колбэк или возврат bool)
    // Возвращает true, если кнопка действия была нажата; без колбэка кнопка
    // видна, но выключена
    static bool Render(const char* deviceName, 
                       DeviceState state, 
                       const char* statusText = nullptr, 
                       const char* buttonLabel = "[P]",
                       std::function<void()> onAction = nullptr,
                       float width = 0.0f,
                       const DeviceStatusStyle* customStyle = nullptr);

    // Отрисовка с автоматическим расчетом ширины по переданным кандидатам
    static bool RenderAutoWidth(const char* deviceName,
                                DeviceState state,
                                const std::vector<std::string>& titleCandidates,
                                const char* statusText = nullptr,
                                const char* buttonLabel = "[P]",
                                std::function<void()> onAction = nullptr,
                                const DeviceStatusStyle* customStyle = nullptr);

    // Вспомогательный метод для получения стандартного текста состояния
    static const char* GetDefaultStateText(DeviceState state);

    // Вспомогательный метод для получения цвета светодиода
    static ImU32 GetStateLedColor(DeviceState state, const DeviceStatusStyle& style);
};
