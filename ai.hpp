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
  pair<Move, int> search_best_move();
};

int material_value(char piece);