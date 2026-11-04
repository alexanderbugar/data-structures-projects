#ifndef LINKED_TREE_H
#define LINKED_TREE_H

#include <ostream>
#include <string>
#include <vector>

class LinkedTree {
public:
    // a read-only copy of a node for displays. parent and children are list indexes.
    struct NodeInfo {
        int position;
        std::string edgeLabel;
        std::string content;
        int parent;
        std::vector<int> children;
    };

    // copy the nodes in preorder without exposing the tree's pointers.
    std::vector<NodeInfo> nodes() const;

    // start with an empty tree.
    LinkedTree() = default;
    // free all of the nodes when the tree is done being used.
    ~LinkedTree();

    // prevent copies so two trees do not try to delete the same nodes.
    LinkedTree(const LinkedTree&) = delete;
    LinkedTree& operator=(const LinkedTree&) = delete;

    // read the file and connect the nodes using their levels and positions.
    void loadFromFile(const std::string& filename);

    // write the tree, node lists, and properties to the output stream.
    void writeReport(std::ostream& output) const;

    // show a node and its relatives. return false if the position is not found.
    bool printNodeInfo(int position, std::ostream& output) const;

private:
    struct Node {
        int position;
        std::string edgeLabel;
        std::string content;
        Node* parent = nullptr;
        // children are linked together so a node can have more than two.
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

    // add a node and its children to the display copy.
    static void collectNodes(const Node* node, int parent, std::vector<NodeInfo>& result);

    // delete the children before deleting their parent.
    static void deleteSubtree(Node* node);

    // search the tree for the position given in the file.
    static const Node* findNode(const Node* node, int position);

    // display the tree with two dashes per level and labels in brackets.
    static void printTree(const Node* node, int level, std::ostream& output);

    // list either internal nodes or leaves in preorder.
    static void printNodeList(const Node* node, bool internal, std::ostream& output);

    // count the nodes, find the height, and check the tree's shape.
    static Statistics analyze(const Node* node);
};

#endif
