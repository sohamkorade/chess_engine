# chess_engine
A UCI chess engine written in C++. GUI is developed using GTK4.

[Coding blog: What, when, why I wrote what I wrote](https://yonderchess.blogspot.com/)

[@SohamChessBot](https://lichess.org/@/SohamChessBot) is on LiChess: [Watch](https://lichess.org/@/SohamChessBot/tv) or [Challenge](https://lichess.org/?user=SohamChessBot#friend)


[![lichess-rapid](https://lichess-shield.vercel.app/api?username=SohamChessBot&format=bullet)](https://lichess.org/@/SohamChessBot/perf/bullet)
[![lichess-rapid](https://lichess-shield.vercel.app/api?username=SohamChessBot&format=blitz)](https://lichess.org/@/SohamChessBot/perf/blitz)
[![lichess-rapid](https://lichess-shield.vercel.app/api?username=SohamChessBot&format=rapid)](https://lichess.org/@/SohamChessBot/perf/rapid)
[![lichess-classical](https://lichess-shield.vercel.app/api?username=SohamChessBot&format=classical)](https://lichess.org/@/SohamChessBot/perf/classical)

# about
## features
- Board representation
	- Array of 64 pieces
- Search
	- Minimax with α-β pruning
	- Quiescence search
	- Iterative deepening
	- Mate distance pruning
	- Null move pruning
	- Late move reduction
	- Check extension
	- Repetition draw detection using transposition table
- Move ordering
	- Capture, en passant, promotion moves are ordered first
- Evaluation
	- Piece-square tables
	- Material
	- Mop-up evaluation
- Perft
- Time management
	- Time per move
	- Time per game
	- Infinite
- UCI protocol
- GUI
	- Premoves
	- Flip board
	- Promotion
	- Open in LiChess
	- Move navigation
	- FEN board setup

## supported UCI options
None as of now.

## why did I make this?
I wanted to learn more about chess engines and how they work. I also wanted to learn more about C++.

## how did I make this?
I read a lot of articles and watched a few videos on chess programming. I used [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page) as a reference time and again. I also spent a lot of time reading the source code of [Stockfish](https://github.com/official-stockfish/Stockfish).

# how to run

## using Makefile
Linux:
```bash
cd src
make
./main # for UCI engine
./gui  # for GUI chess application
```

## using CMake

### prerequisites
- CMake 3.20 or higher
- C++23 compatible compiler (GCC, Clang, etc.)
- GTK4 and pkg-config (optional - GUI will only build if GTK4 is found)

### building with CMake
```bash
# Configure the build
cmake -B cmake-build-debug -S . -DCMAKE_BUILD_TYPE=Debug

# Build all executables
cmake --build cmake-build-debug

# Or build specific targets
cmake --build cmake-build-debug --target main    # UCI engine
cmake --build cmake-build-debug --target runtest # Test executable
cmake --build cmake-build-debug --target gui     # GUI (requires GTK4)
```

### installing GTK4 (for GUI)
**macOS:**
```bash
brew install gtk4 pkg-config
```

**Linux:**
```bash
sudo apt-get install libgtk-4-dev pkg-config
```

# license
Haven't thought of yet.