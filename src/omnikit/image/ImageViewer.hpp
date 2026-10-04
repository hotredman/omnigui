#pragma once

#include "ImageTypes.hpp"
#include "ImageStyle.hpp"
#include "ImageHistogram.hpp"
#include "imgui_ext/recorder.h"

#include <vector>
#include <cstdint>
#include <imgui.h>

namespace OmniKit {

class ImageViewer {
public:
    ImageViewer();
    ~ImageViewer();

    void LoadImage16(const uint16_t* data, int width, int height);

    void SetParams(const WindowLevelParams& params);
    const WindowLevelParams& GetParams() const { return m_params; }
    WindowLevelParams& GetParamsMutable() { return m_params; }

    // Fits window & level based on histogram data
    void AutoFitWindow();

    // Style configuration (Clean Architecture: independent from UiTheme)
    void SetStyle(const ImageViewerStyle& style) { m_style = style; m_hasCustomStyle = true; }
    const ImageViewerStyle& GetStyle() const { return m_style; }

    // View navigation
    void FitToView();
    void ResetZoom();
    void SetZoom(float zoom);
    float GetZoom() const { return m_zoom; }

    // Rotation (0, 90, 180, 270 degrees)
    void SetRotation(int degrees);
    int GetRotation() const { return m_rotation; }
    void RotateCW();
    void RotateCCW();

    // Flips (mirroring)
    void SetFlipHorizontal(bool v);
    void SetFlipVertical(bool v);
    bool GetFlipHorizontal() const { return m_flipH; }
    bool GetFlipVertical() const { return m_flipV; }
    void ToggleFlipHorizontal();
    void ToggleFlipVertical();

    // Profile line tool
    void SetProfileTool(bool enabled);
    bool IsProfileToolEnabled() const { return m_profileTool; }
    void ClearProfileLine();

    // Hover & metrics
    ImageHoverInfo GetHoverInfo() const { return m_hoverInfo; }
    int GetWidth() const { return m_origWidth; }
    int GetHeight() const { return m_origHeight; }
    int GetDisplayedWidth() const { return (m_rotation == 90 || m_rotation == 270) ? m_origHeight : m_origWidth; }
    int GetDisplayedHeight() const { return (m_rotation == 90 || m_rotation == 270) ? m_origWidth : m_origHeight; }
    bool HasImage() const { return !m_raw16.empty() && m_origWidth > 0 && m_origHeight > 0; }

    const ImageHistogram& GetHistogram() const { return m_histogram; }
    ImageHistogram& GetHistogram() { return m_histogram; }

    // Main render method: draws canvas, image, hover inspection, profile tool
    void Render(float width = -1.0f, float height = -1.0f, const ImageViewerStyle* style = nullptr);

private:
    void UpdateDisplayBuffer();
    bool ScreenToImageCoords(const ImVec2& screenPos, float ox, float oy, int& imgX, int& imgY) const;
    ImVec2 ImageToScreenCoords(int imgX, int imgY, float ox, float oy) const;
    bool DisplayToImageCoords(int dx, int dy, int& imgX, int& imgY) const;
    void ImageToDisplayCoords(int imgX, int imgY, int& dx, int& dy) const;
    void DrawProfileGraph(ImDrawList* dl, const ImVec2& canvasPos, float cw, float ch, float ox, float oy, const ImageViewerStyle& st);

    std::vector<uint16_t> m_raw16;
    int m_origWidth  = 0;
    int m_origHeight = 0;

    int m_dispWidth  = 0;
    int m_dispHeight = 0;
    std::vector<uint32_t> m_displayRgba;
    ImageTextureDescriptor m_texDesc;

    WindowLevelParams m_params;
    ImageHistogram    m_histogram;

    // View state
    float m_zoom = 1.0f;
    float m_panX = 0.0f;
    float m_panY = 0.0f;
    bool  m_needFit = true;
    bool  m_isDragging = false;
    ImVec2 m_dragStartPos{0, 0};
    float m_dragPanStartX = 0.0f;
    float m_dragPanStartY = 0.0f;

    // Transform
    int  m_rotation = 0;
    bool m_flipH = false;
    bool m_flipV = false;

    // Dirty tracking
    bool m_displayDirty = true;
    WindowLevelParams m_lastParams;
    int  m_lastRotation = -1;
    bool m_lastFlipH = false;
    bool m_lastFlipV = false;

    // Profile tool
    bool m_profileTool = false;
    bool m_profileDragging = false;
    bool m_profileHasLine = false;
    int  m_profileX0 = 0, m_profileY0 = 0;
    int  m_profileX1 = 0, m_profileY1 = 0;

    // Inspection
    mutable ImageHoverInfo m_hoverInfo;

    // Style
    ImageViewerStyle m_style = ImageViewerStyle::Dark();
    bool m_hasCustomStyle = false;
};

} // namespace OmniKit
