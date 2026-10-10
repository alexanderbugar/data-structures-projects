#pragma once

#include "playerdealer.hpp"
#include <deque>
#include <random>

enum class RoundPhase { Betting, PlayerTurn, DealerTurn, Finished };

// Owns the deck and game rules. The view only reads this game's cards.
class BlackjackGame {
private:
    std::deque<Card> deck;
    Player player;
    Dealer dealer;
    std::mt19937 random;
    RoundPhase phase = RoundPhase::Betting;
    std::string message = "Enter your name to begin.";
    bool front_peeked = false;
    bool back_peeked = false;

    // Fill the main deque with the 52 cards of a standard deck.
    void initialize_deck();
    // Remove and return a card, refilling an exhausted deck when needed.
    Card draw_from_front();
    Card draw_from_back();
    // Compare the hands and settle the player's bet.
    void finish_round();

public:
    // Start a game with a shuffled deck and a $100 bankroll.
    explicit BlackjackGame(std::string name = "Player");
    // Accept a valid bet and deal the opening hands.
    bool start_round(int bet);
    // Add a card from the chosen end during the player's turn.
    void hit(bool from_back);
    // Let the dealer play after the player stands.
    void stand();
    // Play one dealer step; return true when a card was drawn.
    bool dealer_step();
    // Reveal an end without removing its card from the deque.
    void peek_front();
    void peek_back();
    // Shuffle the existing deque and invalidate previous peeks.
    void shuffle_deck();
    // Return to betting after a completed round.
    void next_round();
    // Set the name entered on the welcome screen.
    void set_player_name(const std::string& name);

    // Expose read-only game state without copying the deck or hands.
    const std::deque<Card>& get_deck() const { return deck; }
    const Player& get_player() const { return player; }
    const Dealer& get_dealer() const { return dealer; }
    RoundPhase get_phase() const { return phase; }
    const std::string& get_message() const { return message; }
    bool is_front_peeked() const { return front_peeked; }
    bool is_back_peeked() const { return back_peeked; }
};
