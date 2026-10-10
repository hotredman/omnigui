#include "kvant/backend/ThorCanvas.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <thread>
#include "thorvg.h"

namespace kvant {

ThorCanvas::ThorCanvas() {
    uint32_t threads = std::max(1u, std::thread::hardware_concurrency());
    tvg::Initializer::init(threads);
    m_canvas = tvg::SwCanvas::gen(tvg::EngineOption::Default);

    // Auto-detect default fonts across common asset and system locations
    const char* defaultFontCandidates[] = {
        "assets/fonts/Roboto-Regular.ttf",
        "../assets/fonts/Roboto-Regular.ttf",
        "../../assets/fonts/Roboto-Regular.ttf",
        "../../../assets/fonts/Roboto-Regular.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf"
    };
    for (const char* candidate : defaultFontCandidates) {
        if (LoadFontFile(candidate)) {
            break;
        }
    }
}

ThorCanvas::~ThorCanvas() {
    if (m_measureText) {
        m_measureText->unref();
        m_measureText = nullptr;
    }
    if (m_canvas) {
        m_canvas->sync();
        delete m_canvas;
        m_canvas = nullptr;
    }
    tvg::Initializer::term();
}

void ThorCanvas::SetTarget(uint32_t* buffer, uint32_t width, uint32_t height, uint32_t stride) {
    m_targetBuffer = buffer;
    m_width = width;
    m_height = height;
    m_stride = stride;

    if (m_canvas && m_targetBuffer && m_width > 0 && m_height > 0) {
        m_canvas->target(m_targetBuffer, m_stride, m_width, m_height, tvg::ColorSpace::ABGR8888S);
    }
}

bool ThorCanvas::LoadFontFile(const std::string& path) {
    if (path.empty()) return false;
    tvg::Result res = tvg::Text::load(path.c_str());
    if (res == tvg::Result::Success) {
        m_defaultFontPath = path;
        m_fontLoaded = true;
        m_textMeasureCache.clear();
        return true;
    }
    return false;
}

void ThorCanvas::Begin(uint32_t width, uint32_t height) {
    m_clipStack.clear();
    if (m_width != width || m_height != height) {
        m_width = width;
        m_height = height;
        if (m_targetBuffer && m_canvas) {
            m_canvas->target(m_targetBuffer, m_stride, m_width, m_height, tvg::ColorSpace::ABGR8888S);
        }
    }
}

void ThorCanvas::End() {
    if (m_canvas) {
        m_canvas->draw(false);
        m_canvas->sync();
        m_canvas->remove();
    }
}

void ThorCanvas::Clear(Color color, std::optional<Rect> rect) {
    if (!m_targetBuffer || m_width == 0 || m_height == 0) return;

    const uint32_t rawColor = color.ToUint32();

    if (!rect.has_value()) {
        for (uint32_t y = 0; y < m_height; ++y) {
            std::fill_n(m_targetBuffer + y * m_stride, m_width, rawColor);
        }
    } else {
        const int startX = std::max(0, static_cast<int>(std::floor(rect->x)));
        const int startY = std::max(0, static_cast<int>(std::floor(rect->y)));
        const int endX = std::min(static_cast<int>(m_width), static_cast<int>(std::ceil(rect->Right())));
        const int endY = std::min(static_cast<int>(m_height), static_cast<int>(std::ceil(rect->Bottom())));
        const int w = endX - startX;
        if (w > 0 && endY > startY) {
            for (int y = startY; y < endY; ++y) {
                std::fill_n(m_targetBuffer + y * m_stride + startX, w, rawColor);
            }
        }
    }
}

void ThorCanvas::PushClip(Rect clipRect) {
    if (!m_clipStack.empty()) {
        const Rect& parent = m_clipStack.back();
        const float x1 = std::max(parent.x, clipRect.x);
        const float y1 = std::max(parent.y, clipRect.y);
        const float x2 = std::min(parent.Right(), clipRect.Right());
        const float y2 = std::min(parent.Bottom(), clipRect.Bottom());
        m_clipStack.emplace_back(x1, y1, std::max(0.0f, x2 - x1), std::max(0.0f, y2 - y1));
    } else {
        m_clipStack.push_back(clipRect);
    }
}

void ThorCanvas::PopClip() {
    if (!m_clipStack.empty()) {
        m_clipStack.pop_back();
    }
}

void ThorCanvas::ApplyClip(tvg::Paint* paint) {
    if (!paint || m_clipStack.empty()) return;
    const Rect& cr = m_clipStack.back();
    // If the active clip rect covers the entire canvas (or larger), no clipping shape is needed!
    if (cr.x <= 0.0f && cr.y <= 0.0f &&
        cr.w >= static_cast<float>(m_width) &&
        cr.h >= static_cast<float>(m_height)) {
        return;
    }
    auto clipShape = tvg::Shape::gen();
    if (cr.w <= 0.0f || cr.h <= 0.0f) {
        clipShape->appendRect(0.0f, 0.0f, 0.0f, 0.0f);
    } else {
        clipShape->appendRect(cr.x, cr.y, cr.w, cr.h);
    }
    paint->clip(clipShape);
}

void ThorCanvas::DrawRect(Rect rect, Color fill) {
    if (fill.a == 0 || rect.w <= 0.0f || rect.h <= 0.0f || !m_canvas) return;
    if (!m_clipStack.empty()) {
        const Rect& cr = m_clipStack.back();
        if (cr.w <= 0.0f || cr.h <= 0.0f || !rect.Intersects(cr)) return;
    }

    auto shape = tvg::Shape::gen();
    shape->appendRect(rect.x, rect.y, rect.w, rect.h);
    shape->fill(fill.r, fill.g, fill.b, fill.a);

    ApplyClip(shape);
    m_canvas->add(shape);
}

void ThorCanvas::DrawRoundRect(
    Rect rect,
    float radius,
    Color fill,
    Color stroke,
    float strokeWidth
) {
    if (rect.w <= 0.0f || rect.h <= 0.0f || !m_canvas) return;
    if (!m_clipStack.empty()) {
        const Rect& cr = m_clipStack.back();
        if (cr.w <= 0.0f || cr.h <= 0.0f || !rect.Intersects(cr)) return;
    }

    auto shape = tvg::Shape::gen();
    shape->appendRect(rect.x, rect.y, rect.w, rect.h, radius, radius);

    if (fill.a > 0) {
        shape->fill(fill.r, fill.g, fill.b, fill.a);
    }

    if (stroke.a > 0 && strokeWidth > 0.0f) {
        shape->strokeWidth(strokeWidth);
        shape->strokeFill(stroke.r, stroke.g, stroke.b, stroke.a);
    }

    ApplyClip(shape);
    m_canvas->add(shape);
}

Vec2 ThorCanvas::MeasureText(std::string_view text, float fontSize) {
    if (text.empty() || fontSize <= 0.0f) {
        return {0.0f, 0.0f};
    }

    uint64_t h = 14695981039346656037ULL;
    for (char c : text) {
        h = (h ^ static_cast<uint8_t>(c)) * 1099511628211ULL;
    }
    uint32_t fsBits = 0;
    std::memcpy(&fsBits, &fontSize, sizeof(float));
    const TextMeasureKey key{h, fsBits};

    auto it = m_textMeasureCache.find(key);
    if (it != m_textMeasureCache.end()) {
        return it->second;
    }

    if (!m_measureText) {
        m_measureText = tvg::Text::gen();
    }
    if (m_fontLoaded) {
        if (m_defaultFontPath.empty() || m_measureText->font(m_defaultFontPath.c_str()) != tvg::Result::Success) {
            m_measureText->font(nullptr);
        }
    }

    m_measureText->size(fontSize);
    std::string textStr(text);
    m_measureText->text(textStr.c_str());

    float totalAdvance = 0.0f;
    const char* cur = textStr.c_str();
    while (*cur) {
        tvg::GlyphMetrics gm{};
        const char* next = nullptr;
        if (m_measureText->metrics(cur, gm, &next) == tvg::Result::Success && next) {
            totalAdvance += gm.advance;
            cur = next;
        } else {
            totalAdvance += fontSize * 0.60f;
            cur++;
        }
    }
    const Vec2 result{totalAdvance, fontSize * 1.35f};

    if (m_textMeasureCache.size() > 2048) {
        m_textMeasureCache.clear();
    }
    m_textMeasureCache.emplace(key, result);
    return result;
}

void ThorCanvas::DrawText(std::string_view text, Vec2 pos, float fontSize, Color color) {
    if (text.empty() || color.a == 0 || !m_canvas) return;
    if (!m_clipStack.empty() && (m_clipStack.back().w <= 0.0f || m_clipStack.back().h <= 0.0f)) return;

    auto txt = tvg::Text::gen();
    std::string textStr(text);
    txt->text(textStr.c_str());

    if (m_fontLoaded) {
        if (m_defaultFontPath.empty() || txt->font(m_defaultFontPath.c_str()) != tvg::Result::Success) {
            txt->font(nullptr);
        }
    }

    txt->size(fontSize);
    txt->fill(color.r, color.g, color.b);
    txt->opacity(color.a);
    txt->align(0.0f, 0.0f);
    txt->translate(pos.x, pos.y);

    ApplyClip(txt);
    m_canvas->add(txt);
}

void ThorCanvas::DrawTextBox(
    std::string_view text,
    Rect bounds,
    float fontSize,
    Color color,
    float alignX,
    float alignY
) {
    if (text.empty() || color.a == 0 || bounds.w <= 0.0f || bounds.h <= 0.0f || !m_canvas) return;
    if (!m_clipStack.empty()) {
        const Rect& cr = m_clipStack.back();
        if (cr.w <= 0.0f || cr.h <= 0.0f || !bounds.Intersects(cr)) return;
    }

    auto txt = tvg::Text::gen();
    std::string textStr(text);
    txt->text(textStr.c_str());

    if (m_fontLoaded) {
        if (m_defaultFontPath.empty() || txt->font(m_defaultFontPath.c_str()) != tvg::Result::Success) {
            txt->font(nullptr);
        }
    }

    txt->size(fontSize);
    txt->fill(color.r, color.g, color.b);
    txt->opacity(color.a);
    txt->layout(bounds.w, bounds.h);
    txt->align(alignX, alignY);
    txt->translate(bounds.x, bounds.y);

    ApplyClip(txt);
    m_canvas->add(txt);
}

void ThorCanvas::DrawLine(Vec2 p1, Vec2 p2, Color color, float strokeWidth) {
    if (color.a == 0 || strokeWidth <= 0.0f || !m_canvas) return;
    if (!m_clipStack.empty()) {
        const Rect& cr = m_clipStack.back();
        if (cr.w <= 0.0f || cr.h <= 0.0f) return;
        const Rect lineBounds(
            std::min(p1.x, p2.x) - strokeWidth * 0.5f,
            std::min(p1.y, p2.y) - strokeWidth * 0.5f,
            std::abs(p2.x - p1.x) + strokeWidth,
            std::abs(p2.y - p1.y) + strokeWidth
        );
        if (!lineBounds.Intersects(cr)) return;
    }

    auto shape = tvg::Shape::gen();
    shape->moveTo(p1.x, p1.y);
    shape->lineTo(p2.x, p2.y);
    shape->strokeWidth(strokeWidth);
    shape->strokeCap(tvg::StrokeCap::Round);
    shape->strokeFill(color.r, color.g, color.b, color.a);

    ApplyClip(shape);
    m_canvas->add(shape);
}

} // namespace kvant
