#include "ai.hpp"

#include "knowledge.hpp"

int piece_val['z'] = {0};

pair<Move, int> AI::search_best_move(multiset<string>& transpositions) {
  piece_val['K'] = 10000;
  piece_val['Q'] = 900;
  piece_val['R'] = 500;
  piece_val['B'] = 300;
  piece_val['N'] = 250;
  piece_val['P'] = 100;
  piece_val['k'] = -10000;
  piece_val['q'] = -900;
  piece_val['r'] = -500;
  piece_val['b'] = -300;
  piece_val['n'] = -250;
  piece_val['p'] = -100;
  int bestscore = -1e8;
  Move bestmove;
  Board temp = board;
  auto movelist = get_best_moves();
  for (int i = 1; i >= 0; i--) {
    for (auto& m : movelist) {
      temp.make_move(m.first);
      int tp = transpositions.count(temp.pos_hash());
      temp.unmake_move(m.first);
      if (tp > i) continue;
      if (m.second > bestscore) {
        bestscore = m.second;
        bestmove = m.first;
      }
    }
    if (bestmove.from != bestmove.to) break;
  }
  cout << "info bestmove: " << board.to_san(bestmove) << " = " << bestscore
       << endl;
  return {bestmove, bestscore};
}

vector<pair<Move, int>> AI::get_best_moves() {
  vector<pair<Move, int>> bestmoves;
  // Move bestmove;
  // int bestscore = -1e8;
  // int max_breadth = 5;
  // {
  //   ofstream log("log.txt");
  //   log << "";
  //   log.close();
  // }
  int depth = 3;
  int time = 0;

  auto legals = board.generate_legal_moves();
  int score, score2;
  for (auto& move : legals) {
    // if (!max_breadth--) break;
    board.make_move(move);
    auto t1 = chrono::high_resolution_clock::now();
    score = -alphabeta(depth, -1e6, +1e6);
    // score2 = -negamax(depth);
    auto diff = chrono::duration_cast<chrono::milliseconds>(
                    chrono::high_resolution_clock::now() - t1)
                    .count();
    time += diff;
    // ofstream log("log.txt", std::ios_base::app);
    // log << board.to_san(move) << " = " << score << "\n\n";
    // log.close();}
    board.unmake_move(move);
    // cout << "info " << board.to_san(move) << " = " << score2 << endl;
    cout << "info score cp " << score << " depth " << depth << " nodes 1 time "
         << diff << " pv " << board.to_san(move) << endl;

    // if (score > bestscore) {
    //   bestscore = score;
    //   bestmove = move;
    // }
    bestmoves.push_back({move, score});
  }

  // {
  //   ofstream log("log.txt", std::ios_base::app);
  //   log << "best: " << board.to_san(bestmove) << " = " << bestscore <<
  //   "\n\n"; log.close();
  // }
  // if (bestmove.from == bestmove.to) bestmove = legals.front();
  // cout << "info time: " << time << endl;
  // cout << "negamax time: " << negamax_time << endl;
  // cout << "info best: " << board.to_san(bestmove) << " = " << bestscore <<
  // endl;
  return bestmoves;
}

inline int material_value(char piece) {
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

inline int AI::eval() {
  // int w_material = 1;

  // int white_material = 0, black_material = 0;
  // for (auto& piece : board.board) {
  //   int v = material_value(piece);
  //   if (v < 0)
  //     black_material -= v;
  //   else
  //     white_material += v;
  // }
  // int material_score = white_material - black_material;

  // return w_material * material_score;

  int material_score = 0;
  int pst_score = 0;
  int queens = 0;
  for (int i = 0; i < 64; i++) {
    char piece = board.board[i];
    if (piece == 'Q' || piece == 'q') queens++;
    material_score += piece_val[piece];
    switch (piece) {
      case 'P':
        pst_score += pst_p[i];
        break;
      case 'N':
        pst_score += pst_n[i];
        break;
      case 'B':
        pst_score += pst_b[i];
        break;
      case 'R':
        pst_score += pst_r[i];
        break;
      case 'Q':
        pst_score += pst_q[i];
        break;
      case 'p':
        pst_score -= pst_p[63 - i];
        break;
      case 'n':
        pst_score -= pst_n[63 - i];
        break;
      case 'b':
        pst_score -= pst_b[63 - i];
        break;
      case 'r':
        pst_score -= pst_r[63 - i];
        break;
      case 'q':
        pst_score -= pst_q[63 - i];
        break;
    }
  }

  if (queens == 0) {  // assume near endgame
    pst_score += pst_k_end[board.Kpos] - pst_k_end[63 - board.kpos];
    // https://www.chessprogramming.org/Mop-up_Evaluation
    pst_score += 5 * (material_score > 0 ? pst_cmd[board.Kpos]
                                         : pst_cmd[63 - board.kpos]);
    int file1 = board.Kpos % 8, rank1 = board.Kpos / 8;
    int file2 = board.kpos % 8, rank2 = board.kpos / 8;
    pst_score += 2 * (14 - (abs(rank2 - rank1) + abs(file2 - file1)));
  } else {
    pst_score += pst_k_middle[board.Kpos] - pst_k_middle[63 - board.kpos];
  }

  return material_score + pst_score;
}

int AI::negamax(int depth) {
  while (g_main_context_pending(0)) g_main_context_iteration(0, 0);

  if (depth == 0) return eval() * board.turn;
  int bestscore = -1e6;
  int max_breadth = 5;
  auto legals = board.generate_legal_moves();
  for (auto& move : legals) {
    board.make_move(move);
    int score = -negamax(depth - 1);
    board.unmake_move(move);
    bestscore = max(bestscore, score);
  }
  if (legals.size() == 0) return board.is_in_check(board.turn) ? -1e4 : 0;

  return bestscore;
}

int AI::alphabeta(int depth, int alpha, int beta) {
  while (g_main_context_pending(0)) g_main_context_iteration(0, 0);

  if (depth == 0) return quiesce(alpha, beta);
  int bestscore = -1e6;

  auto legals = board.generate_legal_moves();

  for (auto& move : legals) {
    board.make_move(move);
    int score = -alphabeta(depth - 1, -beta, -alpha);
    board.unmake_move(move);
    bestscore = max(bestscore, score);
    alpha = max(alpha, score);
    if (alpha >= beta) return alpha + legals.size() + depth;
  }
  if (legals.size() == 0) return board.is_in_check(board.turn) ? -1e6 : 0;

  return bestscore + legals.size() + depth;
}

int AI::quiesce(int alpha, int beta) {
  while (g_main_context_pending(0)) g_main_context_iteration(0, 0);

  int pat = eval() * board.turn;
  return pat;
  if (pat >= beta) return beta;

  // // delta pruning, 900=queen value
  // if (alpha - pat > 900) return alpha;

  alpha = max(alpha, pat);

  auto legals = board.generate_legal_moves();

  for (auto& move : legals) {
    if (board.empty(move.to)) continue;  // consider only captures
    board.make_move(move);
    int score = -quiesce(-beta, -alpha);
    board.unmake_move(move);
    if (score >= beta) return beta;
    alpha = max(alpha, score);
  }
  if (legals.size() == 0) return board.is_in_check(board.turn) ? -1e6 : 0;

  return alpha;
}