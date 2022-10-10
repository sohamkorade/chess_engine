#include "board.hpp"

vector<Move> Board::generate_pseudo_moves() {
  vector<Move> pseudo;
  char promotions[4];
  copy_n(turn == Black ? "qrbn" : "QRBN", 4, promotions);
  for (int s = 0; s < 64; s++) {
    char piece = board[s];
    if (hostile(piece, turn == Black ? 'k' : 'K'))
      continue;  // not current player's pieces
    char rel_piece = toupper(piece);

    int file = s % 8, rank = s / 8;

    if (rel_piece == 'K') {
      if (isnt_1(s)) add_move(pseudo, s, S);                // S
      if (isnt_8(s)) add_move(pseudo, s, N);                // N
      if (isnt_H(s)) add_move(pseudo, s, E);                // E
      if (isnt_A(s)) add_move(pseudo, s, W);                // W
      if (isnt_1(s) && isnt_H(s)) add_move(pseudo, s, SE);  // SE
      if (isnt_1(s) && isnt_A(s)) add_move(pseudo, s, SW);  // SW
      if (isnt_8(s) && isnt_H(s)) add_move(pseudo, s, NE);  // NE
      if (isnt_8(s) && isnt_A(s)) add_move(pseudo, s, NW);  // NW
    }
    if (rel_piece == 'P') {
      Direction rel_N = N, rel_NW = NW, rel_NE = NE;
      int rel_rank = rank;
      if (turn == Black) {
        rel_N = S, rel_NW = SW, rel_NE = SE;
        rel_rank = 7 - rank;
      }
      if (empty(s + rel_N)) {
        if (rel_rank == 1) {  // promotion
          for (auto& piece : promotions)
            pseudo.push_back(Move(s, s + rel_N, piece));
        } else
          pseudo.push_back(Move(s, s + rel_N));  // push
        if (rel_rank == 6 && empty(s + rel_N + rel_N))
          pseudo.push_back(Move(s, s + rel_N + rel_N));  // double push
      }
      if (isnt_A(s) && hostile(piece, board[s + rel_NW])) {  // capture NW
        if (rel_rank == 1) {  // promotion capture
          for (auto& piece : promotions)
            pseudo.push_back(Move(s, s + rel_NW, piece));
        } else
          pseudo.push_back(Move(s, s + rel_NW));
      }
      if (isnt_H(s) && hostile(piece, board[s + rel_NE])) {  // capture NE
        if (rel_rank == 1) {  // promotion capture
          for (auto& piece : promotions)
            pseudo.push_back(Move(s, s + rel_NE, piece));
        } else
          pseudo.push_back(Move(s, s + rel_NE));
      }
      if (rel_rank == 3)
        if (s + rel_NW == enpassant_sq_idx)  // capture enpassant NW
          pseudo.push_back(Move(s, s + rel_NW, '.', '.', true));
        else if (s + rel_NE == enpassant_sq_idx)  // capture enpassant NE
          pseudo.push_back(Move(s, s + rel_NE, '.', '.', true));
    }
    if (rel_piece == 'N') {
      if (rank > 1) {
        if (isnt_A(s)) add_move(pseudo, s, N + NW);  // UL
        if (isnt_H(s)) add_move(pseudo, s, N + NE);  // UR
      }
      if (file > 1) {
        if (isnt_8(s)) add_move(pseudo, s, W + NW);  // LU
        if (isnt_1(s)) add_move(pseudo, s, W + SW);  // LD
      }
      if (file < 6) {
        if (isnt_8(s)) add_move(pseudo, s, E + NE);  // RU
        if (isnt_1(s)) add_move(pseudo, s, E + SE);  // RD
      }
      if (rank < 6) {
        if (isnt_A(s)) add_move(pseudo, s, S + SW);  // DL
        if (isnt_H(s)) add_move(pseudo, s, S + SE);  // DR
      }
    }
    if (rel_piece == 'B' || rel_piece == 'Q')
      slide(pseudo, s, {NW, NE, SW, SE});
    if (rel_piece == 'R' || rel_piece == 'Q') slide(pseudo, s, {N, S, E, W});
  }
  // castling
  if (turn == White) {
    // kingside
    if (castling_rights[0] && empty(61) && empty(62))
      pseudo.push_back(Move(60, 60 + E + E, '.', '.', false, true));
    // queenside
    if (castling_rights[1] && empty(57) & empty(58) && empty(59))
      pseudo.push_back(Move(60, 60 + W + W, '.', '.', false, true));
  } else {
    // kingside
    if (castling_rights[2] && empty(5) && empty(6))
      pseudo.push_back(Move(4, 4 + E + E, '.', '.', false, true));
    // queenside
    if (castling_rights[3] && empty(1) && empty(2) && empty(3))
      pseudo.push_back(Move(4, 4 + W + W, '.', '.', false, true));
  }
  return pseudo;
}

vector<Move> Board::generate_legal_moves() {
  auto movelist = generate_pseudo_moves();
  vector<Move> sorted_moves, others;
  Board temp = *this;

  // int K_pos = board.find(turn == White ? 'K' : 'k');
  int K_pos = turn == White ? Kpos : kpos;

  for (auto it = movelist.begin(); it != movelist.end();) {
    Move move = *it;
    bool deleted = false;
    // check if any squares during castling are threatened
    // TODO: keep only the efficient implementation
    if (move.castling) {
      temp.change_turn();
      bool threats[64] = {false};
      for (auto& x : temp.generate_pseudo_moves()) threats[x.to] = true;
      if (  // white kingside castling
          move.from == 60 && move.to == 62 &&
              (threats[60] || threats[61] || threats[62]) ||
          // black queenside castling
          move.from == 4 && move.to == 6 &&
              (threats[4] || threats[5] || threats[6]) ||
          // white kingside castling
          move.from == 60 && move.to == 58 &&
              (threats[60] || threats[59] || threats[58]) ||
          // black queenside castling
          move.from == 4 && move.to == 2 &&
              (threats[4] || threats[3] || threats[2])) {
        it = movelist.erase(it);
        deleted = true;
      }
      // set<int> threats;
      // for (auto& x : temp.generate_pseudo_moves()) threats.insert(x.to);
      // if (  // white kingside castling
      //     move.from == 60 && move.to == 62 &&
      //         (threats.count(60) || threats.count(61) || threats.count(62))
      //         ||
      //     // black queenside castling
      //     move.from == 4 && move.to == 6 &&
      //         (threats.count(4) || threats.count(5) || threats.count(6)) ||
      //     // white kingside castling
      //     move.from == 60 && move.to == 58 &&
      //         (threats.count(60) || threats.count(59) || threats.count(58))
      //         ||
      //     // black queenside castling
      //     move.from == 4 && move.to == 2 &&
      //         (threats.count(4) || threats.count(3) || threats.count(2))) {
      //   it = movelist.erase(it);
      //   deleted = true;
      // }
      temp.change_turn();
      // bool deleted = false;
      // for (int i = move.from;; i += (i < move.to) - (i > move.to)) {
      //   auto temp_move = Move(move.from, i);
      //   i == move.from ? temp.change_turn() : temp.make_move(temp_move);
      //   for (auto& x : temp.generate_pseudo_moves())
      //     if (x.to == i) {
      //       it = movelist.erase(it);
      //       deleted = true;
      //       break;
      //     }
      //   i == move.from ? temp.change_turn() : temp.unmake_move(temp_move);
      //   if (i == move.to || deleted) break;
      // }
    } else {
      temp.make_move(move);
      for (auto& x : temp.generate_pseudo_moves())
        if (x.to == (move.from == K_pos ? move.to : K_pos)) {
          it = movelist.erase(it);
          deleted = true;
          break;
        }
      temp.unmake_move(move);
    }
    if (!deleted) {
      (temp.board[move.to] != '.' || move.promotion != '.' || move.enpassant ||
               move.castling
           ? sorted_moves
           : others)
          .push_back(move);
      ++it;
    }
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
  int sum = 0, temp = 0;
  vector<string> moves;
  auto legals = generate_legal_moves();
  // Board before = *this;
  for (auto& move : legals) {
    make_move(move);
    // Board after = *this;
    temp = perft(depth - 1, 0);
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
    sum += temp;
    moves.push_back(to_uci(move) + ": " + to_string(temp));
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
  // int K_pos = board.find(player == White ? 'K' : 'k');
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

Board Board::mark_threats() {
  Board temp = *this;
  temp.change_turn();
  for (auto& move : temp.generate_pseudo_moves())
    if (hostile(board[move.to], board[move.from])) temp.board[move.to] = 'x';
  return temp;
}

vector<string> Board::list_san(vector<Move> movelist) {
  vector<string> temp;
  for (auto& move : movelist) {
    char piece = toupper(board[move.from]);
    char promotion = toupper(move.promotion);
    string san;
    if (move.castling) {
      if (move.from == 4 && move.to == 6 || move.from == 60 && move.to == 62)
        san = "O-O";
      else if (move.from == 4 && move.to == 2 ||
               move.from == 60 && move.to == 58)
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
        san += move.promotion;
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
