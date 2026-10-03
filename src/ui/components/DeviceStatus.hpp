#pragma once

#include "ui/components/UiTheme.hpp"
#include "ui/components/UiKey.hpp"
#include <string>
#include <vector>
#include <source_location>

enum class DeviceState {
    Disconnected, // Серый: прибор обесточен / оффлайн
    Connecting,   // Жёлтый: опрос, подключение, рукопожатие
    Idle,         // Зелёный: связь есть, прибор в покое и готов к работе
    Running,      // Синий: активный рабочий процесс
    Paused,       // Янтарный/оранжевый: пауза
    Fault         // Красный: авария, перегрузка, сбой
};

// Параметры плашки статуса устройства (designated initializers):
//
//     if (DeviceStatus("Cluster 07", state, {.status = "STANDBY", .action = "S"})) { ... }
//     if (DeviceStatus("Cluster 07", state)) { ... }   // кнопка питания (Connect / Disconnect)
struct DeviceStatusOptions {
    const char* status = nullptr;       // текст статуса; nullptr — стандартный для состояния
    const char* action = nullptr;       // короткий глиф (1–2 символа) в квадратной кнопке; nullptr — иконка питания
    bool        showAction = true;      // показывать кнопку действия
    bool        actionDisabled = false; // кнопка видна, но выключена
    float       width = 0.0f;           // базовые px (масштабируются внутри); 0 — по стилю темы
    UiKey       key = {};               // идентичность; по умолчанию — deviceName

    // Кандидаты на имя устройства: если не пусто, ширина подбирается автоматически под самое
    // длинное имя / статус (плашка не «прыгает» при смене имени); width тогда игнорируется
    std::vector<std::string> titleCandidates;

    const DeviceStatusStyle* style = nullptr;   // оверрайд стиля; nullptr — из темы
};

// Плашка статуса устройства с кнопкой действия.
// Возвращает true, если кнопка действия была нажата
bool DeviceStatus(const char* deviceName, DeviceState state, const DeviceStatusOptions& options = {},
                  std::source_location loc = std::source_location::current());

// Предварительный расчет требуемой ширины плашки с учетом полной геометрии (в финальных px):
// замеряет список кандидатов на имя устройства (titleCandidates), текущее имя (currentTitle),
// все возможные состояния ("ОТКЛЮЧЕН", "ПОДКЛЮЧЕНИЕ" и др.), отступы, светодиод и кнопку действия.
float DeviceStatusWidth(const std::vector<std::string>& titleCandidates = {},
                        const std::string& currentTitle = "",
                        const DeviceStatusStyle* style = nullptr);

// Стандартный текст состояния
const char* DeviceStateText(DeviceState state);

// Цвет светодиода состояния
ImU32 DeviceStateColor(DeviceState state, const DeviceStatusStyle& style);
