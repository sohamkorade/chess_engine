#pragma once

#include "board.hpp"
#include "types.hpp"

struct TTEntry {  // transposition table entry
  int age;
  int depth = 0;
  int score;
  EvalType eval_type;
  Move best_move;
};

class AI {
 public:
  Board board;
  int wtime = 30000, btime = 30000, winc = 0, binc = 0;
  int mtime = 1000;  // move time
  int max_depth = 100;
  SearchType search_type = Time_per_game;
  unordered_map<uint64_t, TTEntry> TT;  // transposition table
  vector<Move> PV;                      // principal variation
  atomic<bool> searching{false};

  string debug = "";

  AI(Board &board);
  pair<Move, int> search(multiset<uint64_t> &transpositions);
  void set_clock(int _wtime, int _btime, int _winc, int _binc);
  int print_eval();
  void prune_TT(int age);  // prune TT entries older than age

 protected:
  int eval();
  int negamax(int depth);
  int alphabeta(int depth, int alpha, int beta);
  int quiesce(int depth, int alpha, int beta);
  vector<pair<int, Move>> iterative_search();
};
