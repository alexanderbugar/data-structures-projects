#include "../include/playerdealer.hpp"

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

void Player::win_bet() {
    balance += current_bet * 2;
    current_bet = 0;
}

void Player::push_bet() {
    balance += current_bet;
    current_bet = 0;
}

void Player::lose_bet() {
    current_bet = 0;
}
