cmake_minimum_required(VERSION 3.14)
project(decision_tree LANGUAGES CXX)

add_executable(decision_tree main.cpp LinkedTree.cpp LinkedTree.h)
set_target_properties(decision_tree PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

# Keep the provided data next to the executable when using a separate build folder.
configure_file(tree-car.txt tree-car.txt COPYONLY)
configure_file(tree-investment.txt tree-investment.txt COPYONLY)
