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
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            background-color: #14141a;
            color: #efefef;
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
            background: #1e1e28;
            border-bottom: 1px solid #323242;
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

        /* ImGui Window Styling - Authentic Dear ImGui Dark */
        .imgui-window {
            position: absolute;
            background-color: #0f0f14;
            border: 1px solid #3d4a60;
            border-radius: 3px;
            box-shadow: 0 6px 20px rgba(0,0,0,0.6);
            overflow: hidden;
        }

        .imgui-header {
            position: absolute;
            top: 0; left: 0; right: 0;
            height: 19px;
            background: linear-gradient(180deg, #2c3e55 0%, #1f2c3d 100%);
            padding: 1px 6px;
            font-weight: 600;
            font-size: 13px;
            color: #ffffff;
            border-bottom: 1px solid #3d4a60;
            cursor: default;
            user-select: none;
            box-sizing: border-box;
            z-index: 10;
            display: flex;
            align-items: center;
            gap: 6px;
            white-space: nowrap;
            overflow: hidden;
            text-overflow: ellipsis;
        }
        .imgui-collapse-btn {
            font-size: 10px;
            color: #9cb5d1;
            cursor: pointer;
            width: 14px;
            height: 14px;
            display: inline-flex;
            align-items: center;
            justify-content: center;
            user-select: none;
            flex-shrink: 0;
            border-radius: 2px;
            transition: color 0.1s, background 0.1s;
        }
        .imgui-collapse-btn:hover {
            color: #ffffff;
            background: rgba(255, 255, 255, 0.15);
        }
        .imgui-win-title {
            overflow: hidden;
            text-overflow: ellipsis;
            white-space: nowrap;
        }

        .imgui-menubar {
            position: absolute;
            left: 0; right: 0;
            height: 19px;
            background: #1a1a24;
            border-bottom: 1px solid #333344;
            box-sizing: border-box;
            z-index: 8;
            display: flex;
            align-items: center;
            padding: 0 4px;
        }

        .imgui-menu-item {
            position: absolute;
            background: transparent;
            color: #d0d8e8;
            border: none;
            border-radius: 2px;
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
            background: #2b4566;
            color: #ffffff;
        }

        .imgui-body {
            position: absolute;
            top: 0; left: 0; right: 0; bottom: 0;
            overflow: hidden;
            z-index: 1;
        }

        /* Native HTML Button styled as ImGui */
        .imgui-btn {
            position: absolute;
            background: #2b4566;
            color: #f0f0f0;
            border: 1px solid #3d608f;
            border-radius: 2px;
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
            background: #3a608c;
            border-color: #5585ba;
            color: #ffffff;
        }
        .imgui-btn:active {
            background: #203550;
            border-color: #304e75;
        }

        /* Native Slider - Authentic rectangular grabber */
        .imgui-slider-box {
            position: absolute;
            display: flex;
            align-items: center;
            gap: 6px;
            color: #ddd;
            font-size: 13px;
            white-space: nowrap;
            user-select: none;
            box-sizing: border-box;
        }
        .imgui-slider-box input[type="range"] {
            -webkit-appearance: none;
            appearance: none;
            flex: 1;
            height: 18px;
            background: #1e2634;
            border: 1px solid #36465d;
            border-radius: 2px;
            outline: none;
            cursor: pointer;
            margin: 0;
            padding: 0;
        }
        .imgui-slider-box input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 10px;
            height: 16px;
            background: #3a608c;
            border: 1px solid #5585ba;
            border-radius: 2px;
            cursor: pointer;
        }
        .imgui-slider-box input[type="range"]::-webkit-slider-thumb:hover {
            background: #487ab5;
            border-color: #6da3db;
        }
        .imgui-slider-box input[type="range"]::-webkit-slider-thumb:active {
            background: #5c9ae0;
        }
        .imgui-slider-box input[type="range"]::-moz-range-thumb {
            width: 10px;
            height: 16px;
            background: #3a608c;
            border: 1px solid #5585ba;
            border-radius: 2px;
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
            color: #efefef;
        }
        .imgui-check-box input[type="checkbox"] {
            -webkit-appearance: none;
            appearance: none;
            width: 15px;
            height: 15px;
            background: #1e2634;
            border: 1px solid #3d608f;
            border-radius: 2px;
            cursor: pointer;
            outline: none;
            position: relative;
            margin: 0;
            flex-shrink: 0;
        }
        .imgui-check-box input[type="checkbox"]:hover {
            border-color: #5585ba;
        }
        .imgui-check-box input[type="checkbox"]:checked {
            background: #2b4566;
            border-color: #5585ba;
        }
        .imgui-check-box input[type="checkbox"]:checked::after {
            content: '';
            position: absolute;
            left: 4px;
            top: 1px;
            width: 4px;
            height: 8px;
            border: solid #ffffff;
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
            background: #1e2634;
            border: 1px solid #3d608f;
            border-radius: 50%;
            cursor: pointer;
            outline: none;
            position: relative;
            margin: 0;
            flex-shrink: 0;
        }
        .imgui-check-box input[type="radio"]:hover {
            border-color: #5585ba;
        }
        .imgui-check-box input[type="radio"]:checked {
            background: #2b4566;
            border-color: #5585ba;
        }
        .imgui-check-box input[type="radio"]:checked::after {
            content: '';
            position: absolute;
            left: 3.5px;
            top: 3.5px;
            width: 6px;
            height: 6px;
            background: #ffffff;
            border-radius: 50%;
        }

        /* Native Text */
        .imgui-text {
            position: absolute;
            color: #eaeaea;
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
        }
        .imgui-input {
            background: #161b24;
            color: #ffffff;
            border: 1px solid #36465d;
            border-radius: 2px;
            padding: 2px 6px;
            font-size: 13px;
            font-family: inherit;
            outline: none;
            flex: 1;
            box-sizing: border-box;
        }
        .imgui-input:focus {
            border-color: #4d7eb8;
            background: #1c232f;
        }

        /* Native Combo / Select */
        .imgui-combo-box {
            position: absolute;
            display: flex;
            align-items: center;
            gap: 6px;
            font-size: 13px;
            white-space: nowrap;
        }
        .imgui-select {
            background: #1e2634;
            color: #ffffff;
            border: 1px solid #36465d;
            border-radius: 2px;
            padding: 2px 6px;
            font-size: 13px;
            font-family: inherit;
            outline: none;
            cursor: pointer;
            flex: 1;
            box-sizing: border-box;
        }
        .imgui-select:focus {
            border-color: #4d7eb8;
        }

        /* Native ProgressBar */
        .imgui-progress-box {
            position: absolute;
            background: #161b24;
            border: 1px solid #36465d;
            border-radius: 2px;
            overflow: hidden;
            display: flex;
            align-items: center;
            justify-content: center;
            box-sizing: border-box;
        }
        .imgui-progress-bar {
            position: absolute;
            left: 0; top: 0; bottom: 0;
            background: linear-gradient(90deg, #2b4566, #3a608c);
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
            border-top: 1px solid #36465d;
            margin: 0;
        }

        /* Native Tree Node */
        .imgui-tree {
            position: absolute;
            color: #d0d8e8;
            font-size: 13px;
            font-weight: 500;
            cursor: pointer;
            user-select: none;
            display: inline-flex;
            align-items: center;
            gap: 6px;
            padding: 1px 4px;
            border-radius: 2px;
            white-space: nowrap;
            box-sizing: border-box;
            transition: background 0.1s, color 0.1s;
        }
        .imgui-tree:hover {
            background: rgba(255, 255, 255, 0.08);
            color: #ffffff;
        }
        .imgui-tree-arrow {
            font-size: 10px;
            color: #8888a0;
            width: 12px;
            display: inline-block;
            text-align: center;
        }

        /* Native Collapsing Header - Authentic Dear ImGui Dark header banner */
        .imgui-collapsing-header {
            position: absolute;
            background: #2b4566;
            border: 1px solid #36465d;
            border-radius: 2px;
            color: #ffffff;
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
            transition: background 0.1s, border-color 0.1s;
        }
        .imgui-collapsing-header:hover {
            background: #3b5c88;
            border-color: #5585ba;
        }
        .imgui-collapsing-header:active {
            background: #1e3450;
        }
        .imgui-collapsing-header .imgui-header-arrow {
            font-size: 10px;
            color: #9cb5d1;
            width: 12px;
            display: inline-block;
            text-align: center;
            flex-shrink: 0;
        }

        .imgui-disabled {
            opacity: 0.45 !important;
            pointer-events: none !important;
        }

        /* Native HTML5 Canvas (Oscilloscope, Vector graphics) */
        .imgui-canvas {
            position: absolute;
            box-sizing: border-box;
            border-radius: 4px;
        }

        /* Native TabBar & TabItems */
        .imgui-tab-bar {
            position: absolute;
            display: flex;
            border-bottom: 1px solid #36465d;
            box-sizing: border-box;
        }
        .imgui-tab-item {
            position: absolute;
            background: #1e2634;
            color: #a0b0c0;
            border: 1px solid #36465d;
            border-bottom: none;
            border-top-left-radius: 3px;
            border-top-right-radius: 3px;
            padding: 2px 8px;
            font-size: 12px;
            cursor: pointer;
            user-select: none;
            box-sizing: border-box;
            display: flex;
            align-items: center;
            justify-content: center;
            transition: background 0.1s, color 0.1s;
        }
        .imgui-tab-item:hover {
            background: #2b3a50;
            color: #ffffff;
        }
        .imgui-tab-item.active {
            background: #2b4566;
            color: #ffffff;
            font-weight: 600;
            border-color: #4d7eb8;
        }

        /* Tooltip Window */
        .imgui-window.tooltip {
            pointer-events: none !important;
            z-index: 99999 !important;
            background-color: #14141c !important;
            border: 1px solid #484860 !important;
            box-shadow: 0 4px 14px rgba(0,0,0,0.8) !important;
            border-radius: 3px !important;
        }
        .imgui-window.tooltip .imgui-header {
            display: none !important;
        }

        /* Native Table Styling */
        .imgui-table-container {
            position: absolute;
            background: rgba(22, 27, 36, 0.8);
            border: 1px solid #36465d;
            border-radius: 2px;
            overflow: hidden;
            box-sizing: border-box;
            pointer-events: none;
        }
        .imgui-table-header-row {
            display: flex;
            background: #232d3d;
            border-bottom: 1px solid #36465d;
            font-size: 12px;
            font-weight: 600;
            color: #d0d8e8;
        }
        .imgui-table-th {
            flex: 1;
            padding: 3px 8px;
            border-right: 1px solid #36465d;
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
        window.addEventListener('mousemove', (e) => {
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

        function updateDom(doc) {
            window.__latestDoc = doc;
            const now = performance.now();
            frameCount++;
            if (now - lastFrameTime >= 1000) {
                fpsBadge.textContent = `${frameCount} FPS`;
                frameCount = 0;
                lastFrameTime = now;
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
                }

                // Window dimensions & stacking z-index
                winEl.style.left = `${win.x}px`;
                winEl.style.top = `${win.y}px`;
                winEl.style.width = `${win.w}px`;
                winEl.style.height = `${win.h}px`;
                winEl.style.zIndex = 10 + winIdx;
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

                    } else if (el.type === 'checkbox') {
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

                    } else if (el.type === 'textarea') {
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
