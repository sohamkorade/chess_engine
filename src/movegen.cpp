#include "movegen.hpp"

template <Direction dir>
inline void only_capture(Position& pos, vector<Move>& movelist, int sq) {
  if (is_safe<dir>(sq) && hostile(pos[sq], pos[sq + dir]))
    movelist.emplace_back(sq, sq + dir);
}
template <Direction dir>
inline bool only_move(Position& pos, vector<Move>& movelist, int sq) {
  if (is_safe<dir>(sq) && pos[sq + dir] == Empty) {
    movelist.emplace_back(sq, sq + dir);
    return true;
  }
  return false;
}

template <Direction dir>
inline void slide(Position& pos, vector<Move>& movelist, int sq) {
  if (!is_safe<dir>(sq)) return;
  for (int dest = sq + dir; !friendly(pos[sq], pos[dest]); dest += dir) {
    movelist.emplace_back(sq, dest);
    if (pos[dest] != Empty) break;
    if (!is_safe<dir>(dest)) break;
  }
}

template <Direction dir, Piece piece>
inline bool is_occupied(Position& pos, int sq) {
  return is_safe<dir>(sq) && pos[sq + dir] == piece;
}

template <Direction dir, Piece p1, Piece p2, Piece King>
inline bool diagonal_threats(Position& pos, int sq) {
  if (!is_safe<dir>(sq)) return false;
  if (pos[sq + dir] == King) return true;
  for (int dest = sq + dir; !friendly(pos[sq], pos[dest]); dest += dir) {
    if (pos[dest] == p1 || pos[dest] == p2) return true;
    if (pos[dest] != Empty) break;  // different from slide
    if (!is_safe<dir>(dest)) break;
  }
  return false;
}

template <Player turn>
bool is_in_threat(Position& pos, int sq) {
  // generate and check reverse threats from sq
  constexpr Piece oppK = Piece(-turn * wK);
  constexpr Piece oppP = Piece(-turn * wP);
  constexpr Piece oppN = Piece(-turn * wN);
  constexpr Piece oppB = Piece(-turn * wB);
  constexpr Piece oppR = Piece(-turn * wR);
  constexpr Piece oppQ = Piece(-turn * wQ);

  // for debugging
  // #define true (printf("%d\n", __LINE__) || true)

  // check for pawn threats (relative to turn)
  if (is_occupied<Direction(turn * NW), oppP>(pos, sq)) return true;
  if (is_occupied<Direction(turn * NE), oppP>(pos, sq)) return true;

  // check for knight threats
  if (is_occupied<NNW, oppN>(pos, sq)) return true;
  if (is_occupied<NNE, oppN>(pos, sq)) return true;
  if (is_occupied<WNW, oppN>(pos, sq)) return true;
  if (is_occupied<WSW, oppN>(pos, sq)) return true;
  if (is_occupied<ENE, oppN>(pos, sq)) return true;
  if (is_occupied<ESE, oppN>(pos, sq)) return true;
  if (is_occupied<SSW, oppN>(pos, sq)) return true;
  if (is_occupied<SSE, oppN>(pos, sq)) return true;

  // check for king, bishop, rook queen threats
  if (diagonal_threats<NW, oppB, oppQ, oppK>(pos, sq)) return true;
  if (diagonal_threats<NE, oppB, oppQ, oppK>(pos, sq)) return true;
  if (diagonal_threats<SW, oppB, oppQ, oppK>(pos, sq)) return true;
  if (diagonal_threats<SE, oppB, oppQ, oppK>(pos, sq)) return true;
  if (diagonal_threats<N, oppR, oppQ, oppK>(pos, sq)) return true;
  if (diagonal_threats<S, oppR, oppQ, oppK>(pos, sq)) return true;
  if (diagonal_threats<E, oppR, oppQ, oppK>(pos, sq)) return true;
  if (diagonal_threats<W, oppR, oppQ, oppK>(pos, sq)) return true;

#undef true

  return false;  // no threats found
}

template <Direction dir>
inline void jump(Position& pos, vector<Move>& movelist, int sq) {
  if (is_safe<dir>(sq) && !friendly(pos[sq], pos[sq + dir]))
    movelist.emplace_back(sq, sq + dir);
}

template <Player turn>
inline void generate_king_moves(Board& board, vector<Move>& pseudo) {
  const int K_pos = turn == White ? board.Kpos : board.kpos;
  jump<NE>(board.board, pseudo, K_pos);
  jump<NW>(board.board, pseudo, K_pos);
  jump<SE>(board.board, pseudo, K_pos);
  jump<SW>(board.board, pseudo, K_pos);
  jump<N>(board.board, pseudo, K_pos);
  jump<S>(board.board, pseudo, K_pos);
  jump<E>(board.board, pseudo, K_pos);
  jump<W>(board.board, pseudo, K_pos);
}

template <Player turn>
inline void generate_promotions_and_ep(Board& board, vector<Move>& pseudo) {
  constexpr int prom_start_sq = turn == White ? 8 : 48;
  constexpr Direction rel_N = turn == White ? N : S;
  constexpr Direction rel_NW = turn == White ? NW : SW;
  constexpr Direction rel_NE = turn == White ? NE : SE;
  constexpr Direction rel_SW = turn == White ? SW : NW;
  constexpr Direction rel_SE = turn == White ? SE : NE;
  constexpr Piece rel_P = turn == White ? wP : bP;

  for (int sq = prom_start_sq; sq < prom_start_sq + 8; sq++) {
    if (board[sq] != rel_P) continue;
    if (board.empty(sq + rel_N))
      for (auto& piece : {wQ, wR, wB, wN})
        pseudo.emplace_back(sq, sq + rel_N, Piece(piece * turn));
    if (isnt_A(sq) && hostile(board[sq], board[sq + rel_NW]))
      for (auto& piece : {wQ, wR, wB, wN})
        pseudo.emplace_back(sq, sq + rel_NW, Piece(piece * turn));
    if (isnt_H(sq) && hostile(board[sq], board[sq + rel_NE]))
      for (auto& piece : {wQ, wR, wB, wN})
        pseudo.emplace_back(sq, sq + rel_NE, Piece(piece * turn));
  }

  const auto& ep_sq = board.enpassant_sq_idx;
  // only one enpassant square can be occupied at a time, so `else` is safe
  if (~ep_sq) {
    // capture enpassant NW
    if (is_occupied<rel_SE, rel_P>(board.board, ep_sq))
      pseudo.emplace_back(ep_sq + rel_SE, ep_sq, Empty, Empty, true);
    // capture enpassant NE
    else if (is_occupied<rel_SW, rel_P>(board.board, ep_sq))
      pseudo.emplace_back(ep_sq + rel_SW, ep_sq, Empty, Empty, true);
  }
}

template <Player turn>
void generate_pawn_moves(Board& board, vector<Move>& pseudo, int sq) {
  const int rank = sq / 8;
  const int rel_rank = turn == White ? rank : 7 - rank;

  // capture NW and NE
  only_capture<Direction(NW * turn)>(board.board, pseudo, sq);
  only_capture<Direction(NE * turn)>(board.board, pseudo, sq);

  // push
  if (only_move<Direction(N * turn)>(board.board, pseudo, sq))
    // double push only if push succeeds
    if (rel_rank == 6) only_move<Direction(NN * turn)>(board.board, pseudo, sq);
}

template <Player turn>
inline void generate_castling_moves(Board& board, vector<Move>& pseudo) {
  auto castling_rights = board.castling_rights;
#define empty board.empty

  if constexpr (turn == White) {
    // kingside
    if (castling_rights[0] && empty(61) && empty(62))
      pseudo.emplace_back(60, 60 + E + E, Empty, Empty, false, true);
    // queenside
    if (castling_rights[1] && empty(57) & empty(58) && empty(59))
      pseudo.emplace_back(60, 60 + W + W, Empty, Empty, false, true);
  } else {
    // kingside
    if (castling_rights[2] && empty(5) && empty(6))
      pseudo.emplace_back(4, 4 + E + E, Empty, Empty, false, true);
    // queenside
    if (castling_rights[3] && empty(1) && empty(2) && empty(3))
      pseudo.emplace_back(4, 4 + W + W, Empty, Empty, false, true);
  }

#undef empty
}

int perft(Board& board, int depth) {
  if (depth <= 0) return 1;

  auto legal = generate_legal_moves(board);
  if (depth == 1) return legal.size();

  int nodes = 0;

  for (auto& move : legal) {
    board.make_move(move);
    nodes += perft(board, depth - 1);
    board.unmake_move(move);
  }
  return nodes;
}

int divide(Board& board, int depth) {
  int sum = 0;
  vector<string> moves;
  moves.reserve(40);
  auto t1 = chrono::high_resolution_clock::now();
  auto legal = generate_legal_moves(board);
  // Board before = board;
  for (auto& move : legal) {
    board.make_move(move);
    // Board after = board;
    int nodes = perft(board, depth - 1);
    board.unmake_move(move);
    // if (before.to_fen() != board.to_fen()) {
    //   cerr << "! "
    //        << "undo failed" << endl;
    //   cerr << "move: ";
    //   move.print();
    //   cerr << "before:       " << before.to_fen() << endl;
    //   cerr << "after move:   " << after.to_fen() << endl;
    //   cerr << "after unmove: " << board.to_fen() << endl;
    //   break;
    // }
    sum += nodes;
    moves.push_back(board.to_uci(move) + ": " + to_string(nodes));
  }
  auto t2 = chrono::high_resolution_clock::now();
  auto diff = chrono::duration_cast<chrono::nanoseconds>(t2 - t1).count();

  sort(moves.begin(), moves.end());
  for (auto& move : moves) cerr << move << endl;
  cerr << "Moves: " << legal.size() << endl;
  cerr << "Nodes: " << sum << endl;
  cerr << "Nodes/sec: " << int(diff ? sum * 1e9 / diff : -1) << endl;
  return sum;
}

template <Player turn>
vector<Move> generate_pseudo_moves(Board& board) {
  // cout << "gen pseudo @" << zobrist_hash() << endl;
  vector<Move> pseudo;
  pseudo.reserve(40);  // average number of pseudo moves per position

  // we know where the king is
  generate_king_moves<turn>(board, pseudo);
  generate_castling_moves<turn>(board, pseudo);
  generate_promotions_and_ep<turn>(board, pseudo);

  constexpr Piece rel_P = Piece(wP * turn);
  constexpr Piece rel_N = Piece(wN * turn);
  constexpr Piece rel_B = Piece(wB * turn);
  constexpr Piece rel_R = Piece(wR * turn);
  constexpr Piece rel_Q = Piece(wQ * turn);

  for (int sq = 0; sq < 64; sq++) {
    const int rank = sq / 8;
    const int rel_rank = turn == White ? rank : 7 - rank;
    const Piece p = board[sq];
    if (p == rel_P && rel_rank != 1)
      generate_pawn_moves<turn>(board, pseudo, sq);
    else if (p == rel_N) {
      jump<NNW>(board.board, pseudo, sq);
      jump<NNE>(board.board, pseudo, sq);
      jump<WNW>(board.board, pseudo, sq);
      jump<WSW>(board.board, pseudo, sq);
      jump<ENE>(board.board, pseudo, sq);
      jump<ESE>(board.board, pseudo, sq);
      jump<SSW>(board.board, pseudo, sq);
      jump<SSE>(board.board, pseudo, sq);
    }
    if (p == rel_B || p == rel_Q) {
      slide<NW>(board.board, pseudo, sq);
      slide<NE>(board.board, pseudo, sq);
      slide<SW>(board.board, pseudo, sq);
      slide<SE>(board.board, pseudo, sq);
    }
    if (p == rel_R || p == rel_Q) {
      slide<N>(board.board, pseudo, sq);
      slide<S>(board.board, pseudo, sq);
      slide<E>(board.board, pseudo, sq);
      slide<W>(board.board, pseudo, sq);
    }
  }
  return pseudo;
}

vector<Move> generate_pseudo_moves(Board& board) {
  if (board.turn == White)
    return generate_pseudo_moves<White>(board);
  else
    return generate_pseudo_moves<Black>(board);
}

// check if a pseudo-legal move is legal
template <Player turn>
bool is_legal(Board& board, Move& move) {
  bool legal = true;
  if (move.castling) {
    // check if any squares that king moves to are threatened
    auto threats = get_threats<Player(-turn)>(board);
    if (  // white kingside castling
        (move.from == 60 && move.to == 62 &&
         (threats[60] || threats[61] || threats[62])) ||
        // black queenside castling
        (move.from == 4 && move.to == 6 &&
         (threats[4] || threats[5] || threats[6])) ||
        // white kingside castling
        (move.from == 60 && move.to == 58 &&
         (threats[60] || threats[59] || threats[58])) ||
        // black queenside castling
        (move.from == 4 && move.to == 2 &&
         (threats[4] || threats[3] || threats[2]))) {
      legal = false;
    }
  } else {
    // check if king is threatened
    board.make_move(move);
    legal = !is_in_check(board, turn);
    board.unmake_move(move);
  }
  return legal;
}

template <Player turn>
vector<Move> generate_legal_moves(Board& board) {
  // cout << "gen legal @" << zobrist_hash() << endl;
  auto movelist = generate_pseudo_moves<turn>(board);
  vector<Move> better_moves, others;
  better_moves.reserve(movelist.size());
  others.reserve(movelist.size());

  for (auto& move : movelist) {
    if (is_legal<turn>(board, move))
      (board.empty(move.to) || move.promotion != Empty || move.enpassant ||
               move.castling
           ? better_moves
           : others)
          .push_back(move);
  }

  better_moves.insert(better_moves.end(), others.begin(), others.end());
  return better_moves;
}

vector<Move> generate_legal_moves(Board& board) {
  if (board.turn == White)
    return generate_legal_moves<White>(board);
  else
    return generate_legal_moves<Black>(board);
}

bool make_move_if_legal(Board& board, string move) {
  int l = move.length();
  if (l == 4 || l == 5) {
    const int from = sq2idx(move[0], move[1]);
    const int to = sq2idx(move[2], move[3]);
    Piece promotion = Empty;
    if (l == 5)
      promotion =
          char2piece(board.turn == White ? toupper(move[4]) : tolower(move[4]));
    for (auto& m : generate_legal_moves(board))
      if (m.from == from && m.to == to && m.promotion == promotion) {
        board.make_move(m);
        return true;
      }
  }
  return false;
}

string to_san(Board& board, Move move) {
  string san;
  // castling
  if (move.castling) {
    if ((move.from == 4 && move.to == 6) || (move.from == 60 && move.to == 62))
      san = "O-O";
    else if ((move.from == 4 && move.to == 2) ||
             (move.from == 60 && move.to == 58))
      san = "O-O-O";
  } else {
    char piece = toupper(piece2char(board.board[move.from]));
    string origin = idx2sq(move.from);
    string target = idx2sq(move.to);

    // piece identification
    if (piece != 'P') san = piece;

    // disambiguation
    bool same_file = false, same_rank = false;
    for (auto& move2 : generate_pseudo_moves(board)) {
      if (move2.from != move.from)                    // don't compare to self
        if (move2.to == move.to)                      // same destination
          if (board[move2.from] == board[move.from])  // same kind of piece
          {
            if (move2.from / 8 == move.from / 8)  // same rank
              same_file = true;
            if (move2.from % 8 == move.from % 8)  // same file
              same_rank = true;
          }
    }
    if (same_file && piece != 'P') san += origin[0];  // add file, if not pawn
    if (same_rank) san += origin[1];                  // add rank

    // capture
    if (!board.empty(move.to) || move.enpassant) {
      if (piece == 'P') san += origin[0];  // add file if piece is pawn
      san += 'x';
    }

    // target
    san += target;

    // promotion
    if (move.promotion != Empty) {
      san += '=';
      san += toupper(piece2char(move.promotion));
    }
  }
  board.make_move(move);
  if (generate_legal_moves(board).size() == 0)  // mate indication
    san += '#';
  else if (is_in_check(board, board.turn))  // check indication
    san += '+';
  board.unmake_move(move);
  return san;
}
