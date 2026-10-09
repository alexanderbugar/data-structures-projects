#include "include/blackjackgame.hpp"
#include <algorithm>
#include <iostream>
#include <random>

using namespace std;

BlackjackGame::BlackjackGame(string name) : player(name, 100), dealer() {
	initialize_deck();
	shuffle_deck();
}

void BlackjackGame::initialize_deck() {

	deck.clear();

	string suits[] = { "Hearts", "Diamonds", "Clubs", "Spades" };
	string ranks[] = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "Jack", "Queen", "King", "Ace" };
	int values[] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 11 };

	for (const auto& suit : suits) {
		for (int i = 0; i < 13; ++i) {
			Card new_card{ suit, ranks[i], values[i] };
			deck.push_back(new_card);
		}
	}
}

void BlackjackGame::shuffle_deck() {

	random_device rd;
	mt19937 g(rd());
	shuffle(deck.begin(), deck.end(), g);
}

Card BlackjackGame::draw_card() {

	if (deck.empty()) {
		cout << "Deck is empty, reshuffling..." << endl;
		initialize_deck();
		shuffle_deck();
	}
	Card card = deck.front();
	deck.pop_front();
	return card;
}

Card BlackjackGame::draw_from_front() {

	if (deck.empty()) {
		cout << "Deck is empty, reshuffling..." << endl;
		initialize_deck();
		shuffle_deck();
	}
	Card card = deck.front();
	deck.pop_front();
	return card;
}

Card BlackjackGame::draw_from_back() {

	if (deck.empty()) {
		cout << "Deck is empty, reshuffling..." << endl;
		initialize_deck();
		shuffle_deck();
	}
	Card card = deck.back();
	deck.pop_back();
	return card;
}

void BlackjackGame::peek_front() {

	if (!deck.empty()) {
		Card c = deck.front();
		cout << "Top card: [" << c.rank << " of " << c.suit << "]" << "(Value: " << c.value << ")" << endl;
	}
}

void BlackjackGame::peek_back() {

	if (!deck.empty()) {
		Card c = deck.back();
		cout << "Bottom card: [" << c.rank << " of " << c.suit << "]" << "(Value: " << c.value << ")" << endl;
	}
}

void BlackjackGame::play_round() {

	cout << "\n==============\n";
	cout << "Balance: $" << player.get_bank_roll() << endl;
	cout << "Cards remaining: " << deck.size() << endl;
	cout << "==============\n";

	int bet;

	while (true) {

		cout << "\nEnter your bet amount: $";
		cin >> bet;
		
		if (cin.fail()) {
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Invalid input. Please enter a numeric value." << endl;
			continue;
		}
		break;
	}

	if (!player.place_bet(bet)) {
		cout << "Invalid bet selection. Please try again." << endl;
		cout << "Bet amount: $" << bet << ", is higher than your balance of: $" << player.get_bank_roll() << endl;
		return;
	}

	player.clear_hand();
	dealer.clear_hand();
	player.add_card_to_hand(draw_card());
	dealer.add_card_to_hand(draw_card());
	player.add_card_to_hand(draw_card());
	dealer.add_card_to_hand(draw_card());

	while (player.calculate_hand_value() < 21) {

		dealer.display_hand(true);
		player.display_hand();

		cout << "\nOptions:\n"
			<< "[1] Stand\n"
			<< "[2] Draw from the FRONT (Top of Deck)\n"
			<< "[3] Draw from the BACK (Bottom of Deck)\n"
			<< "[4] Peek at the FRONT (Top Card)\n"
			<< "[5] Peek at the BACK (Bottom Card)\n"
			<< "[6] Shuffle the Deck\n"
			<< ">> ";

		int choice;

		cin >> choice;
		cout << endl;

		if (choice == 1) {
			break;
		}
		else if (choice == 2) {
			Card c = draw_from_front();
			cout << "You drew from the FRONT: " << c.rank << " of " << c.suit << "(Value: " << c.value << ")" << endl;
			player.add_card_to_hand(c);
		}
		else if (choice == 3) {
			Card c = draw_from_back();
			cout << "You drew from the BACK: " << c.rank << " of " << c.suit << "(Value: " << c.value << ")" << endl;
			player.add_card_to_hand(c);
		}
		else if (choice == 4) {
			peek_front();
		}
		else if (choice == 5) {
			peek_back();
		}
		else if (choice == 6) {
			shuffle_deck();
			cout << "Deck shuffled." << endl;
		}
		else {
			cout << "Invalid choice. Please try again." << endl;
		}

	}

	int player_final_value = player.calculate_hand_value();
	player.display_hand();

	if (player_final_value > 21) {
		cout << "You busted!" << endl;
		player.lose_bet();
		return;
	}

	cout << "\n--- Dealer's Turn ---\n";
	dealer.display_hand(false);
	while (dealer.should_hit()) {
		Card c = draw_card();
		cout << "Dealer draws: [" << c.rank << " of " << c.suit << "]\n";
		dealer.add_card_to_hand(c);
		dealer.display_hand(false);
	}

	int dealerFinal = dealer.calculate_hand_value();

	// Game resolution logic
	if (dealerFinal > 21 || player_final_value > dealerFinal) {
		std::cout << "You win!\n";
		player.win_bet(false);
	}
	else if (player_final_value < dealerFinal) {
		std::cout << "House wins. Better luck next time!\n";
		player.lose_bet();
	}
	else {
		std::cout << "It's a push (Tie)!\n";
		player.push_bet();
	}
}
