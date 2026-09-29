#include "ui/components/Icon.hpp"
#include "core/Assets.hpp"
#if defined(OMNIGUI_USE_OPENGL)
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <SDL3/SDL_opengl.h>
#endif
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <vector>

namespace {

struct TextureInfo {
    ImTextureID textureId = 0;
    ImVec2 size = ImVec2(0, 0);
};

static std::unordered_map<std::string, TextureInfo> s_fileTextureCache;

static TextureInfo CreateTextureFromRgba(const unsigned char* data, int width, int height) {
#if defined(OMNIGUI_USE_OPENGL)
    if (!data || width <= 0 || height <= 0) return {};

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    TextureInfo info;
    info.textureId = (ImTextureID)(intptr_t)tex;
    info.size = ImVec2(static_cast<float>(width), static_cast<float>(height));
    return info;
#else
    (void)data; (void)width; (void)height;
    return {};
#endif
}

static TextureInfo LoadTextureFromFile(const std::string& filePath) {
#if defined(OMNIGUI_USE_OPENGL)
    auto it = s_fileTextureCache.find(filePath);
    if (it != s_fileTextureCache.end()) {
        return it->second;
    }

    std::string path = filePath;
    std::error_code ec;
    if (!std::filesystem::exists(std::filesystem::u8path(path), ec)) {
        path = Assets::Resolve(filePath);
    }

    if (!path.empty() && std::filesystem::exists(std::filesystem::u8path(path), ec)) {
        int w = 0, h = 0, comp = 0;
        unsigned char* data = stbi_load(path.c_str(), &w, &h, &comp, 4);
        if (data) {
            TextureInfo info = CreateTextureFromRgba(data, w, h);
            stbi_image_free(data);
            s_fileTextureCache[filePath] = info;
            return info;
        }
    }

    s_fileTextureCache[filePath] = {};
    return {};
#else
    (void)filePath;
    return {};
#endif
}

static TextureInfo LoadTextureFromAsset(const std::string& assetPath) {
    return LoadTextureFromFile(assetPath);
}

// Встроенная процедурная векторная отрисовка
static void DrawBuiltinVector(Icon::Id id, ImDrawList* dl, ImVec2 center, float size, ImU32 color) {
    if (!dl || id == Icon::None) return;

    float cx = center.x;
    float cy = center.y;
    float s = size / 14.0f;
    float stroke = std::max(1.0f, 1.3f * s);

    switch (id) {
        // --- Сайдбар и общие пиктограммы ---
        case Icon::Database: {
            // База данных: 3 цилиндрических диска
            dl->AddRect(ImVec2(cx - 6.5f * s, cy - 6.0f * s), ImVec2(cx + 6.5f * s, cy - 2.5f * s), color, 1.5f * s, 0, stroke);
            dl->AddRect(ImVec2(cx - 6.5f * s, cy - 1.5f * s), ImVec2(cx + 6.5f * s, cy + 2.0f * s), color, 1.5f * s, 0, stroke);
            dl->AddRect(ImVec2(cx - 6.5f * s, cy + 3.0f * s), ImVec2(cx + 6.5f * s, cy + 6.5f * s), color, 1.5f * s, 0, stroke);
            break;
        }

        case Icon::List: {
            // Журнал: 3 горизонтальные строки со списками-буллетами
            dl->AddLine(ImVec2(cx - 4.5f * s, cy - 4.5f * s), ImVec2(cx + 6.5f * s, cy - 4.5f * s), color, stroke);
            dl->AddLine(ImVec2(cx - 4.5f * s, cy),             ImVec2(cx + 6.5f * s, cy),             color, stroke);
            dl->AddLine(ImVec2(cx - 4.5f * s, cy + 4.5f * s), ImVec2(cx + 4.5f * s, cy + 4.5f * s), color, stroke);
            dl->AddCircleFilled(ImVec2(cx - 6.5f * s, cy - 4.5f * s), 1.1f * s, color);
            dl->AddCircleFilled(ImVec2(cx - 6.5f * s, cy),             1.1f * s, color);
            dl->AddCircleFilled(ImVec2(cx - 6.5f * s, cy + 4.5f * s), 1.1f * s, color);
            break;
        }

        case Icon::Cog: {
            // Шестерня машины: обод со спицами
            dl->AddCircle(center, 4.2f * s, color, 16, stroke);
            dl->AddCircleFilled(center, 1.6f * s, color);
            for (int i = 0; i < 8; ++i) {
                float angle = i * (3.14159265f / 4.0f);
                float cosA = std::cos(angle);
                float sinA = std::sin(angle);
                dl->AddLine(ImVec2(cx + cosA * 4.2f * s, cy + sinA * 4.2f * s),
                            ImVec2(cx + cosA * 6.5f * s, cy + sinA * 6.5f * s), color, stroke * 1.3f);
            }
            break;
        }

        case Icon::Target: {
            // Мишень калибровки: окружность и перекрестие
            dl->AddCircle(center, 5.5f * s, color, 16, stroke);
            dl->AddCircleFilled(center, 1.5f * s, color);
            dl->AddLine(ImVec2(cx - 7.5f * s, cy), ImVec2(cx - 4.5f * s, cy), color, stroke);
            dl->AddLine(ImVec2(cx + 4.5f * s, cy), ImVec2(cx + 7.5f * s, cy), color, stroke);
            dl->AddLine(ImVec2(cx, cy - 7.5f * s), ImVec2(cx, cy - 4.5f * s), color, stroke);
            dl->AddLine(ImVec2(cx, cy + 4.5f * s), ImVec2(cx, cy + 7.5f * s), color, stroke);
            break;
        }

        case Icon::Gamepad: {
            // Корпус геймпада
            dl->AddRect(ImVec2(cx - 7.0f * s, cy - 4.5f * s), ImVec2(cx + 7.0f * s, cy + 4.5f * s), color, 2.0f * s, 0, stroke);
            dl->AddLine(ImVec2(cx - 4.5f * s, cy - 2.0f * s), ImVec2(cx - 4.5f * s, cy + 2.0f * s), color, stroke);
            dl->AddLine(ImVec2(cx - 6.5f * s, cy),             ImVec2(cx - 2.5f * s, cy),             color, stroke);
            dl->AddCircleFilled(ImVec2(cx + 3.2f * s, cy - 1.2f * s), 1.0f * s, color);
            dl->AddCircleFilled(ImVec2(cx + 5.0f * s, cy + 1.2f * s), 1.0f * s, color);
            break;
        }

        case Icon::Gear: {
            // Шестеренка установок (6 зубцов)
            dl->AddCircle(center, 3.8f * s, color, 12, stroke);
            dl->AddCircleFilled(center, 1.4f * s, color);
            for (int i = 0; i < 6; ++i) {
                float angle = i * (3.14159265f / 3.0f);
                float cosA = std::cos(angle);
                float sinA = std::sin(angle);
                dl->AddLine(ImVec2(cx + cosA * 3.8f * s, cy + sinA * 3.8f * s),
                            ImVec2(cx + cosA * 6.2f * s, cy + sinA * 6.2f * s), color, stroke * 1.3f);
            }
            break;
        }

        case Icon::LineChart: {
            // Документ с ломаной линией диаграммы
            dl->AddRect(ImVec2(cx - 5.5f * s, cy - 6.5f * s), ImVec2(cx + 5.5f * s, cy + 6.5f * s), color, 1.5f * s, 0, stroke);
            ImVec2 pts[4] = {
                ImVec2(cx - 3.5f * s, cy + 2.5f * s),
                ImVec2(cx - 1.0f * s, cy - 1.5f * s),
                ImVec2(cx + 1.2f * s, cy + 1.0f * s),
                ImVec2(cx + 3.5f * s, cy - 3.0f * s)
            };
            dl->AddPolyline(pts, 4, color, 0, stroke);
            break;
        }

        case Icon::Clipboard: {
            // Планшет с зажимом (клипборд)
            dl->AddRect(ImVec2(cx - 5.5f * s, cy - 6.0f * s), ImVec2(cx + 5.5f * s, cy + 6.5f * s), color, 1.5f * s, 0, stroke);
            dl->AddRectFilled(ImVec2(cx - 2.5f * s, cy - 7.5f * s), ImVec2(cx + 2.5f * s, cy - 5.0f * s), color, 1.0f * s);
            dl->AddLine(ImVec2(cx - 3.5f * s, cy - 1.0f * s), ImVec2(cx + 3.5f * s, cy - 1.0f * s), color, stroke);
            dl->AddLine(ImVec2(cx - 3.5f * s, cy + 2.5f * s), ImVec2(cx + 3.5f * s, cy + 2.5f * s), color, stroke);
            break;
        }

        case Icon::BarChart: {
            // Столбчатая гистограмма (3 столбика разной высоты)
            dl->AddRectFilled(ImVec2(cx - 5.5f * s, cy + 1.0f * s), ImVec2(cx - 2.5f * s, cy + 6.0f * s), color, 0.5f * s);
            dl->AddRectFilled(ImVec2(cx - 1.5f * s, cy - 2.5f * s), ImVec2(cx + 1.5f * s, cy + 6.0f * s), color, 0.5f * s);
            dl->AddRectFilled(ImVec2(cx + 2.5f * s, cy - 6.0f * s), ImVec2(cx + 5.5f * s, cy + 6.0f * s), color, 0.5f * s);
            break;
        }

        // --- Пульт и управление приводом (Remote Control) ---
        case Icon::Play: {
            // Треугольник вправо (ПУСК)
            ImVec2 p1(cx - 4.0f * s, cy - 5.5f * s);
            ImVec2 p2(cx - 4.0f * s, cy + 5.5f * s);
            ImVec2 p3(cx + 5.5f * s, cy);
            dl->AddTriangleFilled(p1, p2, p3, color);
            break;
        }

        case Icon::Square: {
            // Правильный квадрат (СТОП)
            dl->AddRectFilled(ImVec2(cx - 5.0f * s, cy - 5.0f * s), ImVec2(cx + 5.0f * s, cy + 5.0f * s), color, 1.2f * s);
            break;
        }

        case Icon::Pause: {
            // Две параллельные вертикальные полосы (Пауза)
            dl->AddRectFilled(ImVec2(cx - 4.8f * s, cy - 5.5f * s), ImVec2(cx - 1.5f * s, cy + 5.5f * s), color, 0.8f * s);
            dl->AddRectFilled(ImVec2(cx + 1.5f * s, cy - 5.5f * s), ImVec2(cx + 4.8f * s, cy + 5.5f * s), color, 0.8f * s);
            break;
        }

        case Icon::RotateCcw: {
            // Возврат: круговая стрелка с горизонтальным плечом и четким треугольным наконечником влево
            float r = 4.6f * s;
            ImVec2 arcCenter(cx + 0.6f * s, cy + 0.5f * s);
            float topY = arcCenter.y - r;

            // Наконечник стрелки (треугольник, смотрящий строго влево)
            ImVec2 tip(cx - 5.2f * s, topY);
            ImVec2 baseTop(cx - 1.4f * s, topY - 2.6f * s);
            ImVec2 baseBot(cx - 1.4f * s, topY + 2.6f * s);
            dl->AddTriangleFilled(tip, baseTop, baseBot, color);

            // Плечо от основания треугольника и дуга по часовой стрелке до нижнего сектора
            const int nArc = 14;
            ImVec2 arcPts[1 + nArc];
            arcPts[0] = ImVec2(cx - 1.4f * s, topY);
            for (int i = 0; i < nArc; ++i) {
                float a = -0.5f * 3.14159265f + (1.25f * 3.14159265f) * (float(i) / (nArc - 1));
                arcPts[1 + i] = ImVec2(arcCenter.x + r * std::cos(a), arcCenter.y + r * std::sin(a));
            }
            dl->AddPolyline(arcPts, 1 + nArc, color, 0, stroke);
            dl->AddCircleFilled(arcPts[nArc], stroke * 0.5f, color);
            break;
        }

        case Icon::ArrowDownToLine: {
            // Стрелка вниз в линию/платформу (Разгрузка)
            dl->AddLine(ImVec2(cx - 6.0f * s, cy + 5.5f * s), ImVec2(cx + 6.0f * s, cy + 5.5f * s), color, stroke * 1.3f);
            dl->AddLine(ImVec2(cx, cy - 6.0f * s), ImVec2(cx, cy + 2.5f * s), color, stroke);
            dl->AddLine(ImVec2(cx, cy + 2.5f * s), ImVec2(cx - 3.8f * s, cy - 1.0f * s), color, stroke);
            dl->AddLine(ImVec2(cx, cy + 2.5f * s), ImVec2(cx + 3.8f * s, cy - 1.0f * s), color, stroke);
            break;
        }

        case Icon::Refresh:
        case Icon::RefreshCw: {
            // Сброс аварии / Заводские: круговая стрелка по часовой стрелке (Refresh / Reset)
            float r = 4.6f * s;
            ImVec2 arcCenter(cx - 0.6f * s, cy + 0.5f * s);
            float topY = arcCenter.y - r;

            // Наконечник стрелки (треугольник, смотрящий строго вправо)
            ImVec2 tip(cx + 5.2f * s, topY);
            ImVec2 baseTop(cx + 1.4f * s, topY - 2.6f * s);
            ImVec2 baseBot(cx + 1.4f * s, topY + 2.6f * s);
            dl->AddTriangleFilled(tip, baseTop, baseBot, color);

            // Плечо от вершины дуги к основанию стрелки и дуга по часовой стрелке (через низ и лево к вершине)
            const int nArc = 14;
            ImVec2 arcPts[1 + nArc];
            arcPts[0] = ImVec2(cx + 1.4f * s, topY);
            for (int i = 0; i < nArc; ++i) {
                float a = -0.5f * 3.14159265f - (1.25f * 3.14159265f) * (float(i) / (nArc - 1));
                arcPts[1 + i] = ImVec2(arcCenter.x + r * std::cos(a), arcCenter.y + r * std::sin(a));
            }
            dl->AddPolyline(arcPts, 1 + nArc, color, 0, stroke);
            dl->AddCircleFilled(arcPts[nArc], stroke * 0.5f, color);
            break;
        }

        case Icon::Upload:
        case Icon::ArrowUp: {
            // Стрелка вверх (ВВЕРХ / Записать)
            dl->AddLine(ImVec2(cx, cy + 5.5f * s), ImVec2(cx, cy - 5.0f * s), color, stroke);
            dl->AddLine(ImVec2(cx, cy - 5.0f * s), ImVec2(cx - 4.5f * s, cy - 0.5f * s), color, stroke);
            dl->AddLine(ImVec2(cx, cy - 5.0f * s), ImVec2(cx + 4.5f * s, cy - 0.5f * s), color, stroke);
            break;
        }

        case Icon::Download:
        case Icon::ArrowDown: {
            // Стрелка вниз (ВНИЗ / Прочитать)
            dl->AddLine(ImVec2(cx, cy - 5.5f * s), ImVec2(cx, cy + 5.0f * s), color, stroke);
            dl->AddLine(ImVec2(cx, cy + 5.0f * s), ImVec2(cx - 4.5f * s, cy + 0.5f * s), color, stroke);
            dl->AddLine(ImVec2(cx, cy + 5.0f * s), ImVec2(cx + 4.5f * s, cy + 0.5f * s), color, stroke);
            break;
        }

        case Icon::ChevronsUp: {
            // Сдвоенные шевроны вверх
            dl->AddLine(ImVec2(cx - 5.0f * s, cy - 2.0f * s), ImVec2(cx, cy - 6.0f * s), color, stroke);
            dl->AddLine(ImVec2(cx + 5.0f * s, cy - 2.0f * s), ImVec2(cx, cy - 6.0f * s), color, stroke);
            dl->AddLine(ImVec2(cx - 5.0f * s, cy + 3.5f * s), ImVec2(cx, cy - 0.5f * s), color, stroke);
            dl->AddLine(ImVec2(cx + 5.0f * s, cy + 3.5f * s), ImVec2(cx, cy - 0.5f * s), color, stroke);
            break;
        }

        case Icon::ChevronsDown: {
            // Сдвоенные шевроны вниз
            dl->AddLine(ImVec2(cx - 5.0f * s, cy - 3.5f * s), ImVec2(cx, cy + 0.5f * s), color, stroke);
            dl->AddLine(ImVec2(cx + 5.0f * s, cy - 3.5f * s), ImVec2(cx, cy + 0.5f * s), color, stroke);
            dl->AddLine(ImVec2(cx - 5.0f * s, cy + 2.0f * s), ImVec2(cx, cy + 6.0f * s), color, stroke);
            dl->AddLine(ImVec2(cx + 5.0f * s, cy + 2.0f * s), ImVec2(cx, cy + 6.0f * s), color, stroke);
            break;
        }

        // --- Навигация и прокрутка (Navigation / Carousel / Scrolling) ---
        case Icon::ChevronLeft:
        case Icon::ArrowLeft: {
            // Шеврон влево (<): аккуратный компактный геометрический наконечник
            ImVec2 top(cx + 1.8f * s, cy - 3.8f * s);
            ImVec2 tip(cx - 1.8f * s, cy);
            ImVec2 bot(cx + 1.8f * s, cy + 3.8f * s);
            ImVec2 pts[3] = { top, tip, bot };
            dl->AddPolyline(pts, 3, color, 0, stroke);
            break;
        }

        case Icon::ChevronRight:
        case Icon::ArrowRight: {
            // Шеврон вправо (>): аккуратный компактный геометрический наконечник
            ImVec2 top(cx - 1.8f * s, cy - 3.8f * s);
            ImVec2 tip(cx + 1.8f * s, cy);
            ImVec2 bot(cx - 1.8f * s, cy + 3.8f * s);
            ImVec2 pts[3] = { top, tip, bot };
            dl->AddPolyline(pts, 3, color, 0, stroke);
            break;
        }

        // --- Системные и элементы шапки ---
        case Icon::Power: {
            // Кнопка питания: разомкнутый сверху круг (дуга 306°) и вертикальная черта
            float r = 5.0f * s;
            const int nPts = 24;
            ImVec2 arcPts[nPts];
            // Симметричный разрыв вверху: от -0.35*pi по часовой стрелке до 1.35*pi
            float startAngle = -0.35f * 3.14159265f;
            float endAngle   =  1.35f * 3.14159265f;
            for (int i = 0; i < nPts; ++i) {
                float a = startAngle + (endAngle - startAngle) * (float(i) / (nPts - 1));
                arcPts[i] = ImVec2(cx + r * std::cos(a), cy + r * std::sin(a));
            }
            dl->AddPolyline(arcPts, nPts, color, 0, stroke);
            dl->AddCircleFilled(arcPts[0], stroke * 0.5f, color);
            dl->AddCircleFilled(arcPts[nPts - 1], stroke * 0.5f, color);

            // Вертикальная черта сверху вниз к центру
            ImVec2 lineTop(cx, cy - 5.8f * s);
            ImVec2 lineBot(cx, cy - 0.5f * s);
            dl->AddLine(lineTop, lineBot, color, stroke);
            dl->AddCircleFilled(lineTop, stroke * 0.5f, color);
            dl->AddCircleFilled(lineBot, stroke * 0.5f, color);
            break;
        }

        case Icon::Maximize: {
            // Раздвигающиеся 4 угла полноэкранного режима
            float d = 5.5f * s;
            float l = 2.8f * s;
            dl->AddLine(ImVec2(cx - d, cy - d), ImVec2(cx - d + l, cy - d), color, stroke);
            dl->AddLine(ImVec2(cx - d, cy - d), ImVec2(cx - d, cy - d + l), color, stroke);
            dl->AddLine(ImVec2(cx + d, cy - d), ImVec2(cx + d - l, cy - d), color, stroke);
            dl->AddLine(ImVec2(cx + d, cy - d), ImVec2(cx + d, cy - d + l), color, stroke);
            dl->AddLine(ImVec2(cx - d, cy + d), ImVec2(cx - d + l, cy + d), color, stroke);
            dl->AddLine(ImVec2(cx - d, cy + d), ImVec2(cx - d, cy + d - l), color, stroke);
            dl->AddLine(ImVec2(cx + d, cy + d), ImVec2(cx + d - l, cy + d), color, stroke);
            dl->AddLine(ImVec2(cx + d, cy + d), ImVec2(cx + d, cy + d - l), color, stroke);
            break;
        }

        case Icon::Minimize: {
            // 4 сходящихся угла к центру (Оконный режим / Выход из полного экрана)
            float d = 5.5f * s;
            float m = 1.8f * s;
            // Верхний левый
            dl->AddLine(ImVec2(cx - d, cy - m), ImVec2(cx - m, cy - m), color, stroke);
            dl->AddLine(ImVec2(cx - m, cy - d), ImVec2(cx - m, cy - m), color, stroke);
            // Верхний правый
            dl->AddLine(ImVec2(cx + d, cy - m), ImVec2(cx + m, cy - m), color, stroke);
            dl->AddLine(ImVec2(cx + m, cy - d), ImVec2(cx + m, cy - m), color, stroke);
            // Нижний левый
            dl->AddLine(ImVec2(cx - d, cy + m), ImVec2(cx - m, cy + m), color, stroke);
            dl->AddLine(ImVec2(cx - m, cy + d), ImVec2(cx - m, cy + m), color, stroke);
            // Нижний правый
            dl->AddLine(ImVec2(cx + d, cy + m), ImVec2(cx + m, cy + m), color, stroke);
            dl->AddLine(ImVec2(cx + m, cy + d), ImVec2(cx + m, cy + m), color, stroke);
            break;
        }

        case Icon::Sun: {
            // Солнце: диск и 8 радиальных лучей
            dl->AddCircle(center, 3.2f * s, color, 12, stroke);
            for (int i = 0; i < 8; ++i) {
                float angle = i * (3.14159265f / 4.0f);
                float cosA = std::cos(angle);
                float sinA = std::sin(angle);
                dl->AddLine(ImVec2(cx + cosA * 4.5f * s, cy + sinA * 4.5f * s),
                            ImVec2(cx + cosA * 6.5f * s, cy + sinA * 6.5f * s), color, stroke);
            }
            break;
        }

        case Icon::Moon: {
            // Элегантный полумесяц ночной темы (классический серп Луны)
            // Внешняя дуга диска Луны
            float x1 = cx - 0.4f * s, y1 = cy + 0.4f * s, r1 = 5.5f * s;
            // Внутренняя вырезающая дуга тени (увеличенный радиус тени для изящного силуэта)
            float x2 = cx + 1.25f * s, y2 = cy - 1.25f * s, r2 = 5.25f * s;

            float dx = x2 - x1, dy = y2 - y1;
            float d = std::sqrt(dx * dx + dy * dy);
            float a = (r1 * r1 - r2 * r2 + d * d) / (2.0f * d);
            float h = std::sqrt(std::max(0.0f, r1 * r1 - a * a));

            float ux = dx / d, uy = dy / d;
            float vx = -uy, vy = ux;

            float px = x1 + a * ux, py = y1 + a * uy;
            ImVec2 t1(px + h * vx, py + h * vy);
            ImVec2 t2(px - h * vx, py - h * vy);

            float a1_start = std::atan2(t1.y - y1, t1.x - x1);
            float a1_end   = std::atan2(t2.y - y1, t2.x - x1);
            if (a1_end < a1_start) a1_end += 2.0f * 3.14159265f;

            float a2_start = std::atan2(t2.y - y2, t2.x - x2);
            float a2_end   = std::atan2(t1.y - y2, t1.x - x2);
            if (a2_end > a2_start) a2_end -= 2.0f * 3.14159265f;

            constexpr int N = 16;
            ImVec2 outer[N + 1];
            ImVec2 inner[N + 1];

            for (int i = 0; i <= N; ++i) {
                float f = static_cast<float>(i) / static_cast<float>(N);
                float ang1 = a1_start + (a1_end - a1_start) * f;
                outer[i] = ImVec2(x1 + r1 * std::cos(ang1), y1 + r1 * std::sin(ang1));

                float ang2 = a2_end + (a2_start - a2_end) * f;
                inner[i] = ImVec2(x2 + r2 * std::cos(ang2), y2 + r2 * std::sin(ang2));
            }

            // Заливка триангуляцией выпуклых квад-полос
            for (int i = 0; i < N; ++i) {
                dl->AddTriangleFilled(outer[i], outer[i + 1], inner[i + 1], color);
                dl->AddTriangleFilled(outer[i], inner[i + 1], inner[i], color);
            }

            // Сглаживающий антиалиасинг контура
            ImVec2 poly[N * 2];
            for (int i = 0; i <= N; ++i) {
                poly[i] = outer[i];
            }
            for (int i = 1; i < N; ++i) {
                poly[N + i] = inner[N - i];
            }
            dl->AddPolyline(poly, N * 2, color, ImDrawFlags_Closed, stroke * 0.8f);
            break;
        }

        case Icon::Zero: {
            // Нулевая точка / тарирование
            dl->AddCircle(center, 5.0f * s, color, 16, stroke);
            dl->AddCircleFilled(center, 1.8f * s, color);
            break;
        }

        case Icon::Check: {
            // Галочка подтверждения
            ImVec2 p1(cx - 5.0f * s, cy + 0.2f * s);
            ImVec2 p2(cx - 1.5f * s, cy + 4.2f * s);
            ImVec2 p3(cx + 5.5f * s, cy - 4.5f * s);
            dl->AddLine(p1, p2, color, stroke * 1.2f);
            dl->AddLine(p2, p3, color, stroke * 1.2f);
            break;
        }

        case Icon::Pencil: {
            // Карандаш под 45 градусов (направление от верхнего левого к нижнему правому)
            const float invSqrt2 = 0.70710678f;
            float ux = invSqrt2 * s;
            float uy = invSqrt2 * s;
            float vx = -invSqrt2 * s;
            float vy =  invSqrt2 * s;
            float hw = 2.0f; // полуширина корпуса

            ImVec2 p_tip(cx + 6.2f * ux, cy + 6.2f * uy);
            ImVec2 p_cone_l(cx + 2.6f * ux + hw * vx, cy + 2.6f * uy + hw * vy);
            ImVec2 p_cone_r(cx + 2.6f * ux - hw * vx, cy + 2.6f * uy - hw * vy);

            ImVec2 p_back_l(cx - 5.5f * ux + hw * vx, cy - 5.5f * uy + hw * vy);
            ImVec2 p_back_r(cx - 5.5f * ux - hw * vx, cy - 5.5f * uy - hw * vy);

            // Контур корпуса карандаша
            ImVec2 poly[5] = { p_back_l, p_back_r, p_cone_r, p_tip, p_cone_l };
            dl->AddPolyline(poly, 5, color, ImDrawFlags_Closed, stroke);

            // Линия отсечки грифеля
            dl->AddLine(p_cone_l, p_cone_r, color, stroke);

            // Залитый кончик грифеля
            ImVec2 lead_tip(cx + 6.2f * ux, cy + 6.2f * uy);
            ImVec2 lead_l(cx + 4.4f * ux + 0.9f * vx, cy + 4.4f * uy + 0.9f * vy);
            ImVec2 lead_r(cx + 4.4f * ux - 0.9f * vx, cy + 4.4f * uy - 0.9f * vy);
            ImVec2 lead_poly[3] = { lead_l, lead_r, lead_tip };
            dl->AddConvexPolyFilled(lead_poly, 3, color);
            break;
        }

        case Icon::Plus: {
            // Знак плюс: центрированное перекрестие равной длины
            float arm = 4.6f * s;
            float lineStroke = stroke * 1.35f;
            ImVec2 l(cx - arm, cy), r(cx + arm, cy);
            ImVec2 t(cx, cy - arm), b(cx, cy + arm);
            dl->AddLine(l, r, color, lineStroke);
            dl->AddLine(t, b, color, lineStroke);
            float capR = lineStroke * 0.5f;
            dl->AddCircleFilled(l, capR, color);
            dl->AddCircleFilled(r, capR, color);
            dl->AddCircleFilled(t, capR, color);
            dl->AddCircleFilled(b, capR, color);
            break;
        }

        case Icon::Close: {
            // Крестик закрытия / очистки / сброса: центрированное диагональное перекрестие (X)
            float arm = 3.8f * s;
            float lineStroke = stroke * 1.35f;
            ImVec2 p1(cx - arm, cy - arm), p2(cx + arm, cy + arm);
            ImVec2 p3(cx + arm, cy - arm), p4(cx - arm, cy + arm);
            dl->AddLine(p1, p2, color, lineStroke);
            dl->AddLine(p3, p4, color, lineStroke);
            float capR = lineStroke * 0.5f;
            dl->AddCircleFilled(p1, capR, color);
            dl->AddCircleFilled(p2, capR, color);
            dl->AddCircleFilled(p3, capR, color);
            dl->AddCircleFilled(p4, capR, color);
            break;
        }

        case Icon::Aa: {
            // Пиктограмма двух букв "Aa" (масштаб интерфейса / типографика)
            // 1. Прописная буква 'A' (слева)
            float ax = cx - 0.35f * s; // оптический центрирующий сдвиг пары
            ImVec2 p_top(ax - 3.2f * s, cy - 5.5f * s);
            ImVec2 p_bot_l(ax - 6.4f * s, cy + 5.5f * s);
            ImVec2 p_bot_r(ax + 0.0f * s, cy + 5.5f * s);
            dl->AddLine(p_bot_l, p_top, color, stroke * 1.25f);
            dl->AddLine(p_top, p_bot_r, color, stroke * 1.25f);
            dl->AddLine(ImVec2(ax - 5.0f * s, cy + 1.4f * s),
                        ImVec2(ax - 1.4f * s, cy + 1.4f * s), color, stroke * 1.15f);

            // 2. Строчная буква 'a' (справа)
            ImVec2 bowlCenter(ax + 3.8f * s, cy + 2.5f * s);
            float bowlR = 2.6f * s;
            dl->AddCircle(bowlCenter, bowlR, color, 14, stroke * 1.2f);
            // Вертикальный штрих (ножка) 'a' с легким нижним хвостиком
            dl->AddLine(ImVec2(ax + 6.4f * s, cy - 0.1f * s),
                        ImVec2(ax + 6.4f * s, cy + 4.5f * s), color, stroke * 1.2f);
            dl->AddLine(ImVec2(ax + 7.1f * s, cy + 5.5f * s),
                        ImVec2(ax + 6.4f * s, cy + 4.5f * s), color, stroke * 1.1f);
            break;
        }

        case Icon::Folder: {
            // Папка: верхний ярлык-закладка и прямоугольный корпус
            dl->AddLine(ImVec2(cx - 6.0f * s, cy - 4.5f * s), ImVec2(cx - 2.5f * s, cy - 4.5f * s), color, stroke);
            dl->AddLine(ImVec2(cx - 2.5f * s, cy - 4.5f * s), ImVec2(cx - 1.0f * s, cy - 2.5f * s), color, stroke);
            dl->AddLine(ImVec2(cx - 1.0f * s, cy - 2.5f * s), ImVec2(cx + 6.0f * s, cy - 2.5f * s), color, stroke);
            dl->AddRect(ImVec2(cx - 6.0f * s, cy - 2.5f * s), ImVec2(cx + 6.0f * s, cy + 5.0f * s), color, 1.2f * s, 0, stroke);
            break;
        }

        case Icon::Trash: {
            // Корзина: крышка с ручкой и корпус с 2 вертикальными ребрами
            dl->AddLine(ImVec2(cx - 6.0f * s, cy - 3.5f * s), ImVec2(cx + 6.0f * s, cy - 3.5f * s), color, stroke * 1.2f);
            dl->AddLine(ImVec2(cx - 2.2f * s, cy - 3.5f * s), ImVec2(cx - 2.2f * s, cy - 5.5f * s), color, stroke);
            dl->AddLine(ImVec2(cx - 2.2f * s, cy - 5.5f * s), ImVec2(cx + 2.2f * s, cy - 5.5f * s), color, stroke);
            dl->AddLine(ImVec2(cx + 2.2f * s, cy - 5.5f * s), ImVec2(cx + 2.2f * s, cy - 3.5f * s), color, stroke);
            // Корпус ведра
            dl->AddLine(ImVec2(cx - 4.5f * s, cy - 3.5f * s), ImVec2(cx - 3.8f * s, cy + 5.5f * s), color, stroke);
            dl->AddLine(ImVec2(cx + 4.5f * s, cy - 3.5f * s), ImVec2(cx + 3.8f * s, cy + 5.5f * s), color, stroke);
            dl->AddLine(ImVec2(cx - 3.8f * s, cy + 5.5f * s), ImVec2(cx + 3.8f * s, cy + 5.5f * s), color, stroke);
            // Внутренние ребра
            dl->AddLine(ImVec2(cx - 1.4f * s, cy - 1.5f * s), ImVec2(cx - 1.4f * s, cy + 3.5f * s), color, stroke * 0.9f);
            dl->AddLine(ImVec2(cx + 1.4f * s, cy - 1.5f * s), ImVec2(cx + 1.4f * s, cy + 3.5f * s), color, stroke * 0.9f);
            break;
        }

        case Icon::Copy: {
            // Два наложенных документа (дублирование)
            // Задний лист
            dl->AddRect(ImVec2(cx - 3.0f * s, cy - 6.0f * s), ImVec2(cx + 5.5f * s, cy + 2.5f * s), color, 1.0f * s, 0, stroke);
            // Передний лист
            dl->AddRect(ImVec2(cx - 5.5f * s, cy - 2.5f * s), ImVec2(cx + 3.0f * s, cy + 6.0f * s), color, 1.0f * s, 0, stroke);
            break;
        }

        case Icon::ExternalLink: {
            // Квадрат со стрелкой наружу (открыть)
            dl->AddLine(ImVec2(cx + 1.0f * s, cy - 5.0f * s), ImVec2(cx - 5.0f * s, cy - 5.0f * s), color, stroke);
            dl->AddLine(ImVec2(cx - 5.0f * s, cy - 5.0f * s), ImVec2(cx - 5.0f * s, cy + 5.0f * s), color, stroke);
            dl->AddLine(ImVec2(cx - 5.0f * s, cy + 5.0f * s), ImVec2(cx + 5.0f * s, cy + 5.0f * s), color, stroke);
            dl->AddLine(ImVec2(cx + 5.0f * s, cy + 5.0f * s), ImVec2(cx + 5.0f * s, cy - 1.0f * s), color, stroke);
            // Стрелка наружу в правый верхний угол
            dl->AddLine(ImVec2(cx - 0.5f * s, cy + 0.5f * s), ImVec2(cx + 5.5f * s, cy - 5.5f * s), color, stroke * 1.2f);
            dl->AddLine(ImVec2(cx + 2.0f * s, cy - 5.5f * s), ImVec2(cx + 5.5f * s, cy - 5.5f * s), color, stroke * 1.2f);
            dl->AddLine(ImVec2(cx + 5.5f * s, cy - 2.0f * s), ImVec2(cx + 5.5f * s, cy - 5.5f * s), color, stroke * 1.2f);
            break;
        }

        case Icon::None:
        default:
            break;
    }
}

} // anonymous namespace

// -----------------------------------------------------------------------------
// Методы класса Icon
// -----------------------------------------------------------------------------
void Icon::Draw(ImDrawList* dl, ImVec2 center, float size, ImU32 color) const {
    if (!dl || m_type == SourceType::None || size <= 0.0f) return;

    if (m_type == SourceType::Builtin) {
        DrawBuiltinVector(m_id, dl, center, size, color);
        return;
    }

    TextureInfo img;
    if (m_type == SourceType::Asset) {
        img = LoadTextureFromAsset(m_path);
    } else if (m_type == SourceType::File) {
        img = LoadTextureFromFile(m_path);
    }

    if (img.textureId != 0 && img.size.x > 0.0f && img.size.y > 0.0f) {
        float aspect = img.size.x / img.size.y;
        float drawW = size;
        float drawH = size;
        if (aspect > 1.0f) {
            drawH = size / aspect;
        } else {
            drawW = size * aspect;
        }
        ImVec2 pMin(center.x - drawW * 0.5f, center.y - drawH * 0.5f);
        ImVec2 pMax(center.x + drawW * 0.5f, center.y + drawH * 0.5f);
        ImU32 tint = m_tintable ? color : IM_COL32_WHITE;
        dl->AddImage(img.textureId, pMin, pMax, ImVec2(0, 0), ImVec2(1, 1), tint);
    }
}

void Icon::DrawAt(ImDrawList* dl, ImVec2 pos, float size, ImU32 color) const {
    ImVec2 center(pos.x + size * 0.5f, pos.y + size * 0.5f);
    Draw(dl, center, size, color);
}

void Icon::Render(float size, ImU32 color) const {
    if (IsEmpty()) return;
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    DrawAt(dl, cursor, size, color);
    ImGui::Dummy(ImVec2(size, size));
}
