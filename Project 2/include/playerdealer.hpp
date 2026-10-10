#pragma once

#include "participant.hpp"

class Player : public Participant {
private:
    int balance;
    int current_bet;

public:
    // Give the player a name and starting bankroll.
    Player(std::string name, int starting_balance);
    // Deduct a positive bet that the player can afford.
    bool place_bet(int amount);
    // Pay an even-money win, return a tied bet, or settle a loss.
    void win_bet();
    void push_bet();
    void lose_bet();
    // Read the bankroll and the amount wagered this round.
    int get_bank_roll() const { return balance; }
    int get_current_bet() const { return current_bet; }
};

class Dealer : public Participant {
public:
    // Create the house participant.
    Dealer();
    // The dealer draws below 17 and stands on 17 or higher.
    bool should_hit() const;
};
