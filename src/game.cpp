// TODO: cleanup

#include "game.hpp"

Game::Game() { new_game(); }

bool Game::make_move(string m) { return make_move_if_legal(board, m); }

bool Game::make_move(Move m) {
  if (end > 500) return false;
  if (m.from == m.to) return false;

  if (ply != end) {
    int oldply = ply;
    while (ply < end) next();
    while (ply > oldply) prev(), movelist.pop_back(), repetitions.pop_back();
    end = ply;
    end = ply;
  }
  cout << "making move: " << m.to_uci() << endl;
  repetitions.push_back(board.zobrist_hash());
  board.make_move(m);
  movelist.push_back(m);
  if (m.captured != Empty) {
    auto& alive = (hostile(m.captured, wK) ? black_alive : white_alive);
    auto it = alive.find(m.captured);
    if (it != alive.end()) alive.erase(it);
  }
  result = get_result();
  ply++, end++;
  return true;
}

void Game::prev() {
  if (ply > 0) board.unmake_move(movelist[--ply]), repetitions.pop_back();
  result = get_result();
}
void Game::next() {
  if (ply < end)
    board.make_move(movelist[ply++]),
        repetitions.push_back(board.zobrist_hash());
  result = get_result();
}

void Game::print_movelist() {
  Board temp;
  for (size_t i = 0; i < movelist.size(); i++) {
    cerr << i + 1 << " " << to_san(temp, movelist[i]) << "    ";
    movelist[i].print();
    temp.make_move(movelist[i]);
  }
}

void Game::print_pgn() {
  cerr << "PGN" << endl;
  Board temp;
  for (int i = 0; i < end; i++) {
    if (i % 2 == 0) cerr << (i / 2 + 1) << ".";
    cerr << to_san(temp, movelist[i]) << " ";
    temp.make_move(movelist[i]);
  }
}
void Game::seek(int n) {
  while (ply < n && ply < end) next();
  while (ply > n && ply > 0) prev();
}

Move Game::random_move() {
  vector<Move> legal = generate_legal_moves(board);
  Move bestmove;
  if (legal.size() > 0) {
    random_device rd;
    uniform_int_distribution<int> dist(0, legal.size() - 1);
    bestmove = legal[dist(rd)];
  }
  return bestmove;
}

pair<Move, int> Game::ai_move() {
  pair<Move, int> ai_move;
  string input;
  input.reserve(1000);

  for (auto& m : movelist) {
    input += m.to_uci() + " ";
  }

  // write to a file
  ofstream file;
  file.open("tempConnection");
  file << "uci" << endl;
  file << "position startpos moves " << input << endl;
  file << "go movetime " + to_string(1000) << endl;
  file.close();

  cout << ">>> position startpos moves " << input << endl;

#pragma GCC diagnostic ignored "-Wunused-result"

  // run the engine
  string engine_path = "./main";
  string cmd = engine_path + " < tempConnection > tempConnection2";
  system(cmd.c_str());

  // read the output
  ifstream file2;
  file2.open("tempConnection2");
  string output;
  while (getline(file2, output)) {
    cout << output << endl;
    if (output.size() > 8) {
      if (output.substr(0, 8) == "bestmove") {
        // cout << "[" << output.substr(9, 4) << "]" << endl;
        string move = output.substr(9, 4);
        ai_move.first = get_move_if_legal(board, move);
        break;
      }
    }
  }
  file2.close();

  // cleanup
  system("rm tempConnection tempConnection2");

  return ai_move;
}

Status Game::get_result() {
  int can_move = generate_legal_moves(board).size();

  if (!can_move) {
    if (is_in_check(board, board.turn))
      return board.turn == White ? BlackWins : WhiteWins;
    else {
      cout << "Draw by stalemate" << endl;
      return Draw;
    }
  }

  // obvious draws
  if (board.fifty >= 100) return Draw;

  // size_t w_size = white_alive.size();
  // size_t b_size = black_alive.size();
  // // KvK
  // if (w_size == 1 && b_size == 1) return Draw;
  // // KvKB or KvKN
  // if (w_size == 1 && b_size == 2)
  //   if (black_alive.count(bB) || black_alive.count(bN)) return Draw;
  // // KBvK or KNvK
  // if (b_size == 1 && w_size == 2)
  //   if (white_alive.count(wB) || white_alive.count(wN)) return Draw;
  // // // KBvKB, bishops being the same color
  // // if (b_size == 2 && w_size == 2 &&
  // //     board.sq_color(board.board.find('B')) ==
  // //         board.sq_color(board.board.find('b')))
  // //   return Draw;

  // repetition
  auto hash = board.zobrist_hash();
  int count = 0;
  int n = repetitions.size();
  for (int i = 0; i < n - 1; i++) {
    if (repetitions[i] == hash) count++;
    if (count == 3) {
      cout << "Draw by repetition" << endl;
      return Draw;
    }
  }
  return Undecided;
}

void Game::new_game() {
  ply = end = 0;
  movelist.clear();
  board.load_startpos();
  update_alive();
  result = Undecided;
  repetitions.clear();
}

bool Game::load_fen(string fen) {
  new_game();
  auto temp = board;
  bool valid = temp.load_fen(fen);
  if (valid) {
    board = temp;
    update_alive();
    result = get_result();
  }
  return valid;
}

void Game::update_alive() {
  white_alive.clear();
  black_alive.clear();
  for (auto& x : board.board)
    if (x != Empty) (x < 0 ? black_alive : white_alive).insert(x);
}
