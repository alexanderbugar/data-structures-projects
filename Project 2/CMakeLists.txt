cmake_minimum_required(VERSION 3.14)
project(deque_playground LANGUAGES CXX)

add_executable(deque_playground
        src/main.cpp
        src/deque_controller.cpp
        src/terminal_view.cpp
        src/deque_application.cpp
)

# Match Project 1's language version without changing its target or settings.
set_target_properties(deque_playground PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)
target_include_directories(deque_playground PRIVATE include)
