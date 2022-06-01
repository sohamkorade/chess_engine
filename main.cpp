#include <bits/stdc++.h>

using namespace std;

int sq2idx(char file, char rank) {
  assert(file >= 'a' && file <= 'h' && rank >= '1' && rank <= '8');
  return (file - 'a') + (7 - (rank - '1')) * 8;  // matrix magic
}

class Move {
 public:
  int from, to;
  char promotion = '.', captured = '.';
  bool enpassant = false, castling = false;

  Move(string move = "a1a1") {
    int l = move.length();
    if (l == 4 || l == 5) {
      from = sq2idx(move[0], move[1]);
      to = sq2idx(move[2], move[3]);
      if (l == 5) promotion = move[4];
      castling = is_castling(move);
    }
  }

  bool is_castling(string move) {
    return move == "e1g1" || move == "e1c1" || move == "e8g8" || move == "e8c8";
  }

  void print() {
    cerr << "move: " << from << " " << to << " " << captured << promotion
         << endl;
  }
};

class Board {
 public:
  string board;
  char castling_rights[4];
  int turn, enpassant_sq_idx, fifty, moves;

  Board() { load_startpos(); }

  int sq_color(int sq_idx) { return isupper(board[sq_idx]) ? 1 : -1; }

  void print(string sq = "") {
    if (sq.length() > 2)
      sq[0] = sq[2], sq[1] = sq[3];  // extract destination square from move
    int sq_idx = sq != "" ? sq2idx(sq[0], sq[1]) : -1;

    cout << endl << "|";
    for (int i = 0; i < 64; i++) {
      cout << board[i] << "|";
      if (i % 8 == 7) {
        if (~sq_idx && sq_idx / 8 == i / 8) cout << "<";
        cout << endl;
        if (i != 63) cout << "|";
      }
    }
    if (~sq_idx)
      for (int i = 0; i < 8; i++) cout << (sq_idx % 8 == i ? " ^" : "  ");

    cout << endl;
  }

  void make_move(Move& move) {
    char captured = board[move.to];
    board[move.to] = move.promotion == '.' ? board[move.from] : move.promotion;
    board[move.from] = move.captured;
    move.captured = captured;
  }
  void unmake_move(Move& move, int turn) {
    char captured = board[move.from];
    board[move.from] =
        move.promotion == '.' ? board[move.to] : (turn == 1 ? 'p' : 'P');
    board[move.to] = move.captured;
    move.captured = captured;
  }

  void load_fen(string fen) {
    int part = 0, p = 0;
    char enpassant_sq[2];

    board =
        "........"
        "........"
        "........"
        "........"
        "........"
        "........"
        "........"
        "........";
    enpassant_sq_idx = fifty = moves = 0;

    for (auto& x : fen) {
      if (x == ' ') part++, p = 0;
      if (part == 0) {
        if (isdigit(x))
          for (int dots = x - '0'; dots--;) board[p++] = '.';
        else if (x != '/')
          board[p++] = x;
      } else if (part == 1)
        turn = x == 'w' ? 1 : -1;
      else if (part == 2)
        if (x == '-')
          castling_rights[0] = castling_rights[1] = castling_rights[2] =
              castling_rights[3] = '-';
        else
          castling_rights[p++] = x;
      else if (part == 3)
        if (x == '-')
          enpassant_sq_idx = -1;
        else
          enpassant_sq[p++] = x;
      else if (part == 4) {
        fifty *= 10;
        fifty += x - '0';
      } else if (part == 5) {
        moves *= 10;
        moves += x - '0';
      }
    }

    if (~enpassant_sq_idx)
      enpassant_sq_idx = sq2idx(enpassant_sq[0], enpassant_sq[1]);
  }

  void load_startpos() {
    // board =
    //     "rnbqkbnr"
    //     "pppppppp"
    //     "........"
    //     "........"
    //     "........"
    //     "........"
    //     "PPPPPPPP"
    //     "RNBQKBNR";
    load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  }
};

class Game {
 public:
  Board board;
  vector<Move> movelist;
  int ply = 0, end = 0;

  void make_move(string m) {
    if (ply != end) {
      int oldply = ply;
      cout << "moving to end...";
      while (ply < end) next();
      cout << "moving to ply...";
      while (ply > oldply) prev(), movelist.pop_back();
      end = ply;
    }
    Move move(m);
    board.make_move(move);
    movelist.push_back(move);

    move.print();

    ply++, end++, board.turn *= -1;
  }

  void prev() {
    if (ply > 0) {
      board.unmake_move(movelist[--ply], board.turn);
      board.turn *= -1;
    }
  }
  void next() {
    if (ply < end) {
      board.make_move(movelist[ply++]);
      board.turn *= -1;
    }
    cout << ply;
  }

  void print_movelist() {
    for (int i = 0; i < end; i++) {
      cerr << i << endl;
      movelist[i].print();
    }
  }
  // void goto_move(int n) {

  // }
};

void generate_moves(Board b) {}

void test_fen() {
  string fen =
      "r2qkbnr/pp3ppp/2np4/2pNp3/4P1b1/6P1/PPPPNP1P/R1BQKB1R w KQkq - 2 6 ";
  Board b;
  b.load_fen(fen);
  b.print();
}

void test_navigation() {
  Game g;
  g.make_move("e2e4");
  g.board.print();
  g.make_move("d7d5");
  g.board.print();
  g.make_move("e4d5");
  g.board.print();
  string cmd;
  while (cin >> cmd) {
    if (cmd == "list")
      g.print_movelist();
    else if (cmd == "prev")
      g.prev();
    else if (cmd == "next")
      g.next();
    else
      g.make_move(cmd);
    g.board.print();
    cout << "ply " << g.ply << " end " << g.end << endl;
  }
}

int main() {
  // test_fen();
  test_navigation();
}