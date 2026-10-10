#include "../include/blackjack_view.hpp"

// Create the game and let its view handle terminal interaction.
int main() {
    BlackjackGame game;
    BlackjackView::run(game);
    return 0;
}
