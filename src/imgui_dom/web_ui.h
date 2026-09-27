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
            display: flex;
            flex-direction: column;
            overflow: hidden;
        }

        .imgui-header {
            background: linear-gradient(180deg, #3d4a68 0%, #2b354c 100%);
            padding: 6px 12px;
            font-weight: 600;
            font-size: 13px;
            color: #ffffff;
            border-bottom: 1px solid #454558;
            cursor: default;
        }

        .imgui-body {
            position: relative;
            flex: 1;
            padding: 8px;
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

        /* Real-Time HTML5 Canvas for Oscilloscope */
        .imgui-canvas {
            position: absolute;
            background: #111116;
            border: 1px solid #333342;
            border-radius: 4px;
        }
    </style>
</head>
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

        window.addEventListener('mouseup', () => activeSliders.clear());
        window.addEventListener('touchend', () => activeSliders.clear());

        function updateDom(doc) {
            const now = performance.now();
            frameCount++;
            if (now - lastFrameTime >= 1000) {
                fpsBadge.textContent = `${frameCount} FPS`;
                frameCount = 0;
                lastFrameTime = now;
            }

            // Sync windows
            for (const win of doc.windows) {
                let winEl = document.getElementById(`win_${win.id}`);
                if (!winEl) {
                    winEl = document.createElement('div');
                    winEl.id = `win_${win.id}`;
                    winEl.className = 'imgui-window';
                    winEl.innerHTML = `<div class="imgui-header">${win.title}</div><div class="imgui-body"></div>`;
                    desktop.appendChild(winEl);
                }

                // Window dimensions
                winEl.style.left = `${win.x}px`;
                winEl.style.top = `${win.y}px`;
                winEl.style.width = `${win.w}px`;
                winEl.style.height = `${win.h}px`;

                const body = winEl.querySelector('.imgui-body');

                // Sync elements
                for (const el of win.elements) {
                    let elem = document.getElementById(`el_${el.id}`);
                    const localX = el.x - win.x;
                    const localY = el.y - win.y - 28; // Header offset

                    if (el.type === 'button') {
                        if (!elem) {
                            elem = document.createElement('button');
                            elem.id = `el_${el.id}`;
                            elem.className = 'imgui-btn';
                            elem.onclick = () => {
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
                            elem.className = 'imgui-slider-box';
                            elem.innerHTML = `<span>${el.label}:</span> <input type="range" min="${el.min}" max="${el.max}" step="0.1"> <span class="val-badge">${el.val.toFixed(1)}</span>`;
                            const slider = elem.querySelector('input');
                            slider.onmousedown = () => activeSliders.add(el.id);
                            slider.onmouseup = () => activeSliders.delete(el.id);
                            slider.ontouchstart = () => activeSliders.add(el.id);
                            slider.ontouchend = () => activeSliders.delete(el.id);
                            slider.oninput = (e) => {
                                const val = parseFloat(e.target.value);
                                elem.querySelector('.val-badge').textContent = val.toFixed(1);
                                postEvent({ type: 'slider', id: el.id, val: val });
                            };
                            body.appendChild(elem);
                        }
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
                            elem.className = 'imgui-check-box';
                            elem.innerHTML = `<input type="checkbox"> <span>${el.label}</span>`;
                            const cb = elem.querySelector('input');
                            cb.onchange = (e) => {
                                postEvent({ type: 'checkbox', id: el.id, checked: e.target.checked });
                            };
                            body.appendChild(elem);
                        }
                        elem.querySelector('input').checked = el.checked;
                        elem.style.left = `${localX}px`;
                        elem.style.top = `${localY}px`;
                        elem.style.height = `${el.h}px`;

                    } else if (el.type === 'text') {
                        if (!elem) {
                            elem = document.createElement('div');
                            elem.id = `el_${el.id}`;
                            elem.className = 'imgui-text';
                            body.appendChild(elem);
                        }
                        elem.textContent = el.val || el.label;
                        elem.style.left = `${localX}px`;
                        elem.style.top = `${localY}px`;

                    } else if (el.type === 'canvas') {
                        if (!elem) {
                            elem = document.createElement('canvas');
                            elem.id = `el_${el.id}`;
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
                    }
                }
            }
        }

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
