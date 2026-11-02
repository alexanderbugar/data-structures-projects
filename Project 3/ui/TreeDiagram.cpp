#include "TreeDiagram.h"

#include <algorithm>
#include <sstream>

using namespace ftxui;
using namespace std;

namespace tree_ui {
namespace {
// Wrap by terminal cells, including long words and wide Unicode characters.
vector<string> wrap(const string& value, int width) {
    vector<string> lines;
    string line, word;
    istringstream words(value);
    while (words >> word) {
        if (!line.empty() && string_width(line + " " + word) <= width) {
            line += " " + word;
            continue;
        }
        if (!line.empty()) lines.push_back(line);
        line.clear();
        for (const auto& glyph : Utf8ToGlyphs(word)) {
            if (string_width(line + glyph) > width) {
                lines.push_back(line);
                line.clear();
            }
            line += glyph;
        }
    }
    if (!line.empty()) lines.push_back(line);
    return lines;
}

struct NodeBox {
    int x = 0, y = 0, width = 0, height = 0, span = 0, depth = 0;
    vector<string> lines;
};

class Diagram : public ftxui::Node {
public:
    Diagram(const vector<LinkedTree::NodeInfo>& nodes, int selected, int panX, int panY)
        : nodes(nodes), selected(selected), panX(panX), panY(panY) {}

    // Let the diagram use whatever space is left beside the controls.
    void ComputeRequirement() override {
        requirement_.min_x = 10;
        requirement_.min_y = 5;
        requirement_.flex_grow_x = requirement_.flex_grow_y = 1;
        requirement_.flex_shrink_x = requirement_.flex_shrink_y = 1;
    }

    // Measure each box, reserve space for its subtree, then connect the boxes.
    void Render(Screen& screen) override {
        if (nodes.empty()) return;
        const auto muted = Color::RGB(100, 116, 139);
        const auto accent = Color::RGB(232, 164, 116);
        const auto leaf = Color::RGB(117, 205, 172);
        int viewWidth = box_.x_max - box_.x_min + 1;
        int viewHeight = box_.y_max - box_.y_min + 1;
        if (viewWidth <= 0 || viewHeight <= 0) return;
        vector<NodeBox> boxes(nodes.size());
        int maxDepth = 0;
        for (size_t i = 0; i < nodes.size(); ++i) {
            if (nodes[i].parent >= 0) boxes[i].depth = boxes[nodes[i].parent].depth + 1;
            maxDepth = max(maxDepth, boxes[i].depth);
        }
        int limit = clamp(viewWidth / (maxDepth + 1) - 6, 18, 34);
        vector<int> widths(maxDepth + 1), columns(maxDepth + 1);
        for (size_t i = 0; i < nodes.size(); ++i) {
            auto& b = boxes[i];
            string heading = "#" + to_string(nodes[i].position);
            if (nodes[i].parent >= 0) heading += " [" + nodes[i].edgeLabel + "]";
            b.width = clamp(max(string_width(heading), string_width(nodes[i].content)) + 4, 16, limit);
            b.lines = wrap(heading, b.width - 4);
            auto content = wrap(nodes[i].content, b.width - 4);
            b.lines.insert(b.lines.end(), content.begin(), content.end());
            b.height = static_cast<int>(b.lines.size()) + 2;
            widths[b.depth] = max(widths[b.depth], b.width);
        }
        for (int d = 1; d <= maxDepth; ++d) columns[d] = columns[d - 1] + widths[d - 1] + 6;
        // Children come after their parents in preorder, so measure from the end.
        for (int i = static_cast<int>(nodes.size()) - 1; i >= 0; --i) {
            int childrenHeight = -2;
            for (int child : nodes[i].children) childrenHeight += boxes[child].span + 2;
            boxes[i].span = max(boxes[i].height, childrenHeight);
        }
        vector<int> tops(nodes.size());
        for (size_t i = 0; i < nodes.size(); ++i) {
            auto& b = boxes[i];
            b.x = columns[b.depth];
            b.y = tops[i] + (b.span - b.height) / 2;
            int total = -2;
            for (int child : nodes[i].children) total += boxes[child].span + 2;
            int top = tops[i] + (b.span - max(0, total)) / 2;
            for (int child : nodes[i].children) {
                tops[child] = top;
                top += boxes[child].span + 2;
            }
        }
        int fullWidth = columns.back() + widths.back();
        const auto& current = boxes[selected];
        int offsetX = clamp(current.x + current.width / 2 - viewWidth / 2 + panX,
                            0, max(0, fullWidth - viewWidth));
        int offsetY = clamp(current.y + current.height / 2 - viewHeight / 2 + panY,
                            0, max(0, boxes[0].span - viewHeight));
        vector<bool> path(nodes.size());
        for (int i = selected; i >= 0; i = nodes[i].parent) path[i] = true;

        // Clip drawing to the viewport so large trees do not need a large screen buffer.
        auto put = [&](int x, int y, const string& glyph, Color ink) {
            x += box_.x_min - offsetX;
            y += box_.y_min - offsetY;
            if (x < box_.x_min || x > box_.x_max || y < box_.y_min || y > box_.y_max) return;
            auto& cell = screen.PixelAt(x, y);
            cell.character = glyph;
            cell.foreground_color = ink;
        };
        auto horizontal = [&](int x1, int x2, int y, Color ink) {
            for (int x = max(x1, offsetX); x <= min(x2, offsetX + viewWidth - 1); ++x)
                put(x, y, "─", ink);
        };
        for (size_t i = 0; i < nodes.size(); ++i) {
            if (nodes[i].children.empty()) continue;
            const auto& b = boxes[i];
            int middle = b.y + b.height / 2;
            int junction = b.x + widths[b.depth] + 2;
            const auto& first = boxes[nodes[i].children.front()];
            const auto& last = boxes[nodes[i].children.back()];
            int top = min(middle, first.y + first.height / 2);
            int bottom = max(middle, last.y + last.height / 2);
            // Join only the sides that have an edge, then highlight the chosen path.
            const char* joins[] = {" ", "│", "│", "│", "─", "╯", "╮", "┤",
                                   "─", "╰", "╭", "├", "─", "┴", "┬", "┼"};
            for (int pass = 0; pass < 2; ++pass) {
                for (int child : nodes[i].children) {
                    if (pass == 1 && !path[child]) continue;
                    const auto& c = boxes[child];
                    int cy = c.y + c.height / 2;
                    Color ink = pass == 1 ? accent : muted;
                    horizontal(b.x + b.width, junction, middle, ink);
                    for (int y = max(min(middle, cy), offsetY); y <= min(max(middle, cy), offsetY + viewHeight - 1); ++y)
                        put(junction, y, "│", ink);
                    horizontal(junction, c.x - 1, cy, ink);
                }
            }
            bool straight = false;
            for (int child : nodes[i].children) {
                const auto& c = boxes[child];
                int cy = c.y + c.height / 2;
                straight = straight || cy == middle;
                int mask = (cy > top ? 1 : 0) | (cy < bottom ? 2 : 0) | (cy == middle ? 4 : 0) | 8;
                put(junction, cy, joins[mask], path[child] ? accent : muted);
            }
            int mask = (middle > top ? 1 : 0) | (middle < bottom ? 2 : 0) | 4 | (straight ? 8 : 0);
            put(junction, middle, joins[mask], path[i] ? accent : muted);
        }
        for (size_t i = 0; i < nodes.size(); ++i) {
            const auto& b = boxes[i];
            if (b.y + b.height <= offsetY || b.y >= offsetY + viewHeight ||
                b.x + b.width <= offsetX || b.x >= offsetX + viewWidth) continue;
            Color ink = path[i] ? accent : nodes[i].children.empty() ? leaf : muted;
            horizontal(b.x + 1, b.x + b.width - 2, b.y, ink);
            horizontal(b.x + 1, b.x + b.width - 2, b.y + b.height - 1, ink);
            for (int y = 1; y < b.height - 1; ++y) {
                put(b.x, b.y + y, "│", ink);
                put(b.x + b.width - 1, b.y + y, "│", ink);
            }
            put(b.x, b.y, "╭", ink);
            put(b.x + b.width - 1, b.y, "╮", ink);
            put(b.x, b.y + b.height - 1, "╰", ink);
            put(b.x + b.width - 1, b.y + b.height - 1, "╯", ink);
            if (nodes[i].parent >= 0) put(b.x, b.y + b.height / 2, "┤", ink);
            if (!nodes[i].children.empty()) put(b.x + b.width - 1, b.y + b.height / 2, "├", ink);
            for (size_t line = 0; line < b.lines.size(); ++line) {
                int x = b.x + 2;
                for (const auto& glyph : Utf8ToGlyphs(b.lines[line])) {
                    put(x, b.y + 1 + static_cast<int>(line), glyph,
                        i == static_cast<size_t>(selected) ? Color::White : ink);
                    int cells = string_width(glyph);
                    for (int k = 1; k < cells; ++k) put(x + k, b.y + 1 + static_cast<int>(line), "", ink);
                    x += cells;
                }
            }
        }
    }

private:
    const vector<LinkedTree::NodeInfo>& nodes;
    int selected, panX, panY;
};
}

Element diagram(const vector<LinkedTree::NodeInfo>& nodes, int selected, int panX, int panY) {
    return make_shared<Diagram>(nodes, selected, panX, panY);
}
}
