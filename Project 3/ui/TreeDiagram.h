#ifndef TREE_DIAGRAM_H
#define TREE_DIAGRAM_H

#include "../LinkedTree.h"
#include "ftxui.hpp"

namespace tree_ui {
// Draw the tree around the selected node, with optional keyboard scrolling.
ftxui::Element diagram(const std::vector<LinkedTree::NodeInfo>& nodes,
                       int selected, int panX, int panY);
}

#endif
