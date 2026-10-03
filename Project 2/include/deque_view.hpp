#pragma once

#include <cstddef>
#include <iosfwd>
#include <string>

using std::ostream;
using std::size_t;
using std::string;

// Describes the deque operation being presented, independently of terminal details.
enum class DequeOperation {
    PushFront, PushBack, PopFront, PopBack, PeekFront, PeekBack, Clear
};

// Abstract presentation interface. Applications use it through a reference.
class DequeView {
public:
    virtual ~DequeView() = default;

    // Display the main deque, result, commands, and prompt. Zero columns selects the width.
    virtual void draw(ostream& output, const string& message = "",
                      size_t columns = 0) const = 0;

    // Present a pending operation without changing the main deque.
    // pending_value is used only for a push; output and timing belong to the view.
    virtual void animate(ostream& output, DequeOperation operation,
                         int pending_value = 0) const = 0;
};
