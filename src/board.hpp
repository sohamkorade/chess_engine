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

  Board();
  constexpr Piece operator[](int i) { return board[i]; }
  int piece_color(int sq_idx);
  int sq_color(int sq_idx);
  void print(string sq = "", bool flipped = false);
  inline void change_turn();
  void make_move(Move& move);
  void unmake_move(Move& move);
  bool load_fen(string fen);
  string to_fen();
  string to_uci(Move move);
  string to_san(Move move);
  void load_startpos();
  inline bool empty(int idx);
  vector<string> list_san(vector<Move> movelist);

  uint64_t zobrist_hash();
};

inline void Board::change_turn() { turn = Player(-turn); }

int sq2idx(char file, char rank);
string idx2sq(int idx);

inline bool friendly(Piece a, Piece b) { return a * b > 0; }
inline bool hostile(Piece a, Piece b) { return a * b < 0; }
inline bool in_board(int idx) { return idx >= 0 && idx < 64; }
inline bool Board::empty(int index) { return board[index] == Empty; }

inline bool isnt_H(int idx) { return idx % 8 != 7; }
inline bool isnt_A(int idx) { return idx % 8 != 0; }
inline bool isnt_8(int idx) { return idx / 8 != 0; }
inline bool isnt_1(int idx) { return idx / 8 != 7; }

Piece char2piece(char p);
char piece2char(Piece p);

template <Direction dir>
constexpr bool is_safe(int idx) {
  const int rank = idx / 8, file = idx % 8;
  if constexpr (dir == N)
    return isnt_8(idx);
  else if constexpr (dir == S)
    return isnt_1(idx);
  else if constexpr (dir == E)
    return isnt_H(idx);
  else if constexpr (dir == W)
    return isnt_A(idx);
  else if constexpr (dir == NE)
    return isnt_H(idx) && isnt_8(idx);
  else if constexpr (dir == NW)
    return isnt_A(idx) && isnt_8(idx);
  else if constexpr (dir == SE)
    return isnt_H(idx) && isnt_1(idx);
  else if constexpr (dir == SW)
    return isnt_A(idx) && isnt_1(idx);
  else if constexpr (dir == NNE)
    return isnt_8(idx) && isnt_H(idx) && rank > 1;
  else if constexpr (dir == NNW)
    return isnt_8(idx) && isnt_A(idx) && rank > 1;
  else if constexpr (dir == SSE)
    return isnt_1(idx) && isnt_H(idx) && rank < 6;
  else if constexpr (dir == SSW)
    return isnt_1(idx) && isnt_A(idx) && rank < 6;
  else if constexpr (dir == ENE)
    return isnt_H(idx) && isnt_8(idx) && file < 6;
  else if constexpr (dir == ESE)
    return isnt_H(idx) && isnt_1(idx) && file < 6;
  else if constexpr (dir == WNW)
    return isnt_A(idx) && isnt_8(idx) && file > 1;
  else if constexpr (dir == WSW)
    return isnt_A(idx) && isnt_1(idx) && file > 1;
  else
    return true;
}
