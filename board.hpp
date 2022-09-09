#pragma once
#include <fstream>

#include "types.hpp"

class Move {
 public:
  int from = 0, to = 0;
  char promotion = '.', captured = '.';
  bool enpassant = false, castling = false;
  bool castling_rights[4] = {};
  int enpassant_sq_idx = -1, fifty = 0, moves = 0;
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
  int enpassant_sq_idx = -1, fifty = 0, moves = 1;
  Player turn = White;
  Board();
  char operator[](int i);
  int piece_color(int sq_idx);
  int sq_color(int sq_idx);
  void print(string sq = "", bool flipped = false);
  void change_turn();
  void make_move(Move& move);
  void unmake_move(Move& move);
  bool load_fen(string fen);
  string to_fen();
  string to_uci(Move move);
  string to_san(Move move);
  void load_startpos();
  bool empty(int idx);
  vector<Move> generate_pseudo_moves();
  vector<Move> generate_legal_moves();
  Board mark_threats();
  Move match_san(vector<Move> movelist, string san);
  vector<string> list_san(vector<Move> movelist);
  int divide(int depth);
  int perft(int depth, int K_pos);

  bool is_in_threat(int sq);
  bool is_in_check(Player player);

  int eval();
  int negamax(int depth);
  Move search_best_move();

 protected:
  void slide(vector<Move>& movelist, int sq, vector<Direction> dirs);
  void add_move(vector<Move>& movelist, int sq, int dest);
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
