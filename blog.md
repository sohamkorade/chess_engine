# aim
- to make a working chess engine
- learn chess programming in the process
- use time effectively

# not aim
- defeat stockfish
- clone stockfish
- waste time

# log

## 30 May 2022
### night
- how to write chess engines?
- read some code
- which language to choose?
	- `python`: easy, ready-made libraries, slow
	- `js`: almost same, nice gui
	- `c++`: hard, have to learn
	- `c`: hard and complex, hard to maintain, boring?
## 31 May 2022
### night
- read about chess programming
- uci protocol
- numba+python vs c/c++? what about julia?
- started coding
- c or c++?
- c arrays vs stl arrays/vectors for 2d
- printing unicode pieces or ascii?


## 1 Jun 2022
### after 12 am
- ditching color output for now
- reading

### evening, night
- more reading
- modularizing, exceptions

## 2 Jun 2022
### after 12 am
- started writing this blog lol

### 
- started writing move generation function

## 16 Jun 2022
### after a long break 
(made a snake clone in c++ during this time, check it out!)
- still coding move generation
- moved hard-coded numbers to Enums (`Direction` and `Player`)
- refactored sliding moves (Q, B, R)
- added `pgn` output (ugly but atleast lichess can parse)
- now engine generates moves from both sides, yay!
- separated `main.cpp` into several header and cpp files

## 17 Jun 2022
### after 2 am
- added `fen` output, generates direct url to lichess analysis!

### afternoon
- repaired enpassant and castling moves
- refactored move generation function
- minor `san` notation improvements
- started legal move generation function

## 22 Jun 2022
### after 2 am
- almost completed legal move generation function
- __issue__: sometimes switches black and white during random play
- __issue__: handling castling rights and enpassant squares while undoing a move

## 23 Jun 2022
### after 12 am
- __bugfix__: `san` notation doesn't show promotion
- added `perft` and `divide` function
- now we can find corner cases easily!
## 2 am
- wasted 1 hour on why I'm getting 1 move less than expected
	reason: code generates kingside `else` queenside castling moves 🥲
## 3 am
- __bugfix__: now engine checks threatened squares during castling
- improved `perft` function
- still a lot of things to do (getting wrong `perft` results)

## 27 Jun 2022
### 3 pm
- found one corner case: castling is still allowed after rook capture!

## 9 Jul 2022
### afternoon
- fixed a corner case with promotion (earlier it always promoted to white pieces)

## 10 Jul 2022
### after 12 am
- fixed a few corner cases with castling (pawns don't threaten blank squares)
- move count fixed
- planning to automate perft
- graphics???

### afternoon
- another castling bug (capturing rook with king cleared only king's castling rights)
	reason: `else`


## 10 Sept 2022
### after a loooonngg time!!!
### 4:50 am
- there's a lot to tell you, but firstly, i'm very happy now because i just teached my ai to capture pieces and guess what, it doesn't just look one move into the future. yes guys, i wrote the negamax algorithm!! it's much more fun to play with it now. but still it has to prioritize other things besides captures, like promotions, checks, pins etc. and obviously, mates!
- now i'm going to sleep as i got many works to do :(

## 11 Oct 2022
### long time no see
### 3 am
- well, there's a lot of small and big changes. i'll try to explain by checking the diff.
- AI part:
	- replaced minimax with alpha-beta (similar to negamax version)
	- __TODO__: implement quiescence search
	- added piece-square table scoring in eval function
	- reduced branching in code
- move generation part:
	- __bugfix__: moves like `e1c1` were always interpreted as castling, fixed
	- changed board representation from `string` to `char[64]`
	- now king positions are incrementally saved, so faster to find kings
	- several optimizations in move generation
- general:
	- now engine trys to avoid moves leading to repetitions (but it is not always optimal, so should do something better)
	- fixed several gui bugs in promotion and flipping
	- gui supports (some) premoves
	- fixed some bugs in move hints, still some remaining ig
	- faster makefile
- BIGGEST UPDATES:
	- uci protocol working!
	- made a [lichess bot](https://lichess.org/@/SohamChessBot) running this engine!

## 28 Oct 2022

### 3 am
- bugfixes:
	- fixed hard to find enpassant bug!
- AI part:
	- code refactorings
	- engine understands clock!!
	- BIGGEST UPDATES:
		- iterative deepening
		- mate distance pruning
		- null move pruning
		- late move reduction
- others:
	- simplified position hash
	- engine now avoids draws!
- TODO: verify all of them, i guess i'm rushing into adding more and more features.
- i've spent around one week on this commit, now it's time for me to focus on my academics :|

## 18 Dec 2022

### 2:40 am
- planning to make a major change in the codebase: using `enum` instead of `char` for pieces. This is to elude branching in code?
- added best move tester
- solved a small bug in move generation

### 5 am
- completed the change to `enum` for pieces
- maybe the engine is now faster? (because we can now access piece-square tables directly using index without branching)


## 24 Dec 2022

### 1:30 am
- better `uci` handling, using `istringstream` to parse commands
- better Mate Score output
- added Mate in 2, 3, and 4 tests (and they pass! but slow)
- inlined some functions (maybe faster now?)
- added Search Type: `Infinite`, `Fixed Depth`, `Time per Move`, `Time per Game`, `Ponder` and `Mate`. Not all of them are working yet :')

### 3 am
- `movelist.reserve(40)`: doing this made move generation ~1.5x faster! Now reserving in all vectors.
- modularized move generation
- added `get_threats` function

## 25 Dec 2022

### 1 am
- wrote README
- plans:
	- find a way to make `movelist` faster
	- make move generation faster, I think checking for checks is the bottleneck.

## 27 Dec 2022

### 1 am
- plans:
	- handle `uci` commands on a separate thread
	- optional comments in input using `#`

### 4 am
	- implemented! now engine can handle `stop` command after `go` command

## 28 Dec 2022

## evening
- BIG IMPROVEMENT:
	- wrote hand-crafted `is_in_threat` function, now engine can avoid moves leading to checks!
	- earlier it worked by generating all opponent's pseudo-legal moves and checked if any move attacks a square.
	- this made move generation faster by 3.5x!

...

## 7 Jan 2023
- cleaned up code
- replaced push_back with emplace_back
- refactored out of board checks for moves
- result: reduced movegen time by 15%!
- working `to_san` function
- refactored out legal move checks

## 8 Jan 2023
- increased max node limit in perft tests
- shocked to see test fail! new bug-hunt begins!!
- bug:
	- FEN: r3k2r/p1ppPpb1/1n2pnp1/1b2N3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R b KQkq - 0 2
	- castling move generation
	- I didn't check the case where the squares adjacent to the king are attacked by a pawn on the 7th rank
	- fixed it by using the new `is_in_threat` function

## 12 Jan 2023

### 3 am
- moved `to_san` function to `Move` class
- fixed move sorting bug: was considering non-captures first
- added transposition table
	- hash table that stores scores of recently searched positions
- replaced `endl` with "\n" (idk if it improves performance)
- BUGFIX:
	- mate score was displayed wrong
- TODO:
	- thoroughly test TT
	- incremental zobrist hashing
- TT is working, I'm happy :)

## 10 Feb 2023
- another bugfix:
	- was generating only one en passant capture
	- similar to old one side castling bug
	- `else` was the culprit
	- fixed
- perft test now takes maxnodes as first arg
- ran all perft tests, all passed at full depth!

## 15 May 2023

### 2:45 am
- changed "\n" to endl, maybe solves the issue of not getting the bestmove detected by GUIs
- program still reports incorrect mate scores
- __TODO__: fix mate score

## 30 Jun 2023

### 9:45 pm
- remove unnecessary condition in search function (depth > max_depth) which cannot occur
- increased position eval score weight in endgame
- corrected 'losing king' to mean the opponent's king in endgame pos eval
- `ply` count now replaces depth to calculate mate score
	- Finally now the engine outputs correct mate scores! The bug was due to `depth` being used instead of `ply`. `depth` doesn't increase linearly with `ply`, so it was giving wrong mate scores (because quiescence search, check extensions, etc. make it unpredictable).

### 5:50 am
- `debug_mode` actually works now

### 6:30 am
- added 3-fold repetition check, hopefully it works! Earlier I had tried using multiset to store hashes, but I was not sure how to implement it 'inside' search function. Now I'm using a vector to store hashes, and checking for repetitions after every move.

### 7 am
- added tapered eval, now the engine can play endgames better!
- I am astonished by the level at which the engine is playing now! Earlier it was making blunders in endgames, but now it is playing so well! I'm so happy :D

### morning
- NEW FEATURES:
	- MADE GUI USABLE AGAIN!
	- choose random out of best-scoring moves
	- can connect to external engines (decoupled my own engine from the GUI, yet to test thoroughly)
- fixed infinite recursion bug in a function (although that function was never called)
- fixed usage of `to_san` function, it should be called only before making the move on the board
- draw by repetition yet to be tested in GUI
- main bug was that the `zobrist_init` function was not called at the start of the game, so the hash was same for all positions. Hours of debugging and it was just a single line of code! LOL!!
- removed unnecessary include in `main.cpp` and also in Makefile
- separated out `get_move_if_legal` from `make_move_if_legal`, it was required elsewhere

### evening
- I let my brother play against the engine, and he won! But found a bug in the endgame.
- FEN: 5rk1/2RR1p1p/2p3p1/8/8/p3PK1P/2r3P1/8 w - - 0 28
- Rxf7?? was played, a big blunder! Gotta figure out why.
- fixed gui bug when the board is flipped

### night
- added move manager to avoid deadlock due to recursion, i still dont know whether this is good or bad
- made many GUI improvements, proud of myself :D
- Maybe I should focus more on the engine now. Also, the interprocess communication (whatever it is called, for I hacked up a solution) is working just for my engine executable. For Stockfish, it just spits out a move as fast as it can (any legal move). I have to figure out why this is happening.


## 14 Oct 2023

### 3:30 am
- wrote `test_mate.py` to test mate-in-x positions
- checked enough positions, I'm sure that the engine is working correctly


## 9 Jul 2024

### 11:22 pm
- wow, this is a long break! I was busy with my academics and other stuff.
- pushed some pending changes:
  - improved engine tester code
- There were a lot of binaries as I archived them, so that I could run matches between different versions of the engine. Some of them are broken, so I needed to delete them. Solution: wrote a bash script to delete all bad binaries.
- plans:
  - read all the code again (with fresh and experienced eyes, lol)
  - write a better tester, using some kind of framework
  - learn about profiling and optimize the engine

### 11:40 pm
- added a utility script to convert UCI moves to PGN, helpful for debugging engine output using lichess, for example


## 23 Jul 2024

### 12 am
- learnt how to profile using valgrind/callgrind/kcachegrind
  - steps:
    - compile with `-g` flag
    - run `valgrind --dump-instr=yes --collect-jumps=yes --tool=callgrind ./runtest perft 4000`
    - open the output file (callgrind.out.xxxxx) using `kcachegrind`
- found that `is_safe` function is taking a lot of time, tried to memoize it but it didn't help much (before: 0.75s, after: 5s)
- added utility function: `movegen_speedtest`


## 26 Jul 2024

### 3 am
- modularized `is_in_threat` function
  - wrote `slide_find_end` function that returns the end square if a piece slides in a direction, and pieces on those squares are checked for threats
  - this intends to replace `diagonal_threats` which checked two pieces at a time for threats
  - this change made move generation ~14% faster!


## 27 Jul 2024

### 2:56 am
- made move generation 33% faster!!!
  - this was a result of rewriting the whole legal move generation logic
  - earlier, I was generating all pseudo-legal moves and then checking if they are legal
  - now, I generate only legal moves
    - so many conditions to handle: single check, double check, absolute pins, castling, en passant, x-ray attacks on king, capturing checking pieces, interposing pieces in check, etc.
  - yet to simplify the code, it's become a bit complex now
- also wrote a perft-checking tool in python
  - learnt to properly use `subprocess.Popen`
  - this tool will help me to verify that the engine is still working correctly after optimizations
  - works with any UCI engine, not just mine!
  - employs a Teacher-Student model: a strong engine (teacher) is used to verify the moves of a weaker engine (student). Greatly reduces the time taken to pinpoint a bug. The tool directly bisects the moves where the engines differ, showing missing and extra moves generated
  - thinking of relocating this tool to a whole new repo :)
  - UPDATE: https://github.com/sohamkorade/autoperft

### 8 am
- add MoveGenType enum, to better split code for check evasions and non-evasions.
- try to use last move to know if the current position is in check


## 23 Aug 2024

### 12:50 am
- just added `-std=c++20` flag to Makefile, and got ~6.6% speedup in move generation!
- UPDATE: using `-std=c++23` now


## 22 May 2025

### 2:30 am
- seeing code after a long time, minor changes:
  - refactor: update test target in Makefile to exclude unnecessary steps
    - no need to compile gui to run tests!
  - refactor: create macros for relative and opposite pieces
    - declared macros for `opp_` and `rel_` (both for pieces and directions) instead of using const/constexpr values in each function
  - refactor: simplify promotion move generation in generate_legal_moves2
  - report units with time in seconds in runtest
- tried out compile-time precomputed table for `is_safe` function, was slightly slower :(