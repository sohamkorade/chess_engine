#include "board.hpp"

void Board::generate_king_moves(vector<Move>& pseudo, int sq) {
  if (isnt_1(sq)) move_or_capture(pseudo, sq, S);                 // S
  if (isnt_8(sq)) move_or_capture(pseudo, sq, N);                 // N
  if (isnt_H(sq)) move_or_capture(pseudo, sq, E);                 // E
  if (isnt_A(sq)) move_or_capture(pseudo, sq, W);                 // W
  if (isnt_1(sq) && isnt_H(sq)) move_or_capture(pseudo, sq, SE);  // SE
  if (isnt_1(sq) && isnt_A(sq)) move_or_capture(pseudo, sq, SW);  // SW
  if (isnt_8(sq) && isnt_H(sq)) move_or_capture(pseudo, sq, NE);  // NE
  if (isnt_8(sq) && isnt_A(sq)) move_or_capture(pseudo, sq, NW);  // NW
}

inline void Board::generate_promotions(vector<Move>& pseudo, int sq,
                                       Direction dir) {
  for (auto& piece : {wQ, wR, wB, wN})
    pseudo.push_back(Move(sq, sq + dir, static_cast<Piece>(piece * turn)));
}

template <Player turn>
void Board::generate_pawn_moves(vector<Move>& pseudo, int sq) {
  constexpr Direction rel_N = turn == White ? N : S;
  constexpr Direction rel_NW = turn == White ? NW : SW;
  constexpr Direction rel_NE = turn == White ? NE : SE;
  const int rank = sq / 8;
  const int rel_rank = turn == White ? rank : 7 - rank;
  const Piece piece = board[sq];

  if (empty(sq + rel_N)) {
    if (rel_rank == 1)
      generate_promotions(pseudo, sq, rel_N);  // promotion
    else
      pseudo.push_back(Move(sq, sq + rel_N));  // push
    if (rel_rank == 6 && empty(sq + rel_N + rel_N))
      pseudo.push_back(Move(sq, sq + rel_N + rel_N));  // double push
  }
  if (isnt_A(sq) && hostile(piece, board[sq + rel_NW])) {  // capture NW
    if (rel_rank == 1)
      generate_promotions(pseudo, sq, rel_NW);  // promotion capture
    else
      pseudo.push_back(Move(sq, sq + rel_NW));
  }
  if (isnt_H(sq) && hostile(piece, board[sq + rel_NE])) {  // capture NE
    if (rel_rank == 1)
      generate_promotions(pseudo, sq, rel_NE);  // promotion capture
    else
      pseudo.push_back(Move(sq, sq + rel_NE));
  }
  // for (const auto& dir : {rel_NW, rel_NE}) {
  //   if (is_safe<rel_NW>(sq) && hostile(piece, board[sq + dir])) {
  //     if (rel_rank == 1)
  //       generate_promotions(pseudo, sq, dir);  // promotion capture
  //     else
  //       pseudo.push_back(Move(sq, sq + dir));
  //   }
  // }
  if (rel_rank == 3) {
    if (sq + rel_NW == enpassant_sq_idx)  // capture enpassant NW
      pseudo.push_back(Move(sq, sq + rel_NW, Empty, Empty, true));
    else if (sq + rel_NE == enpassant_sq_idx)  // capture enpassant NE
      pseudo.push_back(Move(sq, sq + rel_NE, Empty, Empty, true));
  }
}

void Board::generate_knight_moves(vector<Move>& pseudo, int sq) {
  const int file = sq % 8, rank = sq / 8;

  if (rank > 1) {
    if (isnt_A(sq)) move_or_capture(pseudo, sq, N + NW);  // UL
    if (isnt_H(sq)) move_or_capture(pseudo, sq, N + NE);  // UR
  }
  if (file > 1) {
    if (isnt_8(sq)) move_or_capture(pseudo, sq, W + NW);  // LU
    if (isnt_1(sq)) move_or_capture(pseudo, sq, W + SW);  // LD
  }
  if (file < 6) {
    if (isnt_8(sq)) move_or_capture(pseudo, sq, E + NE);  // RU
    if (isnt_1(sq)) move_or_capture(pseudo, sq, E + SE);  // RD
  }
  if (rank < 6) {
    if (isnt_A(sq)) move_or_capture(pseudo, sq, S + SW);  // DL
    if (isnt_H(sq)) move_or_capture(pseudo, sq, S + SE);  // DR
  }
}

void Board::generate_castling_moves(vector<Move>& pseudo) {
  // castling
  if (turn == White) {
    // kingside
    if (castling_rights[0] && empty(61) && empty(62))
      pseudo.push_back(Move(60, 60 + E + E, Empty, Empty, false, true));
    // queenside
    if (castling_rights[1] && empty(57) & empty(58) && empty(59))
      pseudo.push_back(Move(60, 60 + W + W, Empty, Empty, false, true));
  } else {
    // kingside
    if (castling_rights[2] && empty(5) && empty(6))
      pseudo.push_back(Move(4, 4 + E + E, Empty, Empty, false, true));
    // queenside
    if (castling_rights[3] && empty(1) && empty(2) && empty(3))
      pseudo.push_back(Move(4, 4 + W + W, Empty, Empty, false, true));
  }
}

vector<Move> Board::generate_pseudo_moves() {
  // cout << "gen pseudo @" << zobrist_hash() << endl;
  vector<Move> pseudo;
  pseudo.reserve(40);  // average number of pseudo moves per position

  generate_castling_moves(pseudo);

  for (int sq = 0; sq < 64; sq++) {
    if (!hostile(board[sq], static_cast<Piece>(turn))) {
      const Piece rel_piece = static_cast<Piece>(abs(board[sq]));
      if (rel_piece == wK)
        generate_king_moves(pseudo, sq);
      else if (rel_piece == wP)
        if (turn == White)
          generate_pawn_moves<White>(pseudo, sq);
        else
          generate_pawn_moves<Black>(pseudo, sq);
      else if (rel_piece == wN)
        generate_knight_moves(pseudo, sq);
      else if (rel_piece == wB)
        slide(pseudo, sq, {NW, NE, SW, SE});
      else if (rel_piece == wR)
        slide(pseudo, sq, {N, S, E, W});
      else if (rel_piece == wQ)
        slide(pseudo, sq, {N, S, E, W, NW, NE, SW, SE});
    }
  }
  return pseudo;
}

vector<Move> Board::generate_legal_moves() {
  // cout << "gen legal @" << zobrist_hash() << endl;
  auto movelist = generate_pseudo_moves();
  vector<Move> sorted_moves, others;
  sorted_moves.reserve(movelist.size());
  others.reserve(movelist.size());

  for (auto& move : movelist) {
    bool valid_move = true;
    // cout << "checking move ";
    // move.print();
    if (move.castling) {
      // check if any squares between king and rook are threatened
      auto threats = get_threats();
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
        valid_move = false;
      }
    } else {
      // check if king is threatened
      make_move(move);
      valid_move = !is_in_check(static_cast<Player>(-turn));
      unmake_move(move);
    }
    if (valid_move)
      (empty(move.to) || move.promotion != Empty || move.enpassant ||
               move.castling
           ? sorted_moves
           : others)
          .push_back(move);
  }
  // cout << "sorted:" << endl;
  // for (auto& x : sorted_moves) {
  //   x.print();
  // }
  // cout << "end sorted" << endl;

  sorted_moves.insert(sorted_moves.end(), others.begin(), others.end());
  return sorted_moves;
}

int Board::perft(int depth) {
  if (depth <= 0) return 1;

  auto legal = generate_legal_moves();
  if (depth == 1) return legal.size();

  int nodes = 0;
  // Board before = *this;

  for (auto& move : legal) {
    make_move(move);
    // Board after = *this;
    nodes += perft(depth - 1);
    unmake_move(move);
    // if (before.to_fen() != this->to_fen()) {
    //   cerr << "! "
    //        << "undo failed" << endl;
    //   cerr << "move: ";
    //   move.print();
    //   cerr << "before:       " << before.to_fen() << endl;
    //   cerr << "after move:   " << after.to_fen() << endl;
    //   cerr << "after unmove: " << to_fen() << endl;
    //   break;
    // }
  }
  return nodes;
}

int Board::divide(int depth) {
  int sum = 0;
  vector<string> moves;
  moves.reserve(40);
  auto legals = generate_legal_moves();
  // Board before = *this;
  for (auto& move : legals) {
    make_move(move);
    // Board after = *this;
    int nodes = perft(depth - 1);
    unmake_move(move);
    // if (before.to_fen() != this->to_fen()) {
    //   cerr << "! "
    //        << "undo failed" << endl;
    //   cerr << "move: ";
    //   move.print();
    //   cerr << "before:       " << before.to_fen() << endl;
    //   cerr << "after move:   " << after.to_fen() << endl;
    //   cerr << "after unmove: " << to_fen() << endl;
    //   break;
    // }
    sum += nodes;
    moves.push_back(to_uci(move) + ": " + to_string(nodes));
  }
  sort(moves.begin(), moves.end());
  for (auto& move : moves) cerr << move << endl;
  cerr << "Moves: " << legals.size() << endl;
  cerr << "Nodes: " << sum << endl;
  return sum;
}

template <Player turn>
bool Board::is_in_threat(int sq) {
  // for (auto& move : generate_pseudo_moves())
  //   if (move.to == sq) return true;
  // return false;

  // generate and check reverse threats from sq
  const int file = sq % 8, rank = sq / 8;
  constexpr Piece oppK = static_cast<Piece>(-turn * wK);
  constexpr Piece oppP = static_cast<Piece>(-turn * wP);
  constexpr Piece oppN = static_cast<Piece>(-turn * wN);
  constexpr Piece oppB = static_cast<Piece>(-turn * wB);
  constexpr Piece oppR = static_cast<Piece>(-turn * wR);
  constexpr Piece oppQ = static_cast<Piece>(-turn * wQ);

  // for debugging
  // #define true (printf("%d\n", __LINE__) || true)
  // check for king threats
  if (isnt_1(sq) && board[sq + S] == oppK) return true;                 // S
  if (isnt_8(sq) && board[sq + N] == oppK) return true;                 // N
  if (isnt_H(sq) && board[sq + E] == oppK) return true;                 // E
  if (isnt_A(sq) && board[sq + W] == oppK) return true;                 // W
  if (isnt_1(sq) && isnt_H(sq) && board[sq + SE] == oppK) return true;  // SE
  if (isnt_1(sq) && isnt_A(sq) && board[sq + SW] == oppK) return true;  // SW
  if (isnt_8(sq) && isnt_H(sq) && board[sq + NE] == oppK) return true;  // NE
  if (isnt_8(sq) && isnt_A(sq) && board[sq + NW] == oppK) return true;  // NW

  // check for pawn threats
  if (turn == White) {
    if (isnt_8(sq)) {
      if (isnt_A(sq) && board[sq + NW] == oppP) return true;  // NW
      if (isnt_H(sq) && board[sq + NE] == oppP) return true;  // NE
    }
  } else {
    if (isnt_1(sq)) {
      if (isnt_A(sq) && board[sq + SW] == oppP) return true;  // SW
      if (isnt_H(sq) && board[sq + SE] == oppP) return true;  // SE
    }
  }

  // check for knight threats
  if (rank > 1) {
    if (isnt_A(sq) && board[sq + N + NW] == oppN) return true;  // UL
    if (isnt_H(sq) && board[sq + N + NE] == oppN) return true;  // UR
  }
  if (file > 1) {
    if (isnt_8(sq) && board[sq + W + NW] == oppN) return true;  // LU
    if (isnt_1(sq) && board[sq + W + SW] == oppN) return true;  // LD
  }
  if (file < 6) {
    if (isnt_8(sq) && board[sq + E + NE] == oppN) return true;  // RU
    if (isnt_1(sq) && board[sq + E + SE] == oppN) return true;  // RD
  }
  if (rank < 6) {
    if (isnt_A(sq) && board[sq + S + SW] == oppN) return true;  // DL
    if (isnt_H(sq) && board[sq + S + SE] == oppN) return true;  // DR
  }

  // check for bishop, rook and queen threats
  for (auto& dir : {NW, NE, SW, SE}) {
    if (westwards(dir) && !isnt_A(sq)) continue;
    if (eastwards(dir) && !isnt_H(sq)) continue;
    for (int dest = sq + dir;
         in_board(dest) && !friendly(board[sq], board[dest]); dest += dir) {
      // cout << "dest: " << dest << endl;
      // cout << "board[dest]: " << board[dest] << endl;
      if (board[dest] == oppB || board[dest] == oppQ) return true;
      if (board[dest] != Empty) break;  // different from slide
      if (westwards(dir) && !isnt_A(dest)) break;
      if (eastwards(dir) && !isnt_H(dest)) break;
    }
  }
  for (auto& dir : {N, S, E, W}) {
    if (westwards(dir) && !isnt_A(sq)) continue;
    if (eastwards(dir) && !isnt_H(sq)) continue;
    for (int dest = sq + dir;
         in_board(dest) && !friendly(board[sq], board[dest]); dest += dir) {
      if (board[dest] == oppR || board[dest] == oppQ) return true;
      if (board[dest] != Empty) break;  // different from slide
      if (westwards(dir) && !isnt_A(dest)) break;
      if (eastwards(dir) && !isnt_H(dest)) break;
    }
  }

#undef true

  return false;  // no threats found
}

bool Board::is_in_check(Player player) {
  // if (check) return player == check;
  // CheckType check = CheckNotChecked;
  if (turn == player)
    return turn == White ? is_in_threat<White>(Kpos)
                         : is_in_threat<Black>(kpos);
  else
    return turn == White ? is_in_threat<Black>(kpos)
                         : is_in_threat<White>(Kpos);
}

array<bool, 64> Board::get_threats() {
  change_turn();
  array<bool, 64> threats{false};
  for (auto& move : generate_pseudo_moves()) threats[move.to] = true;
  change_turn();
  return threats;
}

vector<string> Board::list_san(vector<Move> movelist) {
  vector<string> temp;
  temp.resize(movelist.size());
  for (auto& move : movelist) {
    string san;
    if (move.castling) {
      if ((move.from == 4 && move.to == 6) ||
          (move.from == 60 && move.to == 62))
        san = "O-O";
      else if ((move.from == 4 && move.to == 2) ||
               (move.from == 60 && move.to == 58))
        san = "O-O-O";
    } else {
      char piece = toupper(piece2char(board[move.from]));
      if (piece != 'P') san = piece;
      bool from_file = false;
      bool from_rank = false;
      for (auto& move2 : movelist) {
        if (move2.from != move.from || move2.to != move.to)
          if (board[move2.from] == board[move.from])  // same piece
          {
            if (move2.from / 8 == move.from / 8)  // same rank
              from_file = true;
            if (move2.from % 8 == move.from % 8)  // same file
              from_rank = true;
            if (from_file || from_rank) break;
          }
      }
      if (from_file) san += idx2sq(move.from)[0];
      if (from_rank) san += idx2sq(move.from)[1];
      if (!empty(move.to) || move.enpassant) san += 'x';
      san += idx2sq(move.to);
      if (move.promotion != Empty) {
        san += '=';
        san += toupper(piece2char(move.promotion));
      }
      if (is_in_check(turn)) san += '+';
    }
    temp.push_back(san);
  }
  return temp;
}

inline void Board::slide(vector<Move>& movelist, int sq,
                         vector<Direction> dirs) {
  for (auto& dir : dirs) {
    if (westwards(dir) && !isnt_A(sq)) continue;
    if (eastwards(dir) && !isnt_H(sq)) continue;
    for (int dest = sq + dir;
         in_board(dest) && !friendly(board[sq], board[dest]); dest += dir) {
      movelist.push_back(Move(sq, dest));
      if (hostile(board[sq], board[dest])) break;
      if (westwards(dir) && !isnt_A(dest)) break;
      if (eastwards(dir) && !isnt_H(dest)) break;
    }
  }
}

inline void Board::move_or_capture(vector<Move>& movelist, int sq, int dir) {
  if (!friendly(board[sq], board[sq + dir]))
    movelist.push_back(Move(sq, sq + dir));
}