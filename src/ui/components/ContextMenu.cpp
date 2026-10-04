#include "ui/components/ContextMenu.hpp"
#include <algorithm>

// Default popup identifier
static constexpr const char* kPopupName = "##ctx_menu";

ContextMenu::ContextMenu(bool& isOpen, const ContextMenuOptions& options)
    : m_openRef(&isOpen)
    , m_style(options.style ? *options.style : UiTheme::Get().contextMenu)
{
    ImGui::PushID(&isOpen);
    m_idPushed = true;

    // Flag tracking whether popup has been requested distinguishes "just opened" from "closed by outside click"
    bool* requested = ImGui::GetStateStorage()->GetBoolRef(ImGui::GetID("##ctx_requested"), false);
    if (!isOpen) {
        *requested = false;
        return;
    }
    if (!ImGui::IsPopupOpen(kPopupName)) {
        if (*requested) {
            // ImGui closed the popup automatically (click outside, Esc): reset application flag
            isOpen = false;
            *requested = false;
            return;
        }
        ImGui::OpenPopup(kPopupName);
        *requested = true;
    }

    if (options.pos) {
        ImGui::SetNextWindowPos(*options.pos, ImGuiCond_Appearing);
    }

    PushStyle();
    m_open = ImGui::BeginPopup(kPopupName);
    if (!m_open) {
        PopStyle();
    }
}

ContextMenu::ContextMenu(LastItemTag, const ContextMenuOptions& options)
    : m_style(options.style ? *options.style : UiTheme::Get().contextMenu)
{
    // Identity is the last item (right-click on it opens the menu)
    PushStyle();
    m_open = ImGui::BeginPopupContextItem();
    if (!m_open) {
        PopStyle();
    }
}

ContextMenu::ContextMenu(const char* popupId, const ContextMenuOptions& options)
    : m_style(options.style ? *options.style : UiTheme::Get().contextMenu)
{
    if (options.pos) {
        ImGui::SetNextWindowPos(*options.pos, ImGuiCond_Appearing);
    }

    PushStyle();
    m_open = ImGui::BeginPopup(popupId ? popupId : kPopupName);
    if (!m_open) {
        PopStyle();
    }
}

ContextMenu::~ContextMenu() {
    if (m_open) {
        ImGui::EndPopup();
        PopStyle();
    }
    if (m_idPushed) {
        ImGui::PopID();
    }
}

void ContextMenu::PushStyle() {
    const UiTheme& theme = UiTheme::Get();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(theme.Scale(m_style.windowPaddingX), theme.Scale(m_style.windowPaddingY)));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(theme.Scale(8.0f), theme.Scale(m_style.itemSpacingY)));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, theme.Scale(m_style.cornerRadius));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 1.0f);

    ImGui::PushStyleColor(ImGuiCol_PopupBg, m_style.colBg);
    ImGui::PushStyleColor(ImGuiCol_Border, m_style.colBorder);
}

void ContextMenu::PopStyle() {
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(4);
}

void ContextMenu::Header(const char* text) {
    if (!m_open || !text || text[0] == '\0') return;

    const UiTheme& theme = UiTheme::Get();
    ImFont* headerFont = m_style.headerFont ? m_style.headerFont : theme.smallFont;
    float fontSz = theme.Scale(m_style.headerFontSize);

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddText(headerFont, fontSz, pos, m_style.colHeader, text);

    ImVec2 txtSz = headerFont ?
        headerFont->CalcTextSizeA(fontSz, FLT_MAX, 0.0f, text) :
        ImGui::CalcTextSize(text);
    ImGui::Dummy(ImVec2(txtSz.x, txtSz.y + theme.Scale(6.0f)));
}

bool ContextMenu::Item(const char* label, const MenuItemOptions& options) {
    if (!m_open || !label) return false;
    const bool selected = options.selected;
    const bool enabled = !options.disabled;

    const UiTheme& theme = UiTheme::Get();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 itemPos = ImGui::GetCursorScreenPos();

    float itemHeight = theme.Scale(m_style.itemHeight);
    float checkW = theme.Scale(m_style.checkColWidth);
    float itemRounding = theme.Scale(m_style.itemRounding);

    // Calculate text size for styled item font
    ImFont* itemFont = m_style.itemFont ? m_style.itemFont : theme.defaultFont;
    float fontSz = theme.Scale(m_style.itemFontSize);
    ImVec2 txtSz = itemFont ?
        itemFont->CalcTextSizeA(fontSz, FLT_MAX, 0.0f, label) :
        ImGui::CalcTextSize(label);

    float minW = theme.Scale(m_style.minWidth);
    float totalW = std::max(minW, checkW + txtSz.x + theme.Scale(16.0f));
    ImVec2 itemSize(totalW, itemHeight);

    m_itemCounter++;
    ImGui::PushID(m_itemCounter);

    ImGui::SetCursorScreenPos(itemPos);
    bool clicked = false;
    if (ImGui::InvisibleButton("##ctx_item", itemSize)) {
        if (enabled) {
            clicked = true;
            if (m_openRef) *m_openRef = false;
            ImGui::CloseCurrentPopup();
        }
    }

    bool hovered = ImGui::IsItemHovered();
    bool active  = ImGui::IsItemActive();
    ImGui::PopID();

    // 1. Smooth rounded item highlight on hover/active
    if (enabled && (hovered || active)) {
        ImU32 bgCol = active ? m_style.colActiveBg : m_style.colHoverBg;
        dl->AddRectFilled(itemPos, ImVec2(itemPos.x + itemSize.x, itemPos.y + itemSize.y), bgCol, itemRounding);
    }

    // 2. Vector checkmark for selected item
    if (selected) {
        ImVec2 checkCenter(itemPos.x + theme.Scale(12.0f), itemPos.y + itemHeight * 0.5f);
        float ckSize = theme.Scale(12.0f);
        ImU32 ckCol = m_style.colCheckmark;
        ImVec2 p1(checkCenter.x - ckSize * 0.45f, checkCenter.y + ckSize * 0.05f);
        ImVec2 p2(checkCenter.x - ckSize * 0.10f, checkCenter.y + ckSize * 0.40f);
        ImVec2 p3(checkCenter.x + ckSize * 0.50f, checkCenter.y - ckSize * 0.40f);
        dl->AddLine(p1, p2, ckCol, 2.2f);
        dl->AddLine(p2, p3, ckCol, 2.2f);
    }

    // 3. Item text
    ImU32 textCol = !enabled ? theme.palette.textMuted :
                    selected ? m_style.colTextActive : m_style.colText;
    ImVec2 textPos(itemPos.x + checkW, itemPos.y + (itemHeight - fontSz) * 0.5f);
    dl->AddText(itemFont, fontSz, textPos, textCol, label);

    return clicked;
}

void ContextMenu::Separator() {
    if (!m_open) return;
    const UiTheme& theme = UiTheme::Get();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float availW = ImGui::GetContentRegionAvail().x;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddLine(ImVec2(pos.x, pos.y + theme.Scale(4.0f)),
                ImVec2(pos.x + availW, pos.y + theme.Scale(4.0f)),
                m_style.colSeparator, 1.0f);
    ImGui::Dummy(ImVec2(availW, theme.Scale(8.0f)));
}
