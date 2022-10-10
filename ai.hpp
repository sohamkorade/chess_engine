#pragma once

#include <gtk/gtk.h>

#include "board.hpp"
#include "types.hpp"

class AI {
 public:
  Board &board;

  AI(Board &board) : board(board) {}

  int eval();
  int negamax(int depth);
  int alphabeta(int depth, int alpha, int beta);
  int quiesce(int alpha, int beta);
  vector<pair<Move, int>> get_best_moves();
  pair<Move, int> search_best_move(multiset<string> &transpositions);
};

int material_value(char piece);