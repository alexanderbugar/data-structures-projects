#pragma once

#include "blackjackgame.hpp"
#include "../third_party/ftxui/ftxui.hpp"

// All FTXUI layout, input, and animation live here, outside the game rules.
class BlackjackView : public ftxui::ComponentBase {
private:
    enum class Effect { Deal, PeekFront, PeekBack, Shuffle, Reveal };
    BlackjackGame& game;
    ftxui::App& screen;
    ftxui::Component input;
    ftxui::Component controls;
    std::string entry;
    bool entering_name = true;
    Effect effect = Effect::Deal;
    float progress = 1.0f;
    ftxui::animation::Animator animator{ &progress, 1.0f };
    size_t previous_player_cards = 0;
    size_t previous_dealer_cards = 0;

    // Accept a name or a whole-dollar bet from the input field.
    void submit();
    // Dispatch a numbered game action or the next-round button.
    void choose(int choice);
    // Start a short transition using FTXUI's animation scheduler.
    void animate(Effect next_effect);
    // Draw one card or a hidden card, without copying it.
    ftxui::Element card(const Card* value, bool hidden = false) const;
    // Draw a participant's real hand and its currently visible total.
    ftxui::Element hand(const Participant& participant, bool hide_first, size_t previous_count) const;

public:
    // Open the terminal interface for the game created in main.
    static void run(BlackjackGame& game);
    // Build the input and action components.
    BlackjackView(BlackjackGame& game, ftxui::App& screen);
    // Compose the table, event message, and controls.
    ftxui::Element OnRender() override;
    // Route keyboard events to the current input or buttons.
    bool OnEvent(ftxui::Event event) override;
    // Advance card effects and let the dealer draw between animations.
    void OnAnimation(ftxui::animation::Params& params) override;
    // Focus the input while naming/betting, otherwise the action buttons.
    ftxui::Component ActiveChild() override;
};
