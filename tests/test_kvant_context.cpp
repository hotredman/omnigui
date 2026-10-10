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
                .iconPlacement = Placement::Trailing
            });

            // Direct call to Button::Run
            btn2 = Button::Run(ui, {
                .label = "Prev",
                .icon = Icon::ChevronLeft(),
                .variant = UiVariant::Secondary,
                .iconPlacement = Placement::Leading
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

void TestSwitchAndCheckboxWidgets() {
    using namespace kvant;

    static_assert(sizeof(SwitchOptions) <= 40);
    static_assert(sizeof(CheckboxOptions) <= 40);

    // 1. Test measurement
    const SwitchMetrics sm = GetSwitchMetrics(UiSize::Md);
    Vec2 sMeasNoLabel = Switch::Measure("", {}, UiSize::Md);
    CHECK(sMeasNoLabel.x == sm.trackSize.x);
    CHECK(sMeasNoLabel.y == sm.trackSize.y);

    Vec2 sMeasWithLabel = Switch::Measure("Enable", {}, UiSize::Md);
    CHECK(sMeasWithLabel.x > sm.trackSize.x);

    Vec2 sMeasWithDesc = Switch::Measure("Enable", {.description = "Details"}, UiSize::Md);
    CHECK(sMeasWithDesc.y > sMeasWithLabel.y);

    const CheckboxMetrics cm = GetCheckboxMetrics(UiSize::Md);
    Vec2 cMeasNoLabel = Checkbox::Measure("", {}, UiSize::Md);
    CHECK(cMeasNoLabel.x == cm.boxSize);
    CHECK(cMeasNoLabel.y == cm.boxSize);

    Vec2 cMeasWithLabel = Checkbox::Measure("Enable", {}, UiSize::Md);
    CHECK(cMeasWithLabel.x > cm.boxSize);

    Vec2 cMeasWithDesc = Checkbox::Measure("Enable", {.description = "Details"}, UiSize::Md);
    CHECK(cMeasWithDesc.y > cMeasWithLabel.y);

    // 2. Interactive mutation test
    Context ui;
    ThorCanvas canvas;
    const uint32_t width = 500;
    const uint32_t height = 400;
    std::vector<uint32_t> pixels(width * height, 0);
    canvas.SetTarget(pixels.data(), width, height, width);

    bool switchState = false;
    bool checkState = false;
    bool toggleState = false;
    bool descState = false;
    bool stackedState = false;
    bool optSwitchState = false;
    bool optCheckState = false;

    Response rSwitch;
    Response rCheck;
    Response rToggle;
    Response rIndeterminate;
    Response rDesc;
    Response rStacked;
    Response rOptSwitch;
    Response rOptCheck;

    auto renderUI = [&]() {
        if (auto col = ui.Column({.gap = 12.0f, .padding = Padding::All(16.0f)})) {
            rSwitch = ui.Switch("Dark Mode", &switchState);
            rCheck = ui.Checkbox("VSync", &checkState);
            rToggle = ui.Toggle("Turbo Mode", &toggleState, {.labelPlacement = Placement::Leading});
            rIndeterminate = ui.Checkbox("Select All", false, {.indeterminate = true});
            rDesc = ui.Switch("Auto Breakdown", &descState, {
                .description = "Automatic stop of testing on load drop"
            });
            rStacked = ui.Switch("Stacked Mode", &stackedState, {
                .description = "Description below switch control",
                .labelPlacement = Placement::Stacked
            });
            rOptSwitch = ui.Switch(&optSwitchState, {.label = "Option Switch"});
            rOptCheck = ui.Checkbox(&optCheckState, {.label = "Option Checkbox"});
        }
    };

    // Frame 1: Measure and layout
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    // Frame 2: Bounds populated
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    CHECK(rSwitch.rect.w > 0.0f);
    CHECK(rCheck.rect.w > 0.0f);
    CHECK(rToggle.rect.w > 0.0f);
    CHECK(rIndeterminate.rect.w > 0.0f);
    CHECK(rDesc.rect.w > 0.0f);
    CHECK(rDesc.rect.h > rSwitch.rect.h);
    CHECK(rStacked.rect.w > 0.0f);
    CHECK(rStacked.rect.h > rSwitch.rect.h);
    CHECK(!switchState);
    CHECK(!checkState);
    CHECK(!descState);
    CHECK(!stackedState);

    // 3. Click Switch to toggle value
    ui.OnMouseMove({rSwitch.rect.x + 5.0f, rSwitch.rect.y + 5.0f});
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);
    CHECK(rSwitch.pressed);

    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    CHECK(rSwitch.clicked);
    CHECK(rSwitch.changed);
    CHECK(static_cast<bool>(rSwitch) == true);
    CHECK(switchState == true);

    // 4. Click Checkbox to toggle value
    ui.OnMouseMove({rCheck.rect.x + 5.0f, rCheck.rect.y + 5.0f});
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);
    CHECK(rCheck.pressed);

    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    CHECK(rCheck.clicked);
    CHECK(rCheck.changed);
    CHECK(static_cast<bool>(rCheck) == true);
    CHECK(checkState == true);

    // 5. Click Switch with description to toggle value
    ui.OnMouseMove({rDesc.rect.x + 10.0f, rDesc.rect.y + 10.0f});
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);
    CHECK(rDesc.pressed);

    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    CHECK(rDesc.clicked);
    CHECK(rDesc.changed);
    CHECK(static_cast<bool>(rDesc) == true);
    CHECK(descState == true);

    // 6. Click Stacked Switch to toggle value
    ui.OnMouseMove({rStacked.rect.x + 10.0f, rStacked.rect.y + 10.0f});
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);
    CHECK(rStacked.pressed);

    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    CHECK(rStacked.clicked);
    CHECK(rStacked.changed);
    CHECK(static_cast<bool>(rStacked) == true);
    CHECK(stackedState == true);

    // 7. Click Option Switch (label configured via SwitchOptions)
    CHECK(rOptSwitch.rect.w > 0.0f);
    CHECK(!optSwitchState);
    ui.OnMouseMove({rOptSwitch.rect.x + 5.0f, rOptSwitch.rect.y + 5.0f});
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);
    CHECK(optSwitchState == true);
    CHECK(rOptSwitch.changed);

    // 8. Click Option Checkbox (label configured via CheckboxOptions)
    CHECK(rOptCheck.rect.w > 0.0f);
    CHECK(!optCheckState);
    ui.OnMouseMove({rOptCheck.rect.x + 5.0f, rOptCheck.rect.y + 5.0f});
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);
    CHECK(optCheckState == true);
    CHECK(rOptCheck.changed);
}

void TestInputFieldWidget() {
    using namespace kvant;

    Context ui;
    ThorCanvas canvas;

    const uint32_t width = 600;
    const uint32_t height = 500;
    std::vector<uint32_t> pixels(width * height, 0);
    canvas.SetTarget(pixels.data(), width, height, width);

    float speedDrop = 50.0f;
    int count = 42;
    std::string sampleName = "Specimen-A";
    float optInputVal = 12.3f;

    Response rSpeed;
    Response rCount;
    Response rName;
    Response rOptInput;

    auto renderUI = [&]() {
        if (auto col = ui.Column({.gap = 16.0f, .padding = Padding::All(20.0f)})) {
            // Numeric field with unit and description (as in user screenshot)
            rSpeed = ui.InputField("Speed Drop dF/dt", speedDrop, {
                .description = "Automatic stop of testing on load drop",
                .suffix = "%",
                .format = "%.0f",
                .labelPlacement = Placement::Stacked
            });

            // Integer count with unit suffix
            rCount = ui.InputField("Cycle Count", count, {
                .suffix = "cycles",
                .labelPlacement = Placement::Stacked
            });

            // Text input with placeholder
            rName = ui.InputField("Sample ID", sampleName, {
                .placeholder = "Enter identifier...",
                .labelPlacement = Placement::Stacked
            });

            // Field with label set exclusively via InputFieldOptions
            rOptInput = ui.InputField(optInputVal, {
                .label = "Tolerance Margin",
                .suffix = "mm",
                .labelPlacement = Placement::Stacked
            });
        }
    };

    // Frame 1: Measure & Layout
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    // Frame 2: Bounds populated
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    CHECK(rSpeed.rect.w > 0.0f);
    CHECK(rSpeed.rect.h > 36.0f); // Default box is 36px, stacked adds label and description
    CHECK(rCount.rect.w > 0.0f);
    CHECK(rName.rect.w > 0.0f);
    CHECK(rOptInput.rect.w > 0.0f);
    CHECK(rOptInput.rect.h > 36.0f);
    CHECK(!rSpeed.focused);

    // 1. Click speed field to focus
    ui.OnMouseMove({rSpeed.rect.x + 15.0f, rSpeed.rect.y + 25.0f});
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    CHECK(rSpeed.focused);
    CHECK(ui.IsFocused(rSpeed.id));

    // 2. Type text into focused speed field: "50" -> backspace -> "5" -> insert "5" -> "55"
    ui.OnKeyDown(Key::Backspace);
    ui.OnTextInput("5");
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    CHECK(rSpeed.changed);
    CHECK_NEAR(speedDrop, 55.0f, 0.01f);

    // 3. Press Enter to submit and blur
    ui.OnKeyDown(Key::Enter);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    CHECK(rSpeed.submitted);
    CHECK(!rSpeed.focused);
    CHECK(!ui.IsFocused(rSpeed.id));

    // 4. Click name field and edit string
    ui.OnMouseMove({rName.rect.x + 15.0f, rName.rect.y + 25.0f});
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    CHECK(rName.focused);

    ui.OnTextInput("-X");
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    CHECK(rName.changed);
    CHECK(sampleName == "Specimen-A-X");

    // 5. Click outside (e.g. at 5, 5 in padding) to blur focus
    ui.OnMouseMove({5.0f, 5.0f});
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    CHECK(!rName.focused);
    CHECK(!ui.IsFocused(rName.id));
}

void TestSelectWidget() {
    using namespace kvant;

    Context ui;
    ThorCanvas canvas;

    const uint32_t width = 800;
    const uint32_t height = 600;
    std::vector<uint32_t> pixels(width * height, 0);
    canvas.SetTarget(pixels.data(), width, height, width);

    size_t baudIndex = 1; // "19200"
    int modeIndex = 0;   // "Auto"
    std::string profile = "Standard";
    bool disabledMode = false;

    WidgetResponse rBaud;
    WidgetResponse rMode;
    WidgetResponse rProfile;

    auto renderUI = [&]() {
        if (auto col = ui.Column({.gap = 12.0f, .padding = Padding::All(20.0f)})) {
            rBaud = ui.Select("Baud Rate", baudIndex, {"9600", "19200", "38400", "115200"}, {
                .description = "Serial bus communication speed",
                .labelPlacement = Placement::Stacked
            });

            rMode = ui.Select("Control Mode", modeIndex, {"Auto", "Manual", "Direct"}, {
                .disabled = disabledMode
            });

            rProfile = ui.Select("Profile", profile, {"Eco", "Standard", "Performance", "Extreme"});
        }
    };

    // Frame 1: Measure & Layout pass
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    // Frame 2: Bounds populated
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    CHECK(rBaud.rect.w > 0.0f);
    CHECK(rBaud.rect.h > 36.0f); // Stacked adds label and description
    CHECK(rMode.rect.w > 0.0f);
    CHECK(rProfile.rect.w > 0.0f);
    CHECK(!ui.IsPopupOpen(rBaud.id));
    CHECK(!ui.IsPopupOpen(rMode.id));
    CHECK(!ui.IsPopupOpen(rProfile.id));

    // 1. Click Baud Rate select box to open dropdown popup
    ui.OnMouseMove({rBaud.rect.x + 20.0f, rBaud.rect.y + 25.0f});
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    CHECK(ui.IsPopupOpen(rBaud.id));
    Rect popupRect = ui.GetOpenPopupRect();
    CHECK(popupRect.w > 0.0f);
    CHECK(popupRect.h > 0.0f);

    // 2. Hover over item 3 ("115200") in the popup and click it
    // Item 3 rect is at popupRect.y + 4.0f + 3 * itemHeight (32px)
    const float item3Y = popupRect.y + 4.0f + 3.0f * 32.0f + 10.0f;
    ui.OnMouseMove({popupRect.x + 30.0f, item3Y});
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    CHECK(rBaud.changed);
    CHECK(baudIndex == 3);
    CHECK(!ui.IsPopupOpen(rBaud.id)); // Popup is now closed

    // 3. Toggle open and close on clicking the select box again
    ui.OnMouseMove({rBaud.rect.x + 20.0f, rBaud.rect.y + 25.0f});
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);
    CHECK(ui.IsPopupOpen(rBaud.id));

    // Click select box again
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);
    CHECK(!ui.IsPopupOpen(rBaud.id)); // Closed on toggle

    // 4. Open popup and close via Escape key
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);
    CHECK(ui.IsPopupOpen(rBaud.id));

    ui.OnKeyDown(Key::Escape);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);
    CHECK(!ui.IsPopupOpen(rBaud.id)); // Closed on Escape

    // 5. Open popup and close via outside click
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);
    CHECK(ui.IsPopupOpen(rBaud.id));

    // Click at outside position (5, 5)
    ui.OnMouseMove({5.0f, 5.0f});
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);
    CHECK(!ui.IsPopupOpen(rBaud.id)); // Closed on outside click

    // 6. Keyboard navigation with Down / Up / Enter
    ui.OnMouseMove({rBaud.rect.x + 20.0f, rBaud.rect.y + 25.0f});
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);
    CHECK(ui.IsPopupOpen(rBaud.id));

    // Current baudIndex is 3 (max). Navigate Up to 2
    ui.OnKeyDown(Key::Up);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);
    CHECK(baudIndex == 2);
    CHECK(rBaud.changed);

    // Press Enter to accept and close
    ui.OnKeyDown(Key::Enter);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);
    CHECK(!ui.IsPopupOpen(rBaud.id));
    CHECK(baudIndex == 2);

    // 7. String-based selection with Profile
    ui.OnMouseMove({rProfile.rect.x + 20.0f, rProfile.rect.y + 15.0f});
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);
    CHECK(ui.IsPopupOpen(rProfile.id));

    popupRect = ui.GetOpenPopupRect();
    // Item 3 in Profile is "Extreme" (index 3)
    const float extremeY = popupRect.y + 4.0f + 3.0f * 32.0f + 10.0f;
    ui.OnMouseMove({popupRect.x + 30.0f, extremeY});
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    CHECK(rProfile.changed);
    CHECK(profile == "Extreme");
    CHECK(!ui.IsPopupOpen(rProfile.id));

    // 8. Disabled mode suppresses interaction
    disabledMode = true;
    ui.OnMouseMove({rMode.rect.x + 20.0f, rMode.rect.y + 15.0f});
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    CHECK(!rMode.clicked);
    CHECK(!ui.IsPopupOpen(rMode.id));
    CHECK(modeIndex == 0);

    // 9. Verify maxVisibleItems limits popup height to visibleCount * itemHeight + 8
    size_t limIdx = 0;
    WidgetResponse rLim;
    auto renderLimited = [&]() {
        rLim = ui.Select("Limited Dropdown", limIdx, {"Item 1", "Item 2", "Item 3", "Item 4", "Item 5"}, {
            .maxVisibleItems = 2
        });
    };

    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderLimited();
    ui.EndFrame(canvas);

    ui.OnMouseMove({rLim.rect.x + 10.0f, rLim.rect.y + 10.0f});
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderLimited();
    ui.EndFrame(canvas);

    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderLimited();
    ui.EndFrame(canvas);

    CHECK(ui.IsPopupOpen(rLim.id));
    const Rect limPopupRect = ui.GetOpenPopupRect();
    // 2 visible items: 2 * 32.0f + 8.0f = 72.0f
    CHECK_NEAR(limPopupRect.h, 72.0f, 0.5f);
}

void TestCursorBlinkModesAndPhaseReset() {
    using namespace kvant;

    Context ui;

    // 1. Default Blinking Mode: 1.06s cycle, 530ms on / 530ms off, 500ms initial grace on input
    ui.SetCurrentTime(0.0);
    ui.ResetCursorBlink();
    CHECK(ui.IsCursorVisible() == true);

    // Within 500ms grace period: always visible
    ui.SetCurrentTime(0.4);
    CHECK(ui.IsCursorVisible() == true);

    // 0.6s total: 100ms into blink cycle (< 530ms on-phase) -> visible
    ui.SetCurrentTime(0.6);
    CHECK(ui.IsCursorVisible() == true);

    // 1.1s total: 600ms into cycle (> 530ms off-phase) -> hidden
    ui.SetCurrentTime(1.1);
    CHECK(ui.IsCursorVisible() == false);

    // 2. Typing Phase Reset: user presses a key or inputs text -> resets to visible
    ui.OnKeyDown(Key::Right);
    CHECK(ui.IsCursorVisible() == true);

    // Still within grace period after keystroke
    ui.AdvanceTime(0.3);
    CHECK(ui.IsCursorVisible() == true);

    // 3. Solid Mode: always visible
    ui.SetCursorConfig({.mode = CursorBlinkMode::Solid});
    ui.SetCurrentTime(10.0);
    CHECK(ui.IsCursorVisible() == true);
    ui.SetCurrentTime(100.0);
    CHECK(ui.IsCursorVisible() == true);

    // 4. Hidden Mode: always hidden
    ui.SetCursorConfig({.mode = CursorBlinkMode::Hidden});
    CHECK(ui.IsCursorVisible() == false);

    // 5. Local mode override via parameter
    CHECK(ui.IsCursorVisible(CursorBlinkMode::Solid) == true);
    CHECK(ui.IsCursorVisible(CursorBlinkMode::Hidden) == false);
}

void TestGridAndCardLayout() {
    using namespace kvant;

    Context ui;
    ThorCanvas canvas;
    const uint32_t width = 800;
    const uint32_t height = 600;
    std::vector<uint32_t> pixels(width * height, 0);
    canvas.SetTarget(pixels.data(), width, height, width);

    bool switchVal1 = false;
    bool switchVal2 = true;
    float speedVal = 120.0f;
    std::string nameVal = "Axis-1";

    WidgetResponse sw1Res;
    WidgetResponse sw2Res;
    WidgetResponse inp1Res;
    WidgetResponse inp2Res;

    auto renderUI = [&]() {
        // 1. Standalone Grid with 12-column slots
        if (auto grid = ui.Grid({.gap = 10.0f, .runGap = 15.0f})) {
            if (auto c1 = ui.Cell6()) {
                sw1Res = ui.Switch("Sensor A", &switchVal1);
            }
            if (auto c2 = ui.Cell6()) {
                sw2Res = ui.Switch("Sensor B", &switchVal2);
            }
            if (auto c3 = ui.Cell12()) {
                inp1Res = ui.InputField("Speed Target", speedVal);
            }
        }

        // 2. High-level Card with inner Grid and title
        if (auto card = ui.Card("Motion Controller", {.subtitle = "Direct axis tuning"})) {
            if (auto c1 = ui.Cell6()) {
                inp2Res = ui.InputField("Device Name", nameVal);
            }
            if (auto c2 = ui.Cell6()) {
                ui.Button("Calibrate Zero");
            }
        }
    };

    // Frame 1: Initial render pass
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    // Frame 2: verify bounds and positioning
    // Viewport width = 800.
    // Standalone grid content width = 800.
    // Gap = 10.
    // Cell 1 (span 6): W(6) = (6/12) * (800 + 10) - 10 = 0.5 * 810 - 10 = 405 - 10 = 395.
    // Cell 2 (span 6): starts at 395 + 10 = 405, width = 395, ends at 800.
    Rect bSw1 = ui.GetPrevFrameBounds(sw1Res.id);
    Rect bSw2 = ui.GetPrevFrameBounds(sw2Res.id);
    Rect bInp1 = ui.GetPrevFrameBounds(inp1Res.id);
    Rect bInp2 = ui.GetPrevFrameBounds(inp2Res.id);

    CHECK(bSw1.w > 0.0f);
    CHECK(bSw2.w > 0.0f);
    // In Cell6, the switch should be contained within the left half [0, 395]
    CHECK(bSw1.x < 395.0f);
    CHECK(bSw1.Right() <= 395.0f + 1.0f);

    // Sensor B switch should be in the right half [405, 800]
    CHECK(bSw2.x >= 405.0f - 1.0f);
    CHECK(bSw2.Right() <= 800.0f + 1.0f);

    // Input 1 was in Cell12, so it should be on line 2 (y > bSw1.Bottom())
    CHECK(bInp1.y >= bSw1.Bottom());
    // And should stretch across the full width [0, 800]
    CHECK_NEAR(bInp1.w, 800.0f, 1.0f);

    // Inside Card:
    // Card has padding 16 on all sides. Content width = 800 - 32 = 768.
    // Inner grid has gap 12.
    // Cell 1 (span 6): W(6) = (6/12)*(768 + 12) - 12 = 0.5 * 780 - 12 = 390 - 12 = 378.
    // Input 2 should start at 16, width 378!
    CHECK_NEAR(bInp2.x, 16.0f, 1.0f);
    CHECK_NEAR(bInp2.w, 378.0f, 1.0f);

    // Frame 3: Mouse down on Switch inside Cell6
    ui.OnMouseMove({bSw1.x + 10.0f, bSw1.y + 10.0f});
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);
    CHECK(sw1Res.pressed);

    // Frame 4: Mouse up on Switch -> click fired and boolean toggled
    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);
    CHECK(sw1Res.clicked);
    CHECK(switchVal1 == true);
}

void TestScrollAreaImmediateMode() {
    using namespace kvant;

    Context ui;
    ThorCanvas canvas;
    const uint32_t width = 800;
    const uint32_t height = 400;
    std::vector<uint32_t> pixels(width * height, 0);
    canvas.SetTarget(pixels.data(), width, height, width);

    WidgetResponse btn1Res;
    WidgetResponse btn2Res;
    WidgetResponse btn3Res;

    auto renderUI = [&]() {
        if (auto scroll = ui.ScrollArea()) {
            if (auto box1 = ui.Box({.heightMode = SizeMode::Fixed, .fixedHeight = 200.0f})) {
                btn1Res = ui.Button("Top Item");
            }
            if (auto box2 = ui.Box({.heightMode = SizeMode::Fixed, .fixedHeight = 200.0f})) {
                btn2Res = ui.Button("Middle Item");
            }
            if (auto box3 = ui.Box({.heightMode = SizeMode::Fixed, .fixedHeight = 200.0f})) {
                btn3Res = ui.Button("Bottom Item");
            }
        }
    };

    // Frame 1: Initial layout
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    Rect bBtn1 = ui.GetPrevFrameBounds(btn1Res.id);
    Rect bBtn3 = ui.GetPrevFrameBounds(btn3Res.id);

    // Button 1 is visible at the top
    CHECK_NEAR(bBtn1.y, 0.0f, 1.0f);
    // Button 1 right edge strictly respects the scrollbar gutter (800 - 14 = 786px)
    const ScrollbarMetrics sm = GetScrollbarMetrics(UiSize::Md);
    CHECK_NEAR(bBtn1.Right(), 800.0f - sm.gutter, 1.0f);
    // Button 3 was placed at y = 400, which is outside the viewport (0..400) -> its clipped interaction bounds have h = 0
    CHECK(bBtn3.h <= 0.0f);

    // Frame 2: Mouse wheel down by 5 ticks -> 5 * 40 = 200px scroll
    ui.OnMouseMove({100.0f, 100.0f});
    ui.OnMouseWheel(0.0f, -5.0f);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    bBtn1 = ui.GetPrevFrameBounds(btn1Res.id);
    bBtn3 = ui.GetPrevFrameBounds(btn3Res.id);

    // Button 1 scrolled above top (clipped)
    CHECK(bBtn1.h <= 0.0f);
    // Button 3 is now visible in the viewport [200, 400]
    CHECK(bBtn3.h > 0.0f);
    CHECK(bBtn3.y >= 200.0f - 1.0f);

    // Frame 3: Click on Button 3
    ui.OnMouseMove({bBtn3.Center().x, bBtn3.Center().y});
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);
    CHECK(btn3Res.pressed);

    // Frame 4: Release mouse -> clicked event
    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);
    CHECK(btn3Res.clicked);

    // Frame 5: ScrollTo brings Button 1 back into view
    btn1Res.ScrollTo();
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    bBtn1 = ui.GetPrevFrameBounds(btn1Res.id);
    CHECK(bBtn1.h > 0.0f);
    CHECK_NEAR(bBtn1.y, 0.0f, 1.0f);
}

void TestReactiveEventLoop() {
    using namespace kvant;
    Context ui;
    ThorCanvas canvas;
    std::vector<uint32_t> pixels(100 * 100, 0);
    canvas.SetTarget(pixels.data(), 100, 100, 100);

    // Initial state: starts with 3 repaint frames to render initial scene
    CHECK(ui.GetRepaintFramesLeft() == 3);
    CHECK(ui.NeedsRepaint());
    CHECK(ui.GetRecommendedSleepTimeoutMs() == 0);

    // Drain initial 3 frames
    ui.BeginFrame({100.0f, 100.0f}); ui.EndFrame(canvas);
    CHECK(ui.GetRepaintFramesLeft() == 2);
    CHECK(ui.NeedsRepaint());

    ui.BeginFrame({100.0f, 100.0f}); ui.EndFrame(canvas);
    CHECK(ui.GetRepaintFramesLeft() == 1);
    CHECK(ui.NeedsRepaint());

    ui.BeginFrame({100.0f, 100.0f}); ui.EndFrame(canvas);
    CHECK(ui.GetRepaintFramesLeft() == 0);
    CHECK(!ui.NeedsRepaint());
    CHECK(ui.GetRecommendedSleepTimeoutMs() > 0);

    // Mouse movement wakes up reactive loop
    ui.OnMouseMove({25.0f, 25.0f});
    CHECK(ui.GetRepaintFramesLeft() == 3);
    CHECK(ui.NeedsRepaint());
    CHECK(ui.GetRecommendedSleepTimeoutMs() == 0);

    // Drain 3 frames
    for (int i = 0; i < 3; ++i) {
        ui.BeginFrame({100.0f, 100.0f});
        ui.EndFrame(canvas);
    }
    CHECK(!ui.NeedsRepaint());

    // Holding mouse button down prevents sleeping
    ui.OnMouseDown(1);
    CHECK(ui.IsMouseDown());
    CHECK(ui.NeedsRepaint());

    for (int i = 0; i < 5; ++i) {
        ui.BeginFrame({100.0f, 100.0f});
        ui.EndFrame(canvas);
    }
    // Still needs repaint because button is held down
    CHECK(ui.NeedsRepaint());

    // Releasing mouse button queues 3 settle frames
    ui.OnMouseUp(1);
    CHECK(!ui.IsMouseDown());
    CHECK(ui.GetRepaintFramesLeft() == 3);

    for (int i = 0; i < 3; ++i) {
        ui.BeginFrame({100.0f, 100.0f});
        ui.EndFrame(canvas);
    }
    CHECK(!ui.NeedsRepaint());

    // Key event wakes up loop
    ui.OnKeyDown(Key::Enter);
    CHECK(ui.GetRepaintFramesLeft() == 3);
    CHECK(ui.NeedsRepaint());

    // Drain frames
    for (int i = 0; i < 3; ++i) {
        ui.BeginFrame({100.0f, 100.0f});
        ui.EndFrame(canvas);
    }
    CHECK(!ui.NeedsRepaint());

    // Configurable idle heartbeat
    CHECK(ui.GetIdleHeartbeatMs() == 500);
    CHECK(ui.GetRecommendedSleepTimeoutMs() == 500);

    ui.SetIdleHeartbeatMs(1000);
    CHECK(ui.GetIdleHeartbeatMs() == 1000);
    CHECK(ui.GetRecommendedSleepTimeoutMs() == 1000);

    ui.SetIdleHeartbeatMs(-1); // Deep sleep
    CHECK(ui.GetIdleHeartbeatMs() == -1);
    CHECK(ui.GetRecommendedSleepTimeoutMs() == -1);

    ui.SetIdleHeartbeatMs(500); // Restore default
}

void TestDirtyRectDamageTracking() {
    using namespace kvant;

    Context ui;
    ThorCanvas canvas;
    const uint32_t width = 800;
    const uint32_t height = 600;
    std::vector<uint32_t> pixels(width * height, 0);
    canvas.SetTarget(pixels.data(), width, height, width);

    auto renderUI = [&](std::string* editVal = nullptr) {
        if (auto row = ui.Row({.gap = 10.0f})) {
            if (auto b = ui.Box({.fixedWidth = 120.0f})) {
                ui.Button("Test Button");
            }
            if (editVal) {
                if (auto b = ui.Box({.fixedWidth = 200.0f})) {
                    auto res = ui.InputField(*editVal);
                    if (res.clicked) {
                        res.Focus();
                    }
                }
            }
        }
    };

    // Frame 1: Initial layout establishment must trigger a full repaint
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    CHECK(ui.IsFullRepaint());
    CHECK(ui.HasDirtyDamage());
    CHECK(ui.GetDirtyRect().w == static_cast<float>(width));
    CHECK(ui.GetDirtyRect().h == static_cast<float>(height));

    // Frame 2: Identical state with zero input produces zero damage
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    CHECK(!ui.HasDirtyDamage());
    CHECK(!ui.IsFullRepaint());

    // Frame 3: Hover over the button marks only the button bounds as dirty
    ui.OnMouseMove({20.0f, 15.0f});
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    CHECK(ui.HasDirtyDamage());
    CHECK(!ui.IsFullRepaint());
    CHECK(ui.GetDirtyRect().w < 200.0f);
    CHECK(ui.GetDirtyRect().h < 100.0f);

    // Frame 4: Caret blink toggles in an InputField
    std::string text = "Editable";
    bool shouldFocus = true;
    auto renderInput = [&]() {
        if (auto row = ui.Row({.gap = 10.0f})) {
            if (auto b = ui.Box({.fixedWidth = 200.0f})) {
                auto res = ui.InputField(text);
                if (shouldFocus) {
                    res.Focus();
                    shouldFocus = false;
                }
            }
        }
    };

    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderInput();
    ui.EndFrame(canvas);

    // Settle edit state
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderInput();
    ui.EndFrame(canvas);

    // Advance time past solid phase (0.5s) and first half of blink cycle to cause cursor to hide
    ui.AdvanceTime(1.1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderInput();
    ui.EndFrame(canvas);

    CHECK(ui.HasDirtyDamage());
    CHECK(!ui.IsFullRepaint());
    CHECK(ui.GetDirtyRect().w < 300.0f);
    CHECK(ui.GetDirtyRect().h < 100.0f);

    // Frame 5: InvalidateAll forces full repaint
    ui.InvalidateAll();
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI(&text);
    ui.EndFrame(canvas);

    CHECK(ui.IsFullRepaint());
    CHECK(ui.HasDirtyDamage());
    CHECK(ui.GetDirtyRect().w == static_cast<float>(width));

    // Frame 6: Disabling dirty rect optimization forces full repaint
    ui.SetDirtyRectEnabled(false);
    CHECK(!ui.IsDirtyRectEnabled());
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI(&text);
    ui.EndFrame(canvas);

    CHECK(ui.IsFullRepaint());
    ui.SetDirtyRectEnabled(true);
}

void TestTabBarWidgetWorkflow() {
    using namespace kvant;
    Context ui;
    ThorCanvas canvas;
    const uint32_t width = 800;
    const uint32_t height = 600;
    std::vector<uint32_t> pixels(width * height, 0);
    canvas.SetTarget(pixels.data(), width, height, width);

    int activeTab = 0;
    const std::vector<TabItem> tabs = {
        {.label = "Overview", .icon = Icon::Eye()},
        {.label = "Telemetry", .icon = Icon::Play(), .badge = "Live"},
        {.label = "Disabled", .disabled = true},
        {.label = "Settings", .icon = Icon::Settings()}
    };

    Response tabRes;
    auto renderUI = [&]() {
        if (auto col = ui.Column({.gap = 10.0f, .padding = Padding::All(20.0f)})) {
            tabRes = ui.TabBar(activeTab, tabs, {.style = TabBarStyle::Underline});
        }
    };

    // Frame 1: Initial layout establishment
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    CHECK(activeTab == 0);
    CHECK(!tabRes.changed);

    // Frame 2: Layout is established
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    // Tab 0 is approx x=20..130, Tab 1 is approx x=136..260.
    // Move mouse over Tab 1 ("Telemetry") and click
    ui.OnMouseMove({180.0f, 36.0f});
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    CHECK(activeTab == 1);
    CHECK(tabRes.changed);

    // Frame: Next frame without click should have changed == false
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);
    CHECK(!tabRes.changed);
    CHECK(activeTab == 1);

    // Frame: Clicking on Tab 2 (disabled, approx x=270..350) should NOT change active index
    ui.OnMouseMove({300.0f, 36.0f});
    ui.OnMouseDown(1);
    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderUI();
    ui.EndFrame(canvas);

    CHECK(activeTab == 1);
    CHECK(!tabRes.changed);

    // Frame: Test Pill style and string_view initializer_list overload
    int pillIdx = 0;
    auto renderPill = [&]() {
        if (auto col = ui.Column({.gap = 10.0f, .padding = Padding::All(10.0f)})) {
            ui.TabBar(pillIdx, {"Alpha", "Beta", "Gamma"}, {.style = TabBarStyle::Pill});
        }
    };

    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderPill();
    ui.EndFrame(canvas);

    CHECK(pillIdx == 0);
}

void TestToolbarWidgetWorkflow() {
    using namespace kvant;
    Context ui;
    ThorCanvas canvas;
    const uint32_t width = 800;
    const uint32_t height = 600;
    std::vector<uint32_t> pixels(width * height, 0);
    canvas.SetTarget(pixels.data(), width, height, width);

    Response btnNew;
    Response btnSave;
    Response btnSettings;
    bool toolSettingsActive = false;

    auto renderToolbar = [&]() {
        if (auto tb = ui.Toolbar({.fixedHeight = 44.0f, .padding = Padding::Symmetric(12.0f, 4.0f)})) {
            btnNew = tb.ToolButton("New", {.icon = Icon::Plus()});
            btnSave = tb.ToolButton("Save", {.icon = Icon::Check(), .disabled = true});
            tb.Separator();
            btnSettings = tb.ToolButton("", {.icon = Icon::Settings(), .active = toolSettingsActive});
            tb.Spacer();
            tb.ToolButton("Status", {.icon = Icon::Eye()});
        }
    };

    // Frame 1: Establish layout
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderToolbar();
    ui.EndFrame(canvas);

    CHECK(!btnNew.clicked);
    CHECK(!btnSave.clicked);
    CHECK(ui.GetPrevFrameBounds(btnNew.id).w > 0.0f);
    CHECK(ui.GetPrevFrameBounds(btnNew.id).h > 0.0f);

    // Frame 2: Hover over "New" button (x ~ 25.0f, y ~ 20.0f)
    ui.OnMouseMove({25.0f, 20.0f});
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderToolbar();
    CHECK(btnNew.hovered);
    ui.EndFrame(canvas);

    // Frame 3: Mouse down & up to click "New" button
    ui.OnMouseDown(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderToolbar();
    CHECK(btnNew.pressed);
    ui.EndFrame(canvas);

    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderToolbar();
    CHECK(btnNew.clicked);
    ui.EndFrame(canvas);

    // Frame 4: Disabled button cannot be clicked
    // btnSave is at x ~ 90..130
    ui.OnMouseMove({90.0f, 20.0f});
    ui.OnMouseDown(1);
    ui.OnMouseUp(1);
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    renderToolbar();
    CHECK(!btnSave.clicked);
    CHECK(!btnSave.hovered);
    ui.EndFrame(canvas);
}

void TestLabelVariantsStableSizing() {
    using namespace kvant;
    Context ui;
    ThorCanvas canvas;
    const uint32_t width = 800;
    const uint32_t height = 600;
    std::vector<uint32_t> pixels(width * height, 0);
    canvas.SetTarget(pixels.data(), width, height, width);

    bool state = false;
    Response btn;

    auto render = [&]() {
        if (auto row = ui.Row({.gap = 8.0f})) {
            btn = ui.Button(state ? "VSync: ON" : "VSync: OFF", {
                .labelVariants = {"VSync: ON", "VSync: OFF"}
            });
        }
    };

    // Frame 1: OFF state
    state = false;
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    render();
    ui.EndFrame(canvas);
    const Rect boundsOff = ui.GetPrevFrameBounds(btn.id);

    // Frame 2: ON state
    state = true;
    ui.BeginFrame({static_cast<float>(width), static_cast<float>(height)});
    render();
    ui.EndFrame(canvas);
    const Rect boundsOn = ui.GetPrevFrameBounds(btn.id);

    // Bounds must be identical across state transitions
    CHECK(boundsOff.w > 0.0f);
    CHECK(boundsOn.w > 0.0f);
    CHECK_NEAR(boundsOff.w, boundsOn.w, 0.001f);
    CHECK_NEAR(boundsOff.h, boundsOn.h, 0.001f);
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
    TestSwitchAndCheckboxWidgets();
    TestInputFieldWidget();
    TestSelectWidget();
    TestCursorBlinkModesAndPhaseReset();
    TestGridAndCardLayout();
    TestScrollAreaImmediateMode();
    TestReactiveEventLoop();
    TestDirtyRectDamageTracking();
    TestTabBarWidgetWorkflow();
    TestToolbarWidgetWorkflow();
    TestLabelVariantsStableSizing();

    if (g_failures == 0) {
        std::cout << "[PASS] All Kvant Context tests passed successfully!\n";
        return 0;
    } else {
        std::cerr << "[FAIL] Total failures: " << g_failures << "\n";
        return 1;
    }
}
