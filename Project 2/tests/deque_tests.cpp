#include "deque_application.hpp"
#include "deque_controller.hpp"
#include "terminal_view.hpp"

#include <deque>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using std::cerr;
using std::cout;
using std::deque;
using std::exception;
using std::istringstream;
using std::numeric_limits;
using std::ostream;
using std::ostringstream;
using std::runtime_error;
using std::size_t;
using std::string;
using std::to_string;
using std::vector;

namespace {
// Keep checks active even in release builds with NDEBUG defined.
void require(bool condition, const string& description) {
    if (!condition) throw runtime_error(description);
}

void controller_checks() {
    deque<int> main_deque;
    DequeController controller(main_deque);
    int value = 99;
    require(!controller.pop_front(value) && !controller.pop_back(value), "empty pops");
    require(!controller.peek_front(value) && !controller.peek_back(value), "empty peeks");
    require(value == 99 && controller.empty(), "failed reads must preserve output and deque");

    controller.push_back(12);
    controller.push_front(-4);
    controller.push_back(12);
    require(main_deque == deque<int>({-4, 12, 12}), "both ends and duplicate values");
    require(controller.peek_front(value) && value == -4, "peek front");
    require(controller.peek_back(value) && value == 12 && controller.size() == 3, "peek back");
    require(controller.pop_front(value) && value == -4, "pop front");
    require(controller.pop_back(value) && value == 12, "pop back");
    require(controller.pop_back(value) && value == 12 && controller.empty(), "last element");

    // External mutation must be visible because the controller shares the main deque.
    main_deque.push_back(42);
    require(controller.size() == 1 && controller.peek_front(value) && value == 42,
            "controller references original deque");
    controller.clear();
    controller.clear();
    require(main_deque.empty(), "clear populated and already-empty deque");
}

void view_checks() {
    deque<int> main_deque;
    TerminalView view(main_deque, true);
    ostringstream empty_screen;
    view.draw(empty_screen);
    require(empty_screen.str().find("empty deque") != string::npos, "empty display");

    main_deque.push_back(numeric_limits<int>::min());
    main_deque.push_back(numeric_limits<int>::max());
    main_deque.push_front(-42);
    const auto original = main_deque;
    ostringstream screen;
    view.draw(screen, "", 30);
    const string text = screen.str();
    require(text.find("-42") != string::npos
            && text.find(to_string(numeric_limits<int>::min())) != string::npos
            && text.find(to_string(numeric_limits<int>::max())) != string::npos,
            "view observes external changes and entire integer range");
    require(text.find("FRONT") != string::npos && text.find("BACK") != string::npos,
            "wrapped display labels both ends");
    require(main_deque == original, "rendering must not mutate deque");

    main_deque.clear();
    main_deque.push_back(5);
    ostringstream single_screen;
    view.draw(single_screen, "", 1);
    require(single_screen.str().find("F/B") != string::npos, "single element / narrow width");

    ostringstream skipped_animation;
    view.animate(skipped_animation, DequeOperation::PushBack, 42);
    view.animate(skipped_animation, DequeOperation::PopFront);
    require(skipped_animation.str().empty(), "plain view skips animation output and delay");
    require(main_deque.size() == 1 && main_deque.front() == 5, "view cannot commit an operation");
}

void application_checks() {
    deque<int> main_deque;
    TerminalView view(main_deque, true);
    DequeApplication application(main_deque, view);
    istringstream commands(
        "pop\npeek_back\npush_back 12\npush_front -4\npush 12\n"
        "peek_front\npeek_back\npop_back\npop\nsize\nempty\n"
        "push_back nope\npush_front 1junk\npush_back 999999999999999999999\n"
        "push 1 2\npush_front\npop_front extra\nclear extra\nquit extra\n"
        "unknown\n\nshow\nhelp\nquit\npush_back 999\n");
    ostringstream output;
    application.run(commands, output);
    require(main_deque == deque<int>({12}), "commands, shortcuts, errors, and quit isolation");
    require(output.str().find("deque::front() = -4") != string::npos, "peek feedback");
    require(output.str().find("deque::back() = 12") != string::npos, "back feedback");
    require(output.str().find("empty deque") != string::npos, "safe empty operation feedback");
    require(output.str().find("\033") == string::npos, "plain output contains no ANSI codes");

    istringstream clear_and_eof("clear\nclear\n");
    ostringstream final_output;
    application.run(clear_and_eof, final_output);
    require(main_deque.empty(), "application clear uses original deque");
    require(final_output.str().find("Input closed") != string::npos, "EOF exits cleanly");

    istringstream limits("push_front " + to_string(numeric_limits<int>::min())
        + "\npush_back +" + to_string(numeric_limits<int>::max()) + "   \nquit\n");
    ostringstream limit_output;
    application.run(limits, limit_output);
    require(main_deque.size() == 2 && main_deque.front() == numeric_limits<int>::min()
            && main_deque.back() == numeric_limits<int>::max(), "signed integer boundaries");
}

// This alternate view proves the application uses the base interface, and observes
// the real deque at the moment of presentation rather than after a mutation.
class CheckingView : public DequeView {
public:
    struct Observation {
        DequeOperation operation;
        size_t size;
        int front;
        int back;
        int pending_value;
    };

    explicit CheckingView(const deque<int>& main_deque) : main_deque_(main_deque) {}

    // No terminal output is needed for this behavioural check.
    void draw(ostream&, const string&, size_t) const override {}

    // Record only scalar observations; never change the deque under test.
    void animate(ostream&, DequeOperation operation, int pending_value) const override {
        observations.push_back({operation, main_deque_.size(),
            main_deque_.empty() ? 0 : main_deque_.front(),
            main_deque_.empty() ? 0 : main_deque_.back(), pending_value});
    }

    mutable vector<Observation> observations;

private:
    const deque<int>& main_deque_;
};

// Validate presentation-before-commit order, empty guards, and dynamic dispatch.
void presentation_checks() {
    deque<int> main_deque;
    CheckingView view(main_deque);
    DequeApplication application(main_deque, view);
    istringstream commands(
        "push_front 7\npush_back 7\npeek_front\npeek_back\npop_front\npop_back\n"
        "clear\npop_front\npeek_back\npush_back bad\npush_back 9\nclear\nquit\n");
    ostringstream output;
    application.run(commands, output);

    const DequeOperation expected[] = {
        DequeOperation::PushFront, DequeOperation::PushBack,
        DequeOperation::PeekFront, DequeOperation::PeekBack,
        DequeOperation::PopFront, DequeOperation::PopBack,
        DequeOperation::PushBack, DequeOperation::Clear
    };
    const size_t expected_sizes[] = {0, 1, 2, 2, 2, 1, 0, 1};
    require(view.observations.size() == 8, "invalid/empty commands must not animate");
    for (size_t index = 0; index < 8; ++index) {
        require(view.observations[index].operation == expected[index], "operation dispatch");
        require(view.observations[index].size == expected_sizes[index],
                "presentation must happen before the STL operation, committed exactly once");
    }
    require(view.observations[0].pending_value == 7
            && view.observations[6].pending_value == 9, "pending push values");
    require(view.observations[4].front == 7 && view.observations[5].back == 7,
            "removed elements still belong to main deque during presentation");
    require(main_deque.empty(), "clear commits after its presentation");
}
}

int main() {
    try {
        controller_checks();
        view_checks();
        application_checks();
        presentation_checks();
        cout << "All deque checks passed.\n";
        return 0;
    } catch (const exception& error) {
        cerr << "Check failed: " << error.what() << '\n';
        return 1;
    }
}
