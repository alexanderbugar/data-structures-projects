# Deque Playground

A terminal interface for one `std::deque<int>`. The deque is created in
`main.cpp`; the controller holds a mutable reference, and the terminal view
holds a const reference to the same deque. Neither owns a copy. Boxes show the
logical ordering of elements, not the STL deque's physical memory arrangement.

This draft uses C++14, matching Project 1, with no third-party dependencies.
Push, pop, peek, and clear have terminal animations. Advanced deque operations,
the PDF report, and final submission packaging are outside this draft.

## Presentation abstraction and inheritance

`DequeView` is an abstract base class with a virtual destructor and pure virtual
`draw()` and `animate()` functions. `TerminalView` publicly inherits from it and
overrides both functions. `DequeApplication` receives a `DequeView&` and calls
those functions through the base reference, demonstrating runtime polymorphism.
The terminal implementation contains the ASCII layout, ANSI controls, easing,
timing, and terminal-size detection. The controller contains only deque operations.

No view owns or copies a deque. For a valid operation, the application first asks
the view to animate and then invokes the controller's STL operation once. A push
uses its pending integer as an overlay; pop and clear read existing elements until
the operation commits. Peek highlights the selected box without mutation. Frame
buffers contain rendered characters only. Commands are processed one at a time.

## Build directly on the student cluster

From the repository root:

```sh
cd "Project 2"
mkdir -p build
g++ -std=c++14 -Wall -Wextra -Wpedantic -Iinclude src/*.cpp -o build/deque_playground
./build/deque_playground
```

This needs a C++14-capable compiler. Cluster execution has not yet been verified.
All includes use paths relative to this project, so the `Project 2` directory can
also be built by itself. No CMake, package manager, or internet access is required.

For CLion, reload the root CMake project and select the `deque_playground` target.
Project 1 keeps its existing target and language settings.
Use CLion's terminal or another real terminal to see animations. The ordinary
Run console generally uses plain output because it is not an interactive terminal.

## Commands

| Command | Effect |
| --- | --- |
| `push_front 12` | Add 12 at the front. |
| `push_back -4` | Add -4 at the back. |
| `pop_front` / `pop_back` | Remove an end integer and report its value. |
| `peek_front` / `peek_back` | Read `front()` / `back()` without removing it. |
| `clear` | Remove all elements. |
| `size` / `empty` | Query the main deque. |
| `show` / `help` | Redraw the deque / explain commands. |
| `quit` | Exit. End-of-input also exits cleanly. |

FIFO shortcuts: `push 12` means `push_back 12`, `pop` means `pop_front`, and
`peek` means `peek_front`. Commands are case-sensitive. Integer arguments must
be whole signed decimal values within the platform's `int` range. Empty pops
and peeks, unknown commands, and extra arguments leave the deque unchanged.

Plain output wraps every box to the terminal width; indices appear beneath each box.
Interactive output uses a fixed stage and menu so animations do not move the screen.
`F/B` marks a single element that is both front and back. Interactive terminals
redraw after each command. IDE consoles and redirected input/output use plain
text automatically; `--plain` also forces this mode.

Push boxes enter at the selected end; popped boxes leave while survivors shift
as needed. Peek highlights the selected box with `=` borders; clear moves boxes
off the stage together. Animations last about half a second at 20 frames per second,
using the C++ standard library and sleeping between frames rather than busy waiting.
The displayed size remains the true main deque size until the operation commits.
For large deques, animation focuses on the affected end and identifies the visible
indices; the settled interactive display keeps the same viewport and stage height.
Result text is shortened to fit one screen row; plain output keeps the full text.
Screens smaller than 40 columns or 18 rows use plain output and skip animation. No raw keyboard mode or cursor hiding
is used, so quitting does not require restoring those terminal settings.

`--no-animation` skips transitions while keeping interactive screen redraws.
`--plain` skips both transitions and screen clearing.

Example:

```sh
printf 'push_back 12\npush_front -4\npeek_back\npop_front\nclear\nquit\n' \
  | ./build/deque_playground --plain
```

## Verification

From `Project 2`, compile the behavioural checks without the application main:

```sh
g++ -std=c++14 -Wall -Wextra -Wpedantic -Iinclude \
  tests/deque_tests.cpp src/deque_controller.cpp src/terminal_view.cpp \
  src/deque_application.cpp -o build/deque_tests
./build/deque_tests
```

Specific `using` declarations make standard types and functions readable without
repeating `std::` in expressions. `<iostream>` supplies the stream declarations;
other standard headers are included for their corresponding facilities.
