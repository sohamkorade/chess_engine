#include "ai.hpp"

#include "knowledge.hpp"

int piece_val[13] = {0};

AI::AI(Board& _board) : board(_board) {}
void AI::set_clock(int _wtime, int _btime, int _winc, int _binc) {
  wtime = _wtime;
  btime = _btime;
  winc = _winc;
  binc = _binc;
}

pair<Move, int> AI::search_best_move(multiset<string>& transpositions) {
  piece_val[wK + 6] = 10000;
  piece_val[wQ + 6] = 900;
  piece_val[wR + 6] = 500;
  piece_val[wB + 6] = 300;
  piece_val[wN + 6] = 250;
  piece_val[wP + 6] = 100;
  piece_val[bK + 6] = -10000;
  piece_val[bQ + 6] = -900;
  piece_val[bR + 6] = -500;
  piece_val[bB + 6] = -300;
  piece_val[bN + 6] = -250;
  piece_val[bP + 6] = -100;
  int bestscore = -1e8;
  Move bestmove;
  Board temp = board;
  auto movelist = get_best_moves();
  cout << "info bestmoves: " << movelist.size() << endl;
  // auto movelist = monte_carlo();
  for (int i = 1; i >= 0; i--) {
    for (auto& m : movelist) {
      temp.make_move(m.second);
      int tp = transpositions.count(temp.pos_hash());
      temp.unmake_move(m.second);
      if (tp > i) continue;
      if (m.first > bestscore) {
        bestscore = m.first;
        bestmove = m.second;
      }
    }
    if (bestmove.from != bestmove.to) break;
  }
  cout << "info bestmove: " << board.to_san(bestmove) << " = " << bestscore
       << endl;
  return {bestmove, bestscore};
}

vector<pair<int, Move>> AI::get_best_moves() {
  vector<pair<int, Move>> bestmoves, tempmoves;
  // Move bestmove;
  // int bestscore = -1e8;
  // int max_breadth = 5;
  // {
  //   ofstream log("log.txt");
  //   log << "";
  //   log.close();
  // }
  int time_taken = 0;
  int max_time = (board.turn == White) ? (wtime + winc) : (btime + binc);
  // max_time /= 50 - board.moves;
  // max_time = 60000;
  // from stockfish
  if (max_time == 0) {
    max_time = INT_MAX;
    cout << "info using maxdepth: " << max_depth << endl;
  } else {
    double percentage = 1;  // 0.88;
    max_time *= min((percentage + board.moves / 116.4) / 50, percentage);
    cout << "info using time: " << max_time << endl;
  }

  auto legals = board.generate_legal_moves();
  vector<pair<int, Move>> legalmoves;
  for (auto& move : legals) legalmoves.push_back({0, move});
  // int late_moves = legals.size() / 2;
  // if (late_moves > 5) late_moves = 5;
  for (int depth = 1; time_taken * 2 < max_time && depth <= max_depth;
       depth++) {
    tempmoves = bestmoves;
    bestmoves.clear();
    sort(legalmoves.begin(), legalmoves.end(),
         [](auto& a, auto& b) { return a.first > b.first; });

    // mate found so prune non-mating moves (TODO: verify)
    if (legalmoves.front().first > 1e6 / 2) {
      for (auto& move : legalmoves)
        if (abs(move.first) > 1e6 / 2) bestmoves.push_back(move);
      break;
    }

    if (legalmoves.back().first < -1e6 / 2) {
      if (legalmoves.front().first >= -1e6 / 2) {
        // prune losing moves
        for (auto& move : legalmoves)
          if (move.first >= -1e6 / 2) bestmoves.push_back(move);
        if (bestmoves.size() != 0) legalmoves = bestmoves;
        // remove_if(legalmoves.begin(), legalmoves.end(),
        //           [](auto& move) { return move.first < -1e6 / 2; });
        // break;
      } else {
        // best and worst move is losing, so no point in searching deeper
        for (auto& move : legalmoves) bestmoves.push_back(move);
        break;
      }
    }

    // no need to seach deeper if there's only one legal move
    if (legalmoves.size() == 1) {
      bestmoves.push_back(legalmoves.front());
      break;
    }

    for (auto& score_move : legalmoves) {
      // cout << "searching move: " << board.to_san(score_move.second)
      //      << " score: " << score_move.first << endl;
      // int realdepth = late_moves-- > 0 ? depth : depth - rand() % 2;
      int score;
      // if (!max_breadth--) break;
      board.make_move(score_move.second);
      auto t1 = chrono::high_resolution_clock::now();
      score = -alphabeta(depth, -1e6, +1e6);
      // int score2 = -negamax(depth);
      auto t2 = chrono::high_resolution_clock::now();
      auto diff = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();
      board.unmake_move(score_move.second);
      time_taken += diff;
      if (time_taken >= max_time && bestmoves.size() > 1) {
        for (auto& x : tempmoves) bestmoves.push_back(x);
        break;
      }
      score_move.first = score;
      // ofstream log("log.txt", std::ios_base::app);
      // log << board.to_san(move) << " = " << score << "\n\n";
      // log.close();}
      // cout << "info " << board.to_san(move) << " = " << score2 << endl;
      cout << "info"
           << " depth " << depth << " score cp " << score << " nodes 1 time "
           << diff << " pv " << board.to_uci(score_move.second) << endl;

      // if (score > bestscore) {
      // bestscore = score;
      // bestmove = move;
      // }
      bestmoves.push_back({score, score_move.second});
    }
    cout << "info time: " << time_taken << " depth: " << depth << endl;
  }

  // {
  //   ofstream log("log.txt", std::ios_base::app);
  //   log << "best: " << board.to_san(bestmove) << " = " << bestscore <<
  //   "\n\n"; log.close();
  // }
  // if (bestmove.from == bestmove.to) bestmove = legals.front();
  cout << "info total time: " << time_taken << endl;
  // cout << "negamax time taken: " << negamax_time << endl;
  // cout << "info best: " << board.to_san(bestmove) << " = " << bestscore <<
  // endl;
  return bestmoves;
}

int AI::print_eval() {
  int material_score = 0;
  int pst_score = 0;
  int queens = 0;
  for (int i = 0; i < 64; i++) {
    Piece piece = board.board[i];
    if (abs(piece) == wQ) queens++;
    material_score += piece_val[piece + 6];
    if (piece)
      pst_score +=
          pst[abs(piece) - 1][piece > 0 ? i : 63 - i] * (piece > 0 ? 1 : -1);
  }

  if (queens == 0) {  // assume near endgame
    pst_score += pst_k_end[board.Kpos] - pst_k_end[63 - board.kpos];
    // https://www.chessprogramming.org/Mop-up_Evaluation
    pst_score += 5 * (material_score > 0 ? pst_cmd[board.Kpos]
                                         : pst_cmd[63 - board.kpos]);
    int file1 = board.Kpos % 8, rank1 = board.Kpos / 8;
    int file2 = board.kpos % 8, rank2 = board.kpos / 8;
    pst_score += 2 * (14 - (abs(rank2 - rank1) + abs(file2 - file1)));
  }

  cout << "fen: " << board.to_fen() << endl;
  cout << "material: " << material_score << endl;
  cout << "position: " << pst_score << endl;

  return material_score + pst_score;
}

inline int AI::eval() {
  int material_score = 0;
  int pst_score = 0;
  int mobility_score = 0;
  int queens = 0;
  for (int i = 0; i < 64; i++) {
    Piece piece = board.board[i];
    if (abs(piece) == wQ) queens++;
    material_score += piece_val[piece + 6];
    pst_score += pst[abs(piece) - 1][piece > 0 ? i : 63 - i];
  }

  if (queens == 0) {  // assume near endgame
    pst_score += pst_k_end[board.Kpos] - pst_k_end[63 - board.kpos];
    // https://www.chessprogramming.org/Mop-up_Evaluation
    int cmd =
        (material_score > 0 ? pst_cmd[board.Kpos] : pst_cmd[63 - board.kpos]);
    int file1 = board.Kpos % 8, rank1 = board.Kpos / 8;
    int file2 = board.kpos % 8, rank2 = board.kpos / 8;
    int md = abs(rank2 - rank1) + abs(file2 - file1);
    pst_score += 5 * cmd + 2 * (14 - md);
  }

  // int rel_mobility = board.generate_legal_moves().size();
  // board.change_turn();
  // int opp_mobility = board.generate_legal_moves().size();
  // board.change_turn();
  // mobility_score = (rel_mobility - opp_mobility) * board.turn;

  return material_score + pst_score + mobility_score;
}

int AI::negamax(int depth) {
  // while (g_main_context_pending(0)) g_main_context_iteration(0, 0);

  if (depth == 0) return eval() * board.turn;
  int bestscore = -1e6;
  auto legals = board.generate_legal_moves();
  for (auto& move : legals) {
    board.make_move(move);
    int score = -negamax(depth - 1);
    board.unmake_move(move);
    bestscore = max(bestscore, score);
  }
  if (legals.size() == 0)
    return board.is_in_check(board.turn) ? -1e6 - depth : 0;

  return bestscore;
}

int reduction(int depth, int moves) { return 1; }

int AI::alphabeta(int depth, int alpha, int beta) {
  // while (g_main_context_pending(0)) g_main_context_iteration(0, 0);

  bool is_in_check = board.is_in_check(board.turn);
  if (is_in_check) depth++;
  if (depth <= 0) return quiesce(0, alpha, beta);
  int bestscore = -1e6;

  auto legals = board.generate_legal_moves();

  // if (legals.size() == 1) return 0;

  int moves = 0, late_moves = legals.size() / 2 + 1;
  int score;

  // null move pruning
  if (!is_in_check) {
    board.change_turn();
    score = -alphabeta(depth - reduction(depth, moves), -beta, -alpha);
    board.change_turn();
    if (score >= beta) return score;
  }

  for (auto& move : legals) {
    board.make_move(move);
    // late move reduction
    if (moves < late_moves) {
      score = -alphabeta(depth - 1, -beta, -alpha);
    } else {
      score = -alphabeta(depth - 1 - reduction(depth, moves), -beta, -alpha);
      if (score >= alpha) score = -alphabeta(depth - 1, -beta, -alpha);
    }
    board.unmake_move(move);
    if (score >= beta) return score;
    alpha = max(alpha, score);
    bestscore = max(bestscore, score);
    moves++;
  }
  // TODO: check + or - depth
  if (legals.size() == 0) return is_in_check ? -1e6 - depth : 0;

  return bestscore;
}

int AI::quiesce(int depth, int alpha, int beta) {
  // while (g_main_context_pending(0)) g_main_context_iteration(0, 0);

  int pat = eval() * board.turn;
  if (pat >= beta) return beta;
  if (depth > 5) return pat;

  // // delta pruning, 900=queen value
  // if (alpha - pat > 900) return alpha;

  alpha = max(alpha, pat);

  auto legals = board.generate_legal_moves();

  // if (legals.size() == 1) return 0;

  for (auto& move : legals) {
    if (board.empty(move.to)) continue;  // consider only captures
    board.make_move(move);
    int score = -quiesce(depth + 1, -beta, -alpha);
    board.unmake_move(move);
    if (score >= beta) return score;
    alpha = max(alpha, score);
  }
  if (legals.size() == 0)
    return board.is_in_check(board.turn) ? -1e6 - depth : 0;

  return alpha;
}