#include "ImageViewer.hpp"
#include "WindowLevel.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace OmniKit {

ImageViewer::ImageViewer() {
    m_texDesc.pixels = nullptr;
    m_texDesc.width = 0;
    m_texDesc.height = 0;
}

ImageViewer::~ImageViewer() = default;

void ImageViewer::LoadImage16(const uint16_t* data, int width, int height) {
    if (!data || width <= 0 || height <= 0) return;
    m_origWidth = width;
    m_origHeight = height;
    const size_t total = static_cast<size_t>(width) * height;
    m_raw16.assign(data, data + total);

    m_histogram.Compute(data, width, height);
    AutoFitWindow();

    m_needFit = true;
    m_displayDirty = true;
}

void ImageViewer::SetParams(const WindowLevelParams& params) {
    if (m_params.window != params.window ||
        m_params.level != params.level ||
        m_params.gamma != params.gamma ||
        m_params.invert != params.invert ||
        m_params.colormap != params.colormap) {
        m_params = params;
        m_displayDirty = true;
    }
}

void ImageViewer::AutoFitWindow() {
    if (m_raw16.empty() || m_origWidth <= 0 || m_origHeight <= 0) return;
    float w, l;
    if (ComputeAutoWindow(m_raw16.data(), m_origWidth, m_origHeight, 0.8f, w, l)) {
        m_params.window = w;
        m_params.level = l;
        m_displayDirty = true;
    }
}

void ImageViewer::FitToView() {
    m_needFit = true;
}

void ImageViewer::ResetZoom() {
    m_zoom = 1.0f;
    m_panX = 0.0f;
    m_panY = 0.0f;
}

void ImageViewer::SetZoom(float zoom) {
    m_zoom = std::clamp(zoom, 0.01f, 100.0f);
}

void ImageViewer::SetRotation(int degrees) {
    int rot = ((degrees % 360) + 360) % 360;
    rot = (rot / 90) * 90;
    if (m_rotation != rot) {
        m_rotation = rot;
        m_displayDirty = true;
        m_needFit = true;
    }
}

void ImageViewer::RotateCW() {
    SetRotation(m_rotation + 90);
}

void ImageViewer::RotateCCW() {
    SetRotation(m_rotation - 90);
}

void ImageViewer::SetFlipHorizontal(bool v) {
    if (m_flipH != v) {
        m_flipH = v;
        m_displayDirty = true;
    }
}

void ImageViewer::SetFlipVertical(bool v) {
    if (m_flipV != v) {
        m_flipV = v;
        m_displayDirty = true;
    }
}

void ImageViewer::ToggleFlipHorizontal() {
    SetFlipHorizontal(!m_flipH);
}

void ImageViewer::ToggleFlipVertical() {
    SetFlipVertical(!m_flipV);
}

void ImageViewer::SetProfileTool(bool enabled) {
    m_profileTool = enabled;
    if (!enabled) {
        m_profileHasLine = false;
        m_profileDragging = false;
    }
}

void ImageViewer::ClearProfileLine() {
    m_profileHasLine = false;
    m_profileDragging = false;
}

bool ImageViewer::DisplayToImageCoords(int dx, int dy, int& imgX, int& imgY) const {
    if (dx < 0 || dx >= m_dispWidth || dy < 0 || dy >= m_dispHeight) return false;
    int fx = m_flipH ? (m_dispWidth - 1 - dx) : dx;
    int fy = m_flipV ? (m_dispHeight - 1 - dy) : dy;

    int sx = 0, sy = 0;
    switch (m_rotation) {
        case 0:
            sx = fx;
            sy = fy;
            break;
        case 90:
            sx = fy;
            sy = m_origHeight - 1 - fx;
            break;
        case 180:
            sx = m_origWidth - 1 - fx;
            sy = m_origHeight - 1 - fy;
            break;
        case 270:
            sx = m_origWidth - 1 - fy;
            sy = fx;
            break;
        default:
            sx = fx;
            sy = fy;
            break;
    }
    if (sx < 0 || sx >= m_origWidth || sy < 0 || sy >= m_origHeight) return false;
    imgX = sx;
    imgY = sy;
    return true;
}

void ImageViewer::ImageToDisplayCoords(int imgX, int imgY, int& dx, int& dy) const {
    int fx = 0, fy = 0;
    switch (m_rotation) {
        case 0:
            fx = imgX;
            fy = imgY;
            break;
        case 90:
            fx = m_origHeight - 1 - imgY;
            fy = imgX;
            break;
        case 180:
            fx = m_origWidth - 1 - imgX;
            fy = m_origHeight - 1 - imgY;
            break;
        case 270:
            fx = imgY;
            fy = m_origWidth - 1 - imgX;
            break;
        default:
            fx = imgX;
            fy = imgY;
            break;
    }
    dx = m_flipH ? (m_dispWidth - 1 - fx) : fx;
    dy = m_flipV ? (m_dispHeight - 1 - fy) : fy;
}

bool ImageViewer::ScreenToImageCoords(const ImVec2& screenPos, float ox, float oy, int& imgX, int& imgY) const {
    if (m_zoom <= 0.0f) return false;
    float dx = (screenPos.x - ox) / m_zoom;
    float dy = (screenPos.y - oy) / m_zoom;
    if (dx < 0.0f || dx >= static_cast<float>(m_dispWidth) || dy < 0.0f || dy >= static_cast<float>(m_dispHeight)) {
        return false;
    }
    return DisplayToImageCoords(static_cast<int>(dx), static_cast<int>(dy), imgX, imgY);
}

ImVec2 ImageViewer::ImageToScreenCoords(int imgX, int imgY, float ox, float oy) const {
    int dx = 0, dy = 0;
    ImageToDisplayCoords(imgX, imgY, dx, dy);
    return ImVec2(ox + (static_cast<float>(dx) + 0.5f) * m_zoom,
                  oy + (static_cast<float>(dy) + 0.5f) * m_zoom);
}

void ImageViewer::UpdateDisplayBuffer() {
    if (m_raw16.empty() || m_origWidth <= 0 || m_origHeight <= 0) {
        m_displayRgba.clear();
        m_dispWidth = 0;
        m_dispHeight = 0;
        m_texDesc.pixels = nullptr;
        m_texDesc.width = 0;
        m_texDesc.height = 0;
        m_displayDirty = false;
        return;
    }

    if (m_rotation == 90 || m_rotation == 270) {
        m_dispWidth = m_origHeight;
        m_dispHeight = m_origWidth;
    } else {
        m_dispWidth = m_origWidth;
        m_dispHeight = m_origHeight;
    }

    const size_t numPixels = static_cast<size_t>(m_dispWidth) * m_dispHeight;
    m_displayRgba.resize(numPixels);

    std::vector<uint32_t> lut(65536);
    BuildWindowLUT32(m_params, lut.data());

    for (int dy = 0; dy < m_dispHeight; ++dy) {
        uint32_t* row = &m_displayRgba[static_cast<size_t>(dy) * m_dispWidth];
        for (int dx = 0; dx < m_dispWidth; ++dx) {
            int sx = 0, sy = 0;
            DisplayToImageCoords(dx, dy, sx, sy);
            row[dx] = lut[m_raw16[static_cast<size_t>(sy) * m_origWidth + sx]];
        }
    }

    m_texDesc.pixels = m_displayRgba.data();
    m_texDesc.width = m_dispWidth;
    m_texDesc.height = m_dispHeight;
    m_displayDirty = false;
}

void ImageViewer::Render(float width, float height, const ImageViewerStyle* style) {
    const ImageViewerStyle& st = style ? *style : m_style;

    ImVec2 avail = ImGui::GetContentRegionAvail();
    if (width <= 0.0f)  width = avail.x;
    if (height <= 0.0f) height = avail.y;
    if (width < 20.0f)  width = 20.0f;
    if (height < 20.0f) height = 20.0f;

    if (!HasImage()) {
        ImGui::BeginChild("##image_viewer_empty", ImVec2(width, height), false,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImVec2 cp = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(cp, ImVec2(cp.x + width, cp.y + height), st.colBg, st.cornerRadius);
        dl->AddRect(cp, ImVec2(cp.x + width, cp.y + height), st.colBorder, st.cornerRadius);

        ImVec2 textSz = ImGui::CalcTextSize("No image loaded");
        ImGui::SetCursorPos(ImVec2((width - textSz.x) * 0.5f, (height - textSz.y) * 0.5f));
        ImGui::PushStyleColor(ImGuiCol_Text, st.colOverlaySecondary);
        ImGui::TextUnformatted("No image loaded");
        ImGui::PopStyleColor();
        ImGui::EndChild();
        m_hoverInfo.valid = false;
        return;
    }

    if (m_displayDirty) {
        UpdateDisplayBuffer();
    }

    if (m_needFit) {
        float sx = width / static_cast<float>(m_dispWidth);
        float sy = height / static_cast<float>(m_dispHeight);
        m_zoom = std::min(sx, sy) * 0.96f;
        m_panX = 0.0f;
        m_panY = 0.0f;
        m_needFit = false;
    }

    ImGui::BeginChild("##image_viewer_canvas", ImVec2(width, height), false,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = io.MousePos;

    bool hovered = (mousePos.x >= canvasPos.x && mousePos.x < canvasPos.x + canvasSize.x &&
                    mousePos.y >= canvasPos.y && mousePos.y < canvasPos.y + canvasSize.y &&
                    !ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId));

    ImGui::InvisibleButton("##canvas_input", canvasSize);

    // Zoom and pan
    {
        if (hovered && std::abs(io.MouseWheel) > 0.0f) {
            float zoomFactor = 1.15f;
            float oldZoom = m_zoom;
            if (io.MouseWheel > 0) {
                m_zoom *= zoomFactor;
            } else {
                m_zoom /= zoomFactor;
            }
            m_zoom = std::clamp(m_zoom, 0.01f, 100.0f);

            float mx = mousePos.x - canvasPos.x - canvasSize.x * 0.5f;
            float my = mousePos.y - canvasPos.y - canvasSize.y * 0.5f;
            m_panX += mx * (1.0f / oldZoom - 1.0f / m_zoom);
            m_panY += my * (1.0f / oldZoom - 1.0f / m_zoom);
        }

        bool wantDrag = hovered && (ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseDown(ImGuiMouseButton_Middle));
        if (wantDrag && !m_isDragging) {
            m_isDragging = true;
            m_dragStartPos = mousePos;
            m_dragPanStartX = m_panX;
            m_dragPanStartY = m_panY;
        }
        if (m_isDragging) {
            if (wantDrag) {
                m_panX = m_dragPanStartX - (mousePos.x - m_dragStartPos.x) / m_zoom;
                m_panY = m_dragPanStartY - (mousePos.y - m_dragStartPos.y) / m_zoom;
            } else {
                m_isDragging = false;
            }
        }
    }

    float cw = canvasSize.x;
    float ch = canvasSize.y;
    float dispW = static_cast<float>(m_dispWidth) * m_zoom;
    float dispH = static_cast<float>(m_dispHeight) * m_zoom;
    float ox = canvasPos.x + cw * 0.5f - (static_cast<float>(m_dispWidth) * 0.5f + m_panX) * m_zoom;
    float oy = canvasPos.y + ch * 0.5f - (static_cast<float>(m_dispHeight) * 0.5f + m_panY) * m_zoom;

    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Viewport background & border
    dl->AddRectFilled(canvasPos, ImVec2(canvasPos.x + cw, canvasPos.y + ch), st.colBg, st.cornerRadius);
    dl->AddRect(canvasPos, ImVec2(canvasPos.x + cw, canvasPos.y + ch), st.colBorder, st.cornerRadius);

    // Render image
    if (m_texDesc.pixels && m_texDesc.width > 0 && m_texDesc.height > 0) {
        ImVec2 p_min(ox, oy);
        ImVec2 p_max(ox + dispW, oy + dispH);
        dl->AddImage(reinterpret_cast<ImTextureID>(&m_texDesc), p_min, p_max);
        dl->AddRect(p_min, p_max, st.colImageBorder);
    }

    // Profile tool interaction
    if (m_profileTool) {
        int ix = 0, iy = 0;
        if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right) &&
            ScreenToImageCoords(mousePos, ox, oy, ix, iy)) {
            m_profileDragging = true;
            m_profileHasLine = false;
            m_profileX0 = m_profileX1 = ix;
            m_profileY0 = m_profileY1 = iy;
        }
        if (m_profileDragging) {
            if (ScreenToImageCoords(mousePos, ox, oy, ix, iy)) {
                m_profileX1 = ix;
                m_profileY1 = iy;
            }
            if (ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
                m_profileDragging = false;
                m_profileHasLine = true;
            }
        }
        if (m_profileDragging || m_profileHasLine) {
            ImVec2 a = ImageToScreenCoords(m_profileX0, m_profileY0, ox, oy);
            ImVec2 b = ImageToScreenCoords(m_profileX1, m_profileY1, ox, oy);
            dl->AddLine(a, b, st.colSliceLine, 2.0f);
            dl->AddCircleFilled(a, 4.0f, st.colSliceHandle);
            dl->AddCircleFilled(b, 4.0f, st.colSliceHandle);
        }
    }

    // Hover inspection
    m_hoverInfo.valid = false;
    if (hovered) {
        int hx = 0, hy = 0;
        if (ScreenToImageCoords(mousePos, ox, oy, hx, hy)) {
            m_hoverInfo.pixelX = hx;
            m_hoverInfo.pixelY = hy;
            m_hoverInfo.rawValue = m_raw16[static_cast<size_t>(hy) * m_origWidth + hx];
            m_hoverInfo.valid = true;
        }
    }

    // Info overlay (bottom-left)
    {
        char buf[128];
        std::snprintf(buf, sizeof(buf), "Zoom: %.0f%%  |  %d x %d  |  Rot: %d\xC2\xB0",
                      m_zoom * 100.0f, m_origWidth, m_origHeight, m_rotation);
        ImVec2 tsz = ImGui::CalcTextSize(buf);
        ImVec2 textPos(canvasPos.x + 12, canvasPos.y + ch - 24);
        dl->AddRectFilled(ImVec2(textPos.x - 6, textPos.y - 3),
                          ImVec2(textPos.x + tsz.x + 6, textPos.y + tsz.y + 3),
                          st.colOverlayBg, 4.0f);
        dl->AddRect(ImVec2(textPos.x - 6, textPos.y - 3),
                    ImVec2(textPos.x + tsz.x + 6, textPos.y + tsz.y + 3),
                    st.colOverlayBorder, 4.0f);
        dl->AddText(textPos, st.colOverlaySecondary, buf);

        if (m_hoverInfo.valid) {
            std::snprintf(buf, sizeof(buf), "Pixel (%d, %d): %u",
                          m_hoverInfo.pixelX, m_hoverInfo.pixelY, m_hoverInfo.rawValue);
            ImVec2 hsz = ImGui::CalcTextSize(buf);
            ImVec2 tp2(canvasPos.x + 12, canvasPos.y + ch - 50);
            dl->AddRectFilled(ImVec2(tp2.x - 6, tp2.y - 3),
                              ImVec2(tp2.x + hsz.x + 6, tp2.y + hsz.y + 3),
                              st.colOverlayBg, 4.0f);
            dl->AddRect(ImVec2(tp2.x - 6, tp2.y - 3),
                        ImVec2(tp2.x + hsz.x + 6, tp2.y + hsz.y + 3),
                        st.colOverlayBorder, 4.0f);
            dl->AddText(tp2, st.colOverlayText, buf);
        }
    }

    // Profile plot overlay
    if (m_profileTool) {
        DrawProfileGraph(dl, canvasPos, cw, ch, ox, oy, st);
    }

    // Context menu on RMB (when profile tool not active)
    if (hovered && !m_profileTool && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup("##image_viewer_ctx");
    }
    if (ImGui::BeginPopup("##image_viewer_ctx")) {
        if (ImGui::MenuItem("Fit to View"))     FitToView();
        if (ImGui::MenuItem("Reset Zoom 100%")) ResetZoom();
        ImGui::Separator();
        if (ImGui::MenuItem("Zoom 50%"))  SetZoom(0.5f);
        if (ImGui::MenuItem("Zoom 200%")) SetZoom(2.0f);
        if (ImGui::MenuItem("Zoom 400%")) SetZoom(4.0f);
        ImGui::Separator();
        if (ImGui::MenuItem("Rotate CW 90\xC2\xB0"))  RotateCW();
        if (ImGui::MenuItem("Rotate CCW 90\xC2\xB0")) RotateCCW();
        if (ImGui::MenuItem("Flip Horizontal"))      ToggleFlipHorizontal();
        if (ImGui::MenuItem("Flip Vertical"))        ToggleFlipVertical();
        ImGui::EndPopup();
    }

    ImGui::EndChild();
}

void ImageViewer::DrawProfileGraph(ImDrawList* dl, const ImVec2& canvasPos, float cw, float ch, float ox, float oy, const ImageViewerStyle& st) {
    (void)ox; (void)oy;
    float ovH = std::min(140.0f, ch * 0.5f);
    ImVec2 o0(canvasPos.x, canvasPos.y + ch - ovH);
    ImVec2 o1(canvasPos.x + cw, canvasPos.y + ch);

    dl->AddRectFilled(o0, o1, st.colOverlayBg, st.cornerRadius, ImDrawFlags_RoundCornersBottom);
    dl->AddLine(o0, ImVec2(o1.x, o0.y), st.colDivider, 1.0f);

    if (!m_profileHasLine) {
        const char* hint = "Hold right mouse button on the image to draw a slice line";
        ImVec2 ts = ImGui::CalcTextSize(hint);
        dl->AddText(ImVec2(o0.x + (cw - ts.x) * 0.5f, o0.y + (ovH - ts.y) * 0.5f),
                    st.colOverlaySecondary, hint);
        return;
    }

    int ax = m_profileX0, ay = m_profileY0;
    int bx = m_profileX1, by = m_profileY1;
    float fdx = static_cast<float>(bx - ax);
    float fdy = static_cast<float>(by - ay);
    int n = std::max(1, static_cast<int>(std::lround(std::sqrt(fdx * fdx + fdy * fdy))));

    std::vector<float> vals(n + 1);
    float vmin = 65535.0f, vmax = 0.0f;
    for (int i = 0; i <= n; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(n);
        int px = std::clamp(static_cast<int>(std::lround(ax + t * fdx)), 0, m_origWidth - 1);
        int py = std::clamp(static_cast<int>(std::lround(ay + t * fdy)), 0, m_origHeight - 1);
        float v = static_cast<float>(m_raw16[static_cast<size_t>(py) * m_origWidth + px]);
        vals[i] = v;
        if (v < vmin) vmin = v;
        if (v > vmax) vmax = v;
    }
    if (vmax <= vmin) vmax = vmin + 1.0f;

    ImVec2 p0(o0.x + 12, o0.y + 24), p1(o1.x - 12, o1.y - 8);
    float pw = p1.x - p0.x;
    float ph = p1.y - p0.y;
    if (pw > 1.0f && ph > 1.0f) {
        std::vector<ImVec2> pts(n + 1);
        for (int i = 0; i <= n; ++i) {
            float frac = (vals[i] - vmin) / (vmax - vmin);
            pts[i] = ImVec2(p0.x + pw * (static_cast<float>(i) / static_cast<float>(n)),
                            p1.y - ph * frac);
        }
        dl->AddPolyline(pts.data(), static_cast<int>(pts.size()), st.colProfileLine,
                        ImDrawFlags_None, 1.5f);
    }

    char buf[96];
    std::snprintf(buf, sizeof(buf), "Profile: %d px  |  Min: %.0f  |  Max: %.0f", n, vmin, vmax);
    dl->AddText(ImVec2(o0.x + 12, o0.y + 4), st.colOverlayText, buf);
}

} // namespace OmniKit
