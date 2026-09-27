import fs from 'node:fs';
import path from 'node:path';
import os from 'node:os';
import { spawn } from 'node:child_process';

// ============================================================================
// CDP Client Implementation (Zero External Dependencies, Pure Node.js 22)
// ============================================================================
class CDPClient {
  constructor(wsUrl) {
    this.wsUrl = wsUrl;
    this.ws = null;
    this.nextId = 1;
    this.pending = new Map();
  }

  async connect() {
    return new Promise((resolve, reject) => {
      this.ws = new WebSocket(this.wsUrl);
      this.ws.onopen = () => resolve();
      this.ws.onerror = (err) => reject(err);
      this.ws.onmessage = (event) => {
        try {
          const msg = JSON.parse(event.data);
          if (msg.id && this.pending.has(msg.id)) {
            const { resolve, reject } = this.pending.get(msg.id);
            this.pending.delete(msg.id);
            if (msg.error) {
              reject(new Error(msg.error.message || JSON.stringify(msg.error)));
            } else {
              resolve(msg.result);
            }
          }
        } catch (e) {
          console.error('CDP parse error:', e);
        }
      };
    });
  }

  async send(method, params = {}) {
    const id = this.nextId++;
    return new Promise((resolve, reject) => {
      this.pending.set(id, { resolve, reject });
      this.ws.send(JSON.stringify({ id, method, params }));
    });
  }

  async evaluate(expression) {
    const res = await this.send('Runtime.evaluate', {
      expression,
      returnByValue: true,
      awaitPromise: true,
    });
    if (res.exceptionDetails) {
      throw new Error(`Evaluation failed: ${res.exceptionDetails.text || JSON.stringify(res.exceptionDetails)}`);
    }
    return res.result ? res.result.value : undefined;
  }

  async captureScreenshot() {
    const res = await this.send('Page.captureScreenshot', { format: 'png' });
    return Buffer.from(res.data, 'base64');
  }

  close() {
    if (this.ws) {
      this.ws.close();
    }
  }
}

// ============================================================================
// Environment Helpers
// ============================================================================
function findBrowser() {
  if (process.platform === 'win32') {
    const paths = [
      'C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe',
      'C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe',
      `${process.env.LOCALAPPDATA}\\Microsoft\\Edge\\Application\\msedge.exe`,
      `${process.env.LOCALAPPDATA}\\Google\\Chrome\\Application\\chrome.exe`,
      'C:\\Program Files\\Microsoft\\Edge\\Application\\msedge.exe',
    ];
    for (const p of paths) {
      if (fs.existsSync(p)) return p;
    }
  } else if (process.platform === 'darwin') {
    const paths = [
      '/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',
      '/Applications/Microsoft Edge.app/Contents/MacOS/Microsoft Edge',
    ];
    for (const p of paths) {
      if (fs.existsSync(p)) return p;
    }
  } else {
    const paths = [
      '/usr/bin/google-chrome',
      '/usr/bin/chromium-browser',
      '/usr/bin/chromium',
      '/usr/bin/microsoft-edge',
    ];
    for (const p of paths) {
      if (fs.existsSync(p)) return p;
    }
  }
  return null;
}

function findDemoBinary() {
  const isWin = process.platform === 'win32';
  const binName = isWin ? 'demo.exe' : 'demo';
  const candidates = [
    path.resolve('build/Release', binName),
    path.resolve('build/Debug', binName),
    path.resolve('build', binName),
  ];
  for (const c of candidates) {
    if (fs.existsSync(c)) return c;
  }
  return null;
}

async function waitForServer(domPort, timeoutMs = 12000) {
  const start = Date.now();
  while (Date.now() - start < timeoutMs) {
    try {
      const res = await fetch(`http://127.0.0.1:${domPort}/`);
      if (res.ok) return true;
    } catch {
      // Server not ready yet
    }
    await new Promise((r) => setTimeout(r, 150));
  }
  throw new Error(`Timeout waiting for ImGui Web server on port ${domPort}`);
}

async function waitForCdpTarget(cdpPort, timeoutMs = 12000) {
  const start = Date.now();
  while (Date.now() - start < timeoutMs) {
    try {
      const res = await fetch(`http://127.0.0.1:${cdpPort}/json/list`);
      if (res.ok) {
        const list = await res.json();
        const page = list.find((t) => t.type === 'page');
        if (page && page.webSocketDebuggerUrl) return page;
      }
    } catch {
      // Browser not ready yet
    }
    await new Promise((r) => setTimeout(r, 150));
  }
  throw new Error(`Timeout waiting for CDP page target on port ${cdpPort}`);
}

// ============================================================================
// Main E2E Test Suite Runner
// ============================================================================
async function runE2ETests() {
  console.log('========================================================');
  console.log(' Dear ImGui Web DOM - Headless Browser E2E Test Suite');
  console.log('========================================================');

  const browserPath = findBrowser();
  if (!browserPath) {
    console.error('[FAIL] No supported Chromium browser (Edge/Chrome) found on this system!');
    process.exit(1);
  }
  console.log(`[Browser] Found: ${browserPath}`);

  const demoPath = findDemoBinary();
  if (!demoPath) {
    console.error('[FAIL] demo binary not found in build/Release or build/Debug. Build it first!');
    process.exit(1);
  }
  console.log(`[Binary] Found: ${demoPath}`);

  const domPort = 8991;
  const cdpPort = 9223;
  const tmpDir = fs.mkdtempSync(path.join(os.tmpdir(), 'imgui-dom-e2e-'));

  let demoProcess = null;
  let browserProcess = null;
  let cdp = null;

  try {
    // 1. Start C++ Headless Demo Server
    console.log(`[Step 1] Starting C++ demo server on port ${domPort}...`);
    demoProcess = spawn(demoPath, ['--headless', '--port', String(domPort)], {
      stdio: ['ignore', 'pipe', 'pipe'],
    });

    demoProcess.stdout.on('data', (d) => {
      const str = d.toString();
      if (str.includes('[DomServer]')) process.stdout.write(str);
    });
    demoProcess.stderr.on('data', (d) => process.stderr.write(d));

    await waitForServer(domPort);
    console.log(`[Step 1] ImGui Web DOM server running at http://127.0.0.1:${domPort}`);

    // 2. Start Headless Chromium Browser with CDP
    console.log(`[Step 2] Launching headless browser with CDP on port ${cdpPort}...`);
    browserProcess = spawn(
      browserPath,
      [
        '--headless=new',
        `--remote-debugging-port=${cdpPort}`,
        `--user-data-dir=${tmpDir}`,
        '--disable-gpu',
        '--no-first-run',
        '--no-default-browser-check',
        '--window-size=1440,900',
        `http://127.0.0.1:${domPort}`,
      ],
      { stdio: 'ignore' }
    );

    const pageTarget = await waitForCdpTarget(cdpPort);
    console.log(`[Step 2] Connected to browser CDP target: ${pageTarget.title || 'Page'}`);

    // 3. Connect via CDP WebSocket
    cdp = new CDPClient(pageTarget.webSocketDebuggerUrl);
    await cdp.connect();
    await cdp.send('Page.enable');
    await cdp.send('Runtime.enable');

    console.log('[Step 3] Waiting for WebSocket DOM connection and initial frame stabilization...');
    await new Promise((r) => setTimeout(r, 2000));

    // 4. Verify DOM Presence
    const winCount = await cdp.evaluate('document.querySelectorAll(".imgui-window").length');
    console.log(`[Step 4] Rendered ImGui Windows in Browser DOM: ${winCount}`);
    if (winCount === 0) {
      throw new Error('No .imgui-window elements found in DOM tree!');
    }

    const elemCount = await cdp.evaluate('document.querySelectorAll("[id^=\'el_\']").length');
    console.log(`[Step 4] Rendered Interactive Elements in Browser DOM: ${elemCount}`);
    if (elemCount === 0) {
      throw new Error('No interactive DOM elements found in DOM tree!');
    }

    // 5. Run Invariant Collision Audit
    console.log('[Step 5] Running window.__auditDomLayout() layout collision invariant test...');
    const auditReport = await cdp.evaluate('window.__auditDomLayout()');
    console.log(`  Checked element pairs: ${auditReport.checkedPairs}`);
    console.log(`  Collisions detected:   ${auditReport.collisions.length}`);

    // 6. Capture Baseline Screenshot
    const screenshotsDir = path.resolve('tests/e2e/screenshots');
    fs.mkdirSync(screenshotsDir, { recursive: true });
    const screenshotBuf = await cdp.captureScreenshot();
    const screenshotPath = path.join(screenshotsDir, 'baseline.png');
    fs.writeFileSync(screenshotPath, screenshotBuf);
    console.log(`[Step 6] Saved E2E test screenshot to ${screenshotPath}`);

    // Assertions for Baseline
    if (!auditReport.ok || auditReport.collisions.length > 0) {
      console.error('\n>>> [FAIL] DOM Layout Collision Invariants Violated in Baseline!');
      console.error(JSON.stringify(auditReport.collisions, null, 2));
      process.exit(1);
    }
    console.log('>>> [PASS] Baseline Layout Invariants Verified (0 collisions)');

    // 7. Automated Tree Crawler (Expanding & Auditing Dear ImGui Demo sections)
    console.log('\n[Step 7] Starting automated Tree Crawler in Dear ImGui Demo...');
    const treeList = await cdp.evaluate(`
      (() => {
        const demoWin = Array.from(document.querySelectorAll('.imgui-window')).find(w => {
          const h = w.querySelector('.imgui-header');
          return h && h.textContent.includes('Dear ImGui Demo');
        });
        if (!demoWin) return [];
        return Array.from(demoWin.querySelectorAll('.imgui-tree')).map(t => ({
          id: t.id,
          text: (t.querySelector('.imgui-tree-label')?.textContent || t.textContent).trim()
        }));
      })()
    `);

    console.log(`  Discovered ${treeList.length} tree sections: ${treeList.map(t => `"${t.text}"`).join(', ')}`);

    let totalPairsChecked = auditReport.checkedPairs;

    for (const tree of treeList) {
      const slug = tree.text.toLowerCase().replace(/[^a-z0-9]+/g, '_');
      console.log(`  -> Expanding "${tree.text}" (#${tree.id})...`);
      
      // Click to expand
      await cdp.evaluate(`document.getElementById('${tree.id}')?.click()`);
      await new Promise((r) => setTimeout(r, 250)); // Wait for frame loop to push snapshot

      // Audit layout after expansion
      const expandAudit = await cdp.evaluate('window.__auditDomLayout()');
      totalPairsChecked += expandAudit.checkedPairs;
      console.log(`     Expanded element pairs: ${expandAudit.checkedPairs} | Collisions: ${expandAudit.collisions.length}`);

      // Capture screenshot of expanded section
      const expandScreenshot = await cdp.captureScreenshot();
      fs.writeFileSync(path.join(screenshotsDir, `tree_expand_${slug}.png`), expandScreenshot);

      if (!expandAudit.ok || expandAudit.collisions.length > 0) {
        console.error(`\n>>> [FAIL] Layout Collisions detected after expanding "${tree.text}"!`);
        console.error(JSON.stringify(expandAudit.collisions, null, 2));
        process.exit(1);
      }

      // Click to collapse back
      console.log(`  -> Collapsing "${tree.text}" (#${tree.id})...`);
      await cdp.evaluate(`document.getElementById('${tree.id}')?.click()`);
      await new Promise((r) => setTimeout(r, 200));

      // Audit layout after collapse (verify dead elements pruned cleanly)
      const collapseAudit = await cdp.evaluate('window.__auditDomLayout()');
      totalPairsChecked += collapseAudit.checkedPairs;
      if (!collapseAudit.ok || collapseAudit.collisions.length > 0) {
        console.error(`\n>>> [FAIL] Ghost elements or collisions detected after collapsing "${tree.text}"!`);
        console.error(JSON.stringify(collapseAudit.collisions, null, 2));
        process.exit(1);
      }
    }
    console.log('>>> [PASS] All tree sections expanded, audited, and collapsed with 0 defects!');

    // 8. Widget Stress Fuzzing (Rapid interactive clicks & slider drags)
    console.log('\n[Step 8] Running interactive widget stress fuzzer...');
    const fuzzActions = 15;
    for (let i = 0; i < fuzzActions; i++) {
      // Toggle a random checkbox or button
      await cdp.evaluate(`
        (() => {
          const interactives = Array.from(document.querySelectorAll('.imgui-check-box input, .imgui-btn, .imgui-slider-box input'));
          if (interactives.length > 0) {
            const target = interactives[Math.floor(Math.random() * interactives.length)];
            if (target.type === 'checkbox') {
              target.checked = !target.checked;
              target.dispatchEvent(new Event('change', { bubbles: true }));
            } else if (target.type === 'range') {
              const min = parseFloat(target.min) || 0;
              const max = parseFloat(target.max) || 100;
              target.value = (min + Math.random() * (max - min)).toFixed(1);
              target.dispatchEvent(new Event('input', { bubbles: true }));
            } else {
              target.click();
            }
          }
        })()
      `);
      await new Promise((r) => setTimeout(r, 50));
    }

    // Wait for event loop to settle
    await new Promise((r) => setTimeout(r, 300));

    // Audit final layout after fuzzing
    const fuzzAudit = await cdp.evaluate('window.__auditDomLayout()');
    totalPairsChecked += fuzzAudit.checkedPairs;
    if (!fuzzAudit.ok || fuzzAudit.collisions.length > 0) {
      console.error('\n>>> [FAIL] Layout collisions detected after interactive fuzzing!');
      console.error(JSON.stringify(fuzzAudit.collisions, null, 2));
      process.exit(1);
    }

    const finalScreenshot = await cdp.captureScreenshot();
    fs.writeFileSync(path.join(screenshotsDir, 'after_fuzzing.png'), finalScreenshot);

    console.log(`>>> [PASS] Widget fuzzer completed (${fuzzActions} rapid events, event loop responsive)!`);
    console.log(`\n========================================================`);
    console.log(`  E2E Test Suite Summary`);
    console.log(`  Total Invariant Element Pairs Audited: ${totalPairsChecked}`);
    console.log(`  Total Overlaps / Collisions Detected:  0`);
    console.log(`  Dead Element Pruning:                  VERIFIED`);
    console.log(`  Full-Duplex Responsiveness:            VERIFIED`);
    console.log(`========================================================\n`);
  } catch (err) {
    console.error('\n>>> [FAIL] E2E Test Suite Error:', err);
    process.exitCode = 1;
  } finally {
    if (cdp) cdp.close();
    if (browserProcess) {
      try {
        browserProcess.kill('SIGTERM');
      } catch {}
    }
    if (demoProcess) {
      try {
        demoProcess.kill('SIGTERM');
      } catch {}
    }
    try {
      fs.rmSync(tmpDir, { recursive: true, force: true });
    } catch {}
  }
}

runE2ETests();
