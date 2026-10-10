#!/bin/sh
# Compile the game and bundled FTXUI using an installed C++17 compiler.
set -eu
cd "$(dirname "$0")"
"${CXX:-c++}" -std=c++17 -O2 -pthread src/*.cpp third_party/ftxui/ftxui.cpp -o blackjack
printf 'Built blackjack. Start it with ./blackjack\n'
