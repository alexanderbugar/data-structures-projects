#include "terminal_view.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#if defined(__unix__) || defined(__APPLE__)
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace chrono = std::chrono;
using std::deque;
using std::flush;
using std::getenv;
using std::lround;
using std::max;
using std::min;
using std::ostream;
using std::ostringstream;
using std::size_t;
using std::string;
namespace this_thread = std::this_thread;
using std::to_string;
using std::vector;

namespace {
// Terminal dimensions and control codes are private to this implementation.
struct TerminalSize {
    size_t columns = 80;
    size_t rows = 24;
};

// Ordinary IDE consoles and redirected streams get plain, immediate output.
bool interactive_terminal() {
#if defined(__unix__) || defined(__APPLE__)
    const char* terminal_type = getenv("TERM");
    return isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)
        && terminal_type != nullptr && terminal_type[0] != '\0'
        && string(terminal_type) != "dumb";
#else
    return false;
#endif
}

// Query dimensions again each frame so a resize does not leave old text behind.
TerminalSize terminal_size(bool interactive) {
    TerminalSize size;
#if defined(__unix__) || defined(__APPLE__)
    struct winsize dimensions = {};
    if (interactive && ioctl(STDOUT_FILENO, TIOCGWINSZ, &dimensions) == 0) {
        if (dimensions.ws_col != 0) size.columns = dimensions.ws_col;
        if (dimensions.ws_row != 0) size.rows = dimensions.ws_row;
    }
#else
    (void)interactive;
#endif
    return size;
}

// Center text in a fixed-width cell without truncating an integer.
string centered(const string& text, size_t width) {
    const size_t padding = width > text.size() ? width - text.size() : 0;
    return string(padding / 2, ' ') + text
        + string(padding - padding / 2, ' ');
}

// Paint into a text frame with clipping; this buffer contains characters, not a deque.
void paint_text(vector<string>& frame, int x, int y, const string& text) {
    if (y < 0 || y >= static_cast<int>(frame.size())) return;
    string& row = frame[static_cast<size_t>(y)];
    for (size_t offset = 0; offset < text.size(); ++offset) {
        const int column = x + static_cast<int>(offset);
        if (column >= 0 && column < static_cast<int>(row.size())) {
            row[static_cast<size_t>(column)] = text[offset];
        }
    }
}

// Draw one box, its current logical index, and an optional end/highlight label.
void paint_box(vector<string>& frame, int x, int y, size_t inner_width,
               int value, const string& label, const string& index,
               bool highlight = false) {
    const string border = "+" + string(inner_width, highlight ? '=' : '-') + "+";
    paint_text(frame, x, y - 1, centered(label, inner_width + 2));
    paint_text(frame, x, y, border);
    paint_text(frame, x, y + 1, "|" + centered(to_string(value), inner_width) + "|");
    paint_text(frame, x, y + 2, border);
    paint_text(frame, x, y + 3, centered(index, inner_width + 2));
}

// Convert animation intent to the actual STL method being demonstrated.
const char* method_name(DequeOperation operation) {
    switch (operation) {
    case DequeOperation::PushFront: return "push_front";
    case DequeOperation::PushBack: return "push_back";
    case DequeOperation::PopFront: return "pop_front";
    case DequeOperation::PopBack: return "pop_back";
    case DequeOperation::PeekFront: return "front";
    case DequeOperation::PeekBack: return "back";
    case DequeOperation::Clear: return "clear";
    }
    return "";
}

// Measure the live deque and optionally its pending push value; no value snapshot is made.
size_t cell_width(const deque<int>& values, bool pushing = false, int pending = 0) {
    size_t width = 6;
    for (int value : values) width = max(width, to_string(value).size() + 2);
    if (pushing) width = max(width, to_string(pending).size() + 2);
    if (!values.empty()) width = max(width, to_string(values.size() - 1).size() + 2);
    return width;
}

// Leave a row below the prompt for the terminal's normal input echo and newline.
bool fits_panel(const TerminalSize& size) {
    return size.columns >= 40 && size.rows >= 18;
}

// Eight footer rows are shared by every frame, including the last command result.
vector<string> footer(const string& message) {
    return {
        "Indices below. F/B = front + back.",
        "Result: " + message,
        "push_front <int> | push_back <int>",
        "pop_front        | pop_back",
        "peek_front       | peek_back",
        "clear | size | empty | show | help | quit",
        "FIFO shortcuts: push <int>, pop, peek",
        "Command > "
    };
}

// Fit a screen row without triggering terminal wrapping; plain output stays unabridged.
string screen_line(string text, size_t width) {
    if (text.size() > width) text = text.substr(0, width - 3) + "...";
    return text;
}

}

TerminalView::TerminalView(const deque<int>& main_deque, bool plain, bool animations)
    : main_deque_(main_deque), interactive_(!plain && interactive_terminal()),
      animations_(interactive_ && animations) {}

void TerminalView::draw(ostream& output, const string& message,
                        size_t columns) const {
    message_ = message;
    TerminalSize dimensions = terminal_size(interactive_);
    if (columns != 0) dimensions.columns = columns;
    if (!interactive_ || !fits_panel(dimensions)) {
        draw_plain(output, dimensions.columns);
        return;
    }

    const size_t width = cell_width(main_deque_);
    const size_t stride = width + 5;
    const size_t visible = min(main_deque_.size(), (dimensions.columns - 1) / stride);
    const size_t first = focus_back_ ? main_deque_.size() - visible : 0;
    const size_t height = min<size_t>(7, dimensions.rows - 13);
    const int baseline = height >= 6 ? 2 : 1;
    vector<string> canvas(height, string(dimensions.columns - 1, ' '));
    if (main_deque_.empty()) {
        paint_text(canvas, 2, baseline + 1, "FRONT -> [ empty deque ] <- BACK");
    }
    for (size_t offset = 0; offset < visible; ++offset) {
        const size_t index = first + offset;
        string label;
        if (index == 0) label = "FRONT";
        if (index == main_deque_.size() - 1) label = label.empty() ? "BACK" : "F/B";
        paint_box(canvas, 2 + static_cast<int>(offset * stride), baseline, width,
                  main_deque_[index], label, to_string(index));
    }
    present(output, canvas, "Live deque; indices below.", first, visible, dimensions.columns);
}

void TerminalView::draw_plain(ostream& output, size_t columns) const {
    output << "\nDEQUE PLAYGROUND\n"
           << "Size: " << main_deque_.size()
           << " | Empty: " << (main_deque_.empty() ? "true" : "false") << '\n'
           << "Live deque; indices below.\n\n";
    if (main_deque_.empty()) {
        output << "FRONT -> [ empty deque ] <- BACK\n";
    } else {
        const size_t width = cell_width(main_deque_);
        const size_t stride = width + 5;
        columns = max(columns, width + 4);
        const size_t per_row = max<size_t>(1, (columns - 1) / stride);
        for (size_t start = 0; start < main_deque_.size();) {
            const size_t count = min(per_row, main_deque_.size() - start);
            vector<string> canvas(7, string(columns, ' '));
            for (size_t offset = 0; offset < count; ++offset) {
                const size_t index = start + offset;
                string label;
                if (index == 0) label = "FRONT";
                if (index == main_deque_.size() - 1) label = label.empty() ? "BACK" : "F/B";
                paint_box(canvas, 2 + static_cast<int>(offset * stride), 2, width,
                          main_deque_[index], label, to_string(index));
            }
            for (const string& row : canvas) output << row << '\n';
            start += count;
        }
    }
    const auto menu = footer(message_);
    for (size_t index = 0; index < menu.size(); ++index) {
        output << menu[index];
        if (index + 1 < menu.size()) output << '\n';
    }
    output << flush;
}

void TerminalView::present(ostream& output, const vector<string>& canvas,
                           const string& status, size_t first,
                           size_t visible, size_t columns) const {
    ostringstream size;
    size << "Size: " << main_deque_.size()
         << " | Empty: " << (main_deque_.empty() ? "true" : "false");
    ostringstream viewport;
    if (visible < main_deque_.size()) {
        viewport << "Visible: ";
        if (visible != 0) viewport << first << '-' << first + visible - 1;
        else viewport << "none";
        viewport << " of " << main_deque_.size() << " elements.";
    } else {
        viewport << "All elements visible.";
    }
    vector<string> lines = {"DEQUE PLAYGROUND", size.str(), status, viewport.str()};
    lines.insert(lines.end(), canvas.begin(), canvas.end());
    const auto menu = footer(message_);
    lines.insert(lines.end(), menu.begin(), menu.end());

    // Clear each row in place. Keep the prompt above the bottom row to prevent scrolling.
    output << "\033[H";
    for (size_t index = 0; index < lines.size(); ++index) {
        output << "\033[2K" << screen_line(lines[index], columns - 1);
        if (index + 1 < lines.size()) output << '\n';
    }
    output << "\033[J" << flush;
}

void TerminalView::animate(ostream& output, DequeOperation operation,
                           int pending_value) const {
    focus_back_ = operation == DequeOperation::PushBack
        || operation == DequeOperation::PopBack || operation == DequeOperation::PeekBack;
    if (!animations_) return;
    const bool pushing = operation == DequeOperation::PushFront
        || operation == DequeOperation::PushBack;
    if (!pushing && main_deque_.empty()) return;

    TerminalSize dimensions = terminal_size(interactive_);
    // A cramped screen still performs the operation, using the static display.
    if (!fits_panel(dimensions)) return;

    const auto start = chrono::steady_clock::now();
    for (int frame = 0; frame <= 10; ++frame) {
        // Sleeping avoids consuming a CPU while waiting for the next frame.
        this_thread::sleep_until(start + chrono::milliseconds(frame * 50));
        const double elapsed = chrono::duration<double>(
            chrono::steady_clock::now() - start).count();
        const double progress = min(1.0, elapsed / 0.5);
        dimensions = terminal_size(interactive_);
        if (!fits_panel(dimensions)) return;
        draw_animation_frame(output, operation, pending_value, progress,
                             dimensions.columns, dimensions.rows);
        if (progress >= 1.0) return;
    }
}

void TerminalView::draw_animation_frame(ostream& output, DequeOperation operation,
                                      int pending_value, double progress,
                                      size_t columns, size_t rows) const {
    const bool pushing = operation == DequeOperation::PushFront
        || operation == DequeOperation::PushBack;
    const bool popping = operation == DequeOperation::PopFront
        || operation == DequeOperation::PopBack;
    const bool peeking = operation == DequeOperation::PeekFront
        || operation == DequeOperation::PeekBack;
    const bool back = operation == DequeOperation::PushBack
        || operation == DequeOperation::PopBack || operation == DequeOperation::PeekBack;
    const bool clearing = operation == DequeOperation::Clear;

    const size_t inner_width = cell_width(main_deque_, pushing, pending_value);
    const int box_width = static_cast<int>(inner_width + 2);
    const int stride = box_width + 3;
    const size_t slots = (columns - 1) / static_cast<size_t>(stride);
    const size_t visible = min(main_deque_.size(), slots - (pushing ? 1 : 0));
    const size_t first = back ? main_deque_.size() - visible : 0;
    const int origin = 2;
    const size_t height = min<size_t>(7, rows - 13);
    const int baseline = height >= 6 ? 2 : 1;
    const double eased = progress * progress * (3.0 - 2.0 * progress);

    // A character grid is the only frame buffer. Each box reads its value from main_deque_.
    vector<string> canvas(height, string(columns - 1, ' '));
    for (size_t offset = 0; offset < visible; ++offset) {
        const size_t index = first + offset;
        const bool selected = !main_deque_.empty()
            && index == (back ? main_deque_.size() - 1 : 0);
        int x = origin + static_cast<int>(offset) * stride;
        int y = baseline;
        string label;
        if (index == 0) label = "FRONT";
        if (index == main_deque_.size() - 1) label = label.empty() ? "BACK" : "F/B";

        if (operation == DequeOperation::PushFront) {
            x += static_cast<int>(lround(eased * stride));
        } else if (popping && selected) {
            x += static_cast<int>(lround(eased * stride * (back ? 1 : -1)));
            y += static_cast<int>(lround(eased * (height + 1)));
            label = "OUT";
        } else if (operation == DequeOperation::PopFront) {
            x -= static_cast<int>(lround(eased * stride));
        } else if (clearing) {
            y += static_cast<int>(lround(eased * (height + 1)));
        }

        const bool highlight = peeking && selected && progress >= 0.1 && progress < 0.9;
        if (highlight) label = "PEEK";
        paint_box(canvas, x, y, inner_width, main_deque_[index], label,
                  to_string(index), highlight);
    }

    if (pushing) {
        const int target = origin + (back ? static_cast<int>(visible) * stride : 0);
        // Enter horizontally, keeping a full stride between the new box and its neighbour.
        const int entrance = back ? static_cast<int>(columns) : origin - stride;
        const int x = static_cast<int>(lround(entrance + (target - entrance) * eased));
        paint_box(canvas, x, baseline, inner_width, pending_value, "NEW", "pending");
    }

    ostringstream status;
    status << "Pending: " << method_name(operation) << '(';
    if (pushing) status << pending_value;
    status << ')';
    present(output, canvas, status.str(), first, visible, columns);
}
