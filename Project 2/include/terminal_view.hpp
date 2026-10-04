#pragma once

#include "deque_view.hpp"

#include <cstddef>
#include <deque>
#include <iosfwd>
#include <string>
#include <vector>

using std::deque;
using std::ostream;
using std::size_t;
using std::string;
using std::vector;

// Implements the abstract view using ASCII boxes and optional ANSI animation.
// Values are always read through a const reference to the application's deque.
class TerminalView : public DequeView {
public:
    explicit TerminalView(const deque<int>& main_deque,
                          bool plain = false, bool animations = true);

    // Keep an interactive stage fixed in place; plain output wraps all boxes.
    void draw(ostream& output, const string& message = "",
              size_t columns = 0) const override;

    // Animate before the controller commits its operation. Plain output skips this.
    void animate(ostream& output, DequeOperation operation,
                 int pending_value = 0) const override;

private:
    // Plain output can list every element without a screen-height restriction.
    void draw_plain(ostream& output, size_t columns) const;

    // Static and animated states share the same header, stage, menu, and prompt rows.
    void present(ostream& output, const vector<string>& canvas,
                 const string& status, size_t first, size_t visible,
                 size_t columns) const;

    // Compose one text frame, reading the live deque rather than a value snapshot.
    void draw_animation_frame(ostream& output, DequeOperation operation,
                              int pending_value, double progress,
                              size_t columns, size_t rows) const;

    const deque<int>& main_deque_;
    bool interactive_;
    bool animations_;
    // Presentation metadata only; deque contents are always read from main_deque_.
    mutable string message_;
    mutable bool focus_back_ = false;
};
