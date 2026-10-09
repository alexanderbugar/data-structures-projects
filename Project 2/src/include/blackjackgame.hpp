#pragma once

#include "playerdealer.hpp"
#include <deque>

class BlackjackGame {

private:

	std:: deque<Card> deck;
	Player player;
	Dealer dealer;

	void initialize_deck();
	void shuffle_deck();
	Card draw_card();
	Card draw_from_front();
	Card draw_from_back();
	void peek_front();
	void peek_back();

public:

	BlackjackGame(std::string name);

	void play_round();

};