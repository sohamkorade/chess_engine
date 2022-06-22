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

  ply++, end++;
}

void Game::prev() {
  if (ply > 0) board.unmake_move(movelist[--ply]);
}
void Game::next() {
  if (ply < end) board.make_move(movelist[ply++]);
}

void Game::print_movelist() {
  for (int i = 0; i < movelist.size(); i++) {
    cerr << i + 1 << " " << board.to_san(movelist[i]) << "    ";
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
  vector<Move> legal = board.generate_legal_moves();
  make_move(legal[rand() % legal.size()]);
}