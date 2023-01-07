#pragma once

#include "board.hpp"
#include "movegen.hpp"
#include "types.hpp"

struct TTEntry {  // transposition table entry
  int age;
  int depth = 0;
  int score;
  EvalType eval_type;
  Move best_move;
};

class Search {
 public:
  Board board;
  int wtime = 30000, btime = 30000, winc = 0, binc = 0;
  int mtime = 1000;  // move time
  int max_depth = 100;
  SearchType search_type = Time_per_game;
  atomic<bool> searching{false};
  multiset<uint64_t> repetitions;  // for checking repetition

  string debug = "";

  Search();
  pair<Move, int> search();
  void set_clock(int _wtime, int _btime, int _winc, int _binc);
  int print_eval();

 protected:
  int eval();
  int negamax(int depth);
  int alphabeta(int depth, int alpha, int beta);
  int quiesce(int depth, int alpha, int beta);
  vector<pair<int, Move>> iterative_search();
};
