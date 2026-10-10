#include "../include/blackjackgame.hpp"
#include <algorithm>

using namespace std;

BlackjackGame::BlackjackGame(string name)
    : player(name, 100), dealer(), random(random_device{}()) {
    initialize_deck();
    shuffle_deck();
    message = "Enter your name to begin.";
}

void BlackjackGame::initialize_deck() {
    deck.clear();
    string suits[] = { "Hearts", "Diamonds", "Clubs", "Spades" };
    string ranks[] = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "Jack", "Queen", "King", "Ace" };
    int values[] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 11 };
    for (const auto& suit : suits) {
        for (int i = 0; i < 13; ++i) {
            deck.push_back(Card{ suit, ranks[i], values[i] });
        }
    }
}

void BlackjackGame::shuffle_deck() {
    shuffle(deck.begin(), deck.end(), random);
    front_peeked = back_peeked = false;
    message = "Deck shuffled.";
}

Card BlackjackGame::draw_from_front() {
    if (deck.empty()) {
        initialize_deck();
        shuffle_deck();
    }
    Card card = deck.front();
    deck.pop_front();
    front_peeked = back_peeked = false;
    return card;
}

Card BlackjackGame::draw_from_back() {
    if (deck.empty()) {
        initialize_deck();
        shuffle_deck();
    }
    Card card = deck.back();
    deck.pop_back();
    front_peeked = back_peeked = false;
    return card;
}

bool BlackjackGame::start_round(int bet) {
    if (phase != RoundPhase::Betting) return false;
    if (!player.place_bet(bet)) {
        message = "Enter a whole-dollar bet between $1 and $" + to_string(player.get_bank_roll()) + ".";
        return false;
    }
    player.clear_hand();
    dealer.clear_hand();
    player.add_card_to_hand(draw_from_front());
    dealer.add_card_to_hand(draw_from_front());
    player.add_card_to_hand(draw_from_front());
    dealer.add_card_to_hand(draw_from_front());
    phase = player.calculate_hand_value() == 21 ? RoundPhase::DealerTurn : RoundPhase::PlayerTurn;
    message = "Opening hands dealt. Draw, peek, shuffle, or stand.";
    return true;
}

void BlackjackGame::hit(bool from_back) {
    if (phase != RoundPhase::PlayerTurn) return;
    Card card = from_back ? draw_from_back() : draw_from_front();
    player.add_card_to_hand(card);
    message = "Drew from the " + string(from_back ? "back: " : "front: ") + card.rank + " of " + card.suit + ".";
    if (player.calculate_hand_value() > 21) finish_round();
    else if (player.calculate_hand_value() == 21) phase = RoundPhase::DealerTurn;
}

void BlackjackGame::stand() {
    if (phase != RoundPhase::PlayerTurn) return;
    phase = RoundPhase::DealerTurn;
    message = "You stand. Dealer reveals their hand.";
}

bool BlackjackGame::dealer_step() {
    if (phase != RoundPhase::DealerTurn) return false;
    if (!dealer.should_hit()) {
        finish_round();
        return false;
    }
    Card card = draw_from_front();
    dealer.add_card_to_hand(card);
    message = "Dealer drew " + card.rank + " of " + card.suit + ".";
    return true;
}

void BlackjackGame::peek_front() {
    if (phase != RoundPhase::PlayerTurn || deck.empty()) return;
    front_peeked = true;
    message = "Peeking at the front. The card stays in the deque.";
}

void BlackjackGame::peek_back() {
    if (phase != RoundPhase::PlayerTurn || deck.empty()) return;
    back_peeked = true;
    message = "Peeking at the back. The card stays in the deque.";
}

void BlackjackGame::finish_round() {
    int player_final_value = player.calculate_hand_value();
    int dealer_final_value = dealer.calculate_hand_value();
    // Game resolution logic
    if (player_final_value > 21) {
        message = "You busted! Your bet goes to the house.";
        player.lose_bet();
    } else if (dealer_final_value > 21 || player_final_value > dealer_final_value) {
        message = dealer_final_value > 21 ? "Dealer busted. You win!" : "You win!";
        player.win_bet();
    } else if (player_final_value < dealer_final_value) {
        message = "House wins. Better luck next time!";
        player.lose_bet();
    } else {
        message = "It's a push (tie). Your bet is returned.";
        player.push_bet();
    }
    phase = RoundPhase::Finished;
    if (player.get_bank_roll() == 0) message += " Bankroll empty. Thanks for playing!";
}

void BlackjackGame::next_round() {
    if (phase != RoundPhase::Finished || player.get_bank_roll() == 0) return;
    phase = RoundPhase::Betting;
    message = "Place your bet for the next round.";
}

void BlackjackGame::set_player_name(const string& name) {
    player.set_name(name);
    message = "Welcome, " + name + ". Place your bet to begin.";
}
