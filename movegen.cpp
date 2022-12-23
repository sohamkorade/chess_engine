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

void Board::generate_pawn_moves(vector<Move>& pseudo, int sq) {
  const int rank = sq / 8;
  const Piece piece = board[sq];

  Piece promotions[4] = {wQ, wR, wB, wN};
  Piece black_promotions[4] = {bQ, bR, bB, bN};
  if (turn == Black) copy_n(black_promotions, 4, promotions);

  Direction rel_N = turn == White ? N : S;
  Direction rel_NW = turn == White ? NW : SW;
  Direction rel_NE = turn == White ? NE : SE;
  int rel_rank = turn == White ? rank : 7 - rank;

  if (empty(sq + rel_N)) {
    if (rel_rank == 1) {  // promotion
      for (auto& piece : promotions)
        pseudo.push_back(Move(sq, sq + rel_N, piece));
    } else
      pseudo.push_back(Move(sq, sq + rel_N));  // push
    if (rel_rank == 6 && empty(sq + rel_N + rel_N))
      pseudo.push_back(Move(sq, sq + rel_N + rel_N));  // double push
  }
  if (isnt_A(sq) && hostile(piece, board[sq + rel_NW])) {  // capture NW
    if (rel_rank == 1) {                                   // promotion capture
      for (auto& piece : promotions)
        pseudo.push_back(Move(sq, sq + rel_NW, piece));
    } else
      pseudo.push_back(Move(sq, sq + rel_NW));
  }
  if (isnt_H(sq) && hostile(piece, board[sq + rel_NE])) {  // capture NE
    if (rel_rank == 1) {                                   // promotion capture
      for (auto& piece : promotions)
        pseudo.push_back(Move(sq, sq + rel_NE, piece));
    } else
      pseudo.push_back(Move(sq, sq + rel_NE));
  }
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
  vector<Move> pseudo;
  pseudo.reserve(40);  // average number of pseudo moves per position
  for (int sq = 0; sq < 64; sq++) {
    if (hostile(board[sq], turn == Black ? bK : wK))
      continue;  // not current player's pieces
    const Piece rel_piece = static_cast<Piece>(abs(board[sq]));
    if (rel_piece == wK)
      generate_king_moves(pseudo, sq);
    else if (rel_piece == wP)
      generate_pawn_moves(pseudo, sq);
    else if (rel_piece == wN)
      generate_knight_moves(pseudo, sq);
    else if (rel_piece == wB)
      slide(pseudo, sq, {NW, NE, SW, SE});
    else if (rel_piece == wR)
      slide(pseudo, sq, {N, S, E, W});
    else if (rel_piece == wQ)
      slide(pseudo, sq, {N, S, E, W, NW, NE, SW, SE});
  }
  generate_castling_moves(pseudo);
  return pseudo;
}

vector<Move> Board::generate_legal_moves() {
  auto movelist = generate_pseudo_moves();
  vector<Move> sorted_moves, others;
  sorted_moves.reserve(movelist.size());
  others.reserve(movelist.size());

  for (auto& move : movelist) {
    bool valid_move = true;
    // check if any squares during castling are threatened
    if (move.castling) {
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
      make_move(move);
      for (auto& x : generate_pseudo_moves())
        if (x.to == (turn == Black ? Kpos : kpos)) {
          valid_move = false;
          break;
        }
      unmake_move(move);
    }
    if (valid_move)
      (board[move.to] != Empty || move.promotion != Empty || move.enpassant ||
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

int Board::perft(int depth, int K_pos) {
  if (depth <= 0) return 1;

  auto legal = generate_legal_moves();
  if (depth == 1) return legal.size();

  int nodes = 0;
  // Board before = *this;

  for (auto& move : legal) {
    make_move(move);
    // Board after = *this;
    // int temp_K_pos = board.find(turn == White ? 'K' : 'k');
    int temp_K_pos = turn == White ? Kpos : kpos;
    // int temp_K_pos = move.from == K_pos ? move.to : K_pos;
    if (!is_in_threat(temp_K_pos)) nodes += perft(depth - 1, temp_K_pos);
    unmake_move(move);
    // if (before.board != this->board) {
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
  auto legals = generate_legal_moves();
  // Board before = *this;
  for (auto& move : legals) {
    make_move(move);
    // Board after = *this;
    int nodes = perft(depth - 1, 0);
    unmake_move(move);
    // if (before.board != this->board) {
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

bool Board::is_in_threat(int sq) {
  for (auto& move : generate_pseudo_moves())
    if (move.to == sq) return true;
  return false;
}

bool Board::is_in_check(Player player) {
  int K_pos = player == White ? Kpos : kpos;
  bool check = false;
  if (turn == player) {
    change_turn();
    check = is_in_threat(K_pos);
    change_turn();
  } else
    check = is_in_threat(K_pos);
  return check;
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
  for (auto& move : movelist) {
    char piece = toupper(piece2char(board[move.from]));
    char promotion = toupper(piece2char(move.promotion));
    string san;
    if (move.castling) {
      if ((move.from == 4 && move.to == 6) ||
          (move.from == 60 && move.to == 62))
        san = "O-O";
      else if ((move.from == 4 && move.to == 2) ||
               (move.from == 60 && move.to == 58))
        san = "O-O-O";
    } else {
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
      if (move.promotion != '.') {
        san += '=';
        san += promotion;
      }
      change_turn();
      // if (is_in_threat(board.find(turn == White ? 'K' : 'k'))) san += '+';
      if (is_in_threat(turn == White ? Kpos : kpos)) san += '+';
      change_turn();
    }

    temp.push_back(san);
  }
  return temp;
}

void Board::slide(vector<Move>& movelist, int sq, vector<Direction> dirs) {
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

void Board::move_or_capture(vector<Move>& movelist, int sq, int dir) {
  if (!friendly(board[sq], board[sq + dir]))
    movelist.push_back(Move(sq, sq + dir));
}