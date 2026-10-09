#include "include/playerdealer.hpp"

#include <deque>
#include <iostream>
#include <random>
#include <string>

using namespace std;

Dealer::Dealer() : Participant("Dealer") {}

bool Dealer::should_hit() const {
	return calculate_hand_value() < 17;
}

Player::Player(string n, int starting_balance) : Participant(n), balance(starting_balance), current_bet(0) {}

bool Player::place_bet(int amount) {

	if (amount <= balance && amount > 0) {
		current_bet = amount;
		balance -= amount;
		return true;
	}
	return false;
}

void Player::win_bet(bool is_blackjack) {

	if (is_blackjack) {
		balance += current_bet * 2.5;
	}
	else {
		balance += current_bet * 2;
	}
	current_bet = 0;
}

void Player::push_bet() {
	balance += current_bet;
	current_bet = 0;
}

void Player::lose_bet() {
	current_bet = 0;
}

int Player::get_bank_roll() const {
	return balance;
}