#ifndef LINKED_TREE_H
#define LINKED_TREE_H

#include <ostream>
#include <string>

class LinkedTree {
public:
    // Starts with an empty tree; the destructor releases every linked node.
    LinkedTree() = default;
    ~LinkedTree();

    // Each tree owns its nodes, so copying it is disabled.
    LinkedTree(const LinkedTree&) = delete;
    LinkedTree& operator=(const LinkedTree&) = delete;

    // Reads node records and connects them using their levels and preorder positions.
    void loadFromFile(const std::string& filename);

    // Writes the indented tree, preorder lists, and tree properties.
    void writeReport(std::ostream& output) const;

    // Prints a node and one available relative of each kind; false means not found.
    bool printNodeInfo(int position, std::ostream& output) const;

private:
    struct Node {
        int position;
        std::string edgeLabel;
        std::string content;
        Node* parent = nullptr;
        Node* firstChild = nullptr;
        Node* nextSibling = nullptr;
    };

    struct Statistics {
        int internalNodes = 0;
        int leafNodes = 0;
        int height = 0;
        bool binary = true;
        bool proper = true;
        bool perfect = true;
        bool balanced = true;
    };

    Node* root = nullptr;

    // Deletes a node after deleting all of its children.
    static void deleteSubtree(Node* node);

    // Searches the linked tree in preorder for a file position.
    static const Node* findNode(const Node* node, int position);

    // Prints two dashes per level and the incoming edge label for non-root nodes.
    static void printTree(const Node* node, int level, std::ostream& output);

    // Lists either internal nodes or leaves in preorder, including their positions.
    static void printNodeList(const Node* node, bool internal, std::ostream& output);

    // Combines child results to count nodes and check the tree's shape.
    static Statistics analyze(const Node* node);
};

#endif
