#include "game.hpp"

void Game::make_move(string m) { make_move(Move(m)); }

void Game::make_move(Move m) {
  if (ply != end) {
    int oldply = ply;
    while (ply < end) next();
    while (ply > oldply) prev(), movelist.pop_back();
    end = ply;
  }
  board.make_move(m);
  movelist.push_back(m);

  m.print();

  ply++, end++, board.change_turn();
}

void Game::prev() {
  if (ply > 0) {
    board.unmake_move(movelist[--ply], board.turn);
    board.change_turn();
  }
}
void Game::next() {
  if (ply < end) {
    board.make_move(movelist[ply++]);
    board.change_turn();
  }
}

void Game::print_movelist() {
  for (int i = 0; i < end; i++) {
    cerr << i + 1 << " ";
    movelist[i].print();
  }
}
void Game::print_pgn() {
  cerr << "PGN" << endl;
  seek(0);
  for (int i = 0; i < end; i++) {
    if (ply % 2 == 0) cerr << (ply / 2 + 1) << ".";
    cerr << board.to_san(movelist[ply]) << " ";
    next();
  }
}
void Game::seek(int n) {
  while (ply < n) next();
  while (ply > n) prev();
}

void Game::random_move() {
  vector<Move> pseudo = generate_moves(board);
  make_move(pseudo[rand() % pseudo.size()]);
}

void slide(vector<Move>& movelist, Board b, int s, vector<Direction> dirs) {
  for (auto& dir : dirs) {
    if (westwards(dir) && !isnt_A(s)) continue;
    if (eastwards(dir) && !isnt_H(s)) continue;
    for (int dest = s + dir; in_board(dest) && !friendly(b[s], b[dest]);
         dest += dir) {
      movelist.push_back(Move(s, dest));
      if (hostile(b[s], b[dest])) break;
      if (westwards(dir) && !isnt_A(dest)) break;
      if (eastwards(dir) && !isnt_H(dest)) break;
    }
  }
}

vector<Move> generate_moves(Board b) {
  Game temp;
  vector<Move>& pseudo = temp.movelist;
  for (int s = 0; s < 64; s++) {
    char piece = b[s];
    if (hostile(piece, b.turn == Black ? 'k' : 'K')) continue;
    piece = toupper(piece);
    int file = s % 8, rank = s / 8;
    if (piece == 'K') {
      if (isnt_1(s) && !friendly(b[s], b[s + S]))
        pseudo.push_back(Move(s, s + S));  // S
      if (isnt_8(s) && !friendly(b[s], b[s + N]))
        pseudo.push_back(Move(s, s + N));  // N
      if (isnt_H(s) && !friendly(b[s], b[s + E]))
        pseudo.push_back(Move(s, s + E));  // E
      if (isnt_A(s) && !friendly(b[s], b[s + W]))
        pseudo.push_back(Move(s, s + W));  // W
      if (isnt_1(s) && isnt_H(s) && !friendly(b[s], b[s + SE]))
        pseudo.push_back(Move(s, s + SE));  // SE
      if (isnt_1(s) && isnt_A(s) && !friendly(b[s], b[s + SW]))
        pseudo.push_back(Move(s, s + SW));  // SW
      if (isnt_8(s) && isnt_H(s) && !friendly(b[s], b[s + NE]))
        pseudo.push_back(Move(s, s + NE));  // NE
      if (isnt_8(s) && isnt_A(s) && !friendly(b[s], b[s + NW]))
        pseudo.push_back(Move(s, s + NW));  // NW
      if (s == 60 && b.turn == White)
        if (b.castling_rights[0] && empty(b[61]) &&
            empty(b[62]))  // castling kingside
          pseudo.push_back(Move(s, s + 2, '.', '.', false, true));
        else if (b.castling_rights[1] && empty(b[57]) && empty(b[58]) &&
                 empty(b[59]))  // castling queenside
          pseudo.push_back(Move(s, s - 2, '.', '.', false, true));
      if (s == 4 && b.turn == Black)
        if (b.castling_rights[2] && empty(b[5]) &&
            empty(b[6]))  // castling kingside
          pseudo.push_back(Move(s, s + 2, '.', '.', false, true));
        else if (b.castling_rights[3] && empty(b[1]) && empty(b[2]) &&
                 empty(b[3]))  // castling queenside
          pseudo.push_back(Move(s, s - 2, '.', '.', false, true));
    }
    if (piece == 'P') {
      Direction rel_N = N, rel_NW = NW, rel_NE = NE;
      int rel_rank = rank;
      if (b.turn == Black) {
        rel_N = S, rel_NW = SW, rel_NE = SE;
        rel_rank = 7 - rank;
      }

      if (rel_rank > 1 && empty(b[s + rel_N])) {
        pseudo.push_back(Move(s, s + rel_N));  // push
        if (rel_rank == 6 && empty(b[s + rel_N + rel_N]))
          pseudo.push_back(Move(s, s + rel_N + rel_N));  // double push
      }
      if (isnt_A(s) && hostile(piece, b[s + rel_NW]))  // capture NW
        pseudo.push_back(Move(s, s + rel_NW));
      if (isnt_H(s) && hostile(piece, b[s + rel_NE]))  // capture NE
        pseudo.push_back(Move(s, s + rel_NE));
      if (rel_rank == 1) {  // promotion
        pseudo.push_back(Move(s, s + rel_N, 'Q'));
        pseudo.push_back(Move(s, s + rel_N, 'R'));
        pseudo.push_back(Move(s, s + rel_N, 'B'));
        pseudo.push_back(Move(s, s + rel_N, 'N'));
      }
      if (rel_rank == 6)
        if (s + rel_NW == b.enpassant_sq_idx)  // capture enpassant NW
          pseudo.push_back(Move(s, s + rel_NW, '.', '.', true));
        else if (s + rel_NE == b.enpassant_sq_idx)  // capture enpassant NE
          pseudo.push_back(Move(s, s + rel_NE, '.', '.', true));
    }
    if (piece == 'N') {
      if (rank > 1) {
        if (isnt_A(s) && !friendly(b[s], b[s + N + NW]))
          pseudo.push_back(Move(s, s + N + NW));  // UL
        if (isnt_H(s) && !friendly(b[s], b[s + N + NE]))
          pseudo.push_back(Move(s, s + N + NE));  // UR
      }
      if (file > 1) {
        if (isnt_8(s) && !friendly(b[s], b[s + W + NW]))
          pseudo.push_back(Move(s, s + W + NW));  // LU
        if (isnt_1(s) && !friendly(b[s], b[s + W + SW]))
          pseudo.push_back(Move(s, s + W + SW));  // LD
      }
      if (file < 6) {
        if (isnt_8(s) && !friendly(b[s], b[s + E + NE]))
          pseudo.push_back(Move(s, s + E + NE));  // RU
        if (isnt_1(s) && !friendly(b[s], b[s + E + SE]))
          pseudo.push_back(Move(s, s + E + SE));  // RD
      }
      if (rank < 6) {
        if (isnt_A(s) && !friendly(b[s], b[s + S + SW]))
          pseudo.push_back(Move(s, s + S + SW));  // DL
        if (isnt_H(s) && !friendly(b[s], b[s + S + SE]))
          pseudo.push_back(Move(s, s + S + SE));  // DR
      }
    }
    if (piece == 'B' || piece == 'Q') slide(pseudo, b, s, {NW, NE, SW, SE});
    if (piece == 'R' || piece == 'Q') slide(pseudo, b, s, {N, S, E, W});
  }
  temp.end = pseudo.size();
  return pseudo;
}
