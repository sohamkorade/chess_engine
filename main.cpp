#include "ai.hpp"
#include "board.hpp"
#include "game.hpp"

bool debug_mode = false;

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
      for (auto& move : g.board.generate_pseudo_moves()) {
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
    } else if (cmd == "play") {
      vector<Move> movelist;
      Move move;
      bool white = true;
      // g.board.load_startpos();
      cout << "Play as (1) White (2) Black" << endl;
      cin >> cmd;
      if (cmd == "2") {
        move = g.random_move();
        white = false;
      }
      while (cmd != "q") {
        g.board.print(g.board.to_uci(move), !white);
        movelist = g.board.generate_legal_moves();
        cout << "(q) Quit (n) New game" << endl;
        if (movelist.size() > 0) {
          for (size_t i = 0; i < movelist.size(); i++) {
            cout << "(";
            cout.width(2);
            cout << left << i + 1 << flush;
            cout << ") ";
            cout.width(7);
            cout << g.board.to_san(movelist[i]) << flush;
            if (i % 3 == 2) cout << endl;
          }
          cout << endl;
          cout << "Choose move: ";
          cin >> cmd;
          if (cmd == "n") {
            g.new_game();
            continue;
          }
          size_t i = 0;
          if (all_of(cmd.begin(), cmd.end(), ::isdigit)) i = stoi(cmd);
          if (i > 0 && i <= movelist.size())
            g.make_move(movelist[i - 1]);
          else
            cout << "Invalid move.";
          move = g.random_move();
          if (move.from == 0 && move.to == 0) {
            if (g.board.is_in_check(white ? Black : White))
              cout << "You win!" << endl;
            else
              cout << "Stalemate!" << endl;
            break;
          }
        } else {
          if (g.board.is_in_check(white ? White : Black))
            cout << "I win!" << endl;
          else
            cout << "Stalemate!" << endl;
          break;
        }
      }
    } else if (cmd == "quit" || cmd == "q")
      break;
    g.board.print();
    cout << "ply " << g.ply << " end " << g.end << endl
         << "fen " << g.board.to_fen() << endl;
  }
}

vector<string> split(string s, string delimiter) {
  size_t pos_start = 0, pos_end, delim_len = delimiter.length();
  string token;
  vector<string> res;

  while ((pos_end = s.find(delimiter, pos_start)) != string::npos) {
    token = s.substr(pos_start, pos_end - pos_start);
    pos_start = pos_end + delim_len;
    res.push_back(token);
  }

  res.push_back(s.substr(pos_start));
  return res;
}

void uci() {
  multiset<string> transpositions;
  Board board1;
  AI ai(board1);
  auto& board = ai.board;
  string cmd;
  vector<string> cmds;

  int wtime = 60000, btime = 60000, winc = 0, binc = 0;

  while (getline(cin, cmd)) {
    cmds = split(cmd, " ");
    int n = cmds.size() - 1;
    if (cmds[0] == "uci") {
      cout << "id name Chess by Soham" << endl;
      cout << "id author Soham Korade" << endl;
      cout << "uciok" << endl;
    } else if (cmds[0] == "ucinewgame") {
    } else if (cmds[0] == "position") {
      transpositions.clear();
      if (n >= 1) {
        if (cmds[1] == "fen") {
          string fen;
          for (int i = 2; i < 2 + 6; i++) fen += cmds[i] + " ";
          // cout << "--\n{" << fen << "}\n--\n";
          board.load_fen(fen);
          if (n >= 8 && cmds[8] == "moves") {
            for (int i = 9; i < n + 1; i++) {
              board.make_move(cmds[i]);
              transpositions.insert(board.pos_hash());
            }
          }
        } else if (cmds[1] == "startpos") {
          transpositions.clear();
          board.load_startpos();
          if (n >= 2 && cmds[2] == "moves") {
            for (int i = 3; i < n + 1; i++) {
              board.make_move(cmds[i]);
              transpositions.insert(board.pos_hash());
            }
          }
        }
      }
    } else if (cmds[0] == "go") {
      if (n >= 2) {
        if (cmds[1] == "searchmoves") {
        } else if (cmds[1] == "ponder") {
        } else if (cmds[1] == "wtime") {
          // example: go wtime 56329 btime 86370 winc 1000 binc 1000
          wtime = stoi(cmds[2]);
          btime = stoi(cmds[4]);
          winc = stoi(cmds[6]);
          binc = stoi(cmds[8]);
          ai.set_clock(wtime, btime, winc, binc);

        } else if (cmds[1] == "btime") {
        } else if (cmds[1] == "winc") {
        } else if (cmds[1] == "binc") {
        } else if (cmds[1] == "movestogo") {
        } else if (cmds[1] == "depth") {
        } else if (cmds[1] == "nodes") {
        } else if (cmds[1] == "mate") {
        } else if (cmds[1] == "movetime") {
        } else if (cmds[1] == "infinite") {
        } else if (cmds[1] == "startpos") {
          transpositions.clear();
          board.load_startpos();
          for (int i = 2; i < n + 1; i++) {
            if (cmds[i] == "nodes") break;
            board.make_move(cmds[i]);
            transpositions.insert(board.pos_hash());
          }
        }
      }
      // AI ai(board);
      auto [bestmove, bestscore] = ai.search_best_move(transpositions);
      cout << "bestmove " << board.to_uci(bestmove) << endl;
    } else if (cmds[0] == "stop") {
    } else if (cmds[0] == "ponderhit") {
    } else if (cmds[0] == "debug") {
      debug_mode = cmds[1] == "on";
    } else if (cmds[0] == "isready") {
      cout << "readyok" << endl;
    } else if (cmds[0] == "setoption") {
    } else if (cmds[0] == "register") {
    } else if (cmds[0] == "d") {
      board.print();
      cout << "fen: " << board.to_fen() << endl;
    } else if (cmds[0] == "quit") {
      break;

      // additional commands
    } else if (cmds[0] == "pseudo") {
      Game temp;
      temp.board = board;
      temp.movelist = board.generate_pseudo_moves();
      temp.print_movelist();
    } else if (cmds[0] == "legal") {
      Game temp;
      temp.board = board;
      temp.movelist = board.generate_legal_moves();
      temp.print_movelist();
    } else if (cmds[0] == "lichess") {
      string fen = board.to_fen();
      replace(fen.begin(), fen.end(), ' ', '_');
      cout << "https://lichess.org/analysis/" << fen << endl;
    } else if (cmds[0] == "perft" || cmds[0] == "divide") {
      board.divide(stoi(cmds[1]));
    } else if (cmds[0] == "debugmoves") {
      for (int i = 1; i < n + 1; i++) {
        board.make_move(cmds[i]);
        board.print();
        cout << "fen: " << board.to_fen() << endl;
      }
      board.load_startpos();
    } else if (cmds[0] == "eval") {
      // AI ai(board);
      ai.print_eval();
    } else {
      cout << "Invalid command: " << cmd << endl;
    }
  }
}

int main() {
  srand(time(0));
  // test_navigation();
  uci();
}