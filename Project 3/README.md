# Project 3: Decision Trees

A console program that reads a general decision tree from a file, builds linked
nodes, writes `about_tree.txt`, and lets you explore nodes by their preorder positions.
It uses the C++ standard library with no third-party dependencies.

## Compile and run on the student cluster

Open a terminal in the `Project 3` folder, then run:

```sh
g++ -std=c++17 -Wall -Wextra -pedantic main.cpp LinkedTree.cpp -o decision_tree
./decision_tree
```

1. At `Tree file:`, enter `tree-car.txt`, `tree-investment.txt`, or another tree file's path.
2. Open `about_tree.txt` to see the tree and its properties. Each run overwrites it
   with the report for the file you just loaded.
3. Enter a node's preorder position to see its content, one ancestor, one descendant,
   and one sibling. Positions are listed in the report.
4. Enter `exit` to quit. Invalid positions produce `Invalid input. Please try again.`

The program reports the parent as an ancestor, the first child as a descendant,
and the first other child of the same parent as a sibling. A missing relative is
shown as `None`.

For an optional CMake build, this folder also works independently of the repository:

```sh
cmake -S . -B build
cmake --build build
cd build
./decision_tree
```

CMake copies both supplied data files into the build folder.

## File format

The first nonempty line contains the root's content, or the supplied files' format:

```text
0 1 Root question
```

The remaining lines have four tab-separated fields:

```text
level<TAB>preorder position<TAB>edge label<TAB>node content
```

Levels start at 0 for the root. Positions are positive, unique preorder positions.
The root is on the first line; the other records may appear in any order. The
program sorts them by position before connecting each node to its parent at the
preceding level. Node contents and branching choices come entirely from the file.

The provided files use spaces, so that format is accepted too. With spaces,
edge labels must be one word; use tabs for multiword labels. The supplied data
was copied from the Desktop files without changing its contents.

## Linked tree and properties

- `LinkedTree.h` defines a node with links to its parent, first child, and next sibling.
  This supports any number of children. The tree owns and deletes its nodes.
- `LinkedTree.cpp` reads the records, connects the nodes, traverses them recursively,
  computes properties, and writes the report. Temporary vectors help build the
  tree; the finished tree itself consists of linked nodes.
- `main.cpp` handles file selection, report creation, and the exploration prompt.

The visualization uses two dashes per level and `[edge label]` before each non-root
node's content. Internal and leaf nodes are listed in preorder.

Height counts edges: a root-only tree has height 0. A binary tree has at most two
children per node. It is proper when each internal node has exactly two children,
perfect when it is proper and all leaves have equal depth, and height-balanced
when the two child subtree heights differ by at most one at every node. A missing
child has height -1. Proper, perfect, and balanced are reported only for binary trees.

## Submission notes

Submit the necessary source, data, and run instructions with the group's PDF;
leave out executables, generated reports, build folders, and IDE files. The PDF
still needs the group names and number, student-cluster instructions, work
percentages, and the academic integrity statement required by the assignment.
