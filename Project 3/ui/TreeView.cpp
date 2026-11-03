#include "TreeView.h"
#include "TreeDiagram.h"

#include <fstream>
#include <memory>
#include <stdexcept>

using namespace ftxui;
using namespace std;

namespace tree_ui {
namespace {
enum class Page { File, Menu, Walk, Browse };

class TreeView : public ComponentBase {
public:
    explicit TreeView(App& app) : app(app) {
        InputOption inputOptions;
        inputOptions.multiline = false;
        inputOptions.cursor_position = static_cast<int>(filename.size());
        inputOptions.on_enter = [this] { load(); };
        input = Input(&filename, "Tree filename or full path", inputOptions);
        MenuOption options;
        options.on_enter = [this] { choose(); };
        options.on_change = [this] {
            if (page == Page::Browse) select(choice);
        };
        options.entries_option.transform = [](const EntryState& state) {
            auto label = paragraph(string(state.active ? "› " : "  ") + state.label);
            return state.active ? label | color(Color::RGB(232, 164, 116)) | bold : label;
        };
        menu = ftxui::Menu(&choices, &choice, options);
        Add(input);
        Add(menu);
    }

    // only the filename field or the current menu receives keyboard focus.
    Component ActiveChild() override { return page == Page::File ? input : menu; }

    bool OnEvent(Event event) override {
        if (event == Event::Escape) {
            if (page == Page::Menu || !tree) app.Exit();
            else show(Page::Menu);
            return true;
        }
        if (page == Page::File) return input->OnEvent(event);
        if (event == Event::Character('q')) { app.Exit(); return true; }
        if (event == Event::ArrowLeftCtrl) { panX -= 8; return true; }
        if (event == Event::ArrowRightCtrl) { panX += 8; return true; }
        if (event == Event::ArrowUpCtrl) { panY -= 4; return true; }
        if (event == Event::ArrowDownCtrl) { panY += 4; return true; }
        if (event == Event::Character('c')) { panX = panY = 0; return true; }
        if (page == Page::Walk || page == Page::Browse) {
            if (event == Event::Character('r') || event == Event::Home) {
                select(0);
                show(page);
                return true;
            }
            if (event == Event::Backspace || event == Event::ArrowLeft) {
                if (nodes[selected].parent >= 0) select(nodes[selected].parent);
                show(page);
                return true;
            }
            if (page == Page::Browse && event == Event::ArrowRight) {
                if (!nodes[selected].children.empty()) select(nodes[selected].children.front());
                show(page);
                return true;
            }
        }
        return menu->OnEvent(event);
    }

    // reflow the panels when the terminal changes size.
    Element OnRender() override {
        const auto background = Color::RGB(22, 24, 29);
        const auto foreground = Color::RGB(222, 222, 216);
        const auto accent = Color::RGB(232, 164, 116);
        const auto muted = Color::RGB(144, 153, 167);
        auto terminal = Terminal::Size();
        Element body;
        if (page == Page::File) {
            body = vbox({
                text(" Open a decision tree") | bold,
                text(""),
                paragraph("Enter tree-car.txt, tree-investment.txt, or a full file path.") | color(muted),
                input->Render() | borderRounded | color(accent),
                paragraph(status) | color(accent),
                text("Enter  open     Esc  back / quit") | color(muted)
            }) | size(WIDTH, LESS_THAN, 72) | center | flex;
        } else {
            const auto& node = nodes[selected];
            string heading = page == Page::Menu ? "Menu" : page == Page::Walk ?
                (node.children.empty() ? "Decision reached" : "Choose an answer") : "Explore nodes";
            Elements details = {
                text(heading) | color(accent) | bold,
                separator(),
                text("Node #" + to_string(node.position)) | color(muted),
                paragraph(node.content) | bold | vscroll_indicator | yframe |
                    size(HEIGHT, LESS_THAN, max(3, terminal.dimy / 3)),
                separator()
            };
            if (page == Page::Browse) {
                auto label = [&](int index) { return index < 0 ? string("None") : "#" + to_string(nodes[index].position); };
                int sibling = -1;
                if (node.parent >= 0) {
                    for (int index : nodes[node.parent].children) {
                        if (index != selected) { sibling = index; break; }
                    }
                }
                details.push_back(paragraph("Parent: " + label(node.parent) + "   Child: " +
                    label(node.children.empty() ? -1 : node.children.front()) + "   Sibling: " + label(sibling)) | color(muted));
            }
            details.push_back(menu->Render() | vscroll_indicator | yframe | flex);
            auto panel = vbox(std::move(details)) | borderRounded;
            auto map = vbox({
                text(" TREE  ·  " + to_string(nodes.size()) + " nodes  ·  highlighted path") | color(muted),
                diagram(nodes, selected, panX, panY) | flex
            }) | borderRounded | flex;
            if (terminal.dimx >= 100 || (terminal.dimx >= 80 && terminal.dimy < 30)) {
                body = hbox({map, panel | size(WIDTH, EQUAL, terminal.dimx >= 100 ? 36 : 32)}) | flex;
            } else if (terminal.dimx >= 50 && terminal.dimy >= 30) {
                body = vbox({map | size(HEIGHT, EQUAL, terminal.dimy / 2), panel | flex}) | flex;
            } else {
                body = vbox({text("Widen the terminal to show the tree map.") | color(muted), panel | flex}) | flex;
            }
        }
        string help = page == Page::File ? "" : terminal.dimx < 70 ?
            "↑↓ choose · Enter · Esc back · q quit" :
            "↑↓ select · Enter confirm · ← back · r restart · Esc menu · q quit";
        return vbox({
            hbox({text(" ◆ DECISION TREES ") | color(accent) | bold, filler(), text(" GGGamers ") | color(muted)}),
            body,
            text(help) | color(muted),
            text(page == Page::File ? "" : terminal.dimx < 70 ? "← parent · r restart · c center" :
                 "Ctrl+arrows pan map · c center · " + status) | color(muted)
        }) | color(foreground) | bgcolor(background);
    }

private:
    App& app;
    unique_ptr<LinkedTree> tree;
    vector<LinkedTree::NodeInfo> nodes;
    Page page = Page::File;
    string filename = "tree-car.txt", status;
    vector<string> choices;
    int selected = 0, choice = 0, panX = 0, panY = 0;
    Component input, menu;

    // leave a failed filename in the field so it can be corrected and retried.
    void load() {
        try {
            auto next = make_unique<LinkedTree>();
            next->loadFromFile(filename);
            ofstream report("about_tree.txt");
            if (!report) throw runtime_error("Unable to create about_tree.txt.");
            next->writeReport(report);
            report.close();
            if (!report) throw runtime_error("Unable to finish writing about_tree.txt.");
            nodes = next->nodes();
            tree = std::move(next);
            select(0);
            status = "Saved about_tree.txt";
            show(Page::Menu);
        } catch (const runtime_error& error) {
            status = string(error.what()) + " Please try again.";
        }
    }

    // recenter the map when moving to another node.
    void select(int index) {
        selected = index;
        panX = panY = 0;
    }

    // reuse one menu for the main actions, answers, and preorder node list.
    void show(Page next) {
        page = next;
        choice = 0;
        choices.clear();
        if (page == Page::Menu) {
            choices = {"Walk through questions", "Explore all nodes", "Open another file", "Quit"};
        } else if (page == Page::Walk) {
            for (int child : nodes[selected].children) choices.push_back(nodes[child].edgeLabel);
            if (choices.empty()) choices = {"Start again", "Main menu"};
        } else if (page == Page::Browse) {
            for (const auto& node : nodes) choices.push_back("#" + to_string(node.position) + " " + node.content);
            choice = selected;
        }
        ActiveChild()->TakeFocus();
    }

    // follow the selected edge instead of assuming every tree only has yes and no.
    void choose() {
        if (page == Page::Menu) {
            switch (choice) {
                case 0: select(0); show(Page::Walk); break;
                case 1: show(Page::Browse); break;
                case 2: status.clear(); show(Page::File); break;
                case 3: app.Exit(); break;
            }
        } else if (page == Page::Walk) {
            if (nodes[selected].children.empty()) {
                if (choice == 0) { select(0); show(Page::Walk); }
                else show(Page::Menu);
            } else {
                select(nodes[selected].children[choice]);
                show(Page::Walk);
            }
        } else if (page == Page::Browse) {
            show(Page::Walk);
        }
    }
};
}

bool hasTerminal() {
#ifdef _WIN32
    return _isatty(_fileno(stdin)) && _isatty(_fileno(stdout));
#else
    return isatty(fileno(stdin)) && isatty(fileno(stdout));
#endif
}

int run() {
    auto app = App::Fullscreen();
    app.TrackMouse(false);
    app.Loop(make_shared<TreeView>(app));
    return 0;
}
}
