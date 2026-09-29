#include "ui/components/ItemRow.hpp"
#include "ui/components/Badge.hpp"
#include "ui/components/Tag.hpp"
#include "ui/components/ToolButton.hpp"
#include "ui/components/UiTheme.hpp"
#include <imgui.h>
#include <cstdio>
#include <algorithm>
#include <unordered_map>

static std::unordered_map<std::string, float> s_itemRowHeightCache;

ItemRow::ItemRow(const char* id) {
    m_id = (id && id[0] != '\0') ? id : "item";
    ImGui::PushID(m_id.c_str());
    m_open = true;
    m_actionCount = 0;
    m_totalActionsExpected = 1;

    const UiTheme& theme = UiTheme::Get();
    const ItemRowStyle& st = theme.itemRow;
    m_startPos = ImGui::GetCursorScreenPos();
    m_availW = ImGui::GetContentRegionAvail().x;
    m_padX = theme.Scale(st.paddingX);
    m_padY = theme.Scale(st.paddingYTop);

    m_titleRowTopY = m_startPos.y + m_padY;
    m_titleRowHeight = theme.Scale(24.0f);
    m_titleEndX = m_startPos.x + m_padX;

    auto it = s_itemRowHeightCache.find(m_id);
    m_prevHeight = (it != s_itemRowHeightCache.end()) ? it->second : theme.Scale(84.0f);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    m_splitter.Split(dl, 2);
    m_splitter.SetCurrentChannel(dl, 1);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(theme.Scale(6.0f), theme.Scale(st.spacingTitleDesc)));

    ImGui::BeginGroup();
    // Устанавливаем системный отступ для всей группы — все переносимые строки (текст и теги) наследуют m_padX
    ImGui::Indent(m_padX);
    ImGui::SetCursorScreenPos(ImVec2(m_startPos.x + m_padX, m_startPos.y + m_padY));
}

ItemRow::ItemRow(const std::string& id)
    : ItemRow(id.c_str()) {}

ItemRow::~ItemRow() {
    End();
}

void ItemRow::End() {
    if (!m_open) return;

    DrawBadges();

    const UiTheme& theme = UiTheme::Get();
    const ItemRowStyle& st = theme.itemRow;

    ImGui::Unindent(m_padX);
    ImGui::EndGroup();
    ImGui::PopStyleVar();

    float groupEndY = ImGui::GetItemRectMax().y;
    float contentH = groupEndY - m_startPos.y;
    float cardHeight = std::max(theme.Scale(40.0f), contentH + theme.Scale(st.paddingYBottom));
    s_itemRowHeightCache[m_id] = cardHeight;

    ImVec2 pMin = m_startPos;
    ImVec2 pMax = ImVec2(m_startPos.x + m_availW, m_startPos.y + cardHeight);

    bool hovered = ImGui::IsMouseHoveringRect(pMin, pMax);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    m_splitter.SetCurrentChannel(dl, 0);

    ImU32 bgCol = hovered ? st.colBgHover : st.colBg;
    ImU32 borderCol = hovered ? st.colBorderHover : st.colBorder;
    float rounding = theme.Scale(st.cornerRadius);

    // Подложка строки за краем видимой области не рисуется
    if (ImGui::IsRectVisible(pMin, pMax)) {
        dl->AddRectFilled(pMin, pMax, bgCol, rounding);
        dl->AddRect(pMin, pMax, borderCol, rounding, 0, st.borderSize);
    }

    m_splitter.Merge(dl);

    // Точно позиционируем курсор для следующей карточки по st.itemSpacingY
    float gap = theme.Scale(st.itemSpacingY);
    float itemSpacing = ImGui::GetStyle().ItemSpacing.y;
    float nextY = pMax.y + gap;
    ImGui::SetCursorScreenPos(ImVec2(m_startPos.x, nextY - itemSpacing));
    ImGui::Dummy(ImVec2(0.0f, 0.0f));

    ImGui::PopID();
    m_open = false;
}

void ItemRow::Title(const std::string& name, const std::string& symbol, const std::string& unitDisplay) {
    const UiTheme& theme = UiTheme::Get();
    const ItemRowStyle& st = theme.itemRow;

    m_titleRowTopY = ImGui::GetCursorScreenPos().y;

    theme.PushFont(st.titleFont ? st.titleFont : theme.fontBold, st.titleFontSize);
    ImGui::TextUnformatted(name.c_str());
    theme.PopFont();

    if (!symbol.empty()) {
        ImGui::SameLine(0.0f, theme.Scale(6.0f));
        theme.PushFont(st.symbolFont ? st.symbolFont : theme.fontBold, st.symbolFontSize);
        ImGui::PushStyleColor(ImGuiCol_Text, theme.GetVariantStyle(UiVariant::Primary).colText);
        ImGui::Text("(%s)", symbol.c_str());
        ImGui::PopStyleColor();
        theme.PopFont();
    }

    if (!unitDisplay.empty()) {
        ImGui::SameLine(0.0f, theme.Scale(6.0f));
        theme.PushFont(st.unitFont ? st.unitFont : theme.fontMedium, st.unitFontSize);
        ImGui::PushStyleColor(ImGuiCol_Text, theme.palette.textMuted);
        ImGui::Text("[%s]", unitDisplay.c_str());
        ImGui::PopStyleColor();
        theme.PopFont();
    }

    m_titleEndX = ImGui::GetItemRectMax().x;
    m_titleRowHeight = std::max(theme.Scale(22.0f), ImGui::GetItemRectMax().y - m_titleRowTopY);
}

void ItemRow::Badge(const char* text, UiVariant variant) {
    if (!text || text[0] == '\0') return;
    m_badges.push_back({std::string(text), variant});
}

void ItemRow::DrawBadges() {
    if (m_badges.empty()) return;

    const UiTheme& theme = UiTheme::Get();
    const ItemRowStyle& st = theme.itemRow;
    float badgeSpacing = theme.Scale(6.0f);

    int totalActions = std::max(m_actionCount, m_totalActionsExpected);
    float btnW = theme.Scale(st.actionBtnWidth);
    float actionSpacing = theme.Scale(st.actionSpacing);
    float totalActionsW = (totalActions > 0) ? (totalActions * btnW + std::max(0, totalActions - 1) * actionSpacing) : 0.0f;
    float rightEdge = m_startPos.x + m_availW - m_padX;
    if (totalActionsW > 0.0f) {
        rightEdge -= (totalActionsW + theme.Scale(10.0f));
    }

    std::vector<ImVec2> sizes;
    sizes.reserve(m_badges.size());
    float totalBadgesW = 0.0f;
    for (size_t i = 0; i < m_badges.size(); ++i) {
        ImVec2 sz = ::Badge::CalcSize(m_badges[i].text.c_str(), st.badgeFontSize);
        sizes.push_back(sz);
        totalBadgesW += sz.x;
        if (i > 0) totalBadgesW += badgeSpacing;
    }

    float startX = rightEdge - totalBadgesW;
    float minStartX = m_titleEndX + theme.Scale(8.0f);
    if (startX < minStartX) {
        startX = minStartX;
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();
    float curX = startX;
    for (size_t i = 0; i < m_badges.size(); ++i) {
        float badgeH = sizes[i].y;
        float offsetY = std::max(0.0f, (m_titleRowHeight - badgeH) * 0.5f);
        float badgeY = m_titleRowTopY + offsetY;

        ::Badge::Draw(dl, ImVec2(curX, badgeY), m_badges[i].text.c_str(), m_badges[i].variant, st.badgeFontSize);
        curX += sizes[i].x + badgeSpacing;
    }
}

void ItemRow::RightActions(int totalActions) {
    if (totalActions > 0) {
        m_totalActionsExpected = totalActions;
    }
}

bool ItemRow::Action(Icon icon, const char* tooltip, UiVariant variant, bool disabled, int totalActions) {
    if (totalActions > m_totalActionsExpected) {
        m_totalActionsExpected = totalActions;
    }

    const UiTheme& theme = UiTheme::Get();
    const ItemRowStyle& st = theme.itemRow;
    float btnW = theme.Scale(st.actionBtnWidth);
    float btnH = theme.Scale(st.actionBtnHeight);
    float spacing = theme.Scale(st.actionSpacing);

    float totalActionsW = m_totalActionsExpected * btnW + std::max(0, m_totalActionsExpected - 1) * spacing;
    float startX = m_startPos.x + m_availW - m_padX - totalActionsW;
    float btnX = startX + m_actionCount * (btnW + spacing);

    float btnY = m_startPos.y + std::max(m_padY, (m_prevHeight - btnH) * 0.5f);

    ImVec2 savedLeftPos = ImGui::GetCursorPos();
    ImGui::SetCursorScreenPos(ImVec2(btnX, btnY));

    char btnId[32];
    std::snprintf(btnId, sizeof(btnId), "##act_%d", m_actionCount + 1);

    bool clicked = ToolButton::Render(btnId, icon, UiSize::Small, variant, tooltip, false, 0, disabled);

    ImGui::SetCursorPos(savedLeftPos);
    m_actionCount++;

    return clicked;
}

void ItemRow::Description(const std::string& text) {
    if (text.empty()) return;
    const UiTheme& theme = UiTheme::Get();
    const ItemRowStyle& st = theme.itemRow;

    float rightArea = m_totalActionsExpected * theme.Scale(st.actionBtnWidth) + theme.Scale(16.0f);
    float maxWrapW = std::max(theme.Scale(100.0f), m_availW - m_padX * 2.0f - rightArea);

    theme.PushFont(st.descriptionFont ? st.descriptionFont : theme.fontRegular, st.descriptionFontSize);
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + maxWrapW);
    ImGui::PushStyleColor(ImGuiCol_Text, st.colTextDescription);
    ImGui::TextUnformatted(text.c_str());
    ImGui::PopStyleColor();
    ImGui::PopTextWrapPos();
    theme.PopFont();
}

void ItemRow::Formula(const std::string& symbol, const std::string& expression) {
    if (expression.empty()) return;
    const UiTheme& theme = UiTheme::Get();
    const ItemRowStyle& st = theme.itemRow;

    theme.PushFont(st.formulaFont ? st.formulaFont : theme.fontMedium, st.formulaFontSize);
    ImGui::PushStyleColor(ImGuiCol_Text, st.colTextFormula);
    ImGui::Text("%s = %s", symbol.c_str(), expression.c_str());
    ImGui::PopStyleColor();
    theme.PopFont();
}

bool ItemRow::Tags(const std::vector<std::string>& tags, std::string* outClickedTag) {
    if (tags.empty()) return false;
    const UiTheme& theme = UiTheme::Get();
    const ItemRowStyle& st = theme.itemRow;

    float extraGap = theme.Scale(st.spacingDescTags - st.spacingTitleDesc);
    if (extraGap > 0.0f) {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + extraGap);
    }

    float rightArea = m_totalActionsExpected * theme.Scale(st.actionBtnWidth) + theme.Scale(16.0f);
    float maxWrapW = std::max(theme.Scale(100.0f), m_availW - m_padX * 2.0f - rightArea);

    return Tag::RenderList(tags, maxWrapW, outClickedTag);
}

void ItemRow::Status(const char* text, UiVariant variant) {
    Badge(text, variant);
}
