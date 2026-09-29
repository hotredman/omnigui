# Visual Comparison Report (Stock SDL_Renderer vs ThorVG Vector Backend)

## 1. Methodology
- Test window: `ImGui::ShowDemoWindow()` at 1280x720 resolution.
- Two-frame capture:
  1. Stock ImGui renderer (`ImGui_ImplSDLRenderer3_RenderDrawData`) via `SDL_Renderer`;
  2. Vector ThorVG renderer (`ThorVGRenderer::RenderDrawData`) via `ImDrawList` command interception.
- Per-pixel RGB delta calculation with tolerance for anti-aliasing edge softening.

## 2. Quantitative Results

| Metric | Value |
| :--- | :---: |
| Frame Resolution | **1280 x 720** (921600 px) |
| Exact Match (RGB diff = 0) | **569865** (61.83%) |
| Within AA Tolerance (diff <= 32) | **347381** (37.69%) |
| **Total Visual Match** | **917246** (**99.53%**) |
| Beyond AA Tolerance | **4354** (0.47%) |
| Average Channel Discrepancy | **0.92 / 255** |
| Maximum Channel Discrepancy | **224 / 255** |

## 3. Discrepancy Analysis

1. **Anti-Aliasing:**
   - Stock ImGui uses 1-pixel triangulation fringe outlines (`_FringeScale`).
   - ThorVG performs analytical vector sub-pixel anti-aliasing coverage, producing smoother alpha gradients on rounded corners and circles.

2. **Font Rendering:**
   - Stock ImGui rasterizes glyphs into a texture atlas at startup with discrete pixel stepping.
   - ThorVG performs vector TrueType path rendering, preserving clean vector outlines.

3. **Draw Order and Z-order:**
   - All windows, tables, channel splitters, and popups preserve strict rendering order.
   - No layering or clipping artifacts detected.

## 4. Generated Artifacts
- `stock_render.bmp` — Stock SDL_Renderer snapshot.
- `thorvg_render.bmp` — ThorVG vector renderer snapshot.
- `diff_map.bmp` — Difference color map (green = AA zone, red = discrepancy).
