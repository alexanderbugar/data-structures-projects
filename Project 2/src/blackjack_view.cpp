#include "../include/blackjack_view.hpp"
#include <charconv>
#include <chrono>
#include <cstdlib>

using namespace std;
using namespace ftxui;

void BlackjackView::run(BlackjackGame& game) {
    auto screen = App::Fullscreen();
    screen.Loop(make_shared<BlackjackView>(game, screen));
}

BlackjackView::BlackjackView(BlackjackGame& game, App& screen) : game(game), screen(screen) {
    InputOption options;
    options.multiline = false;
    options.on_enter = [this] { submit(); };
    input = Input(&entry, "Type here, then press Enter", options);
    auto playing = [this] { return this->game.get_phase() == RoundPhase::PlayerTurn; };
    auto finished = [this] { return this->game.get_phase() == RoundPhase::Finished; };
    Components buttons;
    const char* labels[] = { "1 Stand", "2 Draw front", "3 Draw back", "4 Peek front", "5 Peek back", "6 Shuffle" };
    for (int i = 0; i < 6; ++i) {
        buttons.push_back(Button(labels[i], [this, i] { choose(i + 1); }, ButtonOption::Ascii()) | Maybe(playing));
    }
    controls = Container::Vertical({
        Container::Horizontal({ buttons[0], buttons[1], buttons[2] }),
        Container::Horizontal({ buttons[3], buttons[4], buttons[5] }),
        Container::Horizontal({
            Button("Play again", [this] { choose(0); }, ButtonOption::Ascii()) | Maybe([this, finished] {
                return finished() && this->game.get_player().get_bank_roll() > 0;
            }),
            Button("Quit", [this] { this->screen.Exit(); }, ButtonOption::Ascii()) | Maybe(finished)
        })
    });
    Add(input);
    Add(controls);
}

Component BlackjackView::ActiveChild() {
    return entering_name || game.get_phase() == RoundPhase::Betting ? input : controls;
}

void BlackjackView::submit() {
    if (entering_name) {
        if (entry.find_first_not_of(' ') == string::npos) return;
        game.set_player_name(entry);
        entering_name = false;
        entry.clear();
        return;
    }
    int bet = 0;
    auto result = from_chars(entry.data(), entry.data() + entry.size(), bet);
    if (result.ec != errc{} || result.ptr != entry.data() + entry.size()) bet = 0;
    if (game.start_round(bet)) {
        entry.clear();
        previous_player_cards = previous_dealer_cards = 0;
        animate(Effect::Deal);
        controls->ChildAt(0)->ChildAt(0)->TakeFocus();
    }
}

void BlackjackView::choose(int choice) {
    if (progress < 1.0f) return;
    if (game.get_phase() == RoundPhase::Finished) {
        game.next_round();
        if (game.get_phase() == RoundPhase::Betting) input->TakeFocus();
        return;
    }
    if (game.get_phase() != RoundPhase::PlayerTurn) return;
    previous_player_cards = game.get_player().get_hand().size();
    previous_dealer_cards = game.get_dealer().get_hand().size();
    switch (choice) {
        case 1: game.stand(); animate(Effect::Reveal); break;
        case 2: game.hit(false); animate(Effect::Deal); break;
        case 3: game.hit(true); animate(Effect::Deal); break;
        case 4: game.peek_front(); animate(Effect::PeekFront); break;
        case 5: game.peek_back(); animate(Effect::PeekBack); break;
        case 6: game.shuffle_deck(); animate(Effect::Shuffle); break;
    }
}

void BlackjackView::animate(Effect next_effect) {
    effect = next_effect;
    progress = 0.0f;
    animator = animation::Animator(&progress, 1.0f, chrono::milliseconds(350), animation::easing::CubicOut);
    screen.RequestAnimationFrame();
}

void BlackjackView::OnAnimation(animation::Params& params) {
    bool was_animating = progress < 1.0f;
    ComponentBase::OnAnimation(params);
    animator.OnAnimation(params);
    if (progress >= 1.0f && game.get_phase() == RoundPhase::DealerTurn) {
        previous_player_cards = game.get_player().get_hand().size();
        previous_dealer_cards = game.get_dealer().get_hand().size();
        if (game.dealer_step()) animate(Effect::Deal);
        else screen.RequestAnimationFrame();
        was_animating = true;
    }
    if (was_animating && progress >= 1.0f && game.get_phase() == RoundPhase::Finished) {
        controls->ChildAt(2)->ChildAt(game.get_player().get_bank_roll() > 0 ? 0 : 1)->TakeFocus();
    }
}

bool BlackjackView::OnEvent(Event event) {
    if (event == Event::Escape || (!entering_name && event == Event::Character('q'))) {
        screen.Exit();
        return true;
    }
    if (progress < 1.0f || game.get_phase() == RoundPhase::DealerTurn) return true;
    if (!entering_name && game.get_phase() == RoundPhase::PlayerTurn && event.is_character()) {
        string key = event.character();
        if (key.size() == 1 && key[0] >= '1' && key[0] <= '6') {
            choose(key[0] - '0');
            return true;
        }
    }
    return ActiveChild()->OnEvent(event);
}

Element BlackjackView::card(const Card* value, bool hidden) const {
    string rank = " ";
    string suit = "?";
    if (value && !hidden) {
        rank = value->rank.size() > 2 ? value->rank.substr(0, 1) : value->rank;
        suit = value->suit == "Hearts" ? "♥" : value->suit == "Diamonds" ? "♦" : value->suit == "Clubs" ? "♣" : "♠";
    }
    Element face = vbox({ hbox({ text(rank), filler() }), text(suit) | center,
                          hbox({ filler(), text(rank) }) });
    if (value && !hidden && !getenv("NO_COLOR") && (value->suit == "Hearts" || value->suit == "Diamonds")) {
        face = face | color(Color::Red);
    }
    return face | borderRounded | size(WIDTH, EQUAL, 9) | size(HEIGHT, EQUAL, 5);
}

Element BlackjackView::hand(const Participant& participant, bool hide_first, size_t previous_count) const {
    const auto& cards = participant.get_hand();
    Elements faces;
    for (size_t i = 0; i < cards.size(); ++i) {
        bool revealing_dealer = &participant == &game.get_dealer() && effect == Effect::Reveal && progress < 0.5f;
        Element face = card(&cards[i], i == 0 && (hide_first || revealing_dealer));
        if (effect == Effect::Deal && i >= previous_count && progress < 1.0f) {
            face = hbox({ text(string(int((1.0f - progress) * 6), ' ')), face });
        }
        if (i + 1 == cards.size()) face = face | focus;
        faces.push_back(face);
    }
    if (faces.empty()) faces.push_back(card(nullptr));
    return vbox({ text(participant.get_name() + "'s hand | " +
                       to_string(participant.calculate_hand_value(hide_first)) + " total") | bold,
                  hbox(std::move(faces)) | xframe | size(HEIGHT, EQUAL, 5) });
}

Element BlackjackView::OnRender() {
    const Player& player = game.get_player();
    bool hide_dealer = game.get_phase() == RoundPhase::PlayerTurn;
    const auto& deck = game.get_deck();
    auto front = card(deck.empty() ? nullptr : &deck.front(),
                      !game.is_front_peeked() || (effect == Effect::PeekFront && progress < 0.5f));
    auto back = card(deck.empty() ? nullptr : &deck.back(),
                     !game.is_back_peeked() || (effect == Effect::PeekBack && progress < 0.5f));
    if (effect == Effect::Shuffle && progress < 1.0f) {
        int offset = int(4 * (1.0f - progress));
        front = hbox({ text(string(offset, ' ')), front });
        back = hbox({ back, text(string(offset, ' ')) });
    }
    Element peeks = vbox({ text("Deck | " + to_string(deck.size()) + " cards") | bold,
                          hbox({ vbox({ text("Front") | center, front }),
                                 vbox({ text("Back") | center, back }) }) });
    Element actions;
    if (entering_name || game.get_phase() == RoundPhase::Betting) {
        actions = hbox({ text(entering_name ? "Name: " : "Bet: $"), input->Render() | flex });
    } else if (game.get_phase() == RoundPhase::DealerTurn) {
        actions = text("Dealer is playing...") | dim;
    } else actions = controls->Render();
    return vbox({
        hbox({ text("Blackjack") | bold, filler(), text("Bankroll $" + to_string(player.get_bank_roll()) +
               " | Bet $" + to_string(player.get_current_bet())) }),
        separator(),
        hbox({ vbox({ hand(game.get_dealer(), hide_dealer, previous_dealer_cards),
                      hand(player, false, previous_player_cards) }) | flex,
               separator(), peeks }),
        filler(),
        paragraph(game.get_message()) | size(HEIGHT, EQUAL, 2),
        separator(),
        actions | size(HEIGHT, EQUAL, 3),
        text("Tab/arrows to navigate · Enter to select · Q/Esc to quit") | dim
    }) | borderRounded;
}
