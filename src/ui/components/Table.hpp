#pragma once

#include "ui/components/UiTheme.hpp"
#include "ui/components/Icon.hpp"
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
    float width = 0.0f;                                 // Ширина в пикселях (или вес растяжения)
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

// Высокопроизводительная виртуализированная таблица (Table<T>)
// Способна отображать десятки и сотни тысяч строк с 60+ FPS благодаря ImGuiListClipper
// и индексной буферизации (фильтрация и сортировка оперируют массивом индексов без копирования моделей)
template <typename T>
class Table {
public:
    Table(std::string tableId = "##Table")
        : m_tableId(std::move(tableId)) {}

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

    // Настройка пустых состояний
    void SetEmptyMessage(std::string title, std::string subtitle = "") {
        m_emptyTitle = std::move(title);
        m_emptySubtitle = std::move(subtitle);
    }

    // Принудительная сортировка программно
    void SortByColumn(int columnIndex, bool ascending) {
        m_sortColumnIndex = columnIndex;
        m_sortAscending = ascending;
        ApplySort();
    }

    void SetRowHeight(float height) {
        m_customRowHeight = height;
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

    void SetSaveSettings(bool save) {
        m_saveSettings = save;
    }

    // Основной цикл отрисовки
    void Render(float availWidth = 0.0f, float availHeight = 0.0f) {
        UiTheme& theme = UiTheme::Get();
        const TableStyle& ts = theme.table;
        float scale = theme.GetScale();
        float rowHeight = (m_customRowHeight > 0.0f) ? theme.Scale(m_customRowHeight) : theme.Scale(ts.rowHeight);

        ImVec2 size(availWidth, availHeight);
        if (size.x <= 0.0f) size.x = ImGui::GetContentRegionAvail().x;
        if (size.y <= 0.0f) size.y = ImGui::GetContentRegionAvail().y;

        // Пустое состояние
        if (m_filteredIndices.empty()) {
            RenderEmptyState(size);
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

        if (!m_saveSettings) {
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
            ImGui::BeginTable(m_tableId.c_str(), static_cast<int>(m_columns.size()), flags, size);
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

                    for (size_t c = 0; c < m_columns.size(); ++c) {
                        if (!m_columns[c].visible) continue;
                        if (ImGui::TableSetColumnIndex(static_cast<int>(c))) {
                            if (m_columns[c].renderCell) {
                                m_columns[c].renderCell(item, itemIdx);
                            }
                        }
                    }
                }
            }

            ImGui::EndTable();
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(7);
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
        std::string title = !m_emptyTitle.empty() ? m_emptyTitle :
            (m_searchQuery.empty() ? "No records" : "No results found");
        ImVec2 titleSz = ImGui::CalcTextSize(title.c_str());
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - titleSz.x) * 0.5f);
        ImGui::TextColored(ImColor(theme.palette.textPrimary).Value, "%s", title.c_str());

        // Subtitle
        std::string sub = !m_emptySubtitle.empty() ? m_emptySubtitle :
            (m_searchQuery.empty() ? "No items available in the dataset" : "Try adjusting your search filter");
        ImVec2 subSz = ImGui::CalcTextSize(sub.c_str());
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - subSz.x) * 0.5f);
        ImGui::TextColored(ImColor(ts.colEmptyText).Value, "%s", sub.c_str());

        ImGui::EndChild();
    }

    std::string m_tableId;
    std::vector<TableColumn<T>> m_columns;
    std::vector<T> m_items;
    std::vector<int> m_filteredIndices;

    FilterPredicate m_filterPredicate;
    std::string m_searchQuery;
    std::string m_emptyTitle;
    std::string m_emptySubtitle;

    int m_sortColumnIndex = -1;
    bool m_sortAscending = true;
    float m_customRowHeight = 0.0f;
    bool m_saveSettings = false;
};

// Псевдоним для полной обратной совместимости
template <typename T>
using VirtualTable = Table<T>;

// Легковесная процедурная таблица для форм, редакторов и модальных окон
// Автоматически управляет жизненным циклом (RAII), палитрой темы и строками
class TableGrid {
public:
    struct Column {
        std::string header;
        ColumnWidthMode widthMode = ColumnWidthMode::Stretch;
        float width = 1.0f; // вес растяжения (Stretch) или пиксели (Fixed)
        bool sortable = false;
    };

    struct Options {
        float height = 0.0f;           // <= 0: автоматически на всё доступное пространство
        bool scrollY = true;
        bool rowBg = true;
        bool bordersOuter = true;
        bool bordersInnerH = true;
        bool bordersInnerV = true;
        bool highlightHeaders = false; // подсветка заголовков при наведении мыши
        float cellPaddingX = -1.0f;    // <= 0: использовать theme.table.cellPaddingX
        float cellPaddingY = 6.0f;
    };

    TableGrid(const char* tableId, const std::vector<Column>& columns, const Options& opts = {}) {
        UiTheme& theme = UiTheme::Get();
        const TableStyle& ts = theme.table;
        const float scale = theme.GetScale();

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

        m_open = ImGui::BeginTable(tableId, static_cast<int>(columns.size()), flags, ImVec2(0.0f, tableH));
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
        End();
    }

    TableGrid(const TableGrid&) = delete;
    TableGrid& operator=(const TableGrid&) = delete;

    TableGrid(TableGrid&& other) noexcept
        : m_open(other.m_open), m_rowIdPushed(other.m_rowIdPushed) {
        other.m_open = false;
        other.m_rowIdPushed = false;
    }

    TableGrid& operator=(TableGrid&& other) noexcept {
        if (this != &other) {
            End();
            m_open = other.m_open;
            m_rowIdPushed = other.m_rowIdPushed;
            other.m_open = false;
            other.m_rowIdPushed = false;
        }
        return *this;
    }

    explicit operator bool() const { return m_open; }
    bool IsOpen() const { return m_open; }

    void End() {
        if (m_rowIdPushed) {
            ImGui::PopID();
            m_rowIdPushed = false;
        }
        if (m_open) {
            ImGui::EndTable();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(9);
            m_open = false;
        }
    }

    void NextRow(float height = 0.0f) {
        if (m_rowIdPushed) {
            ImGui::PopID();
            m_rowIdPushed = false;
        }
        UiTheme& theme = UiTheme::Get();
        float h = (height > 0.0f) ? theme.Scale(height) : (theme.GetMetrics(UiSize::Medium).height + theme.Scale(8.0f));
        m_rowHeightPx = h;
        ImGui::TableNextRow(0, h);
    }

    void NextRow(int rowId, float height = 0.0f) {
        NextRow(height);
        ImGui::PushID(rowId);
        m_rowIdPushed = true;
    }

    void NextRow(const char* rowId, float height = 0.0f) {
        NextRow(height);
        ImGui::PushID(rowId);
        m_rowIdPushed = true;
    }

    bool SetColumn(int columnIndex) {
        return ImGui::TableSetColumnIndex(columnIndex);
    }

    float CellWidth() const {
        return ImGui::GetContentRegionAvail().x;
    }

    // Текст ячейки по центру высоты строки (высота — как у последней NextRow)
    void CellText(const std::string& text, ImU32 color = 0) {
        const UiTheme& theme = UiTheme::Get();
        const float offset = (m_rowHeightPx - ImGui::GetTextLineHeight()) * 0.5f - ImGui::GetStyle().CellPadding.y;
        if (offset > 0.0f)
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offset);
        ImGui::TextColored(ImColor(color != 0 ? color : theme.palette.textPrimary).Value, "%s", text.c_str());
    }

    void CenterNextItem(float itemWidth) {
        float avail = CellWidth();
        if (avail > itemWidth) {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail - itemWidth) * 0.5f);
        }
    }

private:
    bool m_open = false;
    bool m_rowIdPushed = false;
    float m_rowHeightPx = 0.0f;
};
