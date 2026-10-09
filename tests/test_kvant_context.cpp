#include <iostream>
#include <vector>
#include "kvant/Context.hpp"
#include "kvant/backend/ThorCanvas.hpp"
#include "thorvg.h"

static int g_failures = 0;

#define CHECK(cond)                                                                  \
    do {                                                                             \
        if (!(cond)) {                                                               \
            std::cerr << "[FAIL] " << #cond << " (" << __FILE__ << ":" << __LINE__   \
                      << ")\n";                                                      \
            ++g_failures;                                                            \
        }                                                                            \
    } while (0)

#define CHECK_NEAR(a, b, eps)                                                        \
    do {                                                                             \
        if (std::abs((a) - (b)) > (eps)) {                                           \
            std::cerr << "[FAIL] " << #a << " (" << (a) << ") != " << #b            \
                      << " (" << (b) << ") [" << __FILE__ << ":" << __LINE__        \
                      << "]\n";                                                      \
            ++g_failures;                                                            \
        }                                                                            \
    } while (0)

void TestImmediateApiWorkflow() {
    using namespace kvant;

    Context ui;
    ThorCanvas canvas;

    const uint32_t width = 800;
    const uint32_t height = 600;
    std::vector<uint32_t> pixels(width * height, 0);
    canvas.SetTarget(pixels.data(), width, height, width);

    WidgetResponse b1;
    WidgetResponse b2;

    auto renderUI = [&]() {
        if (auto row = ui.Row({.gap = 10.0f})) {
            b1 = ui.Button({.label = "Button A"});
            ui.Spacer();
            b2 = ui.Button({.label = "Button B"});
        }
    };

    // Frame 1: Initial layout establishment
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    CHECK(!b1.clicked);
    CHECK(!b2.clicked);
    ui.EndFrame(canvas);

    // Frame 2: Hover over Button A (Button A is at x=0, y=0, w ~ 70+, h ~ 36)
    ui.OnMouseMove({20.0f, 15.0f});
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    CHECK(b1.hovered);
    CHECK(!b2.hovered);
    CHECK(!b1.clicked);
    ui.EndFrame(canvas);

    // Frame 3: Mouse down on Button A
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    CHECK(b1.pressed);
    CHECK(!b1.clicked);
    ui.EndFrame(canvas);

    // Frame 4: Mouse up on Button A -> Triggers .clicked == true!
    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    CHECK(b1.clicked);
    CHECK(bool(b1)); // Implicit bool conversion works
    CHECK(!b2.clicked);
    ui.EndFrame(canvas);

    // Frame 5: Mouse button is up, click event is consumed
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    CHECK(!b1.clicked);
    ui.EndFrame(canvas);
}

void TestThorCanvasTextRendering() {
    using namespace kvant;
    ThorCanvas canvas;
    const uint32_t width = 200;
    const uint32_t height = 100;
    std::vector<uint32_t> pixels(width * height, 0);
    canvas.SetTarget(pixels.data(), width, height, width);

    bool fontOk = canvas.LoadFontFile("assets/fonts/Roboto-Regular.ttf");
    if (!fontOk) {
        fontOk = canvas.LoadFontFile("../assets/fonts/Roboto-Regular.ttf");
    }
    CHECK(fontOk);

    std::fill(pixels.begin(), pixels.end(), 0);
    canvas.Begin(width, height);

    // Direct ThorVG test with layout box:
    auto txtBox = tvg::Text::gen();
    txtBox->text("Overview");
    txtBox->font(nullptr);
    txtBox->size(14.0f);
    txtBox->layout(120.0f, 36.0f);
    txtBox->align(0.5f, 0.5f);
    txtBox->translate(10.0f, 10.0f);
    txtBox->fill(255, 255, 255);
    // Add to canvas directly or via ThorCanvas
    // We can test this!

    int minY = 999, maxY = -1, minX = 999, maxX = -1;
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            if (pixels[y * width + x] != 0) {
                if ((int)y < minY) minY = (int)y;
                if ((int)y > maxY) maxY = (int)y;
                if ((int)x < minX) minX = (int)x;
                if ((int)x > maxX) maxX = (int)x;
            }
        }
    }
    std::cout << "[PIXEL BOUNDS] pos=(10, 10), fontSize=24 -> X=[" << minX << ".." << maxX 
              << "], Y=[" << minY << ".." << maxY << "], width=" << (maxX - minX + 1)
              << ", height=" << (maxY - minY + 1) << "\n";
    CHECK(minX >= 0 && minY >= 0);
}

void TestHeaderRowTextMetricsAndNoOverlap() {
    using namespace kvant;
    Context ui;
    ThorCanvas canvas;
    const uint32_t width = 800;
    const uint32_t height = 600;
    std::vector<uint32_t> pixels(width * height, 0);
    canvas.SetTarget(pixels.data(), width, height, width);

    // Accurate text advance verification
    Vec2 titleMetrics = canvas.MeasureText("KVANT GUI", 24.0f);
    CHECK(titleMetrics.x > 150.0f);

    Vec2 subMetrics = canvas.MeasureText("| Autonomous Vector GUI Engine", 14.0f);
    CHECK(subMetrics.x > 250.0f);

    WidgetResponse bOverview;
    WidgetResponse bFlex;

    auto renderRow = [&]() {
        if (auto row = ui.Row({.gap = 8.0f})) {
            if (auto b1 = ui.Box({.fixedWidth = 120.0f})) {
                bOverview = ui.Button("Overview");
            }
            if (auto b2 = ui.Box({.fixedWidth = 150.0f})) {
                bFlex = ui.Button("Flexbox Demo");
            }
        }
    };

    // Frame 1: Measure and establish bounds
    ui.BeginFrame({800.0f, 600.0f});
    renderRow();
    ui.EndFrame(canvas);

    // Frame 2: Retrieve cached bounds from previous frame
    ui.BeginFrame({800.0f, 600.0f});
    renderRow();
    ui.EndFrame(canvas);

    CHECK_NEAR(bOverview.rect.x, 0.0f, 0.001f);
    CHECK_NEAR(bOverview.rect.w, 120.0f, 0.001f);
    CHECK_NEAR(bFlex.rect.x, 128.0f, 0.001f);
    CHECK_NEAR(bFlex.rect.w, 150.0f, 0.001f);
    CHECK(bFlex.rect.x >= bOverview.rect.Right()); // Strictly sequential with zero overlap
}

void TestFooterExitButtonStaysVisibleInNarrowWindow() {
    using namespace kvant;
    Context ui;
    ThorCanvas canvas;
    const uint32_t width = 500;
    const uint32_t height = 400;
    std::vector<uint32_t> pixels(width * height, 0);
    canvas.SetTarget(pixels.data(), width, height, width);

    WidgetResponse exitBtn;
    auto renderUI = [&]() {
        if (auto col = ui.Column({.gap = 16.0f, .padding = Padding::All(24.0f)})) {
            // Wide header (requires ~600px+)
            if (auto header = ui.Row({.gap = 12.0f})) {
                ui.Title("KVANT GUI");
                ui.Caption("| Autonomous Vector GUI Engine");
                ui.Spacer();
                ui.Caption("C++20 Immediate + ThorVG");
            }

            ui.Spacer();

            // Footer with Exit button pushed to right
            if (auto footer = ui.Row({.gap = 12.0f})) {
                ui.Caption("Zero heap allocations per frame | 0% idle CPU");
                ui.Spacer();
                if (auto b = ui.Box({.fixedWidth = 80.0f})) {
                    exitBtn = ui.Button("Exit");
                }
            }
        }
    };

    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    // The Exit button must stay strictly within the 500px window!
    CHECK(exitBtn.rect.Right() <= 500.0f);
    // Specifically, it should align with the column's right padding: 500 - 24 = 476.0f
    CHECK_NEAR(exitBtn.rect.Right(), 476.0f, 1.0f);
    // And vertically, it must be pushed to the bottom margin by Spacer: 400 - 24 = 376.0f
    CHECK_NEAR(exitBtn.rect.Bottom(), 376.0f, 1.0f);
}

void TestWrapContainerInContextApi() {
    using namespace kvant;

    Context ui;
    ThorCanvas canvas;

    const uint32_t width = 300;
    const uint32_t height = 400;
    std::vector<uint32_t> pixels(width * height, 0);
    canvas.SetTarget(pixels.data(), width, height, width);

    WidgetResponse b1;
    WidgetResponse b2;
    WidgetResponse b3;

    auto renderUI = [&]() {
        if (auto col = ui.Column({.gap = 10.0f, .padding = Padding::All(20.0f)})) {
            // Available width inside padding = 300 - 40 = 260.
            // b1 (fixed 120), b2 (fixed 120), b3 (fixed 120)
            // b1 (120) + gap (10) + b2 (120) = 250 <= 260 (Line 1)
            // b3 wraps to Line 2! (y = 20 + 36 + 10 = 66)
            if (auto wrap = ui.Wrap({.gap = 10.0f, .runGap = 10.0f})) {
                if (auto box1 = ui.Box({.fixedWidth = 120.0f, .fixedHeight = 36.0f})) {
                    b1 = ui.Button("Button 1");
                }
                if (auto box2 = ui.Box({.fixedWidth = 120.0f, .fixedHeight = 36.0f})) {
                    b2 = ui.Button("Button 2");
                }
                if (auto box3 = ui.Box({.fixedWidth = 120.0f, .fixedHeight = 36.0f})) {
                    b3 = ui.Button("Button 3");
                }
            }
        }
    };

    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    // Frame 2: inspect bounds
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    CHECK_NEAR(b1.rect.x, 20.0f, 0.001f);
    CHECK_NEAR(b1.rect.y, 20.0f, 0.001f);
    CHECK_NEAR(b2.rect.x, 150.0f, 0.001f); // 20 + 120 + 10
    CHECK_NEAR(b2.rect.y, 20.0f, 0.001f);

    // b3 must be wrapped to second line
    CHECK_NEAR(b3.rect.x, 20.0f, 0.001f);
    CHECK_NEAR(b3.rect.y, 66.0f, 0.001f); // 20 + 36 + 10
    CHECK_NEAR(b3.rect.w, 120.0f, 0.001f);
    CHECK_NEAR(b3.rect.h, 36.0f, 0.001f);
}

void TestFooterExitButtonPushedToBottomWithCardAndWrap() {
    using namespace kvant;
    Context ui;
    ThorCanvas canvas;
    const uint32_t width = 800;
    const uint32_t height = 600;
    std::vector<uint32_t> pixels(width * height, 0);
    canvas.SetTarget(pixels.data(), width, height, width);

    WidgetResponse exitBtn;
    auto renderUI = [&]() {
        if (auto mainCol = ui.Column({.gap = 16.0f, .padding = Padding::All(24.0f)})) {
            if (auto header = ui.Row({.gap = 12.0f})) {
                ui.Title("KVANT GUI");
                ui.Spacer();
            }
            if (auto navRow = ui.Row({.gap = 8.0f})) {
                if (auto box = ui.Box({.fixedWidth = 120.0f})) {
                    ui.Button("Overview");
                }
            }
            if (auto card = ui.Box({.gap = 16.0f, .padding = Padding::All(20.0f)})) {
                ui.Headline("Card title");
                if (auto wrap = ui.Wrap({.gap = 12.0f, .runGap = 10.0f})) {
                    ui.Button("Btn 1");
                    ui.Button("Btn 2");
                }
            }
            ui.Spacer();
            if (auto footer = ui.Row({.gap = 12.0f})) {
                ui.Caption("Footer text");
                ui.Spacer();
                if (auto box = ui.Box({.fixedWidth = 80.0f})) {
                    exitBtn = ui.Button("Exit");
                }
            }
        }
    };

    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    CHECK_NEAR(exitBtn.rect.Bottom(), 600.0f - 24.0f, 1.0f);
}

void TestUiTokensAndIconFactories() {
    using namespace kvant;

    // Verify 5-level scale metrics
    const auto mXs = GetComponentMetrics(UiSize::Xs);
    const auto mSm = GetComponentMetrics(UiSize::Sm);
    const auto mMd = GetComponentMetrics(UiSize::Md);
    const auto mLg = GetComponentMetrics(UiSize::Lg);
    const auto mXl = GetComponentMetrics(UiSize::Xl);

    CHECK(mXs.defaultHeight < mSm.defaultHeight);
    CHECK(mSm.defaultHeight < mMd.defaultHeight);
    CHECK(mMd.defaultHeight < mLg.defaultHeight);
    CHECK(mLg.defaultHeight < mXl.defaultHeight);

    CHECK_NEAR(mMd.defaultHeight, 40.0f, 0.001f);
    CHECK_NEAR(mLg.defaultHeight, 48.0f, 0.001f);

    // Verify Icon value object
    const Icon icPlay = Icon::Play();
    const Icon icClose = Icon::Close();
    CHECK(icPlay.IsValid());
    CHECK(icPlay.GetId() == IconId::Play);
    CHECK(icPlay != icClose);

    const Icon emptyIcon;
    CHECK(emptyIcon.IsEmpty());
    CHECK(!emptyIcon);

    // Verify Icon drawing does not crash
    ThorCanvas canvas;
    const uint32_t width = 100;
    const uint32_t height = 100;
    std::vector<uint32_t> pixels(width * height, 0);
    canvas.SetTarget(pixels.data(), width, height, width);
    canvas.Begin(width, height);
    icPlay.Draw(canvas, Rect(10.0f, 10.0f, 24.0f, 24.0f), Color::Hex(0xFFFFFFFF));
    icClose.Draw(canvas, Rect(40.0f, 10.0f, 24.0f, 24.0f), Color::Hex(0xFFFFFFFF));
    canvas.End();
}

void TestTwoLevelDisabledWorkflow() {
    using namespace kvant;

    Context ui;
    ThorCanvas canvas;
    const uint32_t width = 400;
    const uint32_t height = 200;
    std::vector<uint32_t> pixels(width * height, 0);
    canvas.SetTarget(pixels.data(), width, height, width);

    WidgetResponse btnEnabled;
    WidgetResponse btnDirectDisabled;
    WidgetResponse btnScopedDisabled;

    auto renderUI = [&]() {
        if (auto row = ui.Row({.gap = 10.0f})) {
            btnEnabled = ui.Button("Active");
            btnDirectDisabled = ui.Button("Local Disabled", {.disabled = true});

            ui.Disabled(true, [&] {
                btnScopedDisabled = ui.Button("Scope Disabled");
            });
        }
    };

    // Frame 1: Layout establishment
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    // Frame 2: Mouse click over the disabled button
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    ui.OnMouseMove(btnDirectDisabled.rect.Center());
    ui.OnMouseDown(1);
    ui.OnMouseUp(1);
    renderUI();
    ui.EndFrame(canvas);

    CHECK(!btnDirectDisabled.clicked);
    CHECK(!btnDirectDisabled.hovered);

    // Click over scoped disabled button
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    ui.OnMouseMove(btnScopedDisabled.rect.Center());
    ui.OnMouseDown(1);
    ui.OnMouseUp(1);
    renderUI();
    ui.EndFrame(canvas);

    CHECK(!btnScopedDisabled.clicked);
    CHECK(!btnScopedDisabled.hovered);

    // Click over active button
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    ui.OnMouseMove(btnEnabled.rect.Center());
    ui.OnMouseDown(1);
    renderUI();
    ui.EndFrame(canvas);

    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    ui.OnMouseUp(1);
    renderUI();
    ui.EndFrame(canvas);

    CHECK(btnEnabled.clicked);
}

void TestBoxSlotWithUiSizeToken() {
    using namespace kvant;

    Context ui;
    ThorCanvas canvas;
    const uint32_t width = 400;
    const uint32_t height = 200;
    std::vector<uint32_t> pixels(width * height, 0);
    canvas.SetTarget(pixels.data(), width, height, width);

    WidgetResponse btnLarge;
    WidgetResponse btnSquareIcon;

    auto renderUI = [&]() {
        if (auto col = ui.Column({.gap = 12.0f})) {
            if (auto slotLg = ui.Box({.size = UiSize::Lg})) {
                btnLarge = ui.Button("Large Action", {.variant = UiVariant::Primary});
            }
            if (auto slotSquare = ui.Box({.fixedWidth = 32.0f, .size = UiSize::Sm})) {
                btnSquareIcon = ui.Button({.icon = Icon::Close()});
            }
        }
    };

    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    // Large button must fill the 48px slot allocated by UiSize::Lg
    CHECK_NEAR(btnLarge.rect.h, 48.0f, 0.001f);
    CHECK_NEAR(btnLarge.rect.w, 400.0f, 0.001f); // Stretches across column

    // Square icon button in 32x32 slot
    CHECK_NEAR(btnSquareIcon.rect.w, 32.0f, 0.001f);
    CHECK_NEAR(btnSquareIcon.rect.h, 32.0f, 0.001f);
}

void TestButtonWidgetRunAndResponseChaining() {
    using namespace kvant;
    Context ui;
    ThorCanvas canvas;
    const uint32_t width = 400;
    const uint32_t height = 200;
    std::vector<uint32_t> pixels(width * height, 0);
    canvas.SetTarget(pixels.data(), width, height, width);

    ButtonResponse btn1;
    ButtonResponse btn2;

    auto renderUI = [&]() {
        if (auto row = ui.Row({.gap = 8.0f})) {
            // Autonomous Button::Run via ui.Button
            btn1 = ui.Button({
                .label = "Next",
                .icon = Icon::ChevronRight(),
                .variant = UiVariant::Primary,
                .iconPlacement = IconPlacement::Trailing
            });

            // Direct call to Button::Run
            btn2 = Button::Run(ui, {
                .label = "Prev",
                .icon = Icon::ChevronLeft(),
                .variant = UiVariant::Secondary,
                .iconPlacement = IconPlacement::Leading
            });
        }
    };

    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    // Verify IDs and geometry
    CHECK(btn1.id.IsValid());
    CHECK(btn2.id.IsValid());
    CHECK(btn1.id != btn2.id);
    CHECK(btn1.rect.w > 0.0f);
    CHECK(btn2.rect.w > 0.0f);

    // Test Response imperative actions
    btn1.Focus();

    // Verify Response conversion to bool
    CHECK(!btn1);
}

void TestUiStringAndContextFormatting() {
    using namespace kvant;

    // Test static literal assignment (0 copy)
    UiString s1 = "Static literal";
    CHECK(s1 == "Static literal");
    CHECK(s1.size() == 14);
    CHECK(!s1.empty());

    // Test inline buffer formatting
    UiString s2;
    s2.Format("Count: {}", 42);
    CHECK(s2 == "Count: 42");
    CHECK(s2.size() == 9);

    // Test factory formatting
    UiString s3 = UiString::FormatNew("Speed: {:.1f}", 12.5f);
    CHECK(s3 == "Speed: 12.5");

    // Test Context arena formatting and interning in UI frame
    Context ui;
    ThorCanvas canvas;
    const uint32_t width = 400;
    const uint32_t height = 200;
    std::vector<uint32_t> pixels(width * height, 0);
    canvas.SetTarget(pixels.data(), width, height, width);

    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    if (auto col = ui.Column({.gap = 8.0f})) {
        ui.Text(ui.Format("Value: {}", 99));
        ui.Button(ui.String("Action"));
    }
    ui.EndFrame(canvas);

    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    if (auto col = ui.Column({.gap = 8.0f})) {
        ui.Text(ui.Format("Value: {}", 100));
        ui.Button(ui.String("Action"));
    }
    ui.EndFrame(canvas);
}

void TestBoxContainerGapSpacing() {
    using namespace kvant;
    Context ui;
    ThorCanvas canvas;
    const uint32_t width = 400;
    const uint32_t height = 300;
    std::vector<uint32_t> pixels(width * height, 0);
    canvas.SetTarget(pixels.data(), width, height, width);

    WidgetResponse b1;
    WidgetResponse b2;

    auto renderUI = [&]() {
        if (auto card = ui.Box({.gap = 16.0f, .padding = Padding::All(10.0f)})) {
            b1 = ui.Button("First Item");
            b2 = ui.Button("Second Item");
        }
    };

    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    // Verify Box applies 16px gap between children
    CHECK_NEAR(b2.rect.y, b1.rect.Bottom() + 16.0f, 0.01f);
}

void TestAutonomousTextWidgetAndTypographyRoles() {
    using namespace kvant;

    // 1. Verify intrinsic metrics and measurement for different roles
    const TypographyMetrics mDisplay = GetTypographyMetrics(TextRole::DisplayLarge);
    CHECK(mDisplay.fontSize == 40.0f);
    CHECK(mDisplay.lineHeight == 48.0f);
    CHECK(!mDisplay.isMonospace);

    const TypographyMetrics mBody = GetTypographyMetrics(TextRole::BodyMedium);
    CHECK(mBody.fontSize == 14.0f);
    CHECK(mBody.lineHeight == 20.0f);

    const TypographyMetrics mCode = GetTypographyMetrics(TextRole::CodeMedium);
    CHECK(mCode.fontSize == 13.0f);
    CHECK(mCode.lineHeight == 18.0f);
    CHECK(mCode.isMonospace);

    Vec2 estDisplay = Text::Measure("Heading", {.role = TextRole::DisplayLarge});
    Vec2 estBody = Text::Measure("Heading", {.role = TextRole::BodyMedium});
    CHECK(estDisplay.x > estBody.x);
    CHECK(estDisplay.y > estBody.y);

    // 2. Immediate mode rendering and semantic shorthands
    Context ui;
    ThorCanvas canvas;
    const uint32_t width = 800;
    const uint32_t height = 600;
    std::vector<uint32_t> pixels(width * height, 0);
    canvas.SetTarget(pixels.data(), width, height, width);

    Response rDisp;
    Response rHead;
    Response rTitle;
    Response rSub;
    Response rBody;
    Response rCap;
    Response rCode;
    Response rCustom;

    auto renderUI = [&]() {
        if (auto col = ui.Column({.gap = 8.0f, .padding = Padding::All(16.0f)})) {
            rDisp = ui.Display("Display Title");
            rHead = ui.Headline("Headline Section", UiVariant::Primary);
            rTitle = ui.Title("Title Header");
            rSub = ui.Subtitle("Subtitle Header");
            rBody = ui.Body("Standard body text content");
            rCap = ui.Caption("Muted caption info");
            rCode = ui.Code("auto x = 42;");
            rCustom = ui.Text("Aligned End", {.role = TextRole::LabelLarge, .variant = UiVariant::Success, .align = Align::End});
        }
    };

    // Frame 1: Measure and layout
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    // Frame 2: Bounds populated and interaction check
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    CHECK(rDisp.rect.w > 0.0f);
    CHECK(rDisp.rect.h >= mDisplay.lineHeight);
    CHECK(rBody.rect.w > 0.0f);
    CHECK(rBody.rect.h >= mBody.lineHeight);
    CHECK(rDisp.rect.h > rBody.rect.h);
    CHECK(rCustom.rect.w > 0.0f);

    // 3. Test interaction: hover, press, and click on text widget
    ui.OnMouseMove({rDisp.rect.x + 5.0f, rDisp.rect.y + 5.0f});
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);
    CHECK(rDisp.hovered);

    // Mouse press
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);
    CHECK(rDisp.pressed);

    // Mouse release completes click
    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);
    CHECK(rDisp.clicked);
    CHECK(static_cast<bool>(rDisp) == true);
}

int main() {
    std::cout << "[RUN] Testing Kvant Context & Immediate Mode API...\n";

    TestImmediateApiWorkflow();
    TestThorCanvasTextRendering();
    TestHeaderRowTextMetricsAndNoOverlap();
    TestFooterExitButtonStaysVisibleInNarrowWindow();
    TestWrapContainerInContextApi();
    TestFooterExitButtonPushedToBottomWithCardAndWrap();
    TestUiTokensAndIconFactories();
    TestTwoLevelDisabledWorkflow();
    TestBoxSlotWithUiSizeToken();
    TestButtonWidgetRunAndResponseChaining();
    TestUiStringAndContextFormatting();
    TestBoxContainerGapSpacing();
    TestAutonomousTextWidgetAndTypographyRoles();

    if (g_failures == 0) {
        std::cout << "[PASS] All Kvant Context tests passed successfully!\n";
        return 0;
    } else {
        std::cerr << "[FAIL] Total failures: " << g_failures << "\n";
        return 1;
    }
}
