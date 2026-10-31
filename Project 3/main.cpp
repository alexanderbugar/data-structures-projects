#include "LinkedTree.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;

// Loads the chosen file, saves its report, and lets the user explore node positions.
int main() {
    cout << "Tree file: ";
    string filename;
    if (!getline(cin, filename)) return 0;

    try {
        LinkedTree tree;
        tree.loadFromFile(filename);
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
