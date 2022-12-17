#pragma once

#include <gtk/gtk.h>

#include "board.hpp"
#include "types.hpp"

class AI {
 public:
  Board board;
  int wtime = 60000, btime = 60000, winc = 0, binc = 0;
  int max_depth = 10;
  AI(Board &board);

  int eval();
  int print_eval();
  int negamax(int depth);
  int alphabeta(int depth, int alpha, int beta);
  int quiesce(int depth, int alpha, int beta);
  vector<pair<int, Move>> get_best_moves();
  pair<Move, int> search_best_move(multiset<string> &transpositions);

  vector<pair<int, Move>> monte_carlo();

  void set_clock(int _wtime, int _btime, int _winc, int _binc);
};
