#include "board.hpp"

namespace Zobrist {
uint64_t pst[64][13];   // piece square table
uint64_t turn;          // turn
uint64_t castling[4];   // castling rights
uint64_t enpassant[8];  // enpassant file
}  // namespace Zobrist

int sq2idx(char file, char rank) {
  return (file - 'a') + (7 - (rank - '1')) * 8;  // matrix magic
}

string idx2sq(int idx) {
  string sq;
  sq.append(1, idx % 8 + 'a');
  return sq.append(1, (7 - idx / 8) + '1');
}

Piece char2piece(char p) {
  size_t pos = string("kqrbnp.PNBRQK").find(p);
  if (pos != string::npos) return Piece(pos - 6);
  return Empty;
}

char piece2char(Piece p) { return "kqrbnp.PNBRQK"[p + 6]; }

void Move::print() {
  cerr << "move: " << idx2sq(from) << idx2sq(to) << " " << captured << promotion
       << (castling ? " castling" : "") << (enpassant ? " enpassant" : "")
       << endl;
}

Board::Board() { load_startpos(); }

int Board::piece_color(int sq_idx) { return isupper(board[sq_idx]) ? 1 : -1; }
int Board::sq_color(int sq_idx) {
  return (sq_idx % 2 && (sq_idx / 8) % 2) ||
         (sq_idx % 2 == 0 && (sq_idx / 8) % 2 == 0);
}

void Board::print(string sq, bool flipped) {
  if (sq.length() == 4)
    sq[0] = sq[2], sq[1] = sq[3];  // extract destination square from move
  int sq_idx = sq != "" ? sq2idx(sq[0], sq[1]) : -1;
  char rank = '8', file = 'a';
  if (flipped) {
    if (~sq_idx) sq_idx = 63 - sq_idx;
    reverse(board, board + 64);
    rank = '1', file = 'h';
  }
  cout << endl << " ";
  for (int i = 0; i < 8; i++) cout << " " << (flipped ? file-- : file++);
  cout << endl << (flipped ? rank++ : rank--) << "|";
  for (int i = 0; i < 64; i++) {
    cout << (isupper(board[i]) ? "\e[33m" : "\e[36m");
    if (board[i] == Empty) {
      if ((i % 2 && (i / 8) % 2) || (i % 2 == 0 && (i / 8) % 2 == 0))
        // cout << "\e[47m";
        cout << ".\e[0m|";
      else
        cout << " \e[0m|";
    } else
      cout << piece2char(board[i]) << "\e[0m|";
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
  if (flipped) reverse(board, board + 64);
  cout << endl;

  cout << "fen: " << to_fen() << endl;
  cout << "zobrist key: " << hex << zobrist_hash() << dec << endl;

  cout << "DEBUG:" << endl;
  cout << "ep: " << enpassant_sq_idx << ", fifty:" << fifty
       << ", moves: " << moves << endl;
  cout << "Kpos: " << Kpos << ", kpos: " << kpos << endl;
  cout << "castling rights: ";
  for (int i = 0; i < 4; i++) cout << castling_rights[i] << " ";
  cout << endl;
  cout << "turn: " << turn << endl;
}

void Board::make_move(Move& move) {
  // save current aspects
  copy_n(castling_rights, 4, move.castling_rights);
  move.enpassant_sq_idx = enpassant_sq_idx;
  move.fifty = fifty;
  // move.moves = moves;

  // update half-move clock
  if (board[move.to] != Empty || abs(board[move.from]) == wP)
    fifty = 0;
  else
    fifty++;

  // update castling rights and king pos
  if (Kpos == move.from) {  // white king moved
    castling_rights[0] = castling_rights[1] = false;
    Kpos = move.to;
  } else if (kpos == move.from) {  // black king moved
    castling_rights[2] = castling_rights[3] = false;
    kpos = move.to;
  }
  if (board[move.from] == wR || board[move.to] == wR) {
    if (move.from == 63 || move.to == 63)
      castling_rights[0] = false;
    else if (move.from == 56 || move.to == 56)
      castling_rights[1] = false;
  }
  if (board[move.from] == bR || board[move.to] == bR) {
    if (move.from == 7 || move.to == 7)
      castling_rights[2] = false;
    else if (move.from == 0 || move.to == 0)
      castling_rights[3] = false;
  }

  // update enpassant square
  if (board[move.from] == wP && move.to - move.from == N + N)
    enpassant_sq_idx = move.from + N;
  else if (board[move.from] == bP && move.to - move.from == S + S)
    enpassant_sq_idx = move.from + S;
  else
    enpassant_sq_idx = -1;

  // update board
  const Piece captured = board[move.to];
  board[move.to] = move.promotion == Empty ? board[move.from] : move.promotion;
  board[move.from] = move.captured;
  move.captured = captured;
  // hash ^= Zobrist::pst[move.from][board[move.from] + 6];  // remove moving
  // piece hash ^= Zobrist::pst[move.to][board[move.to] + 6];      // add moving
  // piece hash ^= Zobrist::pst[move.to][move.captured + 6];       // add moving
  // piece

  if (move.castling) {  // move rook when castling
    if (move.from == 4 && move.to == 6)
      board[7] = Empty, board[5] = bR;
    else if (move.from == 4 && move.to == 2)
      board[0] = Empty, board[3] = bR;
    else if (move.from == 60 && move.to == 62)
      board[63] = Empty, board[61] = wR;
    else if (move.from == 60 && move.to == 58)
      board[56] = Empty, board[59] = wR;
  } else if (move.enpassant) {  // remove pawn when enpassant
    int rel_S = turn * S;
    board[move.to + rel_S] = Empty;
  }
  change_turn();

  moves++;

  // debug
  int _Kpos = -1, _kpos = -1;
  for (int i = 0; i < 64; i++)
    if (board[i] == wK)
      _Kpos = i;
    else if (board[i] == bK)
      _kpos = i;
  assert(_Kpos == Kpos);
  assert(_kpos == kpos);
}
void Board::unmake_move(Move& move) {
  // restore current aspects
  copy_n(move.castling_rights, 4, castling_rights);
  enpassant_sq_idx = move.enpassant_sq_idx;
  fifty = move.fifty;
  // moves = move.moves;

  // restore king pos
  if (Kpos == move.to)
    Kpos = move.from;
  else if (kpos == move.to)
    kpos = move.from;

  // update board
  Piece captured = board[move.from];
  board[move.from] = move.promotion == Empty ? board[move.to] : Piece(-turn);
  board[move.to] = move.captured;
  move.captured = captured;

  if (move.castling) {  // move rook when castling
    if (move.from == 4 && move.to == 6)
      board[7] = bR, board[5] = Empty;
    else if (move.from == 4 && move.to == 2)
      board[0] = bR, board[3] = Empty;
    else if (move.from == 60 && move.to == 62)
      board[63] = wR, board[61] = Empty;
    else if (move.from == 60 && move.to == 58)
      board[56] = wR, board[59] = Empty;
  } else if (move.enpassant) {  // add pawn when enpassant
    int rel_N = turn * N;
    board[move.to + rel_N] = Piece(turn * wP);  // TODO:verify
  }
  change_turn();

  moves--;

  // debug
  int _Kpos = -1, _kpos = -1;
  for (int i = 0; i < 64; i++)
    if (board[i] == wK)
      _Kpos = i;
    else if (board[i] == bK)
      _kpos = i;
  assert(_Kpos == Kpos);
  assert(_kpos == kpos);
}

bool Board::load_fen(string fen) {
  fill_n(board, 64, Empty);
  int part = 0, p = 0;
  char enpassant_sq[2];
  enpassant_sq_idx = fifty = moves = 0;
  fill_n(castling_rights, 4, false);

  for (auto& x : fen) {
    if (x == ' ') {
      part++, p = 0;
    } else if (part == 0) {
      if (p > 63) return false;
      if (isdigit(x))
        for (int dots = x - '0'; dots--;) board[p++] = Empty;
      else if (x != '/')
        board[p++] = char2piece(x);
    } else if (part == 1) {
      turn = x == 'w' ? White : Black;
    } else if (part == 2) {
      if (x != '-') {
        if (x == 'K') castling_rights[0] = true;
        if (x == 'Q') castling_rights[1] = true;
        if (x == 'k') castling_rights[2] = true;
        if (x == 'q') castling_rights[3] = true;
      }
    } else if (part == 3) {
      if (x == '-')
        enpassant_sq_idx = -1;
      else
        enpassant_sq[p++] = x;
    } else if (part == 4) {
      fifty *= 10;
      fifty += x - '0';
    } else if (part == 5) {
      moves *= 10;
      moves += x - '0';
    }
    // cout << part << "," << p << ")" << x << ":" << moves << endl;
  }
  if (~enpassant_sq_idx)
    enpassant_sq_idx = sq2idx(enpassant_sq[0], enpassant_sq[1]);
  Kpos = kpos = 64;  // invalid
  for (int i = 0; i < 64; i++)
    if (board[i] == wK)
      Kpos = i;
    else if (board[i] == bK)
      kpos = i;
  if (Kpos == 64 || kpos == 64) return false;
  hash = zobrist_hash();
  return part > 1;
}

string Board::to_fen() {
  string fen = "";
  fen.reserve(100);
  int blanks = 0;
  for (int i = 0; i < 64; i++) {
    if (board[i] == Empty) blanks++;
    if (blanks > 0 && (board[i] != Empty || !isnt_H(i))) {
      fen += '0' + blanks;
      blanks = 0;
    }
    if (board[i] != Empty) fen += piece2char(board[i]);
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
  if (move.promotion != Empty)
    return uci + piece2char(move.promotion);
  else
    return uci;
}

string Board::to_san(Move move) {
  Piece piece = Piece(abs(board[move.from]));
  string san;
  san.reserve(5);
  if (move.castling) {
    if ((move.from == 4 && move.to == 6) || (move.from == 60 && move.to == 62))
      san = "O-O";
    else if ((move.from == 4 && move.to == 2) ||
             (move.from == 60 && move.to == 58))
      san = "O-O-O";
  } else {
    if (piece != wP && piece != Empty) san = piece2char(piece);
    san += idx2sq(move.from);
    if (!empty(move.to) || move.enpassant) san += 'x';
    san += idx2sq(move.to);
    if (move.promotion != Empty) {
      san += '=';
      san += toupper(move.promotion);
    }
  }

  return san;
}

void Board::load_startpos() {
  load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
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

string Board::pos_hash() {
  string fen = "";
  fen.reserve(100);
  for (auto& s : board) fen += piece2char(s);
  fen += '|';
  fen += (turn == White ? 'w' : 'b');
  fen += '|';
  if (castling_rights[0]) fen += 'K';
  if (castling_rights[1]) fen += 'Q';
  if (castling_rights[2]) fen += 'k';
  if (castling_rights[3]) fen += 'q';
  fen += '|';
  fen += idx2sq(enpassant_sq_idx);
  return fen;
}

uint64_t Board::zobrist_hash() {
  uint64_t hash = 0;
  for (int i = 0; i < 64; i++)
    if (board[i] != Empty) hash ^= Zobrist::pst[i][board[i] + 6];
  if (turn == Black) hash ^= Zobrist::turn;
  for (int i = 0; i < 4; i++)
    if (castling_rights[i]) hash ^= Zobrist::castling[i];
  if (~enpassant_sq_idx) hash ^= Zobrist::enpassant[enpassant_sq_idx % 8];
  return hash;
}

void init_zobrist() {
  random_device rd;
  uniform_int_distribution<uint64_t> uni(0, UINT64_MAX);

  for (int i = 0; i < 64; i++)    // squares
    for (int j = 0; j < 12; j++)  // pieces
      Zobrist::pst[i][j] = uni(rd);

  for (int i = 0; i < 4; i++)  // castling rights
    Zobrist::castling[i] = uni(rd);

  Zobrist::turn = uni(rd);  // turn

  for (int i = 0; i < 8; i++)  // enpassant files
    Zobrist::enpassant[i] = uni(rd);
}