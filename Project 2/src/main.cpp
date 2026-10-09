#include "include/blackjackgame.hpp"
#include <iostream>

using namespace std;

int main() {

    cout << "Welcome to Deque Blackjack!\n";
    cout << "Enter your player name: ";

    string name;
    getline(std::cin, name);

    BlackjackGame game(name);

    char playAgain = 'y';

    while (playAgain == 'y' || playAgain == 'Y') {
        game.play_round();
        std::cout << "\nPlay another round? (y/n): ";
        std::cin >> playAgain;
    }

    std::cout << "Thanks for playing!\n";
    return 0;
}

