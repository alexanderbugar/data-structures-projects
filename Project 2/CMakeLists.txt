cmake_minimum_required(VERSION 3.15)
project(blackjack LANGUAGES CXX)

find_package(Threads REQUIRED)
add_executable(blackjack
    src/main.cpp src/blackjackgame.cpp src/participant.cpp
    src/playerdealer.cpp src/blackjack_view.cpp
    third_party/ftxui/ftxui.cpp
)
set_target_properties(blackjack PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
target_link_libraries(blackjack PRIVATE Threads::Threads)
if(MSVC)
    target_compile_options(blackjack PRIVATE /utf-8)
    set_property(TARGET blackjack PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif(MINGW)
    target_link_options(blackjack PRIVATE -static -static-libgcc -static-libstdc++)
endif()
if(WIN32)
    target_compile_definitions(blackjack PRIVATE UNICODE _UNICODE NOMINMAX=)
    set_target_properties(blackjack PROPERTIES OUTPUT_NAME simple)
endif()
