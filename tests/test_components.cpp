// Headless-проверка компонентов OmniKit: каждый рефакторенный компонент
// отрисовывается несколько кадров без окна и без бэкенда.
// В Debug ImGui сам ассертит на несбалансированный стек ID и пары Begin/End,
// поэтому тест особенно полезен в конфигурации Debug.
#include <iostream>
#include "imgui.h"
#include "imgui_internal.h"
#include "omnikit.hpp"
#ifdef _MSC_VER
#include <crtdbg.h>
#include <cstdlib>
#endif

static int g_failures = 0;

#define CHECK(cond)                                                                  \
    do {                                                                             \
        if (!(cond)) {                                                               \
            std::cerr << "[FAIL] " << #cond << " (" << __FILE__ << ":" << __LINE__   \
                      << ")\n";                                                      \
            ++g_failures;                                                            \
        }                                                                            \
    } while (0)

// Кадр: окно на весь экран + тело; проверяет баланс стека ID.
template <class Body>
static void Frame(Body&& body) {
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(1280, 800));
    ImGui::Begin("##test", nullptr, ImGuiWindowFlags_NoDecoration);
    const int idDepth = ImGui::GetCurrentWindow()->IDStack.Size;
    body();
    CHECK(ImGui::GetCurrentWindow()->IDStack.Size == idDepth);
    ImGui::End();
    ImGui::Render();
}

struct Row { int id; std::string name; };

int main() {
#ifdef _MSC_VER
    // Без модального диалога на assert: сообщение уходит в stderr, процесс падает с кодом != 0
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(1280, 800);
    io.DeltaTime = 1.0f / 60.0f;
    io.IniFilename = nullptr;

    UiTheme::Get().SetScale(1.0f);
    UiTheme::Get().LoadFonts();
    unsigned char* pixels = nullptr;
    int w = 0, h = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &w, &h);

    // Stateful-таблицы живут между кадрами
    Table<Row> table({.rowHeight = 28, .emptyTitle = "Nothing here"});
    table.AddColumn({.id = "id", .header = "ID", .width = 60,
                     .comparator = [](const Row& a, const Row& b) { return a.id - b.id; },
                     .renderCell = [](const Row& r, int) { ImGui::Text("%d", r.id); }});
    table.AddColumn({.id = "name", .header = "Name", .width = 1, .widthMode = ColumnWidthMode::Stretch,
                     .renderCell = [](const Row& r, int) { ImGui::TextUnformatted(r.name.c_str()); }});
    table.SetItems({{1, "alpha"}, {2, "beta"}, {3, "gamma"}});
    Table<Row> emptyTable;   // пустое состояние, ключ — по адресу
    emptyTable.AddColumn({.id = "id", .header = "ID"});

    for (int frame = 0; frame < 4; ++frame) {
        // Базовые виджеты
        Frame([&] {
            Button({.label = "Save", .variant = UiVariant::Primary});
            Button({.label = "Mini", .size = UiSize::Mini});
            Badge("OK");
            Tag("tag");
        });

        // List: Scope, заголовок, элементы, пустое состояние
        Frame([&] {
            bool clicked = false;
            if (List list({.key = "runs", .height = 240}); list) {
                clicked = list.Header("RUNS", {.actionIcon = "+", .badge = "3"});
                for (int i = 0; i < 5; ++i) {
                    IdScope id(i);
                    list.Item({.label = "Run", .sublabel = "10:14", .rightText = "1240 pts",
                               .selected = (i == 1)});
                }
                list.Separator();
                list.Empty();
            }
            CHECK(!clicked);   // мышь не двигалась
        });

        // ActionGroup: Scope без собственного ID, строки — через IdScope
        Frame([&] {
            for (int i = 0; i < 3; ++i) {
                IdScope row(i);
                if (ActionGroup actions; actions) {
                    actions.Button({.label = "Edit"});
                    actions.ToolButton({.icon = Icon::Cog, .tooltip = "Settings"});
                }
            }
        });

        // StatusBanner
        Frame([&] {
            StatusBanner("Loading", {.busy = true, .progress = 0.42f});
            StatusBanner("Low disk space", {.variant = UiVariant::Warning});
        });

        // ItemRow
        Frame([&] {
            for (int i = 0; i < 2; ++i) {
                IdScope id(i);
                if (ItemRow row({.key = "row", .actions = 2}); row) {
                    row.Title("Density", {.symbol = "rho", .unit = "kg/m3"});
                    row.Badge("Active", {.variant = UiVariant::Success});
                    row.Action({.icon = Icon::Cog, .tooltip = "Settings"});
                    row.Action({.icon = Icon::Target});
                    row.Description("Mass per unit volume");
                    row.Formula("rho", "m / V");
                    auto clicked = row.Tags({"a", "b"});
                    CHECK(!clicked);
                }
            }
        });

        // Table<T>: данные, пустое состояние; две таблицы не конфликтуют по ID
        Frame([&] {
            table.Render({.height = 200});
            emptyTable.Render({.height = 120});
        });

        // TableGrid
        Frame([&] {
            using Column = TableGrid::Column;
            if (TableGrid grid({Column::Stretch("Name"), Column::Fixed("Size", 80)}, {.height = 120}); grid) {
                for (int i = 0; i < 3; ++i) {
                    grid.Row({.key = "r"});
                    grid.Cell(); grid.CellText("file");
                    grid.Cell(); grid.CellText("12 KB");
                }
            }
        });

        // FilterBar с левой и правой зонами
        Frame([&] {
            if (FilterBar bar({.key = "filters"}); bar) {
                if (auto left = bar.Left()) {
                    FilterBar::Label("Search:");
                    Button({.label = "Go"});
                }
                if (auto right = bar.Right()) {
                    Button({.label = "Export"});
                }
            }
        });
    }

    ImGui::DestroyContext();
    if (g_failures) {
        std::cerr << g_failures << " check(s) failed\n";
        return 1;
    }
    std::cout << ">>> [PASS] components render without ImGui assertion failures\n";
    return 0;
}
