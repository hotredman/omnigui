#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <cstdint>
#include <iomanip>

namespace ImGuiDom {

enum class ElementType {
    Window,
    Text,
    Button,
    Checkbox,
    RadioButton,
    SliderFloat,
    InputText,
    Combo,
    TreeNode,
    CollapsingHeader,
    ProgressBar,
    Separator,
    Canvas,
    TabBar,
    TabItem,
    Tooltip,
    Table,
    ListBox,
    ColorEdit,
    InputTextMultiline,
    MenuItem
};

struct Element {
    uint32_t id = 0;
    ElementType type = ElementType::Text;
    bool is_menu_bar = false;
    std::string label;
    std::string value_str;
    std::string tooltip;
    float value_num = 0.0f;
    float min_val = 0.0f;
    float max_val = 1.0f;
    bool checked = false;
    bool opened = false;
    bool selected = false;
    bool disabled = false;
    float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;

    // For canvas elements (e.g., real-time oscilloscope)
    std::string stream_name;
    std::vector<float> points;

    // For ListBox and Table elements
    std::vector<std::string> items;
    int selected_idx = 0;
    int columns_count = 0;
    bool has_headers = false;
    bool has_alpha = false;
};

struct Window {
    uint32_t id = 0;
    std::string title;
    float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
    float title_bar_h = 24.0f;
    float menu_bar_h = 0.0f;
    float scroll_x = 0.0f;
    float scroll_y = 0.0f;
    bool has_title_bar = true;
    bool has_menu_bar = false;
    bool is_popup = false;
    bool is_modal = false;
    bool is_tooltip = false;
    bool collapsed = false;
    std::vector<Element> elements;
};

struct Document {
    uint64_t frame_index = 0;
    std::vector<Window> windows;

    void Clear() {
        windows.clear();
    }

    std::string ToJson() const {
        std::ostringstream ss;
        ss << "{";
        ss << "\"frame\":" << frame_index << ",";
        ss << "\"windows\":[";

        for (size_t wi = 0; wi < windows.size(); ++wi) {
            const auto& win = windows[wi];
            if (wi > 0) ss << ",";
            ss << "{";
            ss << "\"id\":" << win.id << ",";
            ss << "\"title\":\"" << EscapeJson(win.title) << "\",";
            ss << "\"x\":" << win.x << ",\"y\":" << win.y << ",";
            ss << "\"w\":" << win.w << ",\"h\":" << win.h << ",";
            ss << "\"title_bar_h\":" << win.title_bar_h << ",";
            ss << "\"menu_bar_h\":" << win.menu_bar_h << ",";
            ss << "\"scroll_x\":" << win.scroll_x << ",\"scroll_y\":" << win.scroll_y << ",";
            ss << "\"has_title_bar\":" << (win.has_title_bar ? "true" : "false") << ",";
            ss << "\"has_menu_bar\":" << (win.has_menu_bar ? "true" : "false") << ",";
            ss << "\"is_popup\":" << (win.is_popup ? "true" : "false") << ",";
            ss << "\"is_modal\":" << (win.is_modal ? "true" : "false") << ",";
            ss << "\"is_tooltip\":" << (win.is_tooltip ? "true" : "false") << ",";
            ss << "\"collapsed\":" << (win.collapsed ? "true" : "false") << ",";
            ss << "\"elements\":[";

            for (size_t ei = 0; ei < win.elements.size(); ++ei) {
                const auto& el = win.elements[ei];
                if (ei > 0) ss << ",";
                ss << "{";
                ss << "\"id\":" << el.id << ",";
                ss << "\"type\":\"" << TypeToString(el.type) << "\",";
                ss << "\"label\":\"" << EscapeJson(el.label) << "\",";
                ss << "\"x\":" << el.x << ",\"y\":" << el.y << ",";
                ss << "\"w\":" << el.w << ",\"h\":" << el.h << ",";
                if (el.disabled) ss << "\"disabled\":true,";
                if (el.is_menu_bar) ss << "\"is_menu_bar\":true,";
                if (!el.tooltip.empty()) ss << "\"tooltip\":\"" << EscapeJson(el.tooltip) << "\",";

                switch (el.type) {
                case ElementType::Button:
                case ElementType::Separator:
                case ElementType::TabBar:
                case ElementType::Tooltip:
                    break;
                case ElementType::MenuItem:
                case ElementType::TabItem:
                    ss << "\"selected\":" << (el.selected ? "true" : "false") << ",";
                    break;
                case ElementType::Text:
                    ss << "\"val\":\"" << EscapeJson(el.value_str) << "\",";
                    break;
                case ElementType::Checkbox:
                case ElementType::RadioButton:
                    ss << "\"checked\":" << (el.checked ? "true" : "false") << ",";
                    break;
                case ElementType::SliderFloat:
                    ss << "\"val\":" << el.value_num << ",";
                    ss << "\"min\":" << el.min_val << ",";
                    ss << "\"max\":" << el.max_val << ",";
                    break;
                case ElementType::InputText:
                    ss << "\"val\":\"" << EscapeJson(el.value_str) << "\",";
                    break;
                case ElementType::Combo:
                    ss << "\"val\":\"" << EscapeJson(el.value_str) << "\",";
                    ss << "\"opened\":" << (el.opened ? "true" : "false") << ",";
                    break;
                case ElementType::TreeNode:
                case ElementType::CollapsingHeader:
                    ss << "\"opened\":" << (el.opened ? "true" : "false") << ",";
                    break;
                case ElementType::ProgressBar:
                    ss << "\"val\":" << el.value_num << ",";
                    ss << "\"overlay\":\"" << EscapeJson(el.value_str) << "\",";
                    break;
                case ElementType::Canvas:
                    ss << "\"stream\":\"" << EscapeJson(el.stream_name) << "\",";
                    ss << "\"points\":[";
                    for (size_t pi = 0; pi < el.points.size(); ++pi) {
                        if (pi > 0) ss << ",";
                        ss << std::fixed << std::setprecision(2) << el.points[pi];
                    }
                    ss << "],";
                    break;
                case ElementType::ListBox:
                    ss << "\"selected_idx\":" << el.selected_idx << ",";
                    ss << "\"items\":[";
                    for (size_t ii = 0; ii < el.items.size(); ++ii) {
                        if (ii > 0) ss << ",";
                        ss << "\"" << EscapeJson(el.items[ii]) << "\"";
                    }
                    ss << "],";
                    break;
                case ElementType::Table:
                    ss << "\"columns_count\":" << el.columns_count << ",";
                    ss << "\"has_headers\":" << (el.has_headers ? "true" : "false") << ",";
                    ss << "\"columns\":[";
                    for (size_t ii = 0; ii < el.items.size(); ++ii) {
                        if (ii > 0) ss << ",";
                        ss << "\"" << EscapeJson(el.items[ii]) << "\"";
                    }
                    ss << "],";
                    break;
                case ElementType::ColorEdit:
                    ss << "\"val\":\"" << EscapeJson(el.value_str) << "\",";
                    ss << "\"has_alpha\":" << (el.has_alpha ? "true" : "false") << ",";
                    break;
                case ElementType::InputTextMultiline:
                    ss << "\"val\":\"" << EscapeJson(el.value_str) << "\",";
                    break;
                default:
                    break;
                }
                // Remove trailing comma if present
                std::string el_str = ss.str();
                if (el_str.back() == ',') {
                    el_str.pop_back();
                    ss.str("");
                    ss << el_str;
                }
                ss << "}";
            }
            ss << "]}";
        }
        ss << "]}";
        return ss.str();
    }

private:
    static std::string TypeToString(ElementType t) {
        switch (t) {
        case ElementType::Window: return "window";
        case ElementType::Text: return "text";
        case ElementType::Button: return "button";
        case ElementType::Checkbox: return "checkbox";
        case ElementType::RadioButton: return "radio";
        case ElementType::SliderFloat: return "slider";
        case ElementType::InputText: return "input";
        case ElementType::Combo: return "combo";
        case ElementType::TreeNode: return "treenode";
        case ElementType::CollapsingHeader: return "collapsing_header";
        case ElementType::ProgressBar: return "progress";
        case ElementType::Separator: return "separator";
        case ElementType::Canvas: return "canvas";
        case ElementType::TabBar: return "tabbar";
        case ElementType::TabItem: return "tabitem";
        case ElementType::Tooltip: return "tooltip";
        case ElementType::Table: return "table";
        case ElementType::ListBox: return "listbox";
        case ElementType::ColorEdit: return "coloredit";
        case ElementType::InputTextMultiline: return "textarea";
        case ElementType::MenuItem: return "menuitem";
        default: return "unknown";
        }
    }

    static std::string EscapeJson(const std::string& s) {
        std::ostringstream o;
        for (char c : s) {
            switch (c) {
            case '"': o << "\\\""; break;
            case '\\': o << "\\\\"; break;
            case '\b': o << "\\b"; break;
            case '\f': o << "\\f"; break;
            case '\n': o << "\\n"; break;
            case '\r': o << "\\r"; break;
            case '\t': o << "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) <= 0x1f) {
                    o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c;
                } else {
                    o << c;
                }
            }
        }
        return o.str();
    }
};

} // namespace ImGuiDom
