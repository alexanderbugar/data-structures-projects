#pragma once

#include "card.hpp"
#include <vector>
#include <string>

class Participant {

protected:

	std::vector<Card> hand;
	std::string name;

public:

	Participant(std::string n);
	virtual ~Participant() = default;

	void add_card_to_hand(Card c);
	void clear_hand();
	int calculate_hand_value() const;
	void display_hand(bool hide_first_card = false) const;
	std::string get_name() const;

};