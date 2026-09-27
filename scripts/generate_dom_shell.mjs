import fs from 'node:fs';
import path from 'node:path';

const webUiPath = path.resolve('src/imgui_dom/web_ui.h');
const outputPath = path.resolve('web/shell_dom.html');

const content = fs.readFileSync(webUiPath, 'utf8');
const chunks = [];
const regex = /R"HTML\(([\s\S]*?)\)HTML"/g;
let match;
while ((match = regex.exec(content)) !== null) {
    chunks.push(match[1]);
}
let html = chunks.join('');

const injection = `
    <script type="text/javascript">
        var Module = {
            preRun: [],
            postRun: [function() {
                console.log("[OmniGUI] WebAssembly DOM engine initialized successfully!");
            }],
            print: function(text) { console.log("[WASM stdout] " + text); },
            printErr: function(text) { console.error("[WASM stderr] " + text); }
        };
        window.__omniWasmDirectMode = true;
        window.__omniWasmSendEvent = function(jsonStr) {
            if (typeof Module !== 'undefined' && Module.ccall) {
                Module.ccall('omni_wasm_send_event', null, ['string'], [jsonStr]);
            }
        };
    </script>
    {{{ SCRIPT }}}
</body>`;

html = html.replace('</body>', injection);

if (!fs.existsSync('web')) {
    fs.mkdirSync('web', { recursive: true });
}

fs.writeFileSync(outputPath, html, 'utf8');
console.log('Successfully generated web/shell_dom.html, size:', html.length, 'bytes');
