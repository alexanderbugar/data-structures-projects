#pragma once

#include "deque_controller.hpp"
#include "deque_view.hpp"

#include <deque>
#include <iosfwd>
#include <string>

using std::deque;
using std::istream;
using std::ostream;
using std::string;

// Coordinates input and deque operations through an abstract presentation interface.
class DequeApplication {
public:
    DequeApplication(deque<int>& main_deque, DequeView& view);

    // Read whole command lines until quit or EOF; terminal behaviour stays in the view.
    void run(istream& input, ostream& output);

private:
    // Execute one validated command. False requests a clean exit.
    bool execute(const string& line, string& message, ostream& output);

    DequeController controller_;
    DequeView& view_;
};
