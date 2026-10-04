#pragma once

#include <cstddef>
#include <deque>

using std::deque;
using std::size_t;

// Operates on the deque supplied by main; this class never owns a second deque.
class DequeController {
public:
    explicit DequeController(deque<int>& main_deque);

    // Add an integer at the chosen end using the corresponding STL method.
    void push_front(int value);
    void push_back(int value);

    // Remove and return an end value; return false if the deque is empty.
    bool pop_front(int& removed_value);
    bool pop_back(int& removed_value);

    // Inspect an end value without removing it; return false if empty.
    bool peek_front(int& value) const;
    bool peek_back(int& value) const;

    // Remove all elements or query the current deque properties.
    void clear();
    size_t size() const;
    bool empty() const;

private:
    deque<int>& main_deque_;
};
