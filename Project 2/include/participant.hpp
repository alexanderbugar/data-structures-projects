#pragma once

#include "card.hpp"
#include <vector>

// Shared hand and scoring behavior inherited by Player and Dealer.
class Participant {
protected:
    std::vector<Card> hand;
    std::string name;

public:
    // Give the participant a display name.
    explicit Participant(std::string n);
    virtual ~Participant() = default;
    // Add a dealt card to this participant's hand.
    void add_card_to_hand(Card c);
    // Remove the previous round's cards.
    void clear_hand();
    // Count aces as 1 instead of 11 when needed to avoid a bust.
    int calculate_hand_value(bool hide_first_card = false) const;
    // Read the participant's name and actual hand without copying cards.
    const std::string& get_name() const { return name; }
    const std::vector<Card>& get_hand() const { return hand; }
    // Update the player's name after the welcome screen.
    void set_name(const std::string& n) { name = n; }
};
