#include "deque_application.hpp"
#include "terminal_view.hpp"

#include <deque>
#include <iostream>
#include <string>

using std::cerr;
using std::cin;
using std::cout;
using std::deque;
using std::string;

int main(int argc, char* argv[]) {
    bool plain = false;
    bool animations = true;
    for (int argument = 1; argument < argc; ++argument) {
        const string option(argv[argument]);
        if (option == "--plain") {
            plain = true;
        } else if (option == "--no-animation") {
            animations = false;
        } else if (option == "--help") {
            cout << "Usage: deque_playground [--plain] [--no-animation]\n"
                      << "--plain disables animation and screen clearing.\n"
                      << "--no-animation keeps terminal redraws but skips animation.\n";
            return 0;
        } else {
            cerr << "Unknown option: " << option << "\n"
                      << "Usage: deque_playground [--plain] [--no-animation]\n";
            return 1;
        }
    }

    // This is the application's only deque. Both controller and view refer to it.
    deque<int> main_deque;
    TerminalView view(main_deque, plain, animations);
    DequeApplication application(main_deque, view);
    application.run(cin, cout);
    return 0;
}
