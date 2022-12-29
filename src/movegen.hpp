#include "board.hpp"
#include "types.hpp"

template <Direction dir, Piece piece>
inline bool is_occupied(Position& pos, int sq) {
  return is_safe<dir>(sq) && pos[sq + dir] == piece;
}

template <Direction dir, Piece p1, Piece p2>
inline bool is_occupied_slide(Position& pos, int sq) {
  if (!is_safe<dir>(sq)) return false;
  const int dest = sq + dir;
  if (in_board(dest) && !friendly(pos[sq], pos[dest])) {
    if (pos[dest] == p1 || pos[dest] == p2) return true;
    if (pos[dest] != Empty) return false;  // different from slide
    if (!is_safe<dir>(dest)) return false;
    return is_occupied_slide<dir, p1, p2>(pos, dest);
  }
  return false;
}

template <Player turn>
bool is_in_threat(Position& pos, int sq);

inline bool is_in_check(Board& board, Player turn) {
  return turn == White ? is_in_threat<White>(board.board, board.Kpos)
                       : is_in_threat<Black>(board.board, board.kpos);
}
int divide(Board& board, int depth);

template <Player turn>
vector<Move> generate_pseudo_moves(Board& board);
vector<Move> generate_pseudo_moves(Board& board);
template <Player turn>
vector<Move> generate_legal_moves(Board& board);
vector<Move> generate_legal_moves(Board& board);

template <Player turn>
array<bool, 64> get_threats(Board& board) {
  array<bool, 64> threats{false};
  for (auto& move : generate_pseudo_moves<turn>(board)) threats[move.to] = true;
  return threats;
}

bool make_move_if_legal(Board& board, string move);

vector<string> list_san(Board& board, vector<Move> movelist);
