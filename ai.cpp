#include "ai.hpp"

int material_value(char piece) {
  int color = isupper(piece) ? 1 : -1;
  piece = toupper(piece);
  switch (piece) {
    case 'K':
      return color * 10000;
    case 'Q':
      return color * 900;
    case 'R':
      return color * 500;
    case 'B':
      return color * 300;
    case 'N':
      return color * 250;
    case 'P':
      return color * 100;
  }
  return 0;
}

int AI::eval() {
  int w_material = 1;

  int white_material = 0, black_material = 0;
  for (auto& piece : board.board) {
    int v = material_value(piece);
    if (v < 0)
      black_material -= v;
    else
      white_material += v;
  }
  int material = white_material - black_material;

  return w_material * material;
}

// https://www.chessprogramming.org/Negamax
int AI::negamax(int depth) {
  while (g_main_context_pending(0)) g_main_context_iteration(0, 0);

  if (depth == 0)
    return eval() * (board.turn == White ? 1 : -1) + rand() % 6 - 3;
  int bestscore = -1e6;
  int max_breadth = 5;
  // ofstream log("log.txt", std::ios_base::app);
  auto legals = board.generate_legal_moves();
  for (auto& move : legals) {
    // if (!max_breadth--) break;
    board.make_move(move);
    int score = -negamax(depth - 1);
    board.unmake_move(move);
    // if (score != 0 && depth > 1) {
    // for (int i = 0; i < depth; i++) log << "  ";
    // log << board.to_san(move) << " = " << score << "\n";
    // }
    if (score > bestscore) bestscore = score;
  }
  // log.close();
  // if (legals.size() == 0) return board.is_in_check(board.turn) ? -1e4 : 0;

  return bestscore + legals.size();
}

pair<Move, int> AI::search_best_move() {
  Move bestmove;
  int bestscore = -1e6;
  int max_breadth = 5;
  // {
  //   ofstream log("log.txt");
  //   log << "";
  //   log.close();
  // }

  auto legals = board.generate_legal_moves();
  for (auto& move : legals) {
    // if (!max_breadth--) break;'
    board.make_move(move);
    int score = -negamax(3);
    board.unmake_move(move);
    // ofstream log("log.txt", std::ios_base::app);
    // log << board.to_san(move) << " = " << score << "\n\n";
    // log.close();
    cout << board.to_san(move) << " = " << score << "\n";
    if (score > bestscore) {
      bestscore = score;
      bestmove = move;
    }
  }

  // {
  //   ofstream log("log.txt", std::ios_base::app);
  //   log << "best: " << board.to_san(bestmove) << " = " << bestscore <<
  //   "\n\n"; log.close();
  // }
  cout << "best: " << board.to_san(bestmove) << " = " << bestscore << "\n\n";

  return {bestmove, bestscore};
}