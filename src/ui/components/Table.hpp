#pragma once

#include "ui/components/UiTheme.hpp"
#include "ui/components/Icon.hpp"
#include "ui/components/Scope.hpp"
#include "ui/components/Text.hpp"
#include "ui/components/UiKey.hpp"
#include <imgui.h>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <cstdint>

// Режим расчета ширины колонки
enum class ColumnWidthMode {
    Fixed,   // Фиксированная ширина в пикселях
    Stretch, // Пропорциональное растяжение
    Auto     // По содержимому ячейки
};

// Выравнивание содержимого колонки
enum class ColumnAlign {
    Left,
    Center,
    Right
};

// Декларативное описание столбца таблицы
template <typename T>
struct TableColumn {
    std::string id;                                     // Уникальный идентификатор столбца
    std::string header;                                 // Заголовок столбца
    float width = 0.0f;                                 // Базовые px, масштабируются (или вес растяжения)
    ColumnWidthMode widthMode = ColumnWidthMode::Fixed; // Режим ширины
    ColumnAlign align = ColumnAlign::Left;              // Выравнивание
    bool sortable = true;                               // Разрешена ли сортировка по клику
    bool resizable = true;                              // Разрешено ли менять ширину
    bool hideable = true;                               // Разрешено ли скрывать столбец
    bool visible = true;                                // Видимость столбца (для адаптивного скрытия)

    // Функция сравнения элементов для сортировки (возвращает <0 если a < b, 0 если a == b, >0 если a > b)
    std::function<int(const T& a, const T& b)> comparator;

    // Рендерер содержимого ячейки
    std::function<void(const T& item, int rowIndex)> renderCell;
};

// Постоянные параметры таблицы Table<T> (designated initializers)
struct TableOptions {
    UiKey key = {};                     // идентичность; пустой — по адресу объекта таблицы
    float rowHeight = 0.0f;             // базовые px; 0 — theme.table.rowHeight
    bool saveSettings = false;          // сохранять порядок/ширины колонок в ini ImGui
    const char* emptyTitle = nullptr;   // заголовок пустого состояния; nullptr — по умолчанию
    const char* emptySubtitle = nullptr;
};

// Размер области таблицы в кадре; базовые px, 0 — всё свободное место контейнера
struct TableRenderOptions {
    float width = 0.0f;
    float height = 0.0f;
};

// Высокопроизводительная виртуализированная таблица (Table<T>)
// Способна отображать десятки и сотни тысяч строк с 60+ FPS благодаря ImGuiListClipper
// и индексной буферизации (фильтрация и сортировка оперируют массивом индексов без копирования моделей)
//
// Stateful-объект: хранит данные, колонки, поиск и сортировку между кадрами; создаётся один раз
// (член класса приложения), в кадре вызывается Render. Идентичность — по адресу объекта или key.
//
//     Table<Row> table({.rowHeight = 32});
//     table.AddColumn({.id = "name", .header = "Name", .width = 200, .renderCell = [](const Row& r, int) { ... }});
//     table.SetItems(rows);
//     table.Render({.height = 300});
template <typename T>
class Table {
public:
    explicit Table(const TableOptions& options = {})
        : m_options(options) {}

    // Добавление столбцов
    void AddColumn(TableColumn<T> col) {
        m_columns.push_back(std::move(col));
    }

    void ClearColumns() {
        m_columns.clear();
    }

    const std::vector<TableColumn<T>>& GetColumns() const { return m_columns; }

    // Привязка данных
    void SetItems(std::vector<T> items) {
        m_items = std::move(items);
        RebuildIndices();
    }

    const std::vector<T>& GetItems() const { return m_items; }
    size_t GetTotalCount() const { return m_items.size(); }
    size_t GetFilteredCount() const { return m_filteredIndices.size(); }

    // Настройка предикатов фильтрации и поиска
    using FilterPredicate = std::function<bool(const T& item, const std::string& query)>;
    void SetFilterPredicate(FilterPredicate predicate) {
        m_filterPredicate = std::move(predicate);
        RebuildIndices();
    }

    void SetSearchQuery(const std::string& query) {
        if (m_searchQuery != query) {
            m_searchQuery = query;
            RebuildIndices();
        }
    }

    const std::string& GetSearchQuery() const { return m_searchQuery; }

    // Принудительная сортировка программно
    void SortByColumn(int columnIndex, bool ascending) {
        m_sortColumnIndex = columnIndex;
        m_sortAscending = ascending;
        ApplySort();
    }

    void SetColumnVisible(size_t index, bool visible) {
        if (index < m_columns.size()) {
            m_columns[index].visible = visible;
        }
    }

    void SetColumnVisible(const std::string& id, bool visible) {
        for (auto& col : m_columns) {
            if (col.id == id) {
                col.visible = visible;
                break;
            }
        }
    }

    bool IsColumnVisible(size_t index) const {
        return index < m_columns.size() ? m_columns[index].visible : false;
    }

    // Основной цикл отрисовки
    void Render(const TableRenderOptions& options = {}) {
        UiTheme& theme = UiTheme::Get();
        const TableStyle& ts = theme.table;
        float scale = theme.GetScale();
        float rowHeight = (m_options.rowHeight > 0.0f) ? theme.Scale(m_options.rowHeight) : theme.Scale(ts.rowHeight);

        ImVec2 size(theme.Scale(options.width), theme.Scale(options.height));
        if (size.x <= 0.0f) size.x = ImGui::GetContentRegionAvail().x;
        if (size.y <= 0.0f) size.y = ImGui::GetContentRegionAvail().y;

        if (m_options.key) m_options.key.Push(); else ImGui::PushID(this);

        // Пустое состояние
        if (m_filteredIndices.empty()) {
            RenderEmptyState(size);
            ImGui::PopID();
            return;
        }

        ImGuiTableFlags flags = ImGuiTableFlags_ScrollY |
                               ImGuiTableFlags_RowBg |
                               ImGuiTableFlags_BordersOuter |
                               ImGuiTableFlags_BordersInnerH |
                               ImGuiTableFlags_BordersInnerV |
                               ImGuiTableFlags_Resizable |
                               ImGuiTableFlags_Reorderable |
                               ImGuiTableFlags_Hideable |
                               ImGuiTableFlags_Sortable;

        if (!m_options.saveSettings) {
            flags |= ImGuiTableFlags_NoSavedSettings;
        }

        // Применяем стили из TableStyle темы оформления
        ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, ts.colHeaderBg);
        ImGui::PushStyleColor(ImGuiCol_TableRowBg, ts.colRowBg);
        ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, ts.colRowBgAlt);
        ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, ts.colBorderOuter);
        ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ts.colBorderInner);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ts.colHeaderHovered);
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ts.colHeaderActive);

        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(ts.cellPaddingX * scale, ts.cellPaddingY * scale));

        // Подложка тела — окну прокрутки, которое создаёт BeginTable
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ts.colBodyBg);
        const bool tableOpen =
            ImGui::BeginTable("##Table", static_cast<int>(m_columns.size()), flags, size);
        ImGui::PopStyleColor();
        if (tableOpen) {
            // Настройка столбцов
            for (size_t i = 0; i < m_columns.size(); ++i) {
                const auto& col = m_columns[i];
                ImGuiTableColumnFlags colFlags = ImGuiTableColumnFlags_None;

                if (col.widthMode == ColumnWidthMode::Fixed) {
                    colFlags |= ImGuiTableColumnFlags_WidthFixed;
                } else if (col.widthMode == ColumnWidthMode::Stretch) {
                    colFlags |= ImGuiTableColumnFlags_WidthStretch;
                }

                if (!col.sortable) colFlags |= ImGuiTableColumnFlags_NoSort;
                if (!col.resizable) colFlags |= ImGuiTableColumnFlags_NoResize;
                if (!col.hideable) colFlags |= ImGuiTableColumnFlags_NoHide;

                float widthOrWeight = (col.widthMode == ColumnWidthMode::Auto) ? 0.0f : (col.width * scale);

                if (!col.visible) {
                    colFlags |= ImGuiTableColumnFlags_Disabled;
                    widthOrWeight = 0.0f;
                }

                ImGui::TableSetupColumn(col.header.c_str(), colFlags, widthOrWeight, static_cast<ImGuiID>(i));
            }

            ImGui::TableSetupScrollFreeze(0, 1); // Закрепляем шапку при вертикальном скролле
            theme.PushFont(theme.fontMedium, ts.headerFontSize);
            ImGui::TableHeadersRow();
            theme.PopFont();

            // Проверка сортировки по клику на заголовки таблицы ImGui
            ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs();
            if (sortSpecs && sortSpecs->SpecsDirty) {
                if (sortSpecs->SpecsCount > 0) {
                    const auto& spec = sortSpecs->Specs[0];
                    m_sortColumnIndex = spec.ColumnIndex;
                    m_sortAscending = (spec.SortDirection == ImGuiSortDirection_Ascending);
                    ApplySort();
                }
                sortSpecs->SpecsDirty = false;
            }

            // Виртуализация через ImGuiListClipper (рендер только видимых строк!)
            ImGuiListClipper clipper;
            clipper.Begin(static_cast<int>(m_filteredIndices.size()), rowHeight);

            while (clipper.Step()) {
                for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
                    int itemIdx = m_filteredIndices[row];
                    const T& item = m_items[itemIdx];

                    ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);
                    ImGui::PushID(itemIdx);

                    for (size_t c = 0; c < m_columns.size(); ++c) {
                        if (!m_columns[c].visible) continue;
                        if (ImGui::TableSetColumnIndex(static_cast<int>(c))) {
                            if (m_columns[c].renderCell) {
                                m_columns[c].renderCell(item, itemIdx);
                            }
                        }
                    }

                    ImGui::PopID();
                }
            }

            ImGui::EndTable();
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(7);
        ImGui::PopID();
    }

private:
    void RebuildIndices() {
        m_filteredIndices.clear();
        m_filteredIndices.reserve(m_items.size());

        std::string queryLower = m_searchQuery;
        std::transform(queryLower.begin(), queryLower.end(), queryLower.begin(), ::tolower);

        for (int i = 0; i < static_cast<int>(m_items.size()); ++i) {
            if (queryLower.empty() || !m_filterPredicate || m_filterPredicate(m_items[i], queryLower)) {
                m_filteredIndices.push_back(i);
            }
        }

        ApplySort();
    }

    void ApplySort() {
        if (m_sortColumnIndex < 0 || m_sortColumnIndex >= static_cast<int>(m_columns.size())) {
            return;
        }

        const auto& col = m_columns[m_sortColumnIndex];
        if (!col.comparator) {
            return;
        }

        bool asc = m_sortAscending;
        std::sort(m_filteredIndices.begin(), m_filteredIndices.end(),
            [this, &col, asc](int idxA, int idxB) {
                int cmp = col.comparator(m_items[idxA], m_items[idxB]);
                return asc ? (cmp < 0) : (cmp > 0);
            });
    }

    void RenderEmptyState(ImVec2 size) {
        UiTheme& theme = UiTheme::Get();
        const TableStyle& ts = theme.table;
        float scale = theme.GetScale();

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ts.colBodyBg);
        ImGui::BeginChild("##TableEmptyState", size, true, ImGuiWindowFlags_NoScrollbar);
        ImGui::PopStyleColor();

        float availY = size.y;
        float spacingY = std::max(10.0f, (availY - 120.0f * scale) * 0.4f);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + spacingY);

        // Иконка
        Icon emptyIcon = m_searchQuery.empty() ? Icon::Folder : Icon::Target;
        float iconSize = 42.0f * scale;
        float centerX = ImGui::GetWindowWidth() * 0.5f;

        emptyIcon.DrawAt(ImGui::GetWindowDrawList(),
            ImVec2(ImGui::GetWindowPos().x + centerX - iconSize * 0.5f, ImGui::GetCursorScreenPos().y),
            iconSize, ts.colEmptyIcon);

        ImGui::Dummy(ImVec2(0.0f, iconSize + 12.0f * scale));

        // Title
        std::string title = m_options.emptyTitle ? m_options.emptyTitle :
            (m_searchQuery.empty() ? "No records" : "No results found");
        ImVec2 titleSz = ImGui::CalcTextSize(title.c_str());
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - titleSz.x) * 0.5f);
        ImGui::TextColored(ImColor(theme.palette.textPrimary).Value, "%s", title.c_str());

        // Subtitle
        std::string sub = m_options.emptySubtitle ? m_options.emptySubtitle :
            (m_searchQuery.empty() ? "No items available in the dataset" : "Try adjusting your search filter");
        ImVec2 subSz = ImGui::CalcTextSize(sub.c_str());
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - subSz.x) * 0.5f);
        ImGui::TextColored(ImColor(ts.colEmptyText).Value, "%s", sub.c_str());

        ImGui::EndChild();
    }

    TableOptions m_options;
    std::vector<TableColumn<T>> m_columns;
    std::vector<T> m_items;
    std::vector<int> m_filteredIndices;

    FilterPredicate m_filterPredicate;
    std::string m_searchQuery;

    int m_sortColumnIndex = -1;
    bool m_sortAscending = true;
};

// Параметры табличной сетки (designated initializers):
//
//     TableGrid grid({TableGrid::Column::Stretch("Name"), TableGrid::Column::Fixed("Size", 80)},
//                    {.height = 170, .scrollY = false});
struct TableGridOptions {
    const char* key = nullptr;     // идентичность; nullptr — выводится из заголовков колонок
    float height = 0.0f;           // базовые px; <= 0: автоматически на всё доступное пространство
    bool scrollY = true;
    bool rowBg = true;
    bool bordersOuter = true;
    bool bordersInnerH = true;
    bool bordersInnerV = true;
    bool highlightHeaders = false; // подсветка заголовков при наведении мыши
    float cellPaddingX = -1.0f;    // <= 0: использовать theme.table.cellPaddingX
    float cellPaddingY = 6.0f;
};

// Параметры строки табличной сетки
struct TableRowOptions {
    const char* key = nullptr;     // идентичность строки (обязательна, если в ячейках одинаковые кнопки)
    float height = 0.0f;           // базовые px; 0 — стандартная высота
};

// Легковесная процедурная таблица для форм, редакторов и модальных окон (RAII-область).
// Автоматически управляет палитрой темы и строками:
//
//     if (TableGrid grid({Column::Stretch("Name"), Column::Fixed("Size", 80)}); grid) {
//         grid.Row();
//         grid.Cell(); Text("report.csv");
//         grid.Cell(); Text("12 KB");
//     }
class TableGrid : public Scope {
public:
    struct Column {
        std::string header;
        ColumnWidthMode widthMode = ColumnWidthMode::Stretch;
        float width = 1.0f; // вес растяжения (Stretch) или базовые px (Fixed)
        bool sortable = false;

        static Column Stretch(std::string header, float weight = 1.0f) {
            return Column{std::move(header), ColumnWidthMode::Stretch, weight, false};
        }
        static Column Fixed(std::string header, float widthBasePx) {
            return Column{std::move(header), ColumnWidthMode::Fixed, widthBasePx, false};
        }
    };

    using Options = TableGridOptions;

    explicit TableGrid(const std::vector<Column>& columns, const TableGridOptions& opts = {}) {
        UiTheme& theme = UiTheme::Get();
        const TableStyle& ts = theme.table;
        const float scale = theme.GetScale();

        // Идентичность таблицы: явный key либо склейка заголовков колонок
        std::string tableId = "##grid";
        if (opts.key) {
            tableId += opts.key;
        } else {
            for (const auto& col : columns) tableId += ":" + col.header;
        }

        ImGuiTableFlags flags = 0;
        if (opts.scrollY) flags |= ImGuiTableFlags_ScrollY;
        if (opts.rowBg) flags |= ImGuiTableFlags_RowBg;
        if (opts.bordersOuter) flags |= ImGuiTableFlags_BordersOuter;
        if (opts.bordersInnerH) flags |= ImGuiTableFlags_BordersInnerH;
        if (opts.bordersInnerV) flags |= ImGuiTableFlags_BordersInnerV;

        ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, ts.colHeaderBg);
        ImGui::PushStyleColor(ImGuiCol_TableRowBg, ts.colRowBg);
        ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, ts.colRowBgAlt);
        ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, ts.colBorderOuter);
        ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ts.colBorderInner);
        ImGui::PushStyleColor(ImGuiCol_Header, ts.colHeaderBg);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, opts.highlightHeaders ? ts.colHeaderHovered : ts.colHeaderBg);
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, opts.highlightHeaders ? ts.colHeaderActive : ts.colHeaderBg);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ts.colBodyBg);

        float padX = (opts.cellPaddingX >= 0.0f) ? opts.cellPaddingX : ts.cellPaddingX;
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(padX * scale, opts.cellPaddingY * scale));

        float tableH = (opts.height > 0.0f) ? theme.Scale(opts.height)
                                            : std::max(60.0f, ImGui::GetContentRegionAvail().y - 1.0f);

        m_open = ImGui::BeginTable(tableId.c_str(), static_cast<int>(columns.size()), flags, ImVec2(0.0f, tableH));
        if (m_open) {
            if (opts.scrollY) {
                ImGui::TableSetupScrollFreeze(0, 1);
            }
            for (size_t i = 0; i < columns.size(); ++i) {
                const auto& col = columns[i];
                ImGuiTableColumnFlags colFlags = 0;
                if (col.widthMode == ColumnWidthMode::Fixed) {
                    colFlags |= ImGuiTableColumnFlags_WidthFixed;
                } else if (col.widthMode == ColumnWidthMode::Stretch) {
                    colFlags |= ImGuiTableColumnFlags_WidthStretch;
                }
                if (!col.sortable) colFlags |= ImGuiTableColumnFlags_NoSort;

                float w = (col.widthMode == ColumnWidthMode::Fixed) ? theme.Scale(col.width) : col.width;
                ImGui::TableSetupColumn(col.header.c_str(), colFlags, w, static_cast<ImGuiID>(i));
            }

            theme.PushFont(theme.fontMedium, ts.headerFontSize);
            ImGui::TableHeadersRow();
            theme.PopFont();
        } else {
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(9);
        }
    }

    ~TableGrid() {
        if (m_rowIdPushed) {
            ImGui::PopID();
        }
        if (m_open) {
            ImGui::EndTable();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(9);
        }
    }

    // Новая строка; ячейки заполняются по порядку вызовами Cell()
    void Row(const TableRowOptions& options = {}) {
        if (m_rowIdPushed) {
            ImGui::PopID();
            m_rowIdPushed = false;
        }
        UiTheme& theme = UiTheme::Get();
        float h = (options.height > 0.0f) ? theme.Scale(options.height) : (theme.GetMetrics(UiSize::Medium).height + theme.Scale(8.0f));
        m_rowHeightPx = h;
        m_nextColumn = 0;
        ImGui::TableNextRow(0, h);
        if (options.key) {
            ImGui::PushID(options.key);
            m_rowIdPushed = true;
        }
    }

    // Переход в следующую ячейку строки
    bool Cell() {
        return ImGui::TableSetColumnIndex(m_nextColumn++);
    }

    // Переход в ячейку по номеру колонки
    bool Cell(int columnIndex) {
        m_nextColumn = columnIndex + 1;
        return ImGui::TableSetColumnIndex(columnIndex);
    }

    float CellWidth() const {
        return ImGui::GetContentRegionAvail().x;
    }

    // Текст ячейки по центру высоты строки (высота — как у последней Row)
    void CellText(const std::string& text, const TextOptions& options = {}) {
        const float offset = (m_rowHeightPx - ImGui::GetTextLineHeight()) * 0.5f - ImGui::GetStyle().CellPadding.y;
        if (offset > 0.0f)
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offset);
        ::Text(text, options);
    }

    void CenterNextItem(float itemWidth) {
        float avail = CellWidth();
        if (avail > itemWidth) {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail - itemWidth) * 0.5f);
        }
    }

private:
    bool m_rowIdPushed = false;
    int m_nextColumn = 0;
    float m_rowHeightPx = 0.0f;
};