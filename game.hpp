#pragma once
#include "board.hpp"

class Game {
 public:
  Board board;
  vector<Move> movelist;
  int ply = 0, end = 0;
  void make_move(string m);
  void make_move(Move m);
  void prev();
  void next();
  void print_movelist();
  void print_pgn();
  void seek(int n);
  void random_move();
};