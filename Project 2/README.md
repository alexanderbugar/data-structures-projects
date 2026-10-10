# Blackjack

A blackjack-style terminal game built around one `std::deque<Card>`. Start with
a $100 bankroll, place a bet, and try to beat the dealer without going over 21.
You can draw from either end of the deck, peek at either end, or shuffle the
remaining cards. These extra choices demonstrate how a deque works during play.

FTXUI displays animated cards, hand totals, the remaining deck size, and action
buttons. Hearts and diamonds are red; other text uses the terminal's default
foreground. Peeks stay visible until a draw or shuffle changes the deck. The
game's rules and cards are separate from the terminal presentation.

## Run the game

Extract the play ZIP and open its folder. No compiler or installation is needed.

- Windows 10/11 x64: double-click `simple.exe`.
- macOS 11 or newer, Intel or Apple Silicon: double-click `Start Blackjack.command`.
- Linux x64: open a terminal in the folder and run `./blackjack`.

The play folder contains separate executables for these systems. Keep the entire
folder together when copying it to another computer or a flash drive. The play
ZIP is separate from the source checkout.

## Requirements

- An interactive terminal, preferably at least 80 columns by 24 rows, with a font
  that displays `♥ ♦ ♣ ♠`.
- To build the source: a C++17 compiler. Windows builds use MinGW-w64 `g++`;
  Linux/student-cluster and macOS builds use the installed C++ compiler.
- FTXUI 7.0.3 is already bundled in `third_party/ftxui`. It is compiled into the
  game, so no package manager, network access, or separate FTXUI installation is
  needed.

The `Project 2` folder can be copied and built independently of the rest of the
repository. Executables must match the destination operating system and CPU;
one Windows `.exe` cannot also run natively on macOS or Linux.

## How to play

1. Enter your name and press Enter.
2. Enter a whole-dollar bet between $1 and your available bankroll, then press
   Enter. The opening deal gives you and the dealer two cards each.
3. During your turn, press a number below, or use Tab/arrows and Enter to select
   a button. The dealer's first card stays hidden during your turn.
4. After the round, select **Play again** to place another bet, or **Quit**.
   Play again is available while your bankroll is above zero.

| Key | Action |
| --- | --- |
| `1` | Stand and let the dealer play. |
| `2` | Draw and remove the front card. |
| `3` | Draw and remove the back card. |
| `4` | Peek at the front card without removing it. |
| `5` | Peek at the back card without removing it. |
| `6` | Shuffle the remaining deck. |
| `q` | Quit after entering your name. |
| Esc | Quit at any time, including during an animation. |

Aces count as 11 or 1 to avoid a bust; face cards count as 10. The dealer draws
below 17 and stands on 17 or higher. A win pays even money, a loss costs your
bet, and a tie returns it. Reaching 21 starts the dealer's turn automatically.
An exhausted deck is refilled and shuffled before the next draw.

Long hands scroll to show the newest card. Set the `NO_COLOR` environment
variable to disable red suit colors. Quitting restores the terminal; no name,
bankroll, cards, or session history is saved.

## Deque and class design

`src/main.cpp` creates `BlackjackGame`, which owns the only deck deque.
`BlackjackView` holds a reference to that game and reads the actual deck and
hands through const references. It does not own or copy a deck. Each dealt card
is transferred to a participant's hand, which is stored in a `std::vector<Card>`.

| Code | Responsibility |
| --- | --- |
| `include/card.hpp` | Stores a card's suit, rank, and value. |
| `include/participant.hpp`, `src/participant.cpp` | Shared hand storage and ace-aware scoring. |
| `include/playerdealer.hpp`, `src/playerdealer.cpp` | `Player` and `Dealer` inherit from `Participant`; they add betting and dealer behavior. |
| `include/blackjackgame.hpp`, `src/blackjackgame.cpp` | Owns the deck, deals cards, controls rounds, and settles bets. |
| `include/blackjack_view.hpp`, `src/blackjack_view.cpp` | Uses FTXUI for cards, input, buttons, and animations. |

`initialize_deck()` uses `clear()` and `push_back()` to fill the deque.
`draw_from_front()` uses `front()` and `pop_front()`; `draw_from_back()` uses
`back()` and `pop_back()`. Peeks read either end without removing cards.
`std::shuffle` reorders the same deque through its iterators, while `empty()`
and `size()` support refilling and the display.

The project's own inheritance is `Participant` → `Player` and `Dealer`, sharing
hand and scoring behavior. `BlackjackGame` and `BlackjackView` provide additional
classes for game rules and presentation. `BlackjackView` also inherits from
FTXUI's `ComponentBase` to handle rendering, input, and animation callbacks.
All project headers are in one `include` folder.

## Library credit and verification

[FTXUI 7.0.3](https://github.com/ArthurSonzogni/FTXUI/releases/tag/v7.0.3) is
bundled as its unmodified amalgamated `ftxui.hpp` and `ftxui.cpp`, with its MIT
license in `third_party/ftxui/LICENSE`. Keep that license with redistributed
source and play copies. The group maintains the game and view code; FTXUI
provides the terminal infrastructure.

Source compilation and interactive play have been verified on Apple Silicon
macOS, including building from a copied source folder. Windows x64 and Linux
x64 play binaries were built and their dependencies checked, but native
execution on those platforms and the student cluster has not yet been verified.
