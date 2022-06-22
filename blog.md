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