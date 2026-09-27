#pragma once
#include <string>

namespace ImGuiDom {

inline const std::string& GetWebClientHtml() {
    static const std::string html = []() {
        std::string s;
        s.reserve(65536);
        s += R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ImGui Web DOM Backend</title>
    <style>
        :root {
            --imgui-text: #ffffff;
            --imgui-text-disabled: rgba(128, 128, 128, 1.0);
            --imgui-window-bg: rgba(15, 15, 15, 0.94);
            --imgui-popup-bg: rgba(20, 20, 20, 0.94);
            --imgui-border: rgba(110, 110, 128, 0.50);
            --imgui-frame-bg: rgba(41, 74, 122, 0.54);
            --imgui-frame-bg-hovered: rgba(66, 150, 250, 0.40);
            --imgui-frame-bg-active: rgba(66, 150, 250, 0.67);
            --imgui-title-bg: #0a0a0a;
            --imgui-title-bg-active: #294a7a;
            --imgui-menubar-bg: #242424;
            --imgui-scrollbar-bg: rgba(5, 5, 5, 0.53);
            --imgui-scrollbar-grab: rgba(79, 79, 79, 1.0);
            --imgui-scrollbar-grab-hovered: rgba(105, 105, 105, 1.0);
            --imgui-scrollbar-grab-active: rgba(130, 130, 130, 1.0);
            --imgui-check-mark: #4296fa;
            --imgui-slider-grab: #3d85e0;
            --imgui-slider-grab-active: #4296fa;
            --imgui-btn: rgba(66, 150, 250, 0.40);
            --imgui-btn-hovered: #4296fa;
            --imgui-btn-active: #0f87fa;
            --imgui-header: rgba(66, 150, 250, 0.31);
            --imgui-header-hovered: rgba(66, 150, 250, 0.80);
            --imgui-header-active: #4296fa;
            --imgui-separator: rgba(110, 110, 128, 0.50);
            --imgui-tab: rgba(46, 89, 148, 0.86);
            --imgui-tab-hovered: rgba(66, 150, 250, 0.80);
            --imgui-tab-selected: rgba(51, 105, 173, 1.0);
            --imgui-tab-selected-overline: #4296fa;
            --imgui-table-header-bg: #303033;
            --imgui-table-border: #4f4f59;
            --imgui-table-row-bg-alt: rgba(255, 255, 255, 0.06);
        }

        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            background-color: #0f0f14;
            color: var(--imgui-text);
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            font-size: 13px;
            overflow: hidden;
            width: 100vw;
            height: 100vh;
            user-select: none;
        }

        #topbar {
            position: fixed;
            top: 0; left: 0; right: 0;
            height: 30px;
            background: #14141e;
            border-bottom: 1px solid var(--imgui-border);
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding: 0 16px;
            font-size: 12px;
            font-weight: 500;
            z-index: 1000;
        }

        .status-badge {
            display: inline-flex;
            align-items: center;
            gap: 6px;
            color: #8ae;
        }
        .dot {
            width: 8px; height: 8px;
            border-radius: 50%;
            background: #0f0;
            box-shadow: 0 0 6px #0f0;
        }
        .dot.connecting { background: #ff0; box-shadow: 0 0 6px #ff0; }
        .dot.disconnected { background: #f00; box-shadow: 0 0 6px #f00; }

        #desktop {
            position: absolute;
            top: 30px; left: 0; right: 0; bottom: 0;
            overflow: auto;
        }

        /* ImGui Window Styling - Authentic Dear ImGui Dark (0px rounding, 1px border) */
        .imgui-window {
            position: absolute;
            background-color: var(--imgui-window-bg);
            border: 1px solid var(--imgui-border);
            border-radius: 0px;
            box-shadow: 0 6px 20px rgba(0,0,0,0.6);
            overflow: hidden;
        }

        .imgui-header {
            position: absolute;
            top: 0; left: 0; right: 0;
            height: 19px;
            background: var(--imgui-title-bg-active);
            padding: 1px 6px;
            font-weight: 600;
            font-size: 13px;
            color: var(--imgui-text);
            border-bottom: 1px solid var(--imgui-border);
            cursor: grab;
            user-select: none;
            box-sizing: border-box;
            z-index: 10;
            display: flex;
            align-items: center;
            gap: 6px;
            white-space: nowrap;
            overflow: hidden;
            text-overflow: ellipsis;
            touch-action: none;
        }
        .imgui-header.dragging,
        .imgui-header:active {
            cursor: grabbing;
        }
        .imgui-collapse-btn {
            font-size: 10px;
            color: var(--imgui-text-disabled);
            cursor: pointer;
            width: 14px;
            height: 14px;
            display: inline-flex;
            align-items: center;
            justify-content: center;
            user-select: none;
            flex-shrink: 0;
            border-radius: 0px;
            transition: color 0.1s, background 0.1s;
            pointer-events: auto;
        }
        .imgui-collapse-btn:hover {
            color: #ffffff;
            background: rgba(255, 255, 255, 0.15);
        }
        .imgui-win-title {
            pointer-events: none;
            overflow: hidden;
            text-overflow: ellipsis;
            white-space: nowrap;
        }

        .imgui-menubar {
            position: absolute;
            left: 0; right: 0;
            height: 19px;
            background: var(--imgui-menubar-bg);
            border-bottom: 1px solid var(--imgui-border);
            box-sizing: border-box;
            z-index: 8;
            display: flex;
            align-items: center;
            padding: 0 4px;
        }

        .imgui-menu-item {
            position: absolute;
            background: transparent;
            color: var(--imgui-text);
            border: none;
            border-radius: 0px;
            padding: 2px 6px;
            font-size: 13px;
            font-family: inherit;
            cursor: pointer;
            white-space: nowrap;
            z-index: 9;
            box-sizing: border-box;
            display: inline-flex;
            align-items: center;
        }
        .imgui-menu-item:hover {
            background: var(--imgui-header-hovered);
            color: #ffffff;
        }

        .imgui-body {
            position: absolute;
            top: 0; left: 0; right: 0; bottom: 0;
            overflow: hidden;
            z-index: 1;
        }

        /* Native HTML Button - Authentic Dear ImGui Dark (FrameRounding = 0.0f) */
        .imgui-btn {
            position: absolute;
            background: var(--imgui-btn);
            color: var(--imgui-text);
            border: none;
            border-radius: 0px;
            padding: 2px 8px;
            font-size: 13px;
            font-family: inherit;
            cursor: pointer;
            outline: none;
            white-space: nowrap;
            overflow: hidden;
            text-overflow: ellipsis;
            display: inline-flex;
            align-items: center;
            justify-content: center;
            box-sizing: border-box;
            user-select: none;
        }
        .imgui-btn:hover {
            background: var(--imgui-btn-hovered);
            color: #ffffff;
        }
        .imgui-btn:active {
            background: var(--imgui-btn-active);
            color: #ffffff;
        }

        /* Native Slider - Authentic rectangular grabber */
        .imgui-slider-box {
            position: absolute;
            display: flex;
            align-items: center;
            gap: 6px;
            color: var(--imgui-text);
            font-size: 13px;
            white-space: nowrap;
            user-select: none;
            box-sizing: border-box;
        }
        .imgui-slider-box input[type="range"] {
            -webkit-appearance: none;
            appearance: none;
            flex: 1;
            height: 19px;
            background: var(--imgui-frame-bg);
            border: none;
            border-radius: 0px;
            outline: none;
            cursor: pointer;
            margin: 0;
            padding: 0;
        }
        .imgui-slider-box input[type="range"]:hover {
            background: var(--imgui-frame-bg-hovered);
        }
        .imgui-slider-box input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 12px;
            height: 19px;
            background: var(--imgui-slider-grab);
            border: none;
            border-radius: 0px;
            cursor: pointer;
        }
        .imgui-slider-box input[type="range"]::-webkit-slider-thumb:hover {
            background: var(--imgui-slider-grab-active);
        }
        .imgui-slider-box input[type="range"]::-webkit-slider-thumb:active {
            background: var(--imgui-slider-grab-active);
        }
        .imgui-slider-box input[type="range"]::-moz-range-thumb {
            width: 12px;
            height: 19px;
            background: var(--imgui-slider-grab);
            border: none;
            border-radius: 0px;
            cursor: pointer;
        }
        .val-badge {
            color: #a0c0e0;
            font-size: 11px;
            font-family: monospace;
            min-width: 28px;
        }

        /* Native Checkbox & Radio - Authentic crisp styling */
        .imgui-check-box {
            position: absolute;
            display: inline-flex;
            align-items: center;
            gap: 6px;
            cursor: pointer;
            font-size: 13px;
            white-space: nowrap;
            user-select: none;
            color: var(--imgui-text);
        }
        .imgui-check-box input[type="checkbox"] {
            -webkit-appearance: none;
            appearance: none;
            width: 15px;
            height: 15px;
            background: var(--imgui-frame-bg);
            border: none;
            border-radius: 0px;
            cursor: pointer;
            outline: none;
            position: relative;
            margin: 0;
            flex-shrink: 0;
        }
        .imgui-check-box input[type="checkbox"]:hover {
            background: var(--imgui-frame-bg-hovered);
        }
        .imgui-check-box input[type="checkbox"]:checked {
            background: var(--imgui-frame-bg-active);
        }
        .imgui-check-box input[type="checkbox"]:checked::after {
            content: '';
            position: absolute;
            left: 5px;
            top: 2px;
            width: 4px;
            height: 8px;
            border: solid var(--imgui-check-mark);
            border-width: 0 2px 2px 0;
            transform: rotate(45deg);
        }
)HTML";
        s += R"HTML(
        .imgui-check-box input[type="radio"] {
            -webkit-appearance: none;
            appearance: none;
            width: 15px;
            height: 15px;
            background: var(--imgui-frame-bg);
            border: none;
            border-radius: 50%;
            cursor: pointer;
            outline: none;
            position: relative;
            margin: 0;
            flex-shrink: 0;
        }
        .imgui-check-box input[type="radio"]:hover {
            background: var(--imgui-frame-bg-hovered);
        }
        .imgui-check-box input[type="radio"]:checked {
            background: var(--imgui-frame-bg-active);
        }
        .imgui-check-box input[type="radio"]:checked::after {
            content: '';
            position: absolute;
            left: 4.5px;
            top: 4.5px;
            width: 6px;
            height: 6px;
            background: var(--imgui-check-mark);
            border-radius: 50%;
        }

        /* Native Text */
        .imgui-text {
            position: absolute;
            color: var(--imgui-text);
            font-size: 13px;
            white-space: nowrap;
        }

        /* Native InputText */
        .imgui-input-box {
            position: absolute;
            display: flex;
            align-items: center;
            gap: 6px;
            font-size: 13px;
            white-space: nowrap;
            color: var(--imgui-text);
        }
        .imgui-input {
            background: var(--imgui-frame-bg);
            color: var(--imgui-text);
            border: none;
            border-radius: 0px;
            padding: 2px 6px;
            font-size: 13px;
            font-family: inherit;
            outline: none;
            flex: 1;
            box-sizing: border-box;
        }
        .imgui-input:hover {
            background: var(--imgui-frame-bg-hovered);
        }
        .imgui-input:focus {
            background: var(--imgui-frame-bg-active);
        }

        /* Native Combo / Select */
        .imgui-combo-box {
            position: absolute;
            display: flex;
            align-items: center;
            gap: 6px;
            font-size: 13px;
            white-space: nowrap;
            color: var(--imgui-text);
        }
        .imgui-select {
            background: var(--imgui-frame-bg);
            color: var(--imgui-text);
            border: none;
            border-radius: 0px;
            padding: 2px 6px;
            font-size: 13px;
            font-family: inherit;
            outline: none;
            cursor: pointer;
            flex: 1;
            box-sizing: border-box;
        }
        .imgui-select:hover {
            background: var(--imgui-frame-bg-hovered);
        }
        .imgui-select:focus {
            background: var(--imgui-frame-bg-active);
        }
)HTML";
        s += R"HTML(
        /* Native ProgressBar */
        .imgui-progress-box {
            position: absolute;
            background: var(--imgui-frame-bg);
            border: none;
            border-radius: 0px;
            overflow: hidden;
            display: flex;
            align-items: center;
            justify-content: center;
            box-sizing: border-box;
        }
        .imgui-progress-bar {
            position: absolute;
            left: 0; top: 0; bottom: 0;
            background: var(--imgui-header-active);
            transition: width 0.1s ease;
        }
        .imgui-progress-text {
            position: relative;
            font-size: 11px;
            font-weight: 600;
            color: #ffffff;
            text-shadow: 0 1px 2px rgba(0,0,0,0.8);
            z-index: 1;
        }

        /* Native Separator */
        .imgui-separator {
            position: absolute;
            border: none;
            border-top: 1px solid var(--imgui-separator);
            margin: 0;
        }

        /* Native Tree Node */
        .imgui-tree {
            position: absolute;
            color: var(--imgui-text);
            font-size: 13px;
            font-weight: 500;
            cursor: pointer;
            user-select: none;
            display: inline-flex;
            align-items: center;
            gap: 6px;
            padding: 1px 4px;
            border-radius: 0px;
            white-space: nowrap;
            box-sizing: border-box;
            transition: background 0.05s ease;
        }
        .imgui-tree:hover {
            background: var(--imgui-header-hovered);
            color: #ffffff;
        }
        .imgui-tree-arrow {
            font-size: 10px;
            color: var(--imgui-text-disabled);
            width: 12px;
            display: inline-block;
            text-align: center;
        }

        /* Native Collapsing Header - Authentic Dear ImGui Dark header banner */
        .imgui-collapsing-header {
            position: absolute;
            background: var(--imgui-header);
            border: none;
            border-radius: 0px;
            color: var(--imgui-text);
            font-size: 13px;
            font-weight: 600;
            cursor: pointer;
            user-select: none;
            display: flex;
            align-items: center;
            gap: 6px;
            padding: 2px 6px;
            white-space: nowrap;
            overflow: hidden;
            text-overflow: ellipsis;
            box-sizing: border-box;
            transition: background 0.05s ease;
        }
        .imgui-collapsing-header:hover {
            background: var(--imgui-header-hovered);
        }
        .imgui-collapsing-header:active {
            background: var(--imgui-header-active);
        }
        .imgui-collapsing-header .imgui-header-arrow {
            font-size: 10px;
            color: var(--imgui-text-disabled);
            width: 12px;
            display: inline-block;
            text-align: center;
            flex-shrink: 0;
        }

        .imgui-disabled {
            opacity: 0.60 !important;
            pointer-events: none !important;
        }

        /* Native HTML5 Canvas (Oscilloscope, Vector graphics) */
        .imgui-canvas {
            position: absolute;
            box-sizing: border-box;
            border-radius: 0px;
        }

        /* Native TabBar & TabItems */
        .imgui-tab-bar {
            position: absolute;
            display: flex;
            border-bottom: 1px solid var(--imgui-border);
            box-sizing: border-box;
        }
        .imgui-tab-item {
            position: absolute;
            background: var(--imgui-tab);
            color: var(--imgui-text);
            border: 1px solid var(--imgui-border);
            border-bottom: none;
            border-radius: 0px;
            padding: 2px 8px;
            font-size: 12px;
            cursor: pointer;
            user-select: none;
            box-sizing: border-box;
            display: flex;
            align-items: center;
            justify-content: center;
            transition: background 0.05s ease;
        }
        .imgui-tab-item:hover {
            background: var(--imgui-tab-hovered);
            color: #ffffff;
        }
        .imgui-tab-item.active {
            background: var(--imgui-tab-selected);
            border-top: 2px solid var(--imgui-tab-selected-overline);
            color: #ffffff;
            font-weight: 600;
        }

        /* Tooltip Window */
        .imgui-window.tooltip {
            pointer-events: none !important;
            z-index: 99999 !important;
            background-color: var(--imgui-popup-bg) !important;
            border: 1px solid var(--imgui-border) !important;
            box-shadow: 0 4px 14px rgba(0,0,0,0.8) !important;
            border-radius: 0px !important;
        }
        .imgui-window.tooltip .imgui-header {
            display: none !important;
        }

        /* Native Table Styling */
        .imgui-table-container {
            position: absolute;
            background: var(--imgui-window-bg);
            border: 1px solid var(--imgui-table-border);
            border-radius: 0px;
            overflow: hidden;
            box-sizing: border-box;
            pointer-events: none;
        }
        .imgui-table-header-row {
            display: flex;
            background: var(--imgui-table-header-bg);
            border-bottom: 1px solid var(--imgui-table-border);
            font-size: 12px;
            font-weight: 600;
            color: var(--imgui-text);
        }
        .imgui-table-th {
            flex: 1;
            padding: 3px 8px;
            border-right: 1px solid var(--imgui-table-border);
            white-space: nowrap;
            overflow: hidden;
            text-overflow: ellipsis;
        }
        .imgui-table-th:last-child {
            border-right: none;
        }

        /* Native ListBox Styling */
        .imgui-listbox-box {
            position: absolute;
            display: flex;
            flex-direction: column;
            gap: 4px;
            font-size: 13px;
        }
        .imgui-listbox {
            background: #161b24;
            color: #efefef;
            border: 1px solid #36465d;
            border-radius: 2px;
            padding: 2px;
            font-size: 12px;
            outline: none;
            overflow-y: auto;
            flex: 1;
        }
        .imgui-listbox option {
            padding: 2px 6px;
            border-radius: 2px;
            cursor: pointer;
        }
        .imgui-listbox option:hover {
            background: #253347;
        }
        .imgui-listbox option:checked {
            background: #2b4566;
            color: #ffffff;
        }

        /* Native ColorEdit */
        .imgui-color-box {
            position: absolute;
            display: flex;
            align-items: center;
            gap: 6px;
            font-size: 13px;
        }
        .imgui-color-picker {
            width: 24px;
            height: 20px;
            border: 1px solid #36465d;
            border-radius: 2px;
            padding: 0;
            background: none;
            cursor: pointer;
            outline: none;
        }
        .imgui-color-lbl {
            color: #eaeaea;
            white-space: nowrap;
        }

        /* Native Textarea (InputTextMultiline) */
        .imgui-textarea-box {
            position: absolute;
            display: flex;
            flex-direction: column;
            gap: 4px;
            font-size: 13px;
        }
        .imgui-textarea {
            background: #161b24;
            color: #efefef;
            border: 1px solid #36465d;
            border-radius: 2px;
            padding: 4px 6px;
            font-size: 12px;
            font-family: monospace;
            outline: none;
            resize: none;
            flex: 1;
        }
        .imgui-textarea:focus {
            border-color: #4d7eb8;
        }
    </style>
</head>
)HTML";
        s += R"HTML(
<body>
    <div id="topbar">
        <div style="font-weight: 700; color: #fff;">Dear ImGui + ThorVG &rarr; True HTML5 Web DOM Backend</div>
        <div class="status-badge">
            <div id="statusDot" class="dot connecting"></div>
            <span id="statusText">Connecting to C++ Core...</span>
            <span id="fpsBadge" style="margin-left: 10px; color: #8f8;"></span>
        </div>
    </div>

    <div id="desktop"></div>

    <script>
        const desktop = document.getElementById('desktop');
        const statusDot = document.getElementById('statusDot');
        const statusText = document.getElementById('statusText');
        const fpsBadge = document.getElementById('fpsBadge');

        let lastFrameTime = performance.now();
        let frameCount = 0;
        let activeSliders = new Set();

        let ws = null;
        let useWs = false;

        function postEvent(data) {
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify(data));
            } else {
                fetch('/api/event', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(data)
                }).catch(e => console.error("Event post error:", e));
            }
        }

        function renderCanvasWave(canvas, points) {
            const ctx = canvas.getContext('2d');
            const w = canvas.width;
            const h = canvas.height;

            ctx.fillStyle = "#111116";
            ctx.fillRect(0, 0, w, h);
)HTML";
        s += R"HTML(
            // Grid lines
            ctx.strokeStyle = "#252533";
            ctx.lineWidth = 1;
            ctx.beginPath();
            for (let x = 0; x < w; x += 40) { ctx.moveTo(x, 0); ctx.lineTo(x, h); }
            for (let y = 0; y < h; y += 30) { ctx.moveTo(0, y); ctx.lineTo(w, y); }
            ctx.stroke();

            if (!points || points.length < 2) return;

            // Signal waveform
            ctx.strokeStyle = "#00ffcc";
            ctx.lineWidth = 2;
            ctx.shadowColor = "#00ffcc";
            ctx.shadowBlur = 6;

            ctx.beginPath();
            const step = w / (points.length - 1);
            for (let i = 0; i < points.length; ++i) {
                const px = i * step;
                // Normalizing signal to canvas height
                const py = h * 0.5 - (points[i] / 2.5) * (h * 0.42);
                if (i === 0) ctx.moveTo(px, py);
                else ctx.lineTo(px, py);
            }
            ctx.stroke();
            ctx.shadowBlur = 0;
        }

        let lastMouseMoveTime = 0;
        let draggingWin = null;
        let draggingWinId = null;
        let activeFocusWinId = null;
        let topZIndex = 50;

        function setupWindowDragging(winEl) {
            const header = winEl.querySelector('.imgui-header');
            if (!header) return;

            winEl.addEventListener('pointerdown', () => {
                const winId = parseInt(winEl.dataset.winId, 10);
                const winTitle = winEl.dataset.winTitle || '';
                const topWin = (window.__latestDoc && window.__latestDoc.windows && window.__latestDoc.windows.length > 0)
                    ? window.__latestDoc.windows[window.__latestDoc.windows.length - 1]
                    : null;
                if (!topWin || topWin.id !== winId) {
                    activeFocusWinId = winId;
                    topZIndex++;
                    winEl.style.zIndex = topZIndex;
                    postEvent({ type: 'window_focus', id: winId, title: winTitle });
                }
            }, { passive: true });

            header.addEventListener('pointerdown', (e) => {
                if (e.target.closest('.imgui-collapse-btn')) return;
                if (e.button !== 0) return;

                e.preventDefault();
                e.stopPropagation();

                try {
                    header.setPointerCapture(e.pointerId);
                } catch {}

                const currentLeft = parseFloat(winEl.style.left) || winEl.offsetLeft || 0;
                const currentTop = parseFloat(winEl.style.top) || winEl.offsetTop || 0;
                const winId = parseInt(winEl.dataset.winId, 10);
                const winTitle = winEl.dataset.winTitle || '';

                draggingWin = {
                    id: winId,
                    title: winTitle,
                    elem: winEl,
                    header: header,
                    pointerId: e.pointerId,
                    startX: e.clientX,
                    startY: e.clientY,
                    startLeft: currentLeft,
                    startTop: currentTop,
                    lastX: currentLeft,
                    lastY: currentTop,
                    lastSentTime: 0
                };
                draggingWinId = winId;
                activeFocusWinId = winId;

                topZIndex++;
                winEl.style.zIndex = topZIndex;
                header.classList.add('dragging');

                postEvent({ type: 'window_focus', id: winId, title: winTitle });
            });

            header.addEventListener('pointermove', (e) => {
                if (!draggingWin || draggingWin.elem !== winEl || draggingWin.pointerId !== e.pointerId) return;

                e.preventDefault();
                const dx = e.clientX - draggingWin.startX;
                const dy = e.clientY - draggingWin.startY;

                const newX = Math.round(draggingWin.startLeft + dx);
                const newY = Math.round(draggingWin.startTop + dy);

                winEl.style.left = `${newX}px`;
                winEl.style.top = `${newY}px`;
                draggingWin.lastX = newX;
                draggingWin.lastY = newY;

                const now = performance.now();
                if (now - draggingWin.lastSentTime >= 16) {
                    draggingWin.lastSentTime = now;
                    postEvent({
                        type: 'window_move',
                        id: draggingWin.id,
                        title: draggingWin.title,
                        x: newX,
                        y: newY
                    });
                }
            });

            const onPointerUp = (e) => {
                if (!draggingWin || draggingWin.elem !== winEl || draggingWin.pointerId !== e.pointerId) return;

                try {
                    header.releasePointerCapture(e.pointerId);
                } catch {}

                header.classList.remove('dragging');

                const finalX = draggingWin.lastX;
                const finalY = draggingWin.lastY;
                const winId = draggingWin.id;
                const winTitle = draggingWin.title;

                postEvent({
                    type: 'window_move',
                    id: winId,
                    title: winTitle,
                    x: finalX,
                    y: finalY
                });

                setTimeout(() => {
                    if (draggingWinId === winId) {
                        draggingWin = null;
                        draggingWinId = null;
                    }
                }, 80);
            };

            header.addEventListener('pointerup', onPointerUp);
            header.addEventListener('pointercancel', onPointerUp);
        }

        window.addEventListener('mousemove', (e) => {
            if (draggingWinId) return;
            const now = performance.now();
            if (now - lastMouseMoveTime >= 16) {
                lastMouseMoveTime = now;
                const dRect = desktop.getBoundingClientRect();
                const mx = e.clientX - dRect.left + desktop.scrollLeft;
                const my = e.clientY - dRect.top + desktop.scrollTop;
                postEvent({ type: 'mouse_move', x: mx, y: my });
            }
        });

        window.addEventListener('mousedown', (e) => {
            if (e.target.closest('.imgui-btn, .imgui-menu-item, .imgui-check-box, .imgui-tree, .imgui-slider-box, .imgui-input-box, .imgui-combo-box, .imgui-tab-item, .imgui-listbox-box, .imgui-color-box, .imgui-textarea-box')) {
                return;
            }
            if (e.target.closest('.imgui-header')) {
                return;
            }
            const dRect = desktop.getBoundingClientRect();
            const mx = e.clientX - dRect.left + desktop.scrollLeft;
            const my = e.clientY - dRect.top + desktop.scrollTop;
            postEvent({ type: 'mouse_down', button: e.button, x: mx, y: my });
        });

        window.addEventListener('mouseup', (e) => {
            activeSliders.clear();
            if (e.target.closest('.imgui-btn, .imgui-menu-item, .imgui-check-box, .imgui-tree, .imgui-slider-box, .imgui-input-box, .imgui-combo-box, .imgui-tab-item, .imgui-listbox-box, .imgui-color-box, .imgui-textarea-box')) {
                return;
            }
            if (e.target.closest('.imgui-header')) {
                return;
            }
            const dRect = desktop.getBoundingClientRect();
            const mx = e.clientX - dRect.left + desktop.scrollLeft;
            const my = e.clientY - dRect.top + desktop.scrollTop;
            postEvent({ type: 'mouse_up', button: e.button, x: mx, y: my });
        });
        window.addEventListener('blur', () => {
            activeSliders.clear();
            postEvent({ type: 'mouse_up', button: 0, x: 0, y: 0 });
        });
        window.addEventListener('touchend', () => activeSliders.clear());

        window.addEventListener('wheel', (e) => {
            const dx = -e.deltaX / 100.0;
            const dy = -e.deltaY / 100.0;
            postEvent({ type: 'mouse_wheel', dx: dx, dy: dy });
        }, { passive: true });

        window.addEventListener('keydown', (e) => {
            if (e.target.tagName === 'INPUT' || e.target.tagName === 'TEXTAREA' || e.target.tagName === 'SELECT') return;
            if (e.key.length === 1 && !e.ctrlKey && !e.metaKey && !e.altKey) {
                postEvent({ type: 'char', text: e.key });
            }
        });
)HTML";
        s += R"HTML(
        function updateDom(doc) {
            window.__latestDoc = doc;
            const now = performance.now();
            frameCount++;
            if (now - lastFrameTime >= 1000) {
                fpsBadge.textContent = `${frameCount} FPS`;
                frameCount = 0;
                lastFrameTime = now;
            }

            // Sync active focus acknowledgment from backend display order
            const topDocWin = (doc.windows && doc.windows.length > 0) ? doc.windows[doc.windows.length - 1] : null;
            if (topDocWin && topDocWin.id === activeFocusWinId) {
                activeFocusWinId = null;
            }

            // 1. Collect all active window and element IDs in current snapshot
            const activeWinIds = new Set();
            const activeElIds = new Set();
            for (const win of doc.windows) {
                activeWinIds.add(`win_${win.id}`);
                for (const el of win.elements) {
                    activeElIds.add(`el_${el.id}`);
                }
            }

            // 2. Remove dead windows that are no longer in snapshot
            const currentWinEls = desktop.querySelectorAll('.imgui-window');
            for (const winEl of currentWinEls) {
                if (!activeWinIds.has(winEl.id)) {
                    winEl.remove();
                }
            }

            // 3. Remove dead elements across all windows (prevents overlapping and ghost clicks!)
            const currentElems = desktop.querySelectorAll('[id^="el_"]');
            for (const elemNode of currentElems) {
                if (!activeElIds.has(elemNode.id)) {
                    elemNode.remove();
                }
            }

            // 4. Sync windows with stacking order
            for (let winIdx = 0; winIdx < doc.windows.length; winIdx++) {
                const win = doc.windows[winIdx];
                let winEl = document.getElementById(`win_${win.id}`);
                if (!winEl) {
                    winEl = document.createElement('div');
                    winEl.id = `win_${win.id}`;
                    winEl.className = 'imgui-window';
                    winEl.innerHTML = `<div class="imgui-header"><span class="imgui-collapse-btn">▼</span><span class="imgui-win-title">${win.title}</span></div><div class="imgui-menubar" style="display:none"></div><div class="imgui-body"></div>`;
                    desktop.appendChild(winEl);
                    setupWindowDragging(winEl);
                }

                winEl.dataset.winId = win.id;
                winEl.dataset.winTitle = win.title;

                // Window dimensions & stacking z-index
                if (draggingWinId !== win.id) {
                    winEl.style.left = `${win.x}px`;
                    winEl.style.top = `${win.y}px`;
                }
                winEl.style.width = `${win.w}px`;
                winEl.style.height = `${win.h}px`;
                if (draggingWinId === win.id || activeFocusWinId === win.id) {
                    winEl.style.zIndex = Math.max(10 + winIdx, topZIndex);
                } else {
                    winEl.style.zIndex = 10 + winIdx;
                }
                if (win.is_tooltip) {
                    winEl.classList.add('tooltip');
                } else {
                    winEl.classList.remove('tooltip');
                }

                // Header visibility and height
                const header = winEl.querySelector('.imgui-header');
                if (win.has_title_bar === false) {
                    header.style.display = 'none';
                } else {
                    header.style.display = '';
                    if (win.title_bar_h) header.style.height = `${win.title_bar_h}px`;
                    const collapseBtn = header.querySelector('.imgui-collapse-btn');
                    if (collapseBtn) {
                        collapseBtn.textContent = win.collapsed ? '▶' : '▼';
                        collapseBtn.onpointerdown = (e) => e.stopPropagation();
                        collapseBtn.onmousedown = (e) => e.stopPropagation();
                        collapseBtn.onclick = (e) => {
                            e.stopPropagation();
                            postEvent({ type: 'click', id: win.id, x: win.x + 8, y: win.y + 8 });
                        };
                    }
                    const titleSpan = header.querySelector('.imgui-win-title');
                    if (titleSpan) titleSpan.textContent = win.title;
                }

                // Menubar strip
                const menuBarEl = winEl.querySelector('.imgui-menubar');
                if (win.has_menu_bar) {
                    menuBarEl.style.display = '';
                    menuBarEl.style.top = `${win.has_title_bar !== false ? (win.title_bar_h || 19) : 0}px`;
                    menuBarEl.style.height = `${win.menu_bar_h || 19}px`;
                } else {
                    menuBarEl.style.display = 'none';
                }

                const body = winEl.querySelector('.imgui-body');
                if (win.collapsed) {
                    body.style.display = 'none';
                    winEl.style.height = `${win.title_bar_h || 19}px`;
                } else {
                    body.style.display = '';
                }

                // Sync elements
                for (const el of win.elements) {
                    let elem = document.getElementById(`el_${el.id}`);
                    if (elem && elem.dataset.type !== el.type) {
                        elem.remove();
                        elem = null;
                    }
                    const localX = el.x - win.x;
                    const localY = el.y - win.y;

                    if (el.type === 'button') {
                        if (!elem) {
                            elem = document.createElement('button');
                            elem.id = `el_${el.id}`;
                            elem.dataset.type = el.type;
                            elem.className = 'imgui-btn';
                            elem.onclick = (e) => {
                                e.stopPropagation();
                                postEvent({ type: 'click', id: el.id, x: el.x + el.w/2, y: el.y + el.h/2 });
                            };
                            body.appendChild(elem);
                        }
                        elem.textContent = el.label;
                        elem.style.left = `${localX}px`;
                        elem.style.top = `${localY}px`;
                        elem.style.width = `${el.w}px`;
                        elem.style.height = `${el.h}px`;

                    } else if (el.type === 'slider') {
                        if (!elem) {
                            elem = document.createElement('div');
                            elem.id = `el_${el.id}`;
                            elem.dataset.type = el.type;
                            elem.className = 'imgui-slider-box';
                            elem.innerHTML = `<input type="range" min="${el.min !== undefined ? el.min : 0}" max="${el.max !== undefined ? el.max : 100}" step="any"> <span class="val-badge">${Number(el.val || 0).toFixed(1)}</span> <span class="imgui-slider-lbl"></span>`;
                            const slider = elem.querySelector('input');
                            slider.onmousedown = (e) => { e.stopPropagation(); activeSliders.add(el.id); };
                            slider.onmouseup = (e) => { e.stopPropagation(); activeSliders.delete(el.id); };
                            slider.ontouchstart = (e) => { e.stopPropagation(); activeSliders.add(el.id); };
                            slider.ontouchend = (e) => { e.stopPropagation(); activeSliders.delete(el.id); };
                            slider.oninput = (e) => {
                                e.stopPropagation();
                                const val = parseFloat(e.target.value);
                                elem.querySelector('.val-badge').textContent = val.toFixed(1);
                                postEvent({ type: 'slider', id: el.id, val: val });
                            };
                            body.appendChild(elem);
                        }
                        elem.querySelector('.imgui-slider-lbl').textContent = el.label ? el.label : '';
                        if (!activeSliders.has(el.id)) {
                            elem.querySelector('input').value = el.val;
                            elem.querySelector('.val-badge').textContent = Number(el.val || 0).toFixed(1);
                        }
                        elem.style.left = `${localX}px`;
                        elem.style.top = `${localY}px`;
                        elem.style.width = `${el.w}px`;
                        elem.style.height = `${el.h}px`;

                    }
)HTML";
        s += R"HTML(
                    else if (el.type === 'checkbox') {
                        if (!elem) {
                            elem = document.createElement('label');
                            elem.id = `el_${el.id}`;
                            elem.dataset.type = el.type;
                            elem.className = 'imgui-check-box';
                            elem.innerHTML = `<input type="checkbox"> <span></span>`;
                            const cb = elem.querySelector('input');
                            cb.onchange = (e) => {
                                e.stopPropagation();
                                postEvent({ type: 'checkbox', id: el.id, checked: e.target.checked });
                                postEvent({ type: 'click', id: el.id, x: el.x + 10, y: el.y + el.h/2 });
                            };
                            body.appendChild(elem);
                        }
                        elem.querySelector('input').checked = el.checked;
                        elem.querySelector('span').textContent = el.label;
                        elem.style.left = `${localX}px`;
                        elem.style.top = `${localY}px`;
                        if (el.w) elem.style.width = `${el.w}px`;
                        elem.style.height = `${el.h}px`;

                    }
)HTML";
        s += R"HTML(
                    else if (el.type === 'radio') {
                        if (!elem) {
                            elem = document.createElement('label');
                            elem.id = `el_${el.id}`;
                            elem.dataset.type = el.type;
                            elem.className = 'imgui-check-box';
                            elem.innerHTML = `<input type="radio" name="rad_${win.id}"> <span></span>`;
                            const rb = elem.querySelector('input');
                            rb.onchange = (e) => {
                                e.stopPropagation();
                                postEvent({ type: 'radio', id: el.id, checked: true });
                                postEvent({ type: 'click', id: el.id, x: el.x + 10, y: el.y + el.h/2 });
                            };
                            body.appendChild(elem);
                        }
                        elem.querySelector('input').checked = el.checked;
                        elem.querySelector('span').textContent = el.label;
                        elem.style.left = `${localX}px`;
                        elem.style.top = `${localY}px`;
                        if (el.w) elem.style.width = `${el.w}px`;
                        elem.style.height = `${el.h}px`;

                    } else if (el.type === 'input') {
                        if (!elem) {
                            elem = document.createElement('div');
                            elem.id = `el_${el.id}`;
                            elem.dataset.type = el.type;
                            elem.className = 'imgui-input-box';
                            elem.innerHTML = `<span class="imgui-input-lbl"></span> <input type="text" class="imgui-input">`;
                            const inp = elem.querySelector('input');
                            inp.oninput = (e) => {
                                e.stopPropagation();
                                postEvent({ type: 'input', id: el.id, text: e.target.value });
                            };
                            body.appendChild(elem);
                        }
                        elem.querySelector('.imgui-input-lbl').textContent = el.label ? `${el.label}:` : '';
                        const inp = elem.querySelector('input');
                        if (document.activeElement !== inp) {
                            inp.value = el.val || '';
                        }
                        elem.style.left = `${localX}px`;
                        elem.style.top = `${localY}px`;
                        elem.style.width = `${el.w}px`;
                        elem.style.height = `${el.h}px`;

                    } else if (el.type === 'combo') {
                        if (!elem) {
                            elem = document.createElement('div');
                            elem.id = `el_${el.id}`;
                            elem.dataset.type = el.type;
                            elem.className = 'imgui-combo-box';
                            elem.innerHTML = `<span class="imgui-combo-lbl"></span> <select class="imgui-select"><option selected>${el.val || 'Select...'}</option></select>`;
                            const sel = elem.querySelector('select');
                            sel.onchange = (e) => {
                                e.stopPropagation();
                                postEvent({ type: 'input', id: el.id, text: e.target.value });
                                postEvent({ type: 'click', id: el.id, x: el.x + 10, y: el.y + el.h/2 });
                            };
                            body.appendChild(elem);
                        }
                        elem.querySelector('.imgui-combo-lbl').textContent = el.label ? `${el.label}:` : '';
                        const sel = elem.querySelector('select');
                        sel.querySelector('option').textContent = el.val || 'Select...';
                        elem.style.left = `${localX}px`;
                        elem.style.top = `${localY}px`;
                        elem.style.width = `${el.w}px`;
                        elem.style.height = `${el.h}px`;

                    } else if (el.type === 'treenode') {
                        if (!elem) {
                            elem = document.createElement('div');
                            elem.id = `el_${el.id}`;
                            elem.dataset.type = el.type;
                            elem.className = 'imgui-tree';
                            elem.innerHTML = `<span class="imgui-tree-arrow">▶</span> <span class="imgui-tree-label"></span>`;
                            elem.onclick = (e) => {
                                e.stopPropagation();
                                postEvent({ type: 'click', id: el.id, x: el.x + 10, y: el.y + el.h/2 });
                            };
                            body.appendChild(elem);
                        }
                        const arrow = elem.querySelector('.imgui-tree-arrow');
                        if (arrow) arrow.textContent = el.opened ? '▼' : '▶';
                        const lbl = elem.querySelector('.imgui-tree-label');
                        if (lbl) lbl.textContent = el.label;
                        elem.style.left = `${localX}px`;
                        elem.style.top = `${localY}px`;
                        elem.style.width = `${el.w}px`;
                        elem.style.height = `${el.h || 20}px`;

                    } else if (el.type === 'collapsing_header') {
                        if (!elem) {
                            elem = document.createElement('div');
                            elem.id = `el_${el.id}`;
                            elem.dataset.type = el.type;
                            elem.className = 'imgui-collapsing-header';
                            elem.innerHTML = `<span class="imgui-header-arrow">▶</span> <span class="imgui-header-label"></span>`;
                            elem.onclick = (e) => {
                                e.stopPropagation();
                                postEvent({ type: 'click', id: el.id, x: el.x + 10, y: el.y + el.h/2 });
                            };
                            body.appendChild(elem);
                        }
                        const arrow = elem.querySelector('.imgui-header-arrow');
                        if (arrow) arrow.textContent = el.opened ? '▼' : '▶';
                        const lbl = elem.querySelector('.imgui-header-label');
                        if (lbl) lbl.textContent = el.label;
                        elem.style.left = `${localX}px`;
                        elem.style.top = `${localY}px`;
                        elem.style.width = `${el.w}px`;
                        elem.style.height = `${el.h || 22}px`;

                    } else if (el.type === 'progress') {
                        if (!elem) {
                            elem = document.createElement('div');
                            elem.id = `el_${el.id}`;
                            elem.dataset.type = el.type;
                            elem.className = 'imgui-progress-box';
                            elem.innerHTML = `<div class="imgui-progress-bar"></div><span class="imgui-progress-text"></span>`;
                            body.appendChild(elem);
                        }
                        const pct = Math.max(0, Math.min(100, Math.round(el.val * 100)));
                        elem.querySelector('.imgui-progress-bar').style.width = `${pct}%`;
                        elem.querySelector('.imgui-progress-text').textContent = el.overlay || `${pct}%`;
                        elem.style.left = `${localX}px`;
                        elem.style.top = `${localY}px`;
                        elem.style.width = `${el.w}px`;
                        elem.style.height = `${el.h}px`;

                    } else if (el.type === 'separator') {
                        if (!elem) {
                            elem = document.createElement('hr');
                            elem.id = `el_${el.id}`;
                            elem.dataset.type = el.type;
                            elem.className = 'imgui-separator';
                            body.appendChild(elem);
                        }
                        elem.style.left = `${localX}px`;
                        elem.style.top = `${localY}px`;
                        elem.style.width = `${el.w || 200}px`;

                    } else if (el.type === 'text') {
                        if (!elem) {
                            elem = document.createElement('div');
                            elem.id = `el_${el.id}`;
                            elem.dataset.type = el.type;
                            elem.className = 'imgui-text';
                            body.appendChild(elem);
                        }
                        elem.textContent = el.val || el.label;
                        elem.style.left = `${localX}px`;
                        elem.style.top = `${localY}px`;
                        if (el.w) elem.style.width = `${el.w}px`;
                        if (el.h) elem.style.height = `${el.h}px`;
                    }
)HTML";
        s += R"HTML(
                    else if (el.type === 'canvas') {
                        if (!elem) {
                            elem = document.createElement('canvas');
                            elem.id = `el_${el.id}`;
                            elem.dataset.type = el.type;
                            elem.className = 'imgui-canvas';
                            elem.width = Math.round(el.w);
                            elem.height = Math.round(el.h);
                            body.appendChild(elem);
                        }
                        if (elem.width !== Math.round(el.w) || elem.height !== Math.round(el.h)) {
                            elem.width = Math.round(el.w);
                            elem.height = Math.round(el.h);
                        }
                        elem.style.left = `${localX}px`;
                        elem.style.top = `${localY}px`;
                        elem.style.width = `${el.w}px`;
                        elem.style.height = `${el.h}px`;
                        if (el.points && el.points.length > 0) {
                            renderCanvasWave(elem, el.points);
                        }
                    } else if (el.type === 'tabbar') {
                        if (!elem) {
                            elem = document.createElement('div');
                            elem.id = `el_${el.id}`;
                            elem.dataset.type = el.type;
                            elem.className = 'imgui-tab-bar';
                            body.appendChild(elem);
                        }
                        elem.style.left = `${localX}px`;
                        elem.style.top = `${localY}px`;
                        elem.style.width = `${el.w}px`;
                        elem.style.height = `${el.h}px`;

                    } else if (el.type === 'tabitem') {
                        if (!elem) {
                            elem = document.createElement('div');
                            elem.id = `el_${el.id}`;
                            elem.dataset.type = el.type;
                            elem.className = 'imgui-tab-item';
                            elem.onclick = (e) => {
                                e.stopPropagation();
                                postEvent({ type: 'tab', id: el.id, label: el.label });
                                postEvent({ type: 'click', id: el.id, x: el.x + el.w/2, y: el.y + el.h/2 });
                            };
                            body.appendChild(elem);
                        }
                        elem.textContent = el.label;
                        if (el.selected) {
                            elem.classList.add('active');
                        } else {
                            elem.classList.remove('active');
                        }
                        elem.style.left = `${localX}px`;
                        elem.style.top = `${localY}px`;
                        elem.style.width = `${el.w}px`;
                        elem.style.height = `${el.h}px`;

                    } else if (el.type === 'table') {
                        if (!elem) {
                            elem = document.createElement('div');
                            elem.id = `el_${el.id}`;
                            elem.dataset.type = el.type;
                            elem.className = 'imgui-table-container';
                            body.appendChild(elem);
                        }
                        if (el.has_headers && el.columns && el.columns.length > 0) {
                            let hdr = elem.querySelector('.imgui-table-header-row');
                            if (!hdr) {
                                hdr = document.createElement('div');
                                hdr.className = 'imgui-table-header-row';
                                elem.appendChild(hdr);
                            }
                            hdr.innerHTML = '';
                            for (const col of el.columns) {
                                const th = document.createElement('div');
                                th.className = 'imgui-table-th';
                                th.textContent = col;
                                hdr.appendChild(th);
                            }
                        }
                        elem.style.left = `${localX}px`;
                        elem.style.top = `${localY}px`;
                        elem.style.width = `${el.w}px`;
                        elem.style.height = `${el.h}px`;

                    } else if (el.type === 'listbox') {
                        if (!elem) {
                            elem = document.createElement('div');
                            elem.id = `el_${el.id}`;
                            elem.dataset.type = el.type;
                            elem.className = 'imgui-listbox-box';
                            elem.innerHTML = `<span class="imgui-listbox-lbl"></span> <select class="imgui-listbox"></select>`;
                            const sel = elem.querySelector('select');
                            sel.onchange = (e) => {
                                e.stopPropagation();
                                postEvent({ type: 'listbox', id: el.id, value_num: e.target.selectedIndex });
                            };
                            body.appendChild(elem);
                        }
                        elem.querySelector('.imgui-listbox-lbl').textContent = el.label ? el.label : '';
                        const sel = elem.querySelector('select');
                        sel.size = Math.min(8, Math.max(2, (el.items ? el.items.length : 4)));
                        sel.innerHTML = '';
                        if (el.items) {
                            for (let i = 0; i < el.items.length; i++) {
                                const opt = document.createElement('option');
                                opt.value = i;
                                opt.textContent = el.items[i];
                                if (i === el.selected_idx) opt.selected = true;
                                sel.appendChild(opt);
                            }
                        }
                        elem.style.left = `${localX}px`;
                        elem.style.top = `${localY}px`;
                        elem.style.width = `${el.w}px`;
                        elem.style.height = `${el.h}px`;

                    } else if (el.type === 'coloredit') {
                        if (!elem) {
                            elem = document.createElement('div');
                            elem.id = `el_${el.id}`;
                            elem.dataset.type = el.type;
                            elem.className = 'imgui-color-box';
                            elem.innerHTML = `<input type="color" class="imgui-color-picker"> <span class="imgui-color-lbl"></span>`;
                            const picker = elem.querySelector('input');
                            picker.oninput = (e) => {
                                e.stopPropagation();
                                postEvent({ type: 'color', id: el.id, color: e.target.value, text: e.target.value });
                            };
                            body.appendChild(elem);
                        }
                        elem.querySelector('.imgui-color-lbl').textContent = el.label ? el.label : '';
                        const picker = elem.querySelector('input');
                        if (document.activeElement !== picker) {
                            picker.value = el.val || '#ffffff';
                        }
                        elem.style.left = `${localX}px`;
                        elem.style.top = `${localY}px`;
                        elem.style.width = `${el.w}px`;
                        elem.style.height = `${el.h}px`;

                    }
)HTML";
        s += R"HTML(
                    else if (el.type === 'textarea') {
                        if (!elem) {
                            elem = document.createElement('div');
                            elem.id = `el_${el.id}`;
                            elem.dataset.type = el.type;
                            elem.className = 'imgui-textarea-box';
                            elem.innerHTML = `<textarea class="imgui-textarea" spellcheck="false"></textarea>`;
                            const txt = elem.querySelector('textarea');
                            txt.oninput = (e) => {
                                e.stopPropagation();
                                postEvent({ type: 'input_multiline', id: el.id, text: e.target.value });
                            };
                            body.appendChild(elem);
                        }
                        const txt = elem.querySelector('textarea');
                        if (document.activeElement !== txt) {
                            txt.value = el.val || '';
                        }
                        elem.style.left = `${localX}px`;
                        elem.style.top = `${localY}px`;
                        elem.style.width = `${el.w}px`;
                        elem.style.height = `${el.h}px`;
                    } else if (el.type === 'menuitem') {
                        if (!elem) {
                            elem = document.createElement('button');
                            elem.id = `el_${el.id}`;
                            elem.dataset.type = el.type;
                            elem.className = 'imgui-menu-item';
                            elem.onclick = (e) => {
                                e.stopPropagation();
                                postEvent({ type: 'click', id: el.id, x: el.x + el.w/2, y: el.y + el.h/2 });
                            };
                            winEl.appendChild(elem);
                        }
                        elem.textContent = el.label;
                        elem.style.left = `${localX}px`;
                        elem.style.top = `${localY}px`;
                        elem.style.width = `${el.w}px`;
                        elem.style.height = `${el.h}px`;
                        elem.style.zIndex = 9;
                    }
                    if (elem) {
                        elem.dataset.expectedX = String(el.x);
                        elem.dataset.expectedY = String(el.y);
                        elem.dataset.expectedW = String(el.w);
                        elem.dataset.expectedH = String(el.h);
                        if (el.disabled) elem.classList.add('imgui-disabled');
                        else elem.classList.remove('imgui-disabled');
                        if (el.tooltip) elem.title = el.tooltip;
                        else elem.removeAttribute('title');
                    }
                }
            }
        }
)HTML";
        s += R"HTML(
        window.__auditDomLayout = function() {
            const report = {
                ok: true,
                checkedPairs: 0,
                collisions: [],
                drifts: [],
                timestamp: performance.now()
            };
            const desktopRect = desktop.getBoundingClientRect();
            const windows = document.querySelectorAll('.imgui-window');
            windows.forEach(win => {
                const header = win.querySelector('.imgui-header');
                const winTitle = header ? header.textContent : win.id;
                const interactive = Array.from(win.querySelectorAll(
                    '.imgui-btn, .imgui-menu-item, .imgui-check-box, .imgui-tree, .imgui-collapsing-header, .imgui-slider-box, .imgui-input-box, .imgui-combo-box, .imgui-tab-item, .imgui-listbox-box, .imgui-color-box, .imgui-textarea-box'
                ));

                for (let i = 0; i < interactive.length; i++) {
                    const a = interactive[i];
                    const rA = a.getBoundingClientRect();
                    if (rA.width <= 0 || rA.height <= 0) continue;

                    // Audit coordinate drift against C++ ground truth
                    const clientX = rA.left - desktopRect.left + desktop.scrollLeft;
                    const clientY = rA.top - desktopRect.top + desktop.scrollTop;
                    const expX = parseFloat(a.dataset.expectedX);
                    const expY = parseFloat(a.dataset.expectedY);
                    if (!isNaN(expX) && !isNaN(expY)) {
                        const dx = Math.abs(clientX - expX);
                        const dy = Math.abs(clientY - expY);
                        if (dx > 3.0 || dy > 3.0) {
                            report.drifts.push({
                                window: winTitle,
                                id: a.id,
                                type: a.dataset.type,
                                text: a.textContent.trim().substring(0, 30),
                                actual: { x: clientX, y: clientY },
                                expected: { x: expX, y: expY },
                                delta: { dx, dy }
                            });
                        }
                    }

                    for (let j = i + 1; j < interactive.length; j++) {
                        const b = interactive[j];
                        const rB = b.getBoundingClientRect();
                        if (rB.width <= 0 || rB.height <= 0) continue;

                        report.checkedPairs++;

                        const overlapX = Math.min(rA.right, rB.right) - Math.max(rA.left, rB.left);
                        const overlapY = Math.min(rA.bottom, rB.bottom) - Math.max(rA.top, rB.top);

                        if (overlapX > 2 && overlapY > 2) {
                            report.ok = false;
                            report.collisions.push({
                                window: winTitle,
                                elemA: { id: a.id, type: a.dataset.type, text: a.textContent.trim().substring(0, 30), rect: { x: rA.x, y: rA.y, w: rA.width, h: rA.height } },
                                elemB: { id: b.id, type: b.dataset.type, text: b.textContent.trim().substring(0, 30), rect: { x: rB.x, y: rB.y, w: rB.width, h: rB.height } },
                                overlap: { w: overlapX, h: overlapY }
                            });
                        }
                    }
                }
            });
            if (report.collisions.length > 0 || report.drifts.length > 0) {
                report.ok = false;
            }
            return report;
        };

        window.__auditCoordinateParity = function() {
            if (!window.__latestDoc) return { ok: false, error: "No doc snapshot available" };
            const doc = window.__latestDoc;
            const report = {
                ok: true,
                checkedWindows: 0,
                checkedElements: 0,
                missingElements: [],
                ghostElements: [],
                coordinateMismatches: [],
                dimensionMismatches: [],
                semanticMismatches: [],
                maxDeltaX: 0,
                maxDeltaY: 0,
                maxDeltaW: 0,
                maxDeltaH: 0,
                passRate: 100.0
            };

            const desktopRect = desktop.getBoundingClientRect();
            const activeDocElementIds = new Set();

            for (const win of doc.windows) {
                report.checkedWindows++;
                const winEl = document.getElementById(`win_${win.id}`);
                if (!winEl) {
                    report.ok = false;
                    report.missingElements.push({ type: 'window', id: win.id, title: win.title });
                    continue;
                }

                const winRect = winEl.getBoundingClientRect();
                const clientX = winRect.left - desktopRect.left + desktop.scrollLeft;
                const clientY = winRect.top - desktopRect.top + desktop.scrollTop;
                const dx = Math.abs(clientX - win.x);
                const dy = Math.abs(clientY - win.y);
                const dw = Math.abs(winRect.width - win.w);
                report.maxDeltaX = Math.max(report.maxDeltaX, dx);
                report.maxDeltaY = Math.max(report.maxDeltaY, dy);
                report.maxDeltaW = Math.max(report.maxDeltaW, dw);

                if (dx > 2.0 || dy > 2.0) {
                    report.ok = false;
                    report.coordinateMismatches.push({
                        type: 'window',
                        id: win.id,
                        title: win.title,
                        expected: { x: win.x, y: win.y },
                        actual: { x: clientX, y: clientY },
                        delta: { dx, dy }
                    });
                }

                for (const el of win.elements) {
                    report.checkedElements++;
                    activeDocElementIds.add(`el_${el.id}`);
                    const elem = document.getElementById(`el_${el.id}`);
                    if (!elem) {
                        report.ok = false;
                        report.missingElements.push({
                            window: win.title,
                            id: el.id,
                            type: el.type,
                            label: el.label || el.val
                        });
                        continue;
                    }

                    if (elem.dataset.type !== el.type) {
                        report.ok = false;
                        report.semanticMismatches.push({
                            id: el.id,
                            expectedType: el.type,
                            actualType: elem.dataset.type
                        });
                    }

                    const elemRect = elem.getBoundingClientRect();
                    const elClientX = elemRect.left - desktopRect.left + desktop.scrollLeft;
                    const elClientY = elemRect.top - desktopRect.top + desktop.scrollTop;
                    const elDx = Math.abs(elClientX - el.x);
                    const elDy = Math.abs(elClientY - el.y);
                    report.maxDeltaX = Math.max(report.maxDeltaX, elDx);
                    report.maxDeltaY = Math.max(report.maxDeltaY, elDy);

                    if (elDx > 2.0 || elDy > 2.0) {
                        report.ok = false;
                        report.coordinateMismatches.push({
                            window: win.title,
                            id: el.id,
                            type: el.type,
                            label: el.label || el.val,
                            expected: { x: el.x, y: el.y },
                            actual: { x: elClientX, y: elClientY },
                            delta: { dx: elDx, dy: elDy }
                        });
                    }

                    if (el.w > 0) {
                        const elDw = Math.abs(elemRect.width - el.w);
                        report.maxDeltaW = Math.max(report.maxDeltaW, elDw);
                        if (elDw > 3.0) {
                            report.dimensionMismatches.push({
                                window: win.title,
                                id: el.id,
                                type: el.type,
                                label: el.label || el.val,
                                expectedW: el.w,
                                actualW: elemRect.width,
                                deltaW: elDw
                            });
                        }
                    }
                }
            }

            const domElems = desktop.querySelectorAll('[id^="el_"]');
            for (const domEl of domElems) {
                if (!activeDocElementIds.has(domEl.id)) {
                    report.ok = false;
                    report.ghostElements.push({
                        id: domEl.id,
                        type: domEl.dataset.type,
                        text: domEl.textContent.trim().substring(0, 30)
                    });
                }
            }

            const totalErrors = report.missingElements.length + report.coordinateMismatches.length + report.semanticMismatches.length + report.ghostElements.length;
            if (report.checkedElements > 0) {
                report.passRate = Math.max(0, 100 - (totalErrors / report.checkedElements) * 100);
            }
            return report;
        };
)HTML";
        s += R"HTML(
        window.__auditStyleParity = function() {
            const report = {
                ok: true,
                checkedRules: 0,
                passedRules: 0,
                violations: [],
                metrics: {}
            };

            function check(name, elementSelector, prop, expectedValues, description) {
                report.checkedRules++;
                const el = (typeof elementSelector === 'string') ? document.querySelector(elementSelector) : elementSelector;
                if (!el) {
                    report.violations.push({ rule: name, error: 'Element not found', selector: String(elementSelector) });
                    report.ok = false;
                    return;
                }
                const cs = window.getComputedStyle(el);
                const actual = cs.getPropertyValue(prop).trim();
                const allowed = Array.isArray(expectedValues) ? expectedValues : [expectedValues];
                const match = allowed.some(exp => {
                    if (typeof exp === 'string') return actual.toLowerCase() === exp.toLowerCase();
                    if (exp instanceof RegExp) return exp.test(actual);
                    return false;
                });
                if (!match) {
                    report.violations.push({
                        rule: name,
                        selector: (typeof elementSelector === 'string') ? elementSelector : (el.className || el.tagName),
                        prop,
                        expected: allowed.map(e => String(e)),
                        actual,
                        description
                    });
                    report.ok = false;
                } else {
                    report.passedRules++;
                }
            }

            // 1. Window geometry & style: 0px rounding, Dark theme background
            check('WindowRounding', '.imgui-window', 'border-radius', ['0px'], 'Dear ImGui StyleColorsDark specifies WindowRounding = 0.0f');
            check('WindowBackground', '.imgui-window', 'background-color', ['rgba(15, 15, 15, 0.94)', 'rgb(15, 15, 15)'], 'WindowBg canonical dark color');

            // 2. Buttons: canonical button color, 0px frame rounding, white text
            check('ButtonRounding', '.imgui-btn', 'border-radius', ['0px'], 'Dear ImGui StyleColorsDark specifies FrameRounding = 0.0f');
            check('ButtonBackground', '.imgui-btn', 'background-color', ['rgba(66, 150, 250, 0.4)', 'rgba(66, 150, 250, 0.40)'], 'Button canonical color ImVec4(0.26f, 0.59f, 0.98f, 0.40f)');
            check('ButtonColor', '.imgui-btn', 'color', ['rgb(255, 255, 255)', '#ffffff'], 'Button text color');

            // 3. Disabled element parity: opacity: 0.60, pointer-events: none
            const disabledEl = document.querySelector('.imgui-disabled');
            if (disabledEl) {
                check('DisabledAlpha', disabledEl, 'opacity', ['0.6', '0.60'], 'ImGuiStyle.DisabledAlpha = 0.60f');
                check('DisabledPointerEvents', disabledEl, 'pointer-events', ['none'], 'Disabled widgets must reject pointer interactions');
            } else {
                report.violations.push({ rule: 'DisabledElementCheck', error: 'No element with .imgui-disabled found in DOM' });
                report.ok = false;
            }

            // 4. Inputs / Sliders / FrameBg: canonical frame background and 0px frame rounding
            const frameInput = document.querySelector('.imgui-input');
            if (frameInput) {
                check('InputRounding', frameInput, 'border-radius', ['0px'], 'FrameRounding = 0.0f for inputs');
                check('InputFrameBg', frameInput, 'background-color', ['rgba(41, 74, 122, 0.54)'], 'FrameBg canonical color');
            }
            const sliderInput = document.querySelector('.imgui-slider-box input[type="range"]');
            if (sliderInput) {
                check('SliderRounding', sliderInput, 'border-radius', ['0px'], 'FrameRounding = 0.0f for sliders');
                check('SliderFrameBg', sliderInput, 'background-color', ['rgba(41, 74, 122, 0.54)'], 'FrameBg canonical color');
            }

            // 5. Collapsing Header: canonical header color & 0px rounding
            const headerEl = document.querySelector('.imgui-collapsing-header');
            if (headerEl) {
                check('HeaderRounding', headerEl, 'border-radius', ['0px'], 'Header rounding = 0.0f');
                check('HeaderBg', headerEl, 'background-color', ['rgba(66, 150, 250, 0.31)'], 'Header canonical color ImVec4(0.26f, 0.59f, 0.98f, 0.31f)');
            }

            // 6. Typography & Global Text color
            check('TextGlobalColor', 'body', 'color', ['rgb(255, 255, 255)', '#ffffff'], 'Global text color white');

            report.metrics = {
                checkedRules: report.checkedRules,
                passedRules: report.passedRules,
                parityScore: report.checkedRules > 0 ? ((report.passedRules / report.checkedRules) * 100).toFixed(1) : 0
            };
            return report;
        };
)HTML";
        s += R"HTML(
        function connectWS() {
            if (!window.WebSocket) {
                connectSSE();
                return;
            }
            const wsProto = location.protocol === 'https:' ? 'wss:' : 'ws:';
            const wsUrl = `${wsProto}//${location.host}/ws`;
            try {
                ws = new WebSocket(wsUrl);
                ws.onopen = () => {
                    useWs = true;
                    statusDot.className = 'dot';
                    statusText.textContent = 'Connected (WebSocket Full-Duplex)';
                };
                ws.onmessage = (e) => {
                    try {
                        const doc = JSON.parse(e.data);
                        updateDom(doc);
                    } catch(err) {
                        console.error("DOM Parse error", err);
                    }
                };
                ws.onerror = (e) => {
                    if (!useWs) {
                        connectSSE();
                    }
                };
                ws.onclose = () => {
                    if (useWs) {
                        statusDot.className = 'dot disconnected';
                        statusText.textContent = 'Disconnected. Reconnecting...';
                        useWs = false;
                        setTimeout(connectWS, 1500);
                    }
                };
            } catch(err) {
                console.warn("WebSocket failed, falling back to SSE", err);
                connectSSE();
            }
        }

        // Connect to C++ SSE stream (fallback)
        function connectSSE() {
            if (useWs) return;
            const es = new EventSource('/api/stream');
            es.onopen = () => {
                statusDot.className = 'dot';
                statusText.textContent = 'Connected (HTTP/SSE DOM Fallback)';
            };
            es.onmessage = (e) => {
                try {
                    const doc = JSON.parse(e.data);
                    updateDom(doc);
                } catch(err) {
                    console.error("DOM Parse error", err);
                }
            };
            es.onerror = () => {
                statusDot.className = 'dot disconnected';
                statusText.textContent = 'Disconnected. Reconnecting...';
                es.close();
                setTimeout(connectWS, 1500);
            };
        }

        connectWS();
    </script>
</body>
</html>
)HTML";
        return s;
    }();
    return html;
}

} // namespace ImGuiDom
