@echo off
rem Compile an independent Windows console executable with MinGW-w64.
cd /d "%~dp0"
g++ -std=c++17 -O2 -pthread -DUNICODE -D_UNICODE -DNOMINMAX= ^
    -static -static-libgcc -static-libstdc++ -mconsole ^
    src/main.cpp src/blackjackgame.cpp src/participant.cpp ^
    src/playerdealer.cpp src/blackjack_view.cpp third_party/ftxui/ftxui.cpp -o simple.exe
if errorlevel 1 exit /b 1
echo Built simple.exe. Double-click it to play.
