#pragma once

#include <cstddef>

namespace OmniKitShowcase {

// Состояние прогона, общее для страниц и каркаса (заголовок, строка состояния)
enum class RunState { Idle, Running, Paused };

struct Session {
    bool      connected    = true;           // связь с устройством
    RunState  run          = RunState::Idle;
    double    runTimeS     = 0.0;            // время прогона без пауз
    double    value        = 0.0;            // последнее значение выбранного канала
    float     sampleRateHz = 50.0f;          // частота выборки
    std::size_t samples    = 0;              // сколько точек получено в этом прогоне
};

Session& GetSession();

// Управление прогоном: те же действия вызываются с пульта и из заголовка
void StartRun();
void StopRun();
void PauseRun(bool pause);

// Продвигает поток демо-данных на dtS секунд (вызывается раз в кадр)
void UpdateStream(double dtS);

// Страница «Test Run»: вкладки испытания
void RenderTestRunPage();

}  // namespace OmniKitShowcase
