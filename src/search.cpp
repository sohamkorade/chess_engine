#include "search.hpp"

#include "knowledge.hpp"

const int MateScore = 1000000;
const int TT_miss = 404000;
int TT_size = 1 << 16;

void init_TT(TT_t& TT, int size) {
  // get next power of 2
  int n = 1;
  while (n < size) n <<= 1;
  TT_size = n;

  // allocate memory, half the size if fails
  do TT = new (nothrow) TTEntry[TT_size];
  while (!TT && (TT_size >>= 1));

  for (int i = 0; i < TT_size; i++) {
    TT[i].age = -1;
    TT[i].depth = 0;
  }
}

int normalize_score(int score, int depth) {
  if (score > MateScore) return score - depth;
  if (score < -MateScore) return score + depth;
  return score;
}

inline int TT_probe(TT_t TT, u_int64_t hash, int depth, int alpha, int beta) {
  TT_t entry = &TT[hash % TT_size];
  if (entry->hash != hash) return TT_miss;  // verify hash
  if (entry->age > 0 && entry->depth >= depth) {
    int score = normalize_score(entry->score, depth);
    if (entry->eval_type == Exact) return score;
    if (entry->eval_type == LowerBound && score >= beta) return beta;
    if (entry->eval_type == UpperBound && score <= alpha) return alpha;
  }
  return TT_miss;
}

inline void TT_store(TT_t TT, u_int64_t hash, int depth, int score,
                     EvalType eval_type) {
  TT_t entry = &TT[hash % TT_size];
  if (entry->depth > depth) return;  // don't overwrite deeper scores
  entry->age = 1;
  entry->hash = hash;
  entry->depth = depth;
  entry->score = normalize_score(score, depth);
  entry->eval_type = eval_type;
}

Search::Search() { init_TT(TT, TT_size); }

void Search::set_clock(int _wtime, int _btime, int _winc, int _binc) {
  wtime = _wtime;
  btime = _btime;
  winc = _winc;
  binc = _binc;
}

pair<Move, int> Search::search() {
  searching = true;
  auto movelist = iterative_search();
  searching = false;

  int bestscore = -1e8;
  Move bestmove;
  for (int i = 1; i >= 0; i--) {
    for (auto& m : movelist) {
      board.make_move(m.second);
      int tp = repetitions.count(board.zobrist_hash());
      board.unmake_move(m.second);
      if (tp > i) continue;
      if (m.first > bestscore) {
        bestscore = m.first;
        bestmove = m.second;
      }
    }
    if (bestmove.from != bestmove.to) break;
  }
  cout << "info bestmove: " << bestscore << " = " << to_san(board, bestmove)
       << " out of " << movelist.size() << "\n";
  cout << "bestmove " << bestmove.to_uci() << "\n";
  return {bestmove, bestscore};
}

int get_mate_score(int score) {
  if (score > MateScore / 2)
    return (MateScore - score) / 2 + 1;
  else if (score < -MateScore / 2)
    return (-score - MateScore) / 2 - 1;
  return 0;
}

void print_score(int score) {
  int mate = get_mate_score(score);
  if (mate == 0)
    cout << " score cp " << score;
  else
    cout << " score mate " << mate;
}

void print_info(string infostring, int depth, int score, int nodes_searched,
                int time_taken, string move) {
  cout << infostring << " depth " << depth;
  print_score(score);
  cout << " nodes " << nodes_searched << " time " << time_taken << " pv "
       << move << "\n";
}

// TODO: verify thoroughly
vector<pair<int, Move>> Search::iterative_search() {
  vector<pair<int, Move>> bestmoves, tempmoves;
  int time_taken = 0;
  int max_search_time = (board.turn == White) ? (wtime + winc) : (btime + binc);
  if (search_type == Fixed_depth) {
    max_search_time = INT_MAX;
    cout << "info using maxdepth: " << max_depth << "\n";
  } else if (search_type == Time_per_move) {
    max_search_time = mtime;
    cout << "info using movetime: " << max_search_time << "\n";
  } else if (search_type == Time_per_game) {
    double percentage = 1;  // 0.88;
    max_search_time *= min((percentage + board.moves / 116.4) / 50, percentage);
    cout << "info using time: " << max_search_time << "\n";
  } else {
    max_search_time = INT_MAX;
    cout << "info using infinite: " << max_search_time << "\n";
  }

  const auto start_time = chrono::high_resolution_clock::now();

  // convert moves to score-move pairs
  auto legals = generate_legal_moves(board);
  vector<pair<int, Move>> legalmoves;
  for (auto& move : legals) legalmoves.emplace_back(0, move);

  // iterative deepening
  int depth = 1;
  for (; searching && time_taken * 2 < max_search_time && depth <= max_depth;
       depth++) {
    tempmoves = bestmoves;
    bestmoves.clear();

    // if (search_type != Mate && search_type != Infinite) {
    // no need to seach deeper if there's only one legal move
    if (legalmoves.size() == 1) {
      bestmoves.push_back(legalmoves.front());
      break;
    }

    // mate found so prune non-mating moves (TODO: verify)
    if (get_mate_score(legalmoves.front().first) > 0) {
      for (auto& move : legalmoves)
        if (get_mate_score((move.first)) > 0) bestmoves.push_back(move);
      break;
    }
    // }

    if (get_mate_score(legalmoves.back().first) < 0) {
      // there is atleast one non-losing move
      if (get_mate_score(legalmoves.front().first) == 0) {
        // prune losing moves
        for (auto& move : legalmoves)
          if (get_mate_score(move.first) == 0) bestmoves.push_back(move);
        if (bestmoves.size() != 0) legalmoves = bestmoves;
        // break;
      } else {
        // best and worst move is losing, so no point in searching deeper
        for (auto& move : legalmoves) bestmoves.push_back(move);
        // if (search_type != Mate && search_type != Infinite)
        break;
      }
    }

    for (auto& score_move : legalmoves) {
      nodes_searched = 0;
      board.make_move(score_move.second);
      int score = 0;
      // score = -negamax(depth);
      score = -alphabeta(depth, -MateScore, +MateScore);
      board.unmake_move(score_move.second);
      time_taken = chrono::duration_cast<chrono::milliseconds>(
                       chrono::high_resolution_clock::now() - start_time)
                       .count();
      // break if time is up, but ensure we have atleast one move
      if (!searching || time_taken >= max_search_time)
        if (bestmoves.size() > 1) {
          for (auto& x : tempmoves) bestmoves.push_back(x);
          break;
        }
      score_move.first = score;
      print_info("info string", depth, score, nodes_searched, time_taken,
                 score_move.second.to_uci());
      bestmoves.emplace_back(score, score_move.second);
    }

    // move-ordering
    stable_sort(legalmoves.begin(), legalmoves.end(),
                [](auto& a, auto& b) { return a.first > b.first; });

    time_taken = chrono::duration_cast<chrono::milliseconds>(
                     chrono::high_resolution_clock::now() - start_time)
                     .count();
    print_info("info", depth, legalmoves.front().first, nodes_searched,
               time_taken, legalmoves.front().second.to_uci());

    // TODO: fix this
    if (search_type == Mate)
      debug = to_string(get_mate_score(legalmoves.front().first));
  }

  cout << "info total time: " << time_taken << "\n";

  // TODO: choose random move out of same-scoring moves

  return bestmoves;
}

int Search::print_eval() {
  int material_score = 0;
  int pst_score = 0;
  int mobility_score = 0;
  int queens = 0;
  for (int i = 0; i < 64; i++) {
    const Piece piece = board.board[i];
    if (abs(piece) == wQ) queens++;
    material_score += piece_val[piece + 6];
    if (piece)
      pst_score +=
          pst[abs(piece) - 1][piece > 0 ? i : 63 - i] * (piece > 0 ? 1 : -1);
  }

  if (queens == 0) {  // assume near endgame
    pst_score += pst_k_end[board.Kpos] - pst_k_end[63 - board.kpos];
    // https://www.chessprogramming.org/Mop-up_Evaluation
    const int cmd =
        (material_score > 0 ? pst_cmd[board.Kpos] : pst_cmd[63 - board.kpos]);
    const int file1 = board.Kpos % 8, rank1 = board.Kpos / 8;
    const int file2 = board.kpos % 8, rank2 = board.kpos / 8;
    const int md = abs(rank2 - rank1) + abs(file2 - file1);
    pst_score += 5 * cmd + 2 * (14 - md);
  }

  // int rel_mobility = board.generate_legal_moves().size();
  // board.change_turn();
  // int opp_mobility = board.generate_legal_moves().size();
  // board.change_turn();
  // mobility_score = (rel_mobility - opp_mobility) * board.turn;

  board.print();

  cout << "fen: " << board.to_fen() << "\n";
  cout << "material: " << material_score << "\n";
  cout << "position: " << pst_score << "\n";

  cout << "is in check: " << is_in_check(board, board.turn) << "\n";

  return material_score + pst_score + mobility_score;
}

inline int Search::eval() {
  int material_score = 0;
  int pst_score = 0;
  int mobility_score = 0;
  int queens = 0;
  for (int i = 0; i < 64; i++) {
    const Piece piece = board.board[i];
    if (abs(piece) == wQ) queens++;
    material_score += piece_val[piece + 6];
    if (piece)
      pst_score +=
          pst[abs(piece) - 1][piece > 0 ? i : 63 - i] * (piece > 0 ? 1 : -1);
  }

  if (queens == 0) {  // assume near endgame
    pst_score += pst_k_end[board.Kpos] - pst_k_end[63 - board.kpos];
    // https://www.chessprogramming.org/Mop-up_Evaluation
    const int cmd =
        (material_score > 0 ? pst_cmd[board.Kpos] : pst_cmd[63 - board.kpos]);
    const int file1 = board.Kpos % 8, rank1 = board.Kpos / 8;
    const int file2 = board.kpos % 8, rank2 = board.kpos / 8;
    const int md = abs(rank2 - rank1) + abs(file2 - file1);
    pst_score += 5 * cmd + 2 * (14 - md);
  }

  // int rel_mobility = board.generate_legal_moves().size();
  // board.change_turn();
  // int opp_mobility = board.generate_legal_moves().size();
  // board.change_turn();
  // mobility_score = (rel_mobility - opp_mobility) * board.turn;

  return material_score + pst_score + mobility_score;
}

int Search::negamax(int depth) {
  if (depth == 0) return eval() * board.turn;
  int bestscore = -MateScore;
  auto legals = generate_legal_moves(board);
  for (auto& move : legals) {
    board.make_move(move);
    int score = -negamax(depth - 1);
    board.unmake_move(move);
    if (score > bestscore) {
      bestscore = score;
    }
  }
  if (legals.size() == 0)
    return is_in_check(board, board.turn) ? -MateScore + depth : 0;

  return bestscore;
}

int Search::alphabeta(int depth, int alpha, int beta) {
  nodes_searched++;

  // TODO: check repetition
  // return 0;

  // TODO: probe TT
  const int TT_score = TT_probe(TT, board.zobrist_hash(), depth, alpha, beta);
  if (TT_score != TT_miss) return TT_score;

  if (depth == 0 || depth > max_depth) return quiesce(0, alpha, beta);

  bool in_check = is_in_check(board, board.turn);

  // check extension
  if (in_check) depth++;

  int score = 0;

  auto legals = generate_legal_moves(board);

  EvalType eval_type = UpperBound;

  for (auto& move : legals) {
    board.make_move(move);
    // TODO: late move reduction
    // full window search if not LMR
    score = -alphabeta(depth - 1, -beta, -alpha);
    board.unmake_move(move);
    if (score > alpha) {
      // TODO: PV update
      eval_type = Exact;
      alpha = score;
      if (alpha >= beta) {  // fail-high beta-cutoff
        // TODO: store TT
        TT_store(TT, board.zobrist_hash(), depth, beta, LowerBound);
        return beta;
      }
    }
  }

  // checkmate or stalemate
  if (legals.size() == 0) return in_check ? -MateScore + depth : 0;

  // TODO: store TT
  TT_store(TT, board.zobrist_hash(), depth, alpha, eval_type);
  return alpha;  // fail-low alpha-cutoff
}

int Search::quiesce(int depth, int alpha, int beta) {
  if (depth > max_depth) return eval() * board.turn;

  int stand_pat = eval() * board.turn;
  if (stand_pat >= beta) {  // fail-high beta-cutoff
    return beta;
  }

  if (stand_pat > alpha) {  // fail-low alpha-cutoff
    alpha = stand_pat;
  }

  nodes_searched++;

  auto legals = generate_legal_moves(board);

  for (auto& move : legals) {
    if (board[move.to] == Empty) continue;  // ignore quiet moves
    board.make_move(move);
    int score = -quiesce(depth + 1, -beta, -alpha);
    board.unmake_move(move);
    if (score > alpha) {
      alpha = score;
      if (alpha >= beta) {  // fail-high beta-cutoff
        return beta;
      }
    }
  }

  return alpha;  // fail-low alpha-cutoff
}