#pragma once

#include "types.hpp"

class Move {
 public:
  int from = 0, to = 0;
  int enpassant_sq_idx = -1, fifty = 0, moves = 0;
  char promotion = '.', captured = '.';
  bool castling_rights[4] = {};
  bool enpassant = false, castling = false;
  Move(int _from = 0, int _to = 0, char _promotion = '.', char _captured = '.',
       bool _enpassant = false, bool _castling = false)
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
  char board[64];
  int enpassant_sq_idx = -1, fifty = 0, moves = 1;
  int Kpos = -1, kpos = -1;
  bool castling_rights[4] = {};
  Player turn = White;

  Board();
  char operator[](int i) const;
  int piece_color(int sq_idx);
  int sq_color(int sq_idx);
  void print(string sq = "", bool flipped = false);
  void change_turn();
  void make_move(string move);
  void make_move(Move& move);
  void unmake_move(Move& move);
  bool load_fen(string fen);
  string to_fen();
  string to_uci(Move move);
  string to_san(Move move);
  void load_startpos();
  bool empty(int idx) const;
  vector<Move> generate_pseudo_moves();
  vector<Move> generate_legal_moves();
  Board mark_threats();
  Move match_san(vector<Move> movelist, string san);
  vector<string> list_san(vector<Move> movelist);
  int divide(int depth);
  int perft(int depth, int K_pos);

  bool is_in_threat(int sq);
  bool is_in_check(Player player);

  string pos_hash();

 protected:
  void slide(vector<Move>& movelist, int sq, vector<Direction> dirs);
  void move_or_capture(vector<Move>& movelist, int sq, int dir);
};

int sq2idx(char file, char rank);
string idx2sq(int idx);
bool friendly(char a, char b);
bool hostile(char a, char b);
bool in_board(int idx);
bool isnt_H(int idx);
bool isnt_A(int idx);
bool isnt_8(int idx);
bool isnt_1(int idx);

bool westwards(Direction dir);
bool eastwards(Direction dir);
