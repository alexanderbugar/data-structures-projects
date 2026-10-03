#include "deque_application.hpp"

#include <iostream>
#include <limits>
#include <sstream>

using std::deque;
using std::getline;
using std::istream;
using std::istringstream;
using std::numeric_limits;
using std::ostream;
using std::string;
using std::to_string;

namespace {
// Parse one signed decimal integer, rejecting partial tokens and overflow.
bool parse_integer(const string& token, int& value) {
    istringstream number(token);
    number >> value;
    return !number.fail() && number.eof();
}
}

DequeApplication::DequeApplication(deque<int>& main_deque, DequeView& view)
    : controller_(main_deque), view_(view) {}

void DequeApplication::run(istream& input, ostream& output) {
    string message = "Ready. Add an integer at either end.";
    for (;;) {
        view_.draw(output, message);

        string line;
        if (!getline(input, line)) {
            output << "\nInput closed. Goodbye.\n";
            return;
        }
        if (!execute(line, message, output)) {
            output << "Goodbye.\n";
            return;
        }
    }
}

bool DequeApplication::execute(const string& line, string& message,
                               ostream& output) {
    istringstream command_line(line);
    string command;
    if (!(command_line >> command)) {
        message = "Enter a command, or type help.";
        return true;
    }

    // Shortcuts use conventional FIFO behaviour, while explicit commands expose both ends.
    if (command == "push") command = "push_back";
    if (command == "pop") command = "pop_front";
    if (command == "peek") command = "peek_front";

    if (command == "push_front" || command == "push_back") {
        string token;
        string extra;
        int value;
        if (!(command_line >> token) || (command_line >> extra)
            || !parse_integer(token, value)) {
            message = "Error: " + command + " requires exactly one integer ("
                + to_string(numeric_limits<int>::min()) + " to "
                + to_string(numeric_limits<int>::max()) + ").";
            return true;
        }
        // The view presents the pending change; only the controller mutates the deque.
        if (command == "push_front") {
            view_.animate(output, DequeOperation::PushFront, value);
            controller_.push_front(value);
        } else {
            view_.animate(output, DequeOperation::PushBack, value);
            controller_.push_back(value);
        }
        message = "deque::" + command + "(" + to_string(value) + ") completed.";
        return true;
    }

    const bool known = command == "pop_front" || command == "pop_back"
        || command == "peek_front" || command == "peek_back" || command == "clear"
        || command == "size" || command == "empty" || command == "show"
        || command == "help" || command == "quit";
    if (!known) {
        message = "Error: unknown command '" + command + "'. Type help.";
        return true;
    }
    string extra;
    if (command_line >> extra) {
        message = "Error: " + command + " takes no arguments.";
        return true;
    }

    if (command == "quit") return false;
    if (command == "clear") {
        if (!controller_.empty()) view_.animate(output, DequeOperation::Clear);
        controller_.clear();
        message = "deque::clear() completed.";
    } else if (command == "size") {
        message = "deque::size() = " + to_string(controller_.size());
    } else if (command == "empty") {
        message = string("deque::empty() = ")
            + (controller_.empty() ? "true" : "false");
    } else if (command == "show") {
        message = "Showing the current main deque.";
    } else if (command == "help") {
        message = "Use the commands below. Peek reads front()/back() without removing. "
                  "Push/pop/peek shortcuts use the back/front/front respectively.";
    } else {
        // Guard before animation as well as before the underlying STL end access.
        if (controller_.empty()) {
            message = "Error: cannot " + command + " on an empty deque.";
            return true;
        }
        int value;
        bool succeeded;
        if (command == "pop_front") {
            view_.animate(output, DequeOperation::PopFront);
            succeeded = controller_.pop_front(value);
        } else if (command == "pop_back") {
            view_.animate(output, DequeOperation::PopBack);
            succeeded = controller_.pop_back(value);
        } else if (command == "peek_front") {
            view_.animate(output, DequeOperation::PeekFront);
            succeeded = controller_.peek_front(value);
        } else {
            view_.animate(output, DequeOperation::PeekBack);
            succeeded = controller_.peek_back(value);
        }

        if (!succeeded) {
            message = "Error: cannot " + command + " on an empty deque.";
        } else if (command == "peek_front" || command == "peek_back") {
            message = string("deque::")
                + (command == "peek_front" ? "front() = " : "back() = ")
                + to_string(value) + " (deque unchanged).";
        } else {
            message = "deque::" + command + "() removed " + to_string(value) + ".";
        }
    }
    return true;
}
