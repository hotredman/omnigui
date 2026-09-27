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
    SliderFloat,
    InputText,
    Separator,
    Canvas
};

struct Element {
    uint32_t id = 0;
    ElementType type = ElementType::Text;
    std::string label;
    std::string value_str;
    float value_num = 0.0f;
    float min_val = 0.0f;
    float max_val = 1.0f;
    bool checked = false;
    float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;

    // For canvas elements (e.g., real-time oscilloscope)
    std::string stream_name;
    std::vector<float> points;
};

struct Window {
    uint32_t id = 0;
    std::string title;
    float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
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

                switch (el.type) {
                case ElementType::Button:
                    break;
                case ElementType::Text:
                    ss << "\"val\":\"" << EscapeJson(el.value_str) << "\",";
                    break;
                case ElementType::Checkbox:
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
                case ElementType::Canvas:
                    ss << "\"stream\":\"" << EscapeJson(el.stream_name) << "\",";
                    ss << "\"points\":[";
                    for (size_t pi = 0; pi < el.points.size(); ++pi) {
                        if (pi > 0) ss << ",";
                        ss << std::fixed << std::setprecision(2) << el.points[pi];
                    }
                    ss << "],";
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
        case ElementType::SliderFloat: return "slider";
        case ElementType::InputText: return "input";
        case ElementType::Separator: return "separator";
        case ElementType::Canvas: return "canvas";
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
