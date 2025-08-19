A simple Pac-Man clone written in C using the ncurses library.
Play in your terminal, dodge the ghosts, eat pellets, and try to clear the map!

🎮Features:

Classic Pac-Man–style map and movement.

Player (C) controlled via WASD or Arrow Keys.

Ghosts (M) with basic AI:

Greedy mode (chase you most of the time).

Scatter mode (random movement for unpredictability).

Pellets (.) and Power Pellets (o).

Score system:

+10 for normal pellet.

+50 for power pellet.

3 Lives system.

Win when all pellets are eaten.

Colorful ncurses graphics in your terminal.

🛠️ Requirements

GCC (or any C compiler).

ncurses library installed.

sudo apt install libncurses5-dev libncursesw5-dev   # Ubuntu / Debian
brew install ncurses                                # macOS (Homebrew)


git clone https://github.com/yourusername/pacman-c.git
cd pacman-c
gcc pacman.c -lncurses -o pacman
./pacman
