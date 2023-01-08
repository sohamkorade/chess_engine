#pragma once

#include "types.hpp"

// https://www.chessprogramming.org/Zobrist_Hashing

void zobrist_init();

class Move {
 public:
  int from = 0, to = 0;
  int enpassant_sq_idx = -1, fifty = 0, moves = 0;
  Piece promotion = Empty, captured = Empty;
  bool castling_rights[4] = {};
  bool enpassant = false, castling = false;
  Move(int _from = 0, int _to = 0, Piece _promotion = Empty,
       Piece _captured = Empty, bool _enpassant = false, bool _castling = false)
      : from(_from),
        to(_to),
        promotion(_promotion),
        captured(_captured),
        enpassant(_enpassant),
        castling(_castling) {}
  void print();
  inline bool equals(int _from, int _to) { return from == _from && to == _to; }
  inline bool equals(Move& move) {
    return equals(move.from, move.to) && promotion == move.promotion;
  }
};

class Board {
 public:
  Position board;
  int enpassant_sq_idx = -1, fifty = 0, moves = 1;
  int Kpos = -1, kpos = -1;
  bool castling_rights[4] = {};
  Player turn = White;
  // CheckType check = CheckNotChecked;
  uint64_t hash = 0;

  Board() { load_startpos(); }
  constexpr Piece operator[](int i) { return board[i]; }
  inline void change_turn() { turn = Player(-turn); }
  inline bool empty(int idx) { return board[idx] == Empty; }

  inline int piece_color(int sq_idx);
  void print(string sq = "", bool flipped = false);
  void make_move(Move& move);
  void unmake_move(Move& move);
  bool load_fen(string fen);
  string to_fen();
  string to_uci(Move move);
  string to_san(Move move);
  void load_startpos();

  uint64_t zobrist_hash();
};

int sq2idx(char file, char rank);
string idx2sq(int idx);

inline bool friendly(Piece a, Piece b) { return a * b > 0; }
inline bool hostile(Piece a, Piece b) { return a * b < 0; }
inline bool in_board(int idx) { return idx >= 0 && idx < 64; }

inline bool isnt_H(int idx) { return idx % 8 != 7; }
inline bool isnt_A(int idx) { return idx % 8 != 0; }
inline bool isnt_8(int idx) { return idx / 8 != 0; }
inline bool isnt_1(int idx) { return idx / 8 != 7; }

Piece char2piece(char p);
char piece2char(Piece p);

template <Direction dir>
constexpr bool is_safe(int idx) {
  const int rank = idx / 8, file = idx % 8;

  // northwards
  if (dir == N || dir == NE || dir == NW || dir == ENE || dir == WNW)
    if (rank < 1) return false;
  // northwards (2 squares)
  if (dir == NN || dir == NNE || dir == NNW)
    if (rank < 2) return false;
  // southwards
  if (dir == S || dir == SE || dir == SW || dir == ESE || dir == WSW)
    if (rank > 6) return false;
  // southwards (2 squares)
  if (dir == SS || dir == SSE || dir == SSW)
    if (rank > 5) return false;
  // eastwards
  if (dir == E || dir == NE || dir == SE || dir == NNE || dir == SSE)
    if (file > 6) return false;
  // eastwards (2 squares)
  if (dir == ENE || dir == ESE)
    if (file > 5) return false;
  // westwards
  if (dir == W || dir == NW || dir == SW || dir == NNW || dir == SSW)
    if (file < 1) return false;
  // westwards (2 squares)
  if (dir == WNW || dir == WSW)
    if (file < 2) return false;

  return true;
}

inline int Board::piece_color(int sq_idx) {
  if (board[sq_idx] == Empty) return 0;
  return board[sq_idx] > 0 ? White : Black;
}

inline int sq_color(int sq_idx) {
  return (sq_idx % 2 && (sq_idx / 8) % 2) ||
         (sq_idx % 2 == 0 && (sq_idx / 8) % 2 == 0);
}
