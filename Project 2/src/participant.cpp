#include "include/participant.hpp"
#include <iostream>

Participant::Participant(std::string n) : name(n) {}

void Participant::add_card_to_hand(Card c) {
	hand.push_back(c);
}

void Participant::clear_hand() {
	hand.clear();
}

int Participant::calculate_hand_value() const {

	int total_value = 0;
	int ace_count = 0;

	for (const auto& card : hand) {
		total_value += card.value;
		if (card.rank == "Ace") {
			ace_count++;
		}
	}
	while (total_value > 21 && ace_count > 0) {
		total_value -= 10;
		ace_count--;
	}
	return total_value;
}

void Participant::display_hand(bool hide_first_card) const {

    std::cout << name << "'s Hand: ";

    for (size_t i = 0; i < hand.size(); ++i) {
        if (i == 0 && hide_first_card) {
            std::cout << "[Hidden Card] ";
        }
        else {
            std::cout << "[" << hand[i].rank << " of " << hand[i].suit << "] ";
        }
    }

    if (hide_first_card) {
        std::cout << "(Showing: " << hand[1].value << ")";
    }
    else {
        std::cout << "(Total Value: " << calculate_hand_value() << ")";
    }
    std::cout << std::endl;
}

std::string Participant::get_name() const {
	return name;
}