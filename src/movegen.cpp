#include "movegen.hpp"

template <Direction dir>
inline void only_capture(Position& pos, vector<Move>& movelist, int sq) {
  if (is_safe<dir>(sq) && hostile(pos[sq], pos[sq + dir]))
    movelist.push_back(Move(sq, sq + dir));
}
template <Direction dir>
inline bool only_move(Position& pos, vector<Move>& movelist, int sq) {
  if (is_safe<dir>(sq) && pos[sq + dir] == Empty) {
    movelist.push_back(Move(sq, sq + dir));
    return true;
  }
  return false;
}

template <Direction dir>
inline void slide(Position& pos, vector<Move>& movelist, int sq) {
  if (!is_safe<dir>(sq)) return;
  for (int dest = sq + dir; in_board(dest) && !friendly(pos[sq], pos[dest]);
       dest += dir) {
    movelist.push_back(Move(sq, dest));
    if (hostile(pos[sq], pos[dest])) break;
    if (!is_safe<dir>(dest)) break;
  }
}

template <Direction... dirs>
inline void slides(Position& pos, vector<Move>& movelist, int sq) {
  initializer_list<int>{(slide<dirs>(pos, movelist, sq), 0)...};
}

template <Player turn>
bool is_in_threat(Position& pos, int sq) {
  // for (auto& move : generate_pseudo_moves())
  //   if (move.to == sq) return true;
  // return false;

  // generate and check reverse threats from sq
  constexpr Piece oppK = Piece(-turn * wK);
  constexpr Piece oppP = Piece(-turn * wP);
  constexpr Piece oppN = Piece(-turn * wN);
  constexpr Piece oppB = Piece(-turn * wB);
  constexpr Piece oppR = Piece(-turn * wR);
  constexpr Piece oppQ = Piece(-turn * wQ);

  // for debugging
  // #define true (printf("%d\n", __LINE__) || true)
  // check for king threats
  if (is_occupied<S, oppK>(pos, sq)) return true;
  if (is_occupied<N, oppK>(pos, sq)) return true;
  if (is_occupied<E, oppK>(pos, sq)) return true;
  if (is_occupied<W, oppK>(pos, sq)) return true;
  if (is_occupied<SE, oppK>(pos, sq)) return true;
  if (is_occupied<SW, oppK>(pos, sq)) return true;
  if (is_occupied<NE, oppK>(pos, sq)) return true;
  if (is_occupied<NW, oppK>(pos, sq)) return true;

  // check for pawn threats

  if constexpr (turn == White) {
    if (is_occupied<NW, oppP>(pos, sq)) return true;
    if (is_occupied<NE, oppP>(pos, sq)) return true;
  } else {
    if (is_occupied<SW, oppP>(pos, sq)) return true;
    if (is_occupied<SE, oppP>(pos, sq)) return true;
  }

  // check for knight threats
  if (is_occupied<NNW, oppN>(pos, sq)) return true;
  if (is_occupied<NNE, oppN>(pos, sq)) return true;
  if (is_occupied<WNW, oppN>(pos, sq)) return true;
  if (is_occupied<WSW, oppN>(pos, sq)) return true;
  if (is_occupied<ENE, oppN>(pos, sq)) return true;
  if (is_occupied<ESE, oppN>(pos, sq)) return true;
  if (is_occupied<SSW, oppN>(pos, sq)) return true;
  if (is_occupied<SSE, oppN>(pos, sq)) return true;

  // check for bishop, rook queen threats
  if (is_occupied_slide<NW, oppB, oppQ>(pos, sq)) return true;
  if (is_occupied_slide<NE, oppB, oppQ>(pos, sq)) return true;
  if (is_occupied_slide<SW, oppB, oppQ>(pos, sq)) return true;
  if (is_occupied_slide<SE, oppB, oppQ>(pos, sq)) return true;
  if (is_occupied_slide<N, oppR, oppQ>(pos, sq)) return true;
  if (is_occupied_slide<S, oppR, oppQ>(pos, sq)) return true;
  if (is_occupied_slide<E, oppR, oppQ>(pos, sq)) return true;
  if (is_occupied_slide<W, oppR, oppQ>(pos, sq)) return true;

#undef true

  return false;  // no threats found
}

template <Direction dir>
inline void jump(Position& pos, vector<Move>& movelist, int sq) {
  if (is_safe<dir>(sq) && !friendly(pos[sq], pos[sq + dir]))
    movelist.push_back(Move(sq, sq + dir));
}

template <Direction... dirs>
inline void jumps(Position& pos, vector<Move>& movelist, int sq) {
  initializer_list<int>{(jump<dirs>(pos, movelist, sq), 0)...};
}

template <Player turn>
inline void generate_king_moves(Board& board, vector<Move>& pseudo) {
  jumps<NE, NW, SE, SW, N, S, E, W>(board.board, pseudo,
                                    turn == White ? board.Kpos : board.kpos);
}

template <Player turn>
inline void generate_promotions(Board& board, vector<Move>& pseudo) {
  constexpr int prom_start_sq = turn == White ? 8 : 48;
  constexpr Direction rel_N = turn == White ? N : S;
  constexpr Direction rel_NW = turn == White ? NW : SW;
  constexpr Direction rel_NE = turn == White ? NE : SE;
  constexpr Direction rel_SW = turn == White ? SW : NW;
  constexpr Direction rel_SE = turn == White ? SE : NE;
  constexpr Piece rel_P = turn == White ? wP : bP;

  for (int sq = prom_start_sq; sq < prom_start_sq + 8; sq++) {
    if (board[sq] != rel_P) continue;
    if (board.empty(sq + rel_N))
      for (auto& piece : {wQ, wR, wB, wN})
        pseudo.push_back(Move(sq, sq + rel_N, Piece(piece * turn)));
    if (isnt_A(sq) && hostile(board[sq], board[sq + rel_NW]))
      for (auto& piece : {wQ, wR, wB, wN})
        pseudo.push_back(Move(sq, sq + rel_NW, Piece(piece * turn)));
    if (isnt_H(sq) && hostile(board[sq], board[sq + rel_NE]))
      for (auto& piece : {wQ, wR, wB, wN})
        pseudo.push_back(Move(sq, sq + rel_NE, Piece(piece * turn)));
  }

  const auto& enpassant_sq_idx = board.enpassant_sq_idx;
  if (~enpassant_sq_idx) {
    if (is_occupied<rel_SE, rel_P>(board.board,
                                   enpassant_sq_idx))  // capture enpassant NW
      pseudo.push_back(Move(enpassant_sq_idx + rel_SE, enpassant_sq_idx, Empty,
                            Empty, true));
    else if (is_occupied<rel_SW, rel_P>(
                 board.board, enpassant_sq_idx))  // capture enpassant NE
      pseudo.push_back(Move(enpassant_sq_idx + rel_SW, enpassant_sq_idx, Empty,
                            Empty, true));
  }
}

template <Player turn>
void generate_pawn_moves(Board& board, vector<Move>& pseudo, int sq) {
  constexpr Direction rel_N = turn == White ? N : S;
  constexpr Direction rel_NW = turn == White ? NW : SW;
  constexpr Direction rel_NE = turn == White ? NE : SE;
  constexpr Direction rel_2N = turn == White ? NN : SS;
  const int rank = sq / 8;
  const int rel_rank = turn == White ? rank : 7 - rank;

  only_capture<rel_NW>(board.board, pseudo, sq);  // capture NW
  only_capture<rel_NE>(board.board, pseudo, sq);  // capture NE

  if (only_move<rel_N>(board.board, pseudo, sq))  // push
    if (rel_rank == 6)
      only_move<rel_2N>(board.board, pseudo,
                        sq);  // double push only if push succeeds
}

inline void generate_knight_moves(Board& board, vector<Move>& pseudo, int sq) {
  jumps<NNW, NNE, WNW, WSW, ENE, ESE, SSW, SSE>(board.board, pseudo, sq);
}

template <Player turn>
inline void generate_castling_moves(Board& board, vector<Move>& pseudo) {
  auto castling_rights = board.castling_rights;
#define empty board.empty

  if constexpr (turn == White) {
    // kingside
    if (castling_rights[0] && empty(61) && empty(62))
      pseudo.push_back(Move(60, 60 + E + E, Empty, Empty, false, true));
    // queenside
    if (castling_rights[1] && empty(57) & empty(58) && empty(59))
      pseudo.push_back(Move(60, 60 + W + W, Empty, Empty, false, true));
  } else {
    // kingside
    if (castling_rights[2] && empty(5) && empty(6))
      pseudo.push_back(Move(4, 4 + E + E, Empty, Empty, false, true));
    // queenside
    if (castling_rights[3] && empty(1) && empty(2) && empty(3))
      pseudo.push_back(Move(4, 4 + W + W, Empty, Empty, false, true));
  }

#undef empty
}

int perft(Board& board, int depth) {
  if (depth <= 0) return 1;

  auto legal = generate_legal_moves(board);
  if (depth == 1) return legal.size();

  int nodes = 0;
  // Board before = *this;

  for (auto& move : legal) {
    board.make_move(move);
    // Board after = *this;
    nodes += perft(board, depth - 1);
    board.unmake_move(move);
    // if (before.to_fen() != this->to_fen()) {
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

int divide(Board& board, int depth) {
  int sum = 0;
  vector<string> moves;
  moves.reserve(40);
  auto legal = generate_legal_moves(board);
  Board before = board;
  for (auto& move : legal) {
    board.make_move(move);
    Board after = board;
    int nodes = perft(board, depth - 1);
    board.unmake_move(move);
    if (before.to_fen() != board.to_fen()) {
      cerr << "! "
           << "undo failed" << endl;
      cerr << "move: ";
      move.print();
      cerr << "before:       " << before.to_fen() << endl;
      cerr << "after move:   " << after.to_fen() << endl;
      cerr << "after unmove: " << board.to_fen() << endl;
      break;
    }
    sum += nodes;
    moves.push_back(board.to_uci(move) + ": " + to_string(nodes));
  }
  sort(moves.begin(), moves.end());
  for (auto& move : moves) cerr << move << endl;
  cerr << "Moves: " << legal.size() << endl;
  cerr << "Nodes: " << sum << endl;
  return sum;
}

template <Player turn>
vector<Move> generate_pseudo_moves(Board& board) {
  // cout << "gen pseudo @" << zobrist_hash() << endl;
  vector<Move> pseudo;
  pseudo.reserve(40);  // average number of pseudo moves per position

  // we know where the king is
  generate_king_moves<turn>(board, pseudo);
  generate_castling_moves<turn>(board, pseudo);
  generate_promotions<turn>(board, pseudo);

  constexpr Piece rel_P = Piece(wP * turn);
  constexpr Piece rel_N = Piece(wN * turn);
  constexpr Piece rel_B = Piece(wB * turn);
  constexpr Piece rel_R = Piece(wR * turn);
  constexpr Piece rel_Q = Piece(wQ * turn);

  for (int sq = 0; sq < 64; sq++) {
    const int rank = sq / 8;
    const int rel_rank = turn == White ? rank : 7 - rank;
    const Piece p = board[sq];
    if (p == rel_P && rel_rank != 1)
      generate_pawn_moves<turn>(board, pseudo, sq);
    else if (p == rel_N)
      generate_knight_moves(board, pseudo, sq);
    else if (p == rel_B)
      slides<NW, NE, SW, SE>(board.board, pseudo, sq);
    else if (p == rel_R)
      slides<N, S, E, W>(board.board, pseudo, sq);
    else if (p == rel_Q)
      slides<N, S, E, W, NW, NE, SW, SE>(board.board, pseudo, sq);
  }
  return pseudo;
}

vector<Move> generate_pseudo_moves(Board& board) {
  return board.turn == White ? generate_pseudo_moves<White>(board)
                             : generate_pseudo_moves<Black>(board);
}
template <Player turn>
vector<Move> generate_legal_moves(Board& board) {
  // cout << "gen legal @" << zobrist_hash() << endl;
  auto movelist = generate_pseudo_moves<turn>(board);
  vector<Move> sorted_moves, others;
  sorted_moves.reserve(movelist.size());
  others.reserve(movelist.size());

  for (auto& move : movelist) {
    bool valid_move = true;
    // cout << "checking move ";
    // move.print();
    if (move.castling) {
      // check if any squares that king moves to are threatened
      auto threats = get_threats<Player(-turn)>(board);
      if (  // white kingside castling
          (move.from == 60 && move.to == 62 &&
           (threats[60] || threats[61] || threats[62])) ||
          // black queenside castling
          (move.from == 4 && move.to == 6 &&
           (threats[4] || threats[5] || threats[6])) ||
          // white kingside castling
          (move.from == 60 && move.to == 58 &&
           (threats[60] || threats[59] || threats[58])) ||
          // black queenside castling
          (move.from == 4 && move.to == 2 &&
           (threats[4] || threats[3] || threats[2]))) {
        valid_move = false;
      }
    } else {
      // check if king is threatened
      board.make_move(move);
      valid_move = !is_in_check(board, turn);
      board.unmake_move(move);
    }
    if (valid_move)
      (board.empty(move.to) || move.promotion != Empty || move.enpassant ||
               move.castling
           ? sorted_moves
           : others)
          .push_back(move);
  }
  // cout << "sorted:" << endl;
  // for (auto& x : sorted_moves) {
  //   x.print();
  // }
  // cout << "end sorted" << endl;

  sorted_moves.insert(sorted_moves.end(), others.begin(), others.end());
  return sorted_moves;
}

vector<Move> generate_legal_moves(Board& board) {
  if (board.turn == White)
    return generate_legal_moves<White>(board);
  else
    return generate_legal_moves<Black>(board);
}

bool make_move_if_legal(Board& board, string move) {
  int l = move.length();
  if (l == 4 || l == 5) {
    const int from = sq2idx(move[0], move[1]);
    const int to = sq2idx(move[2], move[3]);
    Piece promotion = Empty;
    if (l == 5)
      promotion =
          char2piece(board.turn == White ? toupper(move[4]) : tolower(move[4]));
    for (auto& m : generate_legal_moves(board))
      if (m.from == from && m.to == to && m.promotion == promotion) {
        board.make_move(m);
        return true;
      }
  }
  return false;
}

vector<string> list_san(Board& board, vector<Move> movelist) {
  vector<string> temp;
  temp.resize(movelist.size());
  for (auto& move : movelist) {
    string san;
    if (move.castling) {
      if ((move.from == 4 && move.to == 6) ||
          (move.from == 60 && move.to == 62))
        san = "O-O";
      else if ((move.from == 4 && move.to == 2) ||
               (move.from == 60 && move.to == 58))
        san = "O-O-O";
    } else {
      char piece = toupper(piece2char(board.board[move.from]));
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
      if (!board.empty(move.to) || move.enpassant) san += 'x';
      san += idx2sq(move.to);
      if (move.promotion != Empty) {
        san += '=';
        san += toupper(piece2char(move.promotion));
      }
      if (is_in_check(board, board.turn)) san += '+';
    }
    temp.push_back(san);
  }
  return temp;
}
