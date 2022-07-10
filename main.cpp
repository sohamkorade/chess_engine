#include "game.hpp"

void test_fen() {
  string fen =
      "r2qkbnr/pp3ppp/2np4/2pNp3/4P1b1/6P1/PPPPNP1P/R1BQKB1R w KQkq - 2 6 ";
  Board b;
  b.load_fen(fen);
  b.print();
}

void test_navigation() {
  Game g;
  // g.board.load_fen("8/8/3k4/8/8/3KP1r1/8/8 w - - 0 1");
  g.board.print();

  string cmd;
  while (cin >> cmd) {
    if (cmd == "list" || cmd == "l")
      g.print_movelist();
    else if (cmd == "perft" || cmd == "divide") {
      int depth;
      cin >> depth;
      g.board.divide(depth);
    } else if (cmd == "rand" || cmd == "r") {
      g.random_move();
    } else if (cmd == "random") {
      int times;
      cin >> times;
      for (int i = 0; i < times; i++) g.random_move();
    } else if (cmd == "prev" || cmd == "p")
      g.prev();
    else if (cmd == "next" || cmd == "n")
      g.next();
    else if (cmd == "start" || cmd == "s")
      g.seek(0);
    else if (cmd == "end" || cmd == "e")
      g.seek(g.end);
    else if (cmd == "seek") {
      int pos;
      cin >> pos;
      g.seek(pos);
    } else if (cmd == "pseudo") {
      Game temp = g;
      temp.movelist = g.board.generate_pseudo_moves();
      temp.print_movelist();
    } else if (cmd == "turn") {
      g.board.change_turn();
    } else if (cmd == "legal") {
      Game temp = g;
      temp.movelist = g.board.generate_legal_moves();
      temp.print_movelist();
    } else if (cmd == "threats") {
      g.board.mark_threats().print();
    } else if (cmd == "move" || cmd == "m") {
      cin >> cmd;
      bool valid = false;
      vector<Move> pseudo = g.board.generate_pseudo_moves();
      for (auto &move : pseudo) {
        if (g.board.to_san(move) == cmd || g.board.to_uci(move) == cmd) {
          cout << "Valid move" << endl;
          g.make_move(move);
          valid = true;
          break;
        }
      }
      if (!valid) cout << "Invalid move" << endl;
    } else if (cmd == "fen") {
      getline(cin, cmd);
      g.board.load_fen(cmd.substr(1));
    } else if (cmd == "pgn") {
      g.print_pgn();
    } else if (cmd == "lichess") {
      string fen = g.board.to_fen();
      replace(fen.begin(), fen.end(), ' ', '_');
      cout << "https://lichess.org/analysis/" << fen << endl;
    } else if (cmd == "display" || cmd == "d") {
      g.board.print();
      cout << "ply " << g.ply << " end " << g.end << endl
           << "fen " << g.board.to_fen() << endl;
    } else if (cmd == "quit" || cmd == "q")
      break;
    g.board.print();
    cout << "ply " << g.ply << " end " << g.end << endl
         << "fen " << g.board.to_fen() << endl;
  }
}

int main() {
  srand(time(0));
  // test_fen();
  test_navigation();
}