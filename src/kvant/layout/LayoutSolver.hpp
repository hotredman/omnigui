#pragma once

#include <algorithm>
#include <cmath>
#include "kvant/backend/ICanvas.hpp"
#include "kvant/layout/Node.hpp"

namespace kvant {

/// Resolves 2D flex layout for an arbitrary tree of Kvant nodes.
class LayoutSolver {
public:
    /// Measures intrinsic content sizes bottom-up.
    static void Measure(Node* root, ICanvas* canvas = nullptr) {
        if (!root) return;
        MeasureNode(root, canvas);
    }

    /// Computes final coordinates and dimensions top-down based on parent constraints.
    static void Arrange(Node* root, Rect availableRect) {
        if (!root) return;
        ArrangeNode(root, availableRect);
    }

private:
    static void MeasureNode(Node* node, ICanvas* canvas = nullptr) {
        // Measure children bottom-up first
        for (Node* child = node->firstChild; child != nullptr; child = child->nextSibling) {
            MeasureNode(child, canvas);
        }

        // If this is a leaf widget, compute its intrinsic dimensions
        if (node->childCount == 0) {
            switch (node->type) {
                case NodeType::Text: {
                    node->intrinsicSize = Text::Measure(node->text, node->textOpts, canvas);
                    break;
                }
                case NodeType::Button: {
                    node->intrinsicSize = Button::Measure(node->buttonOpts, node->uiSize, canvas);
                    break;
                }
                case NodeType::Switch: {
                    node->intrinsicSize = Switch::Measure(node->text, node->switchOpts, node->uiSize, canvas);
                    break;
                }
                case NodeType::Checkbox: {
                    node->intrinsicSize = Checkbox::Measure(node->text, node->checkboxOpts, node->uiSize, canvas);
                    break;
                }
                case NodeType::Spacer: {
                    node->intrinsicSize = {0.0f, 0.0f};
                    break;
                }
                default: {
                    if (node->widthMode == SizeMode::Fixed || node->heightMode == SizeMode::Fixed) {
                        node->intrinsicSize = {node->fixedWidth, node->fixedHeight};
                    } else {
                        node->intrinsicSize = {0.0f, 0.0f};
                    }
                    break;
                }
            }
            return;
        }

        // Container intrinsic size aggregation
        if (node->direction == FlexDirection::Row || node->direction == FlexDirection::Wrap) {
            float totalWidth = node->padding.Horizontal();
            float maxHeight = 0.0f;
            uint32_t activeChildren = 0;

            for (Node* child = node->firstChild; child != nullptr; child = child->nextSibling) {
                const float cw = (child->widthMode == SizeMode::Fixed) ? child->fixedWidth : child->intrinsicSize.x;
                const float ch = (child->heightMode == SizeMode::Fixed) ? child->fixedHeight : child->intrinsicSize.y;
                totalWidth += cw;
                maxHeight = std::max(maxHeight, ch);
                ++activeChildren;
            }

            if (activeChildren > 1) {
                totalWidth += static_cast<float>(activeChildren - 1) * node->gap;
            }
            node->intrinsicSize = {totalWidth, maxHeight + node->padding.Vertical()};
        } else { // FlexDirection::Column
            float totalHeight = node->padding.Vertical();
            float maxWidth = 0.0f;
            uint32_t activeChildren = 0;

            for (Node* child = node->firstChild; child != nullptr; child = child->nextSibling) {
                const float cw = (child->widthMode == SizeMode::Fixed) ? child->fixedWidth : child->intrinsicSize.x;
                const float ch = (child->heightMode == SizeMode::Fixed) ? child->fixedHeight : child->intrinsicSize.y;
                totalHeight += ch;
                maxWidth = std::max(maxWidth, cw);
                ++activeChildren;
            }

            if (activeChildren > 1) {
                totalHeight += static_cast<float>(activeChildren - 1) * node->gap;
            }
            node->intrinsicSize = {maxWidth + node->padding.Horizontal(), totalHeight};
        }
    }

    /// Resolves actual vertical dimension for a node given constrained available width.
    static float ResolveNodeHeight(Node* node, float availableWidth) {
        if (!node) return 0.0f;
        if (node->heightMode == SizeMode::Fixed) {
            return node->fixedHeight;
        }
        if (node->isSpacer) {
            return 0.0f;
        }
        if (node->childCount == 0) {
            return node->intrinsicSize.y;
        }

        if (node->direction == FlexDirection::Wrap) {
            const float maxLineWidth = std::max(0.0f, availableWidth - node->padding.Horizontal());
            Node* lineStart = node->firstChild;
            float totalH = 0.0f;
            bool hasLines = false;

            while (lineStart != nullptr) {
                Node* lineEnd = lineStart;
                float currentLineWidth = 0.0f;
                float lineMaxH = 0.0f;
                uint32_t itemsInLine = 0;

                while (lineEnd != nullptr) {
                    if (lineEnd->isSpacer) {
                        lineEnd = lineEnd->nextSibling;
                        continue;
                    }

                    const float cw = (lineEnd->widthMode == SizeMode::Fixed) ? lineEnd->fixedWidth : lineEnd->intrinsicSize.x;
                    const float ch = (lineEnd->heightMode == SizeMode::Fixed) ? lineEnd->fixedHeight : lineEnd->intrinsicSize.y;
                    const float neededWidth = (itemsInLine > 0) ? (node->gap + cw) : cw;

                    if (itemsInLine > 0 && currentLineWidth + neededWidth > maxLineWidth) {
                        break;
                    }

                    currentLineWidth += neededWidth;
                    lineMaxH = std::max(lineMaxH, ch);
                    ++itemsInLine;
                    lineEnd = lineEnd->nextSibling;
                }

                if (itemsInLine == 0) {
                    break;
                }

                if (hasLines) {
                    totalH += node->runGap;
                }
                totalH += lineMaxH;
                hasLines = true;
                lineStart = lineEnd;
            }

            return totalH + node->padding.Vertical();
        }

        if (node->direction == FlexDirection::Column && node->heightMode == SizeMode::Fit) {
            const float childAvailW = std::max(0.0f, availableWidth - node->padding.Horizontal());
            float sumH = node->padding.Vertical();
            uint32_t activeChildren = 0;

            for (Node* child = node->firstChild; child != nullptr; child = child->nextSibling) {
                if (child->isSpacer) continue;
                const float ch = ResolveNodeHeight(child, childAvailW);
                sumH += ch;
                ++activeChildren;
            }

            if (activeChildren > 1) {
                sumH += static_cast<float>(activeChildren - 1) * node->gap;
            }
            return sumH;
        }

        return node->intrinsicSize.y;
    }

    static void ArrangeNode(Node* node, Rect availableRect) {
        // Resolve this node's own width and height
        float w = availableRect.w;
        float h = availableRect.h;

        if (node->parent == nullptr && !node->isSpacer) {
            if (node->widthMode == SizeMode::Fit) {
                w = node->intrinsicSize.x;
            } else if (node->widthMode == SizeMode::Fixed) {
                w = node->fixedWidth;
            }

            if (node->heightMode == SizeMode::Fit) {
                h = (node->childCount == 0) ? node->intrinsicSize.y : ResolveNodeHeight(node, w);
            } else if (node->heightMode == SizeMode::Fixed) {
                h = node->fixedHeight;
            }
        }

        node->computedRect = Rect(availableRect.x, availableRect.y, w, h);

        if (node->childCount == 0) {
            return;
        }

        const Rect contentRect(
            node->computedRect.x + node->padding.left,
            node->computedRect.y + node->padding.top,
            std::max(0.0f, node->computedRect.w - node->padding.Horizontal()),
            std::max(0.0f, node->computedRect.h - node->padding.Vertical())
        );

        if (node->direction == FlexDirection::Row) {
            ArrangeRow(node, contentRect);
        } else if (node->direction == FlexDirection::Wrap) {
            ArrangeWrap(node, contentRect);
        } else {
            ArrangeColumn(node, contentRect);
        }
    }

    static void ArrangeRow(Node* node, const Rect& contentRect) {
        const float totalGap = (node->childCount > 1)
            ? static_cast<float>(node->childCount - 1) * node->gap
            : 0.0f;

        float baseSum = 0.0f;
        float totalFlexGrow = 0.0f;
        uint32_t flexCount = 0;

        for (Node* child = node->firstChild; child != nullptr; child = child->nextSibling) {
            if (child->isSpacer) {
                totalFlexGrow += std::max(0.01f, child->flexGrow);
                ++flexCount;
            } else if (child->widthMode == SizeMode::Stretch) {
                baseSum += child->intrinsicSize.x;
                totalFlexGrow += std::max(0.01f, child->flexGrow);
                ++flexCount;
            } else if (child->widthMode == SizeMode::Fixed) {
                baseSum += child->fixedWidth;
            } else { // SizeMode::Fit
                baseSum += child->intrinsicSize.x;
            }
        }

        const float freeSpace = std::max(0.0f, contentRect.w - totalGap - baseSum);
        float currentX = contentRect.x;

        // Main axis justification when there are no stretch elements
        if (flexCount == 0 && freeSpace > 0.0f) {
            if (node->justifyMain == Justify::Center) {
                currentX += freeSpace * 0.5f;
            } else if (node->justifyMain == Justify::End) {
                currentX += freeSpace;
            }
        }

        for (Node* child = node->firstChild; child != nullptr; child = child->nextSibling) {
            float childW = 0.0f;
            if (child->isSpacer) {
                childW = (totalFlexGrow > 0.0f)
                    ? (freeSpace * (std::max(0.01f, child->flexGrow) / totalFlexGrow))
                    : 0.0f;
            } else if (child->widthMode == SizeMode::Stretch) {
                const float flexW = (totalFlexGrow > 0.0f)
                    ? (freeSpace * (std::max(0.01f, child->flexGrow) / totalFlexGrow))
                    : 0.0f;
                childW = child->intrinsicSize.x + flexW;
            } else if (child->widthMode == SizeMode::Fixed) {
                childW = child->fixedWidth;
            } else { // SizeMode::Fit
                childW = child->intrinsicSize.x;
            }

            float childH = contentRect.h;
            if (child->heightMode == SizeMode::Fixed) {
                childH = child->fixedHeight;
            } else if (child->heightMode == SizeMode::Fit || node->alignCross != Align::Stretch) {
                childH = child->intrinsicSize.y;
            }

            float childY = contentRect.y;
            if (node->alignCross == Align::Center) {
                childY = contentRect.y + (contentRect.h - childH) * 0.5f;
            } else if (node->alignCross == Align::End) {
                childY = contentRect.Bottom() - childH;
            }

            ArrangeNode(child, Rect(currentX, childY, childW, childH));
            currentX += childW + node->gap;
        }
    }

    static void ArrangeWrap(Node* node, const Rect& contentRect) {
        const float maxLineWidth = std::max(0.0f, contentRect.w);
        Node* lineStart = node->firstChild;
        float currentY = contentRect.y;
        bool hasLines = false;

        while (lineStart != nullptr) {
            Node* lineEnd = lineStart;
            float currentLineWidth = 0.0f;
            float lineMaxH = 0.0f;
            uint32_t itemsInLine = 0;

            while (lineEnd != nullptr) {
                if (lineEnd->isSpacer) {
                    lineEnd = lineEnd->nextSibling;
                    continue;
                }

                const float cw = (lineEnd->widthMode == SizeMode::Fixed) ? lineEnd->fixedWidth : lineEnd->intrinsicSize.x;
                const float ch = (lineEnd->heightMode == SizeMode::Fixed) ? lineEnd->fixedHeight : lineEnd->intrinsicSize.y;
                const float neededWidth = (itemsInLine > 0) ? (node->gap + cw) : cw;

                if (itemsInLine > 0 && currentLineWidth + neededWidth > maxLineWidth) {
                    break;
                }

                currentLineWidth += neededWidth;
                lineMaxH = std::max(lineMaxH, ch);
                ++itemsInLine;
                lineEnd = lineEnd->nextSibling;
            }

            if (itemsInLine == 0) {
                break;
            }

            if (hasLines) {
                currentY += node->runGap;
            }

            // Main axis justification for this line
            float itemX = contentRect.x;
            float itemGap = node->gap;
            const float freeLineWidth = std::max(0.0f, maxLineWidth - currentLineWidth);

            if (node->justifyMain == Justify::Center) {
                itemX += freeLineWidth * 0.5f;
            } else if (node->justifyMain == Justify::End) {
                itemX += freeLineWidth;
            } else if (node->justifyMain == Justify::SpaceBetween && itemsInLine > 1) {
                itemGap += freeLineWidth / static_cast<float>(itemsInLine - 1);
            }

            // Place items along this line
            for (Node* child = lineStart; child != lineEnd; child = child->nextSibling) {
                if (child->isSpacer) continue;

                const float cw = (child->widthMode == SizeMode::Fixed) ? child->fixedWidth : child->intrinsicSize.x;
                float ch = (child->heightMode == SizeMode::Fixed) ? child->fixedHeight : child->intrinsicSize.y;

                if (node->alignCross == Align::Stretch && child->heightMode == SizeMode::Stretch) {
                    ch = lineMaxH;
                }

                float childY = currentY;
                if (node->alignCross == Align::Center) {
                    childY = currentY + (lineMaxH - ch) * 0.5f;
                } else if (node->alignCross == Align::End) {
                    childY = currentY + (lineMaxH - ch);
                }

                ArrangeNode(child, Rect(itemX, childY, cw, ch));
                itemX += cw + itemGap;
            }

            currentY += lineMaxH;
            hasLines = true;
            lineStart = lineEnd;
        }

        if (node->heightMode == SizeMode::Fit) {
            node->computedRect.h = (currentY - contentRect.y) + node->padding.Vertical();
        }
    }

    static void ArrangeColumn(Node* node, const Rect& contentRect) {
        const float totalGap = (node->childCount > 1)
            ? static_cast<float>(node->childCount - 1) * node->gap
            : 0.0f;

        float baseSum = 0.0f;
        float totalFlexGrow = 0.0f;
        uint32_t flexCount = 0;

        for (Node* child = node->firstChild; child != nullptr; child = child->nextSibling) {
            if (child->isSpacer) {
                totalFlexGrow += std::max(0.01f, child->flexGrow);
                ++flexCount;
            } else if (child->heightMode == SizeMode::Stretch) {
                baseSum += child->intrinsicSize.y;
                totalFlexGrow += std::max(0.01f, child->flexGrow);
                ++flexCount;
            } else if (child->heightMode == SizeMode::Fixed) {
                baseSum += child->fixedHeight;
            } else { // SizeMode::Fit
                baseSum += ResolveNodeHeight(child, contentRect.w);
            }
        }

        const float freeSpace = std::max(0.0f, contentRect.h - totalGap - baseSum);
        float currentY = contentRect.y;

        // Main axis justification when there are no stretch elements
        if (flexCount == 0 && freeSpace > 0.0f) {
            if (node->justifyMain == Justify::Center) {
                currentY += freeSpace * 0.5f;
            } else if (node->justifyMain == Justify::End) {
                currentY += freeSpace;
            }
        }

        for (Node* child = node->firstChild; child != nullptr; child = child->nextSibling) {
            float childH = 0.0f;
            if (child->isSpacer) {
                childH = (totalFlexGrow > 0.0f)
                    ? (freeSpace * (std::max(0.01f, child->flexGrow) / totalFlexGrow))
                    : 0.0f;
            } else if (child->heightMode == SizeMode::Stretch) {
                if (node->childCount == 1 && (node->fixedHeight > 0.0f || node->heightMode == SizeMode::Fixed)) {
                    childH = contentRect.h;
                } else {
                    const float flexH = (totalFlexGrow > 0.0f)
                        ? (freeSpace * (std::max(0.01f, child->flexGrow) / totalFlexGrow))
                        : 0.0f;
                    childH = child->intrinsicSize.y + flexH;
                }
            } else if (child->heightMode == SizeMode::Fixed) {
                childH = child->fixedHeight;
            } else { // SizeMode::Fit
                childH = ResolveNodeHeight(child, contentRect.w);
            }

            float childW = contentRect.w;
            if (child->widthMode == SizeMode::Fixed) {
                childW = child->fixedWidth;
            } else if (child->widthMode == SizeMode::Fit || node->alignCross != Align::Stretch) {
                childW = child->intrinsicSize.x;
            }

            float childX = contentRect.x;
            if (node->alignCross == Align::Center) {
                childX = contentRect.x + (contentRect.w - childW) * 0.5f;
            } else if (node->alignCross == Align::End) {
                childX = contentRect.Right() - childW;
            }

            ArrangeNode(child, Rect(childX, currentY, childW, childH));
            currentY += child->computedRect.h + node->gap;
        }

        if (node->heightMode == SizeMode::Fit) {
            node->computedRect.h = (currentY > contentRect.y)
                ? (currentY - contentRect.y - node->gap) + node->padding.Vertical()
                : node->padding.Vertical();
        }
    }
};

} // namespace kvant
