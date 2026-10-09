#pragma once

#include "participant.hpp"

class Player : public Participant {

private:

	int balance;
	int current_bet;

public:

	Player(std::string name, int starting_balance);
	bool place_bet(int amount);
	void win_bet(bool is_blackjack = false);
	void push_bet();
	void lose_bet();
	int get_bank_roll() const;

};

class Dealer : public Participant {

public:

	Dealer();
	bool should_hit() const;

};