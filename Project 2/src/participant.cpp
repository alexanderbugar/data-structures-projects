#include "../include/participant.hpp"

using namespace std;

Participant::Participant(string n) : name(n) {}

void Participant::add_card_to_hand(Card c) {
    hand.push_back(c);
}

void Participant::clear_hand() {
    hand.clear();
}

int Participant::calculate_hand_value(bool hide_first_card) const {
    int total_value = 0;
    int ace_count = 0;
    for (size_t i = hide_first_card ? 1 : 0; i < hand.size(); ++i) {
        total_value += hand[i].value;
        if (hand[i].rank == "Ace") ++ace_count;
    }
    while (total_value > 21 && ace_count > 0) {
        total_value -= 10;
        --ace_count;
    }
    return total_value;
}
