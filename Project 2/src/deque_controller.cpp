#include "deque_controller.hpp"

using std::deque;
using std::size_t;

DequeController::DequeController(deque<int>& main_deque)
    : main_deque_(main_deque) {}

void DequeController::push_front(int value) {
    main_deque_.push_front(value);
}

void DequeController::push_back(int value) {
    main_deque_.push_back(value);
}

bool DequeController::pop_front(int& removed_value) {
    if (main_deque_.empty()) {
        return false;
    }
    removed_value = main_deque_.front();
    main_deque_.pop_front();
    return true;
}

bool DequeController::pop_back(int& removed_value) {
    if (main_deque_.empty()) {
        return false;
    }
    removed_value = main_deque_.back();
    main_deque_.pop_back();
    return true;
}

bool DequeController::peek_front(int& value) const {
    if (main_deque_.empty()) {
        return false;
    }
    value = main_deque_.front();
    return true;
}

bool DequeController::peek_back(int& value) const {
    if (main_deque_.empty()) {
        return false;
    }
    value = main_deque_.back();
    return true;
}

void DequeController::clear() {
    main_deque_.clear();
}

size_t DequeController::size() const {
    return main_deque_.size();
}

bool DequeController::empty() const {
    return main_deque_.empty();
}
