#include "LinkedTree.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;

#ifdef DECISION_TREE_UI
#include "ui/TreeView.h"
#endif

// load the tree file, save its report, and let the user look up nodes by position.
int runConsole() {
    try {
        LinkedTree tree;

        // keep asking for a file until a tree loads or the input ends.
        while (true) {
            cout << "Tree file: ";
            string filename;
            if (!getline(cin, filename)) return 0;

            try {
                tree.loadFromFile(filename);
                break;
            } catch (const runtime_error& error) {
                cout << "Error: " << error.what() << "\nPlease try again.\n";
            }
        }

        // finish writing the report before asking the user for a node.
        ofstream report("about_tree.txt");
        if (!report) throw runtime_error("Unable to create about_tree.txt.");
        tree.writeReport(report);
        report.close();
        if (!report) throw runtime_error("Unable to finish writing about_tree.txt.");
        cout << "Report written to about_tree.txt.\n";

        string input;
        while (true) {
            cout << "Which node would you like to explore (enter position or \"exit\"): ";
            if (!getline(cin, input)) break;

            istringstream words(input);
            string choice, extra;
            if ((words >> choice) && choice == "exit" && !(words >> extra)) break;

            // only accept one whole number, so inputs like "2 extra" do not pass.
            istringstream positionInput(input);
            int position;
            if (!(positionInput >> position) || (positionInput >> extra) ||
                !tree.printNodeInfo(position, cout)) {
                cout << "Invalid input. Please try again.\n";
            }
        }
        cout << "Goodbye!\n";
    } catch (const runtime_error& error) {
        cerr << "Error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}

// use the menu in a terminal, or keep the original console for redirected input.
int main(int argc, char* argv[]) {
    if (argc > 2 || (argc == 2 && string(argv[1]) != "--plain")) {
        cerr << "Usage: decision_tree [--plain]\n";
        return 1;
    }
#ifdef DECISION_TREE_UI
    if (argc == 1 && tree_ui::hasTerminal()) return tree_ui::run();
#endif
    return runConsole();
}
