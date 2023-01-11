#pragma once

#include "board.hpp"
#include "search.hpp"

class Game {
 public:
  Board board;
  vector<Move> movelist;
  int ply = 0, end = 0;
  Status result = Undecided;
  multiset<Piece> white_alive, black_alive;
  multiset<uint64_t> repetitions;

  Game();
  bool make_move(string m);
  bool make_move(Move m);
  void prev();
  void next();
  void print_movelist();
  void print_pgn();
  void seek(int n);
  Move random_move();
  pair<Move, int> ai_move();
  Status get_result();
  void new_game();
  bool load_fen(string fen);
  void update_alive();
};