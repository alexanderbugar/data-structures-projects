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

option(DECISION_TREE_UI "Build the keyboard tree interface" ON)
if(DECISION_TREE_UI)
    # update existing builds that still point to the old shared folder.
    get_filename_component(OLD_FTXUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../Project 2/third_party/ftxui" ABSOLUTE)
    if(FTXUI_DIR STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}/../Project 2/third_party/ftxui"
       OR FTXUI_DIR STREQUAL "${OLD_FTXUI_DIR}")
        unset(FTXUI_DIR CACHE)
    endif()
    # use the library and license included with this project.
    set(FTXUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ftxui"
        CACHE PATH "Folder containing ftxui.hpp, ftxui.cpp, and LICENSE")
    if(NOT EXISTS "${FTXUI_DIR}/ftxui.cpp")
        message(FATAL_ERROR "Set FTXUI_DIR to the bundled FTXUI folder, or use -DDECISION_TREE_UI=OFF.")
    endif()
    find_package(Threads REQUIRED)
    add_library(decision_tree_ftxui STATIC "${FTXUI_DIR}/ftxui.cpp")
    target_include_directories(decision_tree_ftxui SYSTEM PUBLIC "${FTXUI_DIR}")
    target_compile_features(decision_tree_ftxui PUBLIC cxx_std_17)
    target_link_libraries(decision_tree_ftxui PUBLIC Threads::Threads)
    if(WIN32)
        target_compile_definitions(decision_tree_ftxui PUBLIC UNICODE _UNICODE NOMINMAX)
    endif()
    if(MSVC)
        target_compile_options(decision_tree_ftxui PUBLIC /utf-8)
    endif()
    target_sources(decision_tree PRIVATE ui/TreeView.cpp ui/TreeDiagram.cpp)
    target_compile_definitions(decision_tree PRIVATE DECISION_TREE_UI)
    target_link_libraries(decision_tree PRIVATE decision_tree_ftxui)
endif()
