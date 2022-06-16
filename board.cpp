#include "board.hpp"

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
bool empty(char a) { return a == '.'; }
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
           bool _castling) {
  from = _from;
  to = _to;
  promotion = _promotion;
  captured = _captured;
  enpassant = _enpassant;
  castling = _castling;
}

bool Move::is_castling(string move) {
  return move == "e1g1" || move == "e1c1" || move == "e8g8" || move == "e8c8";
}

void Move::print() {
  cerr << "move: " << idx2sq(from) << idx2sq(to) << " " << captured << promotion
       << endl;
}

Board::Board() { load_startpos(); }

char Board::operator[](int i) { return board[i]; }

int Board::piece_color(int sq_idx) { return isupper(board[sq_idx]) ? 1 : -1; }

void Board::print(string sq) {
  if (sq.length() > 2)
    sq[0] = sq[2], sq[1] = sq[3];  // extract destination square from move
  int sq_idx = sq != "" ? sq2idx(sq[0], sq[1]) : -1;

  cout << endl << "|";
  for (int i = 0; i < 64; i++) {
    cout << board[i] << "|";
    if (i % 8 == 7) {
      if (~sq_idx && sq_idx / 8 == i / 8) cout << "<";
      cout << endl;
      if (i != 63) cout << "|";
    }
  }
  if (~sq_idx)
    for (int i = 0; i < 8; i++) cout << (sq_idx % 8 == i ? " ^" : "  ");

  cout << endl;
}

void Board::change_turn() { turn = turn == White ? Black : White; }

void Board::make_move(Move& move) {
  char captured = board[move.to];
  board[move.to] = move.promotion == '.' ? board[move.from] : move.promotion;
  board[move.from] = move.captured;
  move.captured = captured;
}
void Board::unmake_move(Move& move, int turn) {
  char captured = board[move.from];
  board[move.from] =
      move.promotion == '.' ? board[move.to] : (turn = -1 ? 'p' : 'P');
  board[move.to] = move.captured;
  move.captured = captured;
}

void Board::load_fen(string fen) {
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
  enpassant_sq_idx = fifty = moves = 0;

  for (auto& x : fen) {
    if (x == ' ')
      part++, p = 0;
    else if (part == 0) {
      if (isdigit(x))
        for (int dots = x - '0'; dots--;) board[p++] = '.';
      else if (x != '/')
        board[p++] = x;
    } else if (part == 1)
      turn = x == 'w' ? White : Black;
    else if (part == 2)
      if (x == '-') {
        castling_rights[0] = 0;
        castling_rights[1] = 0;
        castling_rights[2] = 0;
        castling_rights[3] = 0;
      } else {
        if (x == 'K') castling_rights[0] = 1;
        if (x == 'Q') castling_rights[1] = 1;
        if (x == 'k') castling_rights[2] = 1;
        if (x == 'q') castling_rights[3] = 1;
      }
    else if (part == 3)
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
  fen += " " + to_string(fifty) + " " + to_string(moves / 2 + 1);
  return fen;
}

string Board::to_san(Move move) {
  char piece = toupper(board[move.from]);
  char promotion = toupper(move.promotion);
  string uci = idx2sq(move.from) + idx2sq(move.to);
  if (piece != 'P') uci = piece + uci;
  if (move.promotion != '.')
    return uci + promotion;
  else
    return uci;
}

void Board::load_startpos() {
  load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}
