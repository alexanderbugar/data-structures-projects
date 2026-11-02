#include "LinkedTree.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace std;

namespace {
    struct TreeRecord {
        int level = 0;
        int position = 1;
        string edgeLabel;
        string content;
    };

    // Remove extra whitespace from the start and end of a line or field.
    string trim(const string& text) {
        size_t first = text.find_first_not_of(" \t\r\n");
        if (first == string::npos) return "";
        size_t last = text.find_last_not_of(" \t\r\n");
        return text.substr(first, last - first + 1);
    }

    // Read a whole number and reject extra text or numbers that do not fit in an int.
    int readInteger(const string& text) {
        istringstream input(text);
        int value;
        string extra;
        if (!(input >> value) || (input >> extra)) {
            throw runtime_error("Expected an integer level or position.");
        }
        return value;
    }

    // Read one node from either a tab-separated file or the provided space-separated files.
    TreeRecord readRecord(const string& line, bool rootLine) {
        TreeRecord record;
        istringstream input(line);

        if (rootLine) {
            int level, position;
            if ((input >> level >> position) && level == 0) {
                record.position = position;
                getline(input, record.content);
            } else {
                // The first line can also contain just the root's question or text.
                record.content = line;
            }
        } else if (line.find('\t') != string::npos) {
            string level, position;
            if (!getline(input, level, '\t') || !getline(input, position, '\t') ||
                !getline(input, record.edgeLabel, '\t') || !getline(input, record.content)) {
                throw runtime_error("Expected level, position, edge label, and content.");
            }
            record.level = readInteger(level);
            record.position = readInteger(position);
        } else {
            if (!(input >> record.level >> record.position >> record.edgeLabel)) {
                throw runtime_error("Expected level, position, edge label, and content.");
            }
            getline(input, record.content);
        }

        record.edgeLabel = trim(record.edgeLabel);
        record.content = trim(record.content);
        if (record.position < 1 || record.content.empty() ||
            (!rootLine && (record.level < 1 || record.edgeLabel.empty()))) {
            throw runtime_error("Invalid node level, position, label, or content.");
        }
        return record;
    }
}

// Free the tree's nodes when the object is destroyed.
LinkedTree::~LinkedTree() {
    deleteSubtree(root);
}

// Return a separate copy so the display cannot change the linked tree.
vector<LinkedTree::NodeInfo> LinkedTree::nodes() const {
    vector<NodeInfo> result;
    if (root) collectNodes(root, -1, result);
    return result;
}

// Remember each child's index while visiting the nodes in preorder.
void LinkedTree::collectNodes(const Node* node, int parent, vector<NodeInfo>& result) {
    int index = static_cast<int>(result.size());
    result.push_back({node->position, node->edgeLabel, node->content, parent, {}});
    if (parent >= 0) result[parent].children.push_back(index);
    for (const Node* child = node->firstChild; child; child = child->nextSibling) {
        collectNodes(child, index, result);
    }
}

// Delete each child's subtree before deleting the current node.
void LinkedTree::deleteSubtree(Node* node) {
    if (!node) return;
    Node* child = node->firstChild;
    while (child) {
        Node* next = child->nextSibling;
        deleteSubtree(child);
        child = next;
    }
    delete node;
}

// Read the node information, check it, and build the linked tree.
void LinkedTree::loadFromFile(const string& filename) {
    ifstream input(filename);
    if (!input) throw runtime_error("Unable to open tree file: " + filename);

    vector<TreeRecord> records;
    string line;
    int lineNumber = 0;
    while (getline(input, line)) {
        ++lineNumber;
        line = trim(line);
        if (line.empty()) continue;
        try {
            records.push_back(readRecord(line, records.empty()));
        } catch (const runtime_error& error) {
            throw runtime_error("Line " + to_string(lineNumber) + ": " + error.what());
        }
    }
    if (input.bad()) throw runtime_error("Unable to finish reading the tree file.");
    if (records.empty()) throw runtime_error("The tree file is empty.");

    // The provided files are grouped by level, so sort by preorder position first.
    sort(records.begin(), records.end(), [](const TreeRecord& a, const TreeRecord& b) {
        return a.position < b.position;
    });

    // A child must have a parent one level above it, and positions cannot repeat.
    for (size_t i = 0; i < records.size(); ++i) {
        if (i == 0 && records[i].level != 0) {
            throw runtime_error("The root must have the first preorder position.");
        }
        if (i > 0 && (records[i].position == records[i - 1].position ||
                      records[i].level == 0 || records[i].level > records[i - 1].level + 1)) {
            throw runtime_error("Duplicate position or invalid level in the tree.");
        }
    }

    deleteSubtree(root);
    root = nullptr;
    // Keep the path to the current node so each new node can find its parent.
    vector<Node*> ancestors;
    for (const TreeRecord& record : records) {
        Node* parent = record.level == 0 ? nullptr : ancestors[record.level - 1];
        Node* node = new Node{record.position, record.edgeLabel, record.content, parent};

        if (!parent) {
            root = node;
        } else if (!parent->firstChild) {
            parent->firstChild = node;
        } else {
            // Add the new child after the parent's existing children.
            Node* lastChild = parent->firstChild;
            while (lastChild->nextSibling) lastChild = lastChild->nextSibling;
            lastChild->nextSibling = node;
        }

        ancestors.resize(record.level);
        ancestors.push_back(node);
    }
}

// Check this node first, then search through each child's subtree.
const LinkedTree::Node* LinkedTree::findNode(const Node* node, int position) {
    if (!node || node->position == position) return node;
    for (const Node* child = node->firstChild; child; child = child->nextSibling) {
        const Node* found = findNode(child, position);
        if (found) return found;
    }
    return nullptr;
}

// Print the node's label and content, adding more dashes for each level.
void LinkedTree::printTree(const Node* node, int level, ostream& output) {
    output << string(2 * level, '-');
    if (node->parent) output << '[' << node->edgeLabel << "] ";
    output << node->content << '\n';
    for (const Node* child = node->firstChild; child; child = child->nextSibling) {
        printTree(child, level + 1, output);
    }
}

// Walk through the tree in preorder and print the requested type of node.
void LinkedTree::printNodeList(const Node* node, bool internal, ostream& output) {
    if ((node->firstChild != nullptr) == internal) {
        output << node->position << ": " << node->content << '\n';
    }
    for (const Node* child = node->firstChild; child; child = child->nextSibling) {
        printNodeList(child, internal, output);
    }
}

// Combine the results from each subtree to find the counts, height, and tree types.
LinkedTree::Statistics LinkedTree::analyze(const Node* node) {
    Statistics result;
    if (!node->firstChild) {
        // A leaf counts as one external node and has a height of zero.
        result.leafNodes = 1;
        return result;
    }

    result.internalNodes = 1;
    int childCount = 0;
    int firstHeight = -1;
    int secondHeight = -1;
    for (const Node* child = node->firstChild; child; child = child->nextSibling) {
        Statistics childResult = analyze(child);
        if (childCount == 0) firstHeight = childResult.height;
        if (childCount == 1) secondHeight = childResult.height;
        ++childCount;

        result.internalNodes += childResult.internalNodes;
        result.leafNodes += childResult.leafNodes;
        result.height = max(result.height, childResult.height + 1);
        result.binary = result.binary && childResult.binary;
        result.proper = result.proper && childResult.proper;
        result.perfect = result.perfect && childResult.perfect;
        result.balanced = result.balanced && childResult.balanced;
    }

    result.binary = result.binary && childCount <= 2;
    result.proper = result.proper && childCount == 2;
    result.perfect = result.perfect && result.proper && firstHeight == secondHeight;
    // Use -1 for a missing child and check the height difference at every node.
    result.balanced = result.balanced && result.binary && abs(firstHeight - secondHeight) <= 1;
    return result;
}

// Write all the required tree information to the report.
void LinkedTree::writeReport(ostream& output) const {
    if (!root) throw runtime_error("No tree is loaded.");
    Statistics stats = analyze(root);

    output << "Tree Visualization\n";
    printTree(root, 0, output);
    output << "\nTree Properties\n"
           << "Root node: " << root->content << '\n'
           << "Number of internal nodes: " << stats.internalNodes << '\n'
           << "Number of external (leaf) nodes: " << stats.leafNodes << '\n'
           << "Tree height: " << stats.height << '\n'
           << "Internal nodes (preorder):\n";
    if (stats.internalNodes == 0) output << "None\n";
    printNodeList(root, true, output);
    output << "External nodes (preorder):\n";
    printNodeList(root, false, output);

    output << "\nBinary Tree Analysis\n"
           << "Is it a Binary Tree? " << (stats.binary ? "Yes" : "No") << '\n';
    if (stats.binary) {
        output << "Proper Tree: " << (stats.proper ? "Yes" : "No") << '\n'
               << "Perfect Tree: " << (stats.perfect ? "Yes" : "No") << '\n'
               << "Balanced Tree: " << (stats.balanced ? "Yes" : "No") << '\n';
    }
}

// Show the node, its parent, its first child, and one sibling when they exist.
bool LinkedTree::printNodeInfo(int position, ostream& output) const {
    const Node* node = findNode(root, position);
    if (!node) return false;

    const Node* sibling = node->parent ? node->parent->firstChild : nullptr;
    // Skip the node itself when looking for a sibling.
    if (sibling == node) sibling = sibling->nextSibling;
    output << "Node's content: " << node->content << '\n'
           << "Ancestor: " << (node->parent ? node->parent->content : "None") << '\n'
           << "Descendant: " << (node->firstChild ? node->firstChild->content : "None") << '\n'
           << "Sibling: " << (sibling ? sibling->content : "None") << '\n';
    return true;
}
