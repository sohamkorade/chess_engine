

#include "board.hpp"
#include "game.hpp"
#include "search.hpp"

bool debug_mode = false;
thread ai_thread;

void parse_and_make_moves(istringstream& iss, Board& board,
                          multiset<uint64_t>& transpositions) {
  string token;
  while (iss >> token) {
    if (make_move_if_legal(board, token))
      transpositions.insert(board.zobrist_hash());
    else
      break;
  }
}

void test_navigation() {
  Game g;
  // g.board.load_fen("8/8/3k4/8/8/3KP1r1/8/8 w - - 0 1");
  g.board.print();

  string cmd;
  while (cin >> cmd) {
    if (cmd == "list" || cmd == "l")
      g.print_movelist();

    else if (cmd == "rand" || cmd == "r") {
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
    } else if (cmd == "turn") {
      g.board.change_turn();
    } else if (cmd == "pgn") {
      g.print_pgn();
    } else if (cmd == "quit" || cmd == "q")
      break;
    g.board.print();
    cout << "ply " << g.ply << " end " << g.end << endl;
  }
}

void uci() {
  multiset<uint64_t> transpositions;
  Board board1;
  Search ai(board1);
  auto& board = ai.board;
  string line, token;

  while (getline(cin, line)) {
    istringstream iss(line);
    iss >> token;
    if (token[0] == '#') continue;  // ignore comments

    if (token == "uci") {
      cout << "id name Chess by Soham" << endl;
      cout << "id author Soham Korade" << endl;
      cout << "uciok" << endl;
    } else if (token == "ucinewgame") {
    } else if (token == "position") {
      iss >> token;
      transpositions.clear();
      if (token == "fen") {
        string fen;
        for (int i = 0; i < 6; i++) {
          if (iss >> token && token != "moves")
            fen += token + " ";
          else
            break;
        }
        // cout << "--\n{" << fen << "}\n--\n";
        board.load_fen(fen);
      } else if (token == "startpos") {
        transpositions.clear();
        board.load_startpos();
      }
      iss >> token;
      if (token == "moves") parse_and_make_moves(iss, board, transpositions);
    } else if (token == "go") {
      // example: go wtime 56329 btime 86370 winc 1000 binc 1000
      transpositions.clear();
      ai.search_type = Time_per_game;
      ai.set_clock(30000, 30000, 0, 0);
      ai.max_depth = 100;
      while (iss >> token) {
        if (token == "searchmoves") {
        } else if (token == "ponder") {
          ai.search_type = Ponder;
        } else if (token == "wtime") {
          iss >> ai.wtime;
        } else if (token == "btime") {
          iss >> ai.btime;
        } else if (token == "winc") {
          iss >> ai.winc;
        } else if (token == "binc") {
          iss >> ai.binc;
        } else if (token == "movestogo") {
        } else if (token == "depth") {
          iss >> ai.max_depth;
          ai.search_type = Fixed_depth;
        } else if (token == "nodes") {
        } else if (token == "mate") {
          ai.search_type = Mate;
        } else if (token == "movetime") {
          iss >> ai.mtime;
          ai.search_type = Time_per_move;
        } else if (token == "infinite") {
          ai.search_type = Infinite;
        } else if (token == "startpos") {
          board.load_startpos();
          iss >> token;
          if (token == "moves")
            parse_and_make_moves(iss, board, transpositions);
        } else if (token == "perft") {
          iss >> token;
          if (!ai.searching) {
            if (ai_thread.joinable()) ai_thread.join();
            ai_thread = thread([&]() { divide(board, stoi(token)); });
          }
        }
      }
      if (!ai.searching) {
        if (ai_thread.joinable()) ai_thread.join();
        ai_thread = thread([&]() { ai.search(transpositions); });
      }
    } else if (token == "stop") {
      ai.searching = false;
      if (ai_thread.joinable()) ai_thread.join();
    } else if (token == "ponderhit") {
    } else if (token == "debug") {
      iss >> token;
      debug_mode = token == "on";
    } else if (token == "isready") {
      cout << "readyok" << endl;
    } else if (token == "setoption") {
    } else if (token == "register") {
    } else if (token == "d") {
      board.print();
    } else if (token == "quit") {
      break;

      // additional commands
    } else if (token == "pseudo") {
      Game temp;
      temp.board = board;
      temp.movelist = generate_pseudo_moves(board);
      temp.print_movelist();
    } else if (token == "legal") {
      Game temp;
      temp.board = board;
      temp.movelist = generate_legal_moves(board);
      temp.print_movelist();
    } else if (token == "lichess") {
      string fen = board.to_fen();
      replace(fen.begin(), fen.end(), ' ', '_');
      cout << "https://lichess.org/analysis/" << fen << endl;
    } else if (token == "perft" || token == "divide") {
      iss >> token;
      if (!ai.searching) {
        if (ai_thread.joinable()) ai_thread.join();
        ai_thread = thread([&]() { divide(board, stoi(token)); });
      }
    } else if (token == "moves") {
      while (iss >> token) {
        if (make_move_if_legal(board, token))
          board.print();
        else
          break;
      }
      board.load_startpos();
    } else if (token == "eval") {
      ai.print_eval();
    } else if (token == "isincheck") {
      cout << is_in_check(board, board.turn) << endl;
    } else {
      cout << "Invalid command: " << line << endl;
    }
  }
  ai.searching = false;
  if (ai_thread.joinable()) ai_thread.join();
}

int main() {
  srand(time(0));
  // test_navigation();
  init_zobrist();
  uci();
}