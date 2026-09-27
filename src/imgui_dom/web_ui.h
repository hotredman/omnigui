#pragma once

namespace ImGuiDom {

inline const char* GetWebClientHtml() {
    return R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ImGui Web DOM Backend</title>
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            background-color: #1a1a22;
            color: #efefef;
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            font-size: 14px;
            overflow: hidden;
            width: 100vw;
            height: 100vh;
            user-select: none;
        }

        #topbar {
            position: fixed;
            top: 0; left: 0; right: 0;
            height: 32px;
            background: #252530;
            border-bottom: 1px solid #363646;
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
            top: 32px; left: 0; right: 0; bottom: 0;
            overflow: auto;
        }

        /* ImGui Window Styling */
        .imgui-window {
            position: absolute;
            background-color: #242430;
            border: 1px solid #454558;
            border-radius: 6px;
            box-shadow: 0 8px 24px rgba(0,0,0,0.5);
            overflow: hidden;
        }

        .imgui-header {
            position: absolute;
            top: 0; left: 0; right: 0;
            background: linear-gradient(180deg, #3d4a68 0%, #2b354c 100%);
            padding: 2px 10px;
            font-weight: 600;
            font-size: 13px;
            color: #ffffff;
            border-bottom: 1px solid #454558;
            cursor: default;
            user-select: none;
            box-sizing: border-box;
            z-index: 1;
            display: flex;
            align-items: center;
        }

        .imgui-menubar {
            position: absolute;
            left: 0; right: 0;
            background: rgba(255, 255, 255, 0.04);
            border-bottom: 1px solid rgba(255, 255, 255, 0.08);
            box-sizing: border-box;
            z-index: 1;
        }

        .imgui-body {
            position: absolute;
            top: 0; left: 0; right: 0; bottom: 0;
            overflow: hidden;
            z-index: 2;
        }

        /* Native HTML Button styled as ImGui */
        .imgui-btn {
            position: absolute;
            background: #2f6690;
            color: #ffffff;
            border: 1px solid #4480aa;
            border-radius: 4px;
            padding: 4px 10px;
            font-size: 13px;
            cursor: pointer;
            outline: none;
            transition: background 0.1s, transform 0.05s;
        }
        .imgui-btn:hover {
            background: #3a7ca5;
            border-color: #5599c5;
        }
        .imgui-btn:active {
            background: #1f4e70;
            transform: translateY(1px);
        }

        /* Native Slider */
        .imgui-slider-box {
            position: absolute;
            display: flex;
            align-items: center;
            gap: 8px;
            color: #ddd;
            font-size: 12px;
        }
        .imgui-slider-box input[type="range"] {
            flex: 1;
            accent-color: #3a7ca5;
            cursor: pointer;
        }

        /* Native Checkbox */
        .imgui-check-box {
            position: absolute;
            display: inline-flex;
            align-items: center;
            gap: 6px;
            cursor: pointer;
            font-size: 13px;
        }
        .imgui-check-box input[type="checkbox"] {
            accent-color: #3a7ca5;
            width: 16px; height: 16px;
            cursor: pointer;
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
        }
        .imgui-input {
            background: #1e1e28;
            color: #fff;
            border: 1px solid #444458;
            border-radius: 4px;
            padding: 3px 8px;
            font-size: 13px;
            outline: none;
            flex: 1;
        }
        .imgui-input:focus {
            border-color: #3a7ca5;
            box-shadow: 0 0 4px rgba(58,124,165,0.5);
        }

        /* Native Combo / Select */
        .imgui-combo-box {
            position: absolute;
            display: flex;
            align-items: center;
            gap: 6px;
            font-size: 13px;
        }
        .imgui-select {
            background: #2a2a38;
            color: #fff;
            border: 1px solid #444458;
            border-radius: 4px;
            padding: 3px 8px;
            font-size: 13px;
            outline: none;
            cursor: pointer;
        }

        /* Native ProgressBar */
        .imgui-progress-box {
            position: absolute;
            background: #1e1e28;
            border: 1px solid #444458;
            border-radius: 4px;
            overflow: hidden;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .imgui-progress-bar {
            position: absolute;
            left: 0; top: 0; bottom: 0;
            background: linear-gradient(90deg, #2f6690, #3a7ca5);
            transition: width 0.1s ease;
        }
        .imgui-progress-text {
            position: relative;
            font-size: 11px;
            font-weight: 600;
            color: #fff;
            text-shadow: 0 1px 2px rgba(0,0,0,0.8);
            z-index: 1;
        }

        /* Native Separator */
        .imgui-separator {
            position: absolute;
            border: none;
            border-top: 1px solid #444458;
            margin: 0;
        }

        /* Native Tree Node */
        .imgui-tree {
            position: absolute;
            color: #d0d0e0;
            font-size: 13px;
            font-weight: 500;
            cursor: pointer;
            user-select: none;
            display: inline-flex;
            align-items: center;
            gap: 6px;
            padding: 2px 4px;
            border-radius: 3px;
            white-space: nowrap;
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

        .imgui-disabled {
            opacity: 0.45 !important;
            pointer-events: none !important;
        }

        /* Native TabBar & TabItems */
        .imgui-tab-bar {
            position: absolute;
            display: flex;
            border-bottom: 1px solid #444458;
            box-sizing: border-box;
        }
        .imgui-tab-item {
            position: absolute;
            background: #252530;
            color: #aaaab8;
            border: 1px solid #363646;
            border-bottom: none;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
            padding: 3px 10px;
            font-size: 12px;
            cursor: pointer;
            user-select: none;
            transition: background 0.1s, color 0.1s;
            box-sizing: border-box;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .imgui-tab-item:hover {
            background: #323240;
            color: #fff;
        }
        .imgui-tab-item.active {
            background: #2f6690;
            color: #fff;
            font-weight: 600;
            border-color: #4480aa;
        }

        /* Tooltip Window */
        .imgui-window.tooltip {
            pointer-events: none;
            z-index: 9999 !important;
            background-color: #1a1a24;
            border-color: #55556a;
            box-shadow: 0 4px 14px rgba(0,0,0,0.7);
        }

        /* Native Table Styling */
        .imgui-table-container {
            position: absolute;
            background: rgba(30, 30, 40, 0.6);
            border: 1px solid #444458;
            border-radius: 4px;
            overflow: hidden;
            box-sizing: border-box;
            pointer-events: none;
        }
        .imgui-table-header-row {
            display: flex;
            background: #2a3448;
            border-bottom: 1px solid #444458;
            font-size: 12px;
            font-weight: 600;
            color: #d0d8e8;
        }
        .imgui-table-th {
            flex: 1;
            padding: 4px 8px;
            border-right: 1px solid #3d4a60;
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
            background: #1e1e28;
            color: #efefef;
            border: 1px solid #444458;
            border-radius: 4px;
            padding: 2px;
            font-size: 12px;
            outline: none;
            overflow-y: auto;
            flex: 1;
        }
        .imgui-listbox option {
            padding: 3px 8px;
            border-radius: 2px;
            cursor: pointer;
        }
        .imgui-listbox option:hover {
            background: #2c3850;
        }
        .imgui-listbox option:checked {
            background: #2f6690;
            color: #fff;
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
            border: 1px solid #444458;
            border-radius: 3px;
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
            background: #1e1e28;
            color: #efefef;
            border: 1px solid #444458;
            border-radius: 4px;
            padding: 6px;
            font-size: 12px;
            font-family: monospace;
            outline: none;
            resize: none;
            flex: 1;
        }
        .imgui-textarea:focus {
            border-color: #3a7ca5;
            box-shadow: 0 0 4px rgba(58,124,165,0.5);
        }
    </style>
</head>
)HTML"
R"HTML(
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
)HTML"
R"HTML(
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
            if (e.target.closest('.imgui-btn, .imgui-check-box, .imgui-tree, .imgui-slider-box, .imgui-input-box, .imgui-combo-box, .imgui-tab-item, .imgui-listbox-box, .imgui-color-box, .imgui-textarea-box')) {
                return;
            }
            const dRect = desktop.getBoundingClientRect();
            const mx = e.clientX - dRect.left + desktop.scrollLeft;
            const my = e.clientY - dRect.top + desktop.scrollTop;
            postEvent({ type: 'mouse_down', button: e.button, x: mx, y: my });
        });

        window.addEventListener('mouseup', (e) => {
            activeSliders.clear();
            if (e.target.closest('.imgui-btn, .imgui-check-box, .imgui-tree, .imgui-slider-box, .imgui-input-box, .imgui-combo-box, .imgui-tab-item, .imgui-listbox-box, .imgui-color-box, .imgui-textarea-box')) {
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
                    winEl.innerHTML = `<div class="imgui-header">${win.title}</div><div class="imgui-menubar" style="display:none"></div><div class="imgui-body"></div>`;
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
                            elem.innerHTML = `<span class="imgui-slider-lbl"></span> <input type="range" min="${el.min}" max="${el.max}" step="0.1"> <span class="val-badge">${el.val.toFixed(1)}</span>`;
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
                        elem.querySelector('.imgui-slider-lbl').textContent = el.label ? `${el.label}:` : '';
                        if (!activeSliders.has(el.id)) {
                            elem.querySelector('input').value = el.val;
                            elem.querySelector('.val-badge').textContent = el.val.toFixed(1);
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
                        elem.style.height = `${el.h}px`;

                    }
)HTML"
R"HTML(
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

                    }
)HTML"
R"HTML(
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
                    }
                    if (elem) {
                        if (el.disabled) elem.classList.add('imgui-disabled');
                        else elem.classList.remove('imgui-disabled');
                        if (el.tooltip) elem.title = el.tooltip;
                        else elem.removeAttribute('title');
                    }
                }
            }
        }
)HTML"
R"HTML(
        window.__auditDomLayout = function() {
            const report = {
                ok: true,
                checkedPairs: 0,
                collisions: [],
                timestamp: performance.now()
            };
            const windows = document.querySelectorAll('.imgui-window');
            windows.forEach(win => {
                const header = win.querySelector('.imgui-header');
                const winTitle = header ? header.textContent : win.id;
                const interactive = Array.from(win.querySelectorAll(
                    '.imgui-btn, .imgui-check-box, .imgui-tree, .imgui-slider-box, .imgui-input-box, .imgui-combo-box, .imgui-tab-item, .imgui-listbox-box, .imgui-color-box, .imgui-textarea-box'
                ));

                for (let i = 0; i < interactive.length; i++) {
                    const a = interactive[i];
                    const rA = a.getBoundingClientRect();
                    if (rA.width <= 0 || rA.height <= 0) continue;

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
}

} // namespace ImGuiDom
