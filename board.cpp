#include "board.hpp"

#include <unordered_set>

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

int sq2idx(char file, char rank) {
  return (file - 'a') + (7 - (rank - '1')) * 8;  // matrix magic
}

string idx2sq(int idx) {
  string sq;
  sq.append(1, idx % 8 + 'a');
  return sq.append(1, (7 - idx / 8) + '1');
}

bool friendly(char a, char b) {
  return isupper(a) && isupper(b) || islower(a) && islower(b);
}

bool hostile(char a, char b) {
  return isupper(a) && islower(b) || islower(a) && isupper(b);
}

bool in_board(int idx) { return idx >= 0 && idx < 64; }
bool isnt_H(int idx) { return idx % 8 != 7; }
bool isnt_A(int idx) { return idx % 8 != 0; }
bool isnt_8(int idx) { return idx / 8 != 0; }
bool isnt_1(int idx) { return idx / 8 != 7; }

bool westwards(Direction dir) { return dir == NW || dir == SW || dir == W; }
bool eastwards(Direction dir) { return dir == NE || dir == SE || dir == E; }

Move::Move(string move) {
  int l = move.length();
  if (l == 4 || l == 5) {
    from = sq2idx(move[0], move[1]);
    to = sq2idx(move[2], move[3]);
    if (l == 5) promotion = move[4];
    castling = is_castling(move);
  }
}

Move::Move(int _from, int _to, char _promotion, char _captured, bool _enpassant,
           bool _castling)
    : from(_from),
      to(_to),
      promotion(_promotion),
      captured(_captured),
      enpassant(_enpassant),
      castling(_castling) {}

bool Move::is_castling(string move) {
  return move == "e1g1" || move == "e1c1" || move == "e8g8" || move == "e8c8";
}

void Move::print() {
  cerr << "move: " << idx2sq(from) << idx2sq(to) << " " << captured << promotion
       << (castling ? " castling" : "") << (enpassant ? " enpassant" : "")
       << endl;
}

Board::Board() { load_startpos(); }

char Board::operator[](int i) { return board[i]; }

int Board::piece_color(int sq_idx) { return isupper(board[sq_idx]) ? 1 : -1; }
int Board::sq_color(int sq_idx) {
  return sq_idx % 2 && (sq_idx / 8) % 2 ||
         sq_idx % 2 == 0 && (sq_idx / 8) % 2 == 0;
}

void Board::print(string sq, bool flipped) {
  if (sq.length() > 2)
    sq[0] = sq[2], sq[1] = sq[3];  // extract destination square from move
  int sq_idx = sq != "" ? sq2idx(sq[0], sq[1]) : -1;
  char rank = '8', file = 'a';
  if (flipped) {
    if (~sq_idx) sq_idx = 63 - sq_idx;
    reverse(board.begin(), board.end());
    rank = '1', file = 'h';
  }
  cout << endl << " ";
  for (int i = 0; i < 8; i++) cout << " " << (flipped ? file-- : file++);
  cout << endl << (flipped ? rank++ : rank--) << "|";
  for (int i = 0; i < 64; i++) {
    cout << (isupper(board[i]) ? "\e[33m" : "\e[36m");
    if (board[i] == '.') {
      if (i % 2 && (i / 8) % 2 || i % 2 == 0 && (i / 8) % 2 == 0)
        // cout << "\e[47m";
        cout << ".\e[0m|";
      else
        cout << " \e[0m|";
    } else
      cout << board[i] << "\e[0m|";
    if (i % 8 == 7) {
      if (~sq_idx && sq_idx / 8 == i / 8) cout << "<";
      cout << endl;
      if (i != 63) cout << (flipped ? rank++ : rank--) << "|";
    }
  }
  if (~sq_idx) {
    cout << " ";
    for (int i = 0; i < 8; i++) cout << (sq_idx % 8 == i ? " ^" : "  ");
  }
  if (flipped) reverse(board.begin(), board.end());
  cout << endl;
}

void Board::change_turn() { turn = turn == White ? Black : White; }

void Board::make_move(Move& move) {
  // save current aspects
  copy(castling_rights, castling_rights + 4, move.castling_rights);
  move.enpassant_sq_idx = enpassant_sq_idx;
  move.fifty = fifty;
  move.moves = moves;

  // update half-move clock
  if (board[move.to] != '.' || board[move.from] == 'P' ||
      board[move.from] == 'p')
    fifty = 0;
  else
    fifty++;

  // update castling rights
  if (board[move.from] == 'K')
    castling_rights[0] = castling_rights[1] = false;
  else if (board[move.from] == 'k')
    castling_rights[2] = castling_rights[3] = false;
  if (board[move.from] == 'R' || board[move.to] == 'R') {
    if (move.from == 63 || move.to == 63)
      castling_rights[0] = false;
    else if (move.from == 56 || move.to == 56)
      castling_rights[1] = false;
  }
  if (board[move.from] == 'r' || board[move.to] == 'r') {
    if (move.from == 7 || move.to == 7)
      castling_rights[2] = false;
    else if (move.from == 0 || move.to == 0)
      castling_rights[3] = false;
  }

  // update enpassant square
  if (board[move.from] == 'P' && move.to - move.from == N + N)
    enpassant_sq_idx = move.from + N;
  else if (board[move.from] == 'p' && move.to - move.from == S + S)
    enpassant_sq_idx = move.from + S;
  else
    enpassant_sq_idx = -1;

  // update board
  char captured = board[move.to];
  board[move.to] = move.promotion == '.' ? board[move.from] : move.promotion;
  board[move.from] = move.captured;
  move.captured = captured;

  if (move.castling) {  // move rook when castling
    if (move.from == 4 && move.to == 6)
      board[7] = '.', board[5] = 'r';
    else if (move.from == 4 && move.to == 2)
      board[0] = '.', board[3] = 'r';
    else if (move.from == 60 && move.to == 62)
      board[63] = '.', board[61] = 'R';
    else if (move.from == 60 && move.to == 58)
      board[56] = '.', board[59] = 'R';
  } else if (move.enpassant) {  // remove pawn when enpassant
    int rel_S = (turn == White ? S : N);
    board[move.to + rel_S] = '.';
  }
  change_turn();
}
void Board::unmake_move(Move& move) {
  // restore current aspects
  copy(move.castling_rights, move.castling_rights + 4, castling_rights);
  enpassant_sq_idx = move.enpassant_sq_idx;
  fifty = move.fifty;
  moves = move.moves;

  // update board
  char captured = board[move.from];
  board[move.from] =
      move.promotion == '.' ? board[move.to] : (turn == Black ? 'P' : 'p');
  board[move.to] = move.captured;
  move.captured = captured;

  if (move.castling) {  // move rook when castling
    if (move.from == 4 && move.to == 6)
      board[7] = 'r', board[5] = '.';
    else if (move.from == 4 && move.to == 2)
      board[0] = 'r', board[3] = '.';
    else if (move.from == 60 && move.to == 62)
      board[63] = 'R', board[61] = '.';
    else if (move.from == 60 && move.to == 58)
      board[56] = 'R', board[59] = '.';
  } else if (move.enpassant) {  // add pawn when enpassant
    int rel_N = (turn == White ? N : S);
    board[move.to + rel_N] = (turn == White ? 'P' : 'p');  // TODO:verify
  }
  change_turn();
}

bool Board::load_fen(string fen) {
  int part = 0, p = 0;
  char enpassant_sq[2];
  board =
      "........"
      "........"
      "........"
      "........"
      "........"
      "........"
      "........"
      "........";
  enpassant_sq_idx = -1, fifty = 0, moves = 1;
  fill(castling_rights, castling_rights + 4, 0);

  for (auto& x : fen) {
    if (x == ' ')
      part++, p = 0;
    else if (part == 0) {
      if (p > 63) return false;
      if (isdigit(x))
        for (int dots = x - '0'; dots--;) board[p++] = '.';
      else if (x != '/')
        board[p++] = x;
    } else if (part == 1)
      turn = x == 'w' ? White : Black;
    else if (part == 2)
      if (x != '-') {
        if (x == 'K') castling_rights[0] = 1;
        if (x == 'Q') castling_rights[1] = 1;
        if (x == 'k') castling_rights[2] = 1;
        if (x == 'q') castling_rights[3] = 1;
      } else if (part == 3)
        if (x == '-')
          enpassant_sq_idx = -1;
        else
          enpassant_sq[p++] = x;
      else if (part == 4) {
        fifty *= 10;
        fifty += x - '0';
      } else if (part == 5) {
        moves *= 10;
        moves += x - '0';
      }
  }
  if (~enpassant_sq_idx)
    enpassant_sq_idx = sq2idx(enpassant_sq[0], enpassant_sq[1]);
  return part > 1;
}

string Board::to_fen() {
  string fen = "";
  int blanks = 0;
  for (int i = 0; i < 64; i++) {
    if (board[i] == '.') blanks++;
    if (blanks > 0 && (board[i] != '.' || !isnt_H(i))) {
      fen += '0' + blanks;
      blanks = 0;
    }
    if (board[i] != '.') fen += board[i];
    if (!isnt_H(i) && i != 63) fen += '/';
  }
  fen += " ";
  fen += (turn == White ? "w" : "b");
  string castling = "";
  if (castling_rights[0]) castling += "K";
  if (castling_rights[1]) castling += "Q";
  if (castling_rights[2]) castling += "k";
  if (castling_rights[3]) castling += "q";
  if (castling != "")
    fen += " " + castling;
  else
    fen += " -";
  if (~enpassant_sq_idx)
    fen += " " + idx2sq(enpassant_sq_idx);
  else
    fen += " -";
  fen += " " + to_string(fifty) + " " + to_string(moves);
  return fen;
}

string Board::to_uci(Move move) {
  string uci = idx2sq(move.from) + idx2sq(move.to);
  if (move.promotion != '.')
    return uci + move.promotion;
  else
    return uci;
}

string Board::to_san(Move move) {
  char piece = toupper(board[move.from]);
  char promotion = toupper(move.promotion);
  string san;
  if (move.castling) {
    if (move.from == 4 && move.to == 6 || move.from == 60 && move.to == 62)
      san = "O-O";
    else if (move.from == 4 && move.to == 2 || move.from == 60 && move.to == 58)
      san = "O-O-O";
  } else {
    if (piece != 'P') san = piece;
    san += idx2sq(move.from);
    if (!empty(move.to) || move.enpassant) san += 'x';
    san += idx2sq(move.to);
    if (move.promotion != '.') {
      san += '=';
      san += move.promotion;
    }
  }

  return san;
}

void Board::load_startpos() {
  load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

bool Board::empty(int idx) { return board[idx] == '.'; }

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

// Move Board::match_san(vector<Move> movelist, string san) {
//   // TODO
//   // int count = 0;
//   // for (char c : {'+', '#', '-', '+'})
//   //   san.erase(std::remove(san.begin(), san.end(), c), san.end());

//   // for (auto& move : movelist) {
//   //   if (move.to = 0)
//   //     ;
//   // }
// }

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
      if (is_in_threat(board.find(turn == White ? 'K' : 'k'))) san += '+';
      change_turn();
    }

    temp.push_back(san);
  }
  return temp;
}

void Board::add_move(vector<Move>& movelist, int sq, int dir) {
  if (!friendly(board[sq], board[sq + dir]))
    movelist.push_back(Move(sq, sq + dir));
}

vector<Move> Board::generate_pseudo_moves() {
  vector<Move> pseudo;
  for (int s = 0; s < 64; s++) {
    char piece = board[s];
    if (hostile(piece, turn == Black ? 'k' : 'K'))
      continue;  // not current player's pieces
    char rel_piece = toupper(piece);
    vector<char> promotions = {'Q', 'R', 'B', 'N'};
    if (turn == Black) promotions = {'q', 'r', 'b', 'n'};

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
      // castling
      if (s == 60 && turn == White) {
        // kingside
        if (castling_rights[0] && empty(61) && empty(62))
          pseudo.push_back(Move(s, s + E + E, '.', '.', false, true));
        // queenside
        if (castling_rights[1] && empty(57) & empty(58) && empty(59))
          pseudo.push_back(Move(s, s + W + W, '.', '.', false, true));
      }
      if (s == 4 && turn == Black) {
        // kingside
        if (castling_rights[2] && empty(5) && empty(6))
          pseudo.push_back(Move(s, s + E + E, '.', '.', false, true));
        // queenside
        if (castling_rights[3] && empty(1) && empty(2) && empty(3))
          pseudo.push_back(Move(s, s + W + W, '.', '.', false, true));
      }
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
  return pseudo;
}

bool Board::is_in_threat(int sq) {
  for (auto& move : generate_pseudo_moves())
    if (move.to == sq) return true;
  return false;
}

bool Board::is_in_check(Player player) {
  int K_pos = board.find(player == White ? 'K' : 'k');
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
  auto pseudo = temp.generate_pseudo_moves();
  for (auto& move : pseudo)
    if (hostile(board[move.to], board[move.from])) temp.board[move.to] = 'x';
  return temp;
}

vector<Move> Board::generate_legal_moves() {
  auto movelist = generate_pseudo_moves();
  Board temp = *this;

  int K_pos = board.find(turn == White ? 'K' : 'k');

  for (auto it = movelist.begin(); it != movelist.end();) {
    Move move = *it;

    // check if any squares during castling are threatened
    if (move.castling) {
      // temp.change_turn();
      // unordered_set<int> threats;
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
      //         (threats.count(4) || threats.count(3) || threats.count(2)))
      //   it = movelist.erase(it);
      // else
      //   ++it;
      // temp.change_turn();
      bool flag = false;
      for (int i = move.from;; i += (i < move.to) - (i > move.to)) {
        auto temp_move = Move(move.from, i);
        i == move.from ? temp.change_turn() : temp.make_move(temp_move);
        for (auto& x : temp.generate_pseudo_moves())
          if (x.to == i) {
            it = movelist.erase(it);
            flag = true;
            break;
          }
        i == move.from ? temp.change_turn() : temp.unmake_move(temp_move);
        if (i == move.to || flag) break;
      }
      if (!flag) ++it;
    } else {
      temp.make_move(move);
      bool flag = false;
      for (auto& x : temp.generate_pseudo_moves())
        if (x.to == (move.from == K_pos ? move.to : K_pos)) {
          it = movelist.erase(it);
          flag = true;
          break;
        }
      if (!flag) ++it;
      temp.unmake_move(move);
    }
  }
  return movelist;
}

int Board::perft(int depth, int K_pos) {
  if (depth <= 0) return 1;

  auto legal = generate_legal_moves();
  if (depth == 1) return legal.size();

  int nodes = 0;
  Board before = *this;

  for (auto& move : legal) {
    make_move(move);
    Board after = *this;
    int temp_K_pos = board.find(turn == White ? 'K' : 'k');
    // int temp_K_pos = move.from == K_pos ? move.to : K_pos;
    if (!is_in_threat(temp_K_pos)) nodes += perft(depth - 1, temp_K_pos);
    unmake_move(move);
    if (before.board != this->board) {
      cerr << "! "
           << "undo failed" << endl;
      cerr << "move: ";
      move.print();
      cerr << "before:       " << before.to_fen() << endl;
      cerr << "after move:   " << after.to_fen() << endl;
      cerr << "after unmove: " << to_fen() << endl;
      break;
    }
  }
  return nodes;
}

int Board::divide(int depth) {
  int sum = 0, temp = 0;
  vector<string> moves;
  auto legals = generate_legal_moves();
  Board before = *this;
  for (auto& move : legals) {
    make_move(move);
    Board after = *this;
    temp = perft(depth - 1, 0);
    unmake_move(move);
    if (before.board != this->board) {
      cerr << "! "
           << "undo failed" << endl;
      cerr << "move: ";
      move.print();
      cerr << "before:       " << before.to_fen() << endl;
      cerr << "after move:   " << after.to_fen() << endl;
      cerr << "after unmove: " << to_fen() << endl;
      break;
    }
    sum += temp;
    moves.push_back(to_uci(move) + ": " + to_string(temp));
  }
  sort(moves.begin(), moves.end());
  for (auto& move : moves) cerr << move << endl;
  cerr << "Moves: " << legals.size() << endl;
  cerr << "Nodes: " << sum << endl;
  return sum;
}

int Board::eval() {
  int material_white = 0, material_black = 0;
  for (auto& piece : board) {
    int v = material_value(piece);
    if (v < 0)
      material_black -= v;
    else
      material_white += v;
  }
  return material_white - material_black;
}

// https://www.chessprogramming.org/Negamax
int Board::negamax(int depth) {
  if (depth == 0) return eval() * (turn == White ? 1 : -1) + rand() % 6 - 3;
  int bestscore = -1e6;
  int max_breadth = 5;
  ofstream log("log.txt", std::ios_base::app);
  for (auto& move : generate_legal_moves()) {
    // if (!max_breadth--) break;
    make_move(move);
    int score = -negamax(depth - 1);
    unmake_move(move);
    if (score != 0 && depth > 1) {
      for (int i = 0; i < depth; i++) log << "  ";
      log << to_san(move) << " = " << score << "\n";
    }
    if (score > bestscore && score != 1e6) bestscore = score;
  }
  log.close();
  return bestscore;
}

Move Board::search_best_move() {
  Move bestmove;
  int bestscore = -1e6;
  int max_breadth = 5;
  {
    ofstream log("log.txt");
    log << "";
    log.close();
  }
  for (auto& move : generate_legal_moves()) {
    // if (!max_breadth--) break;'
    make_move(move);
    int score = -negamax(2);
    unmake_move(move);
    ofstream log("log.txt", std::ios_base::app);
    log << to_san(move) << " = " << score << "\n\n";
    log.close();
    if (score > bestscore && score != 1e6) {
      bestscore = score;
      bestmove = move;
    }
  }

  {
    ofstream log("log.txt", std::ios_base::app);
    log << "best: " << to_san(bestmove) << " = " << bestscore << "\n\n";
    log.close();
  }
  return bestmove;
}