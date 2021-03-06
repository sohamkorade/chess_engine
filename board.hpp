#pragma once
#include "types.hpp"

class Move {
 public:
  int from = 0, to = 0;
  char promotion = '.', captured = '.';
  bool enpassant = false, castling = false;
  Move(string move = "a1a1");
  Move(int _from, int _to, char _promotion = '.', char _captured = '.',
       bool _enpassant = false, bool _castling = false);
  bool is_castling(string move);
  void print();
};

class Board {
 public:
  string board;
  bool castling_rights[4] = {};
  Player turn = White;
  int enpassant_sq_idx = -1, fifty = 0, moves = 0;
  Board();
  char operator[](int i);
  int piece_color(int sq_idx);
  void print(string sq = "");
  void change_turn();
  void make_move(Move& move);
  void unmake_move(Move& move, int turn);
  void load_fen(string fen);
  string to_fen();
  string to_san(Move move);
  void load_startpos();
};

int sq2idx(char file, char rank);
string idx2sq(int idx);
bool friendly(char a, char b);
bool hostile(char a, char b);
bool empty(char a);
bool in_board(int idx);
bool isnt_H(int idx);
bool isnt_A(int idx);
bool isnt_8(int idx);
bool isnt_1(int idx);

bool westwards(Direction dir);
bool eastwards(Direction dir);