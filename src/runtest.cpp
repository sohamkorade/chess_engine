#include <unistd.h>

#include <cstring>

#include "ai.hpp"
#include "board.hpp"

multiset<uint64_t> transpositions;

int perft(int argc, char* argv[]) {
  // cout.setstate(ios_base::failbit);
  string filename = "../tests/perftsuite.epd";
  if (argc > 1) filename = argv[1];
  ifstream epd(filename);
  Board board;
  string line;
  int k = 1000;
  int count = 1;
  double total = 0;
  cerr.setstate(ios_base::failbit);
  while (getline(epd, line) && k--) {
    cout << "Case #" << count++ << ":" << endl;
    vector<string> parts;
    int start = 0;
    int end = line.find(" ;");
    do {
      parts.push_back(line.substr(start, end - start));
      start = end + 2;
      end = line.find(" ;", start);
    } while (~end);
    parts.push_back(line.substr(start, end - start));
    board.load_fen(parts[0]);
    cout << "fen: " << parts[0] << endl;
    for (size_t i = 1; i < parts.size(); i++) {
      cout << parts[i] << endl;
      int expected = stoi(parts[i].substr(parts[i].find(" ") + 1));
      cout << "               ";
      if (expected > 1e6) {
        cout << "skipped" << endl;
        continue;
      }
      auto begin = chrono::high_resolution_clock::now();
      int found = divide(board, i);
      auto end = chrono::high_resolution_clock::now();

      auto elapsed =
          chrono::duration_cast<chrono::milliseconds>(end - begin).count() *
          1e-3;
      total += elapsed;

      cout << (expected == found ? "\e[32mPASS\e[0m" : "\e[31mFAIL\e[0m")
           << " [" << elapsed << "s]" << endl;

      if (expected != found) {
        cout << "expected '" << expected << "' but found '" << found << "'"
             << endl;
        return -1;
      }
    }
  }
  cerr.clear();
  cout << "Total time taken: " << total << endl;
  return 0;
}

int bestmove(int argc, char* argv[]) {
  // cout.setstate(ios_base::failbit);
  string filename = "../tests/bestmovetest.epd";
  if (argc > 1) filename = argv[1];
  ifstream epd(filename);
  Board b;
  AI ai(b);
  ai.set_clock(50 * 10000, 50 * 10000, 0, 0);
  // ai.max_depth = 3;
  string line;
  int k = 1000;
  int count = 1;
  double total = 0;
  cerr.setstate(ios_base::failbit);
  while (getline(epd, line) && k--) {
    cout << "Case #" << count++ << ":" << endl;
    vector<string> parts;
    int start = 0;
    int end = line.find(" bm ");
    do {
      parts.push_back(line.substr(start, end - start));
      start = end + 4;
      end = line.find(" bm ", start);
    } while (~end);
    parts.push_back(line.substr(start, end - start));
    if (!b.load_fen(parts[0])) cout << "Unable to parse FEN" << endl;

    cout << "\e[35m";
    ai.board = b;
    auto begint = chrono::high_resolution_clock::now();
    string found = b.to_san(ai.search(transpositions).first);
    auto endt = chrono::high_resolution_clock::now();
    cout << "\e[0m";

    auto elapsed =
        chrono::duration_cast<chrono::milliseconds>(endt - begint).count() *
        1e-3;
    total += elapsed;

    b.print();
    cout << "fen: " << parts[0] << endl;
    cout << parts[1] << endl;
    cout << "is '" << found << "' correct? [0|1]: " << flush;
    int correct;
    cin >> correct;
    cout << (correct ? "\e[32mPASS\e[0m" : "\e[31mFAIL\e[0m") << " [" << elapsed
         << "s]" << endl;

    // if (!correct) break;
  }
  cerr.clear();
  cout << "total time: " << total << "s" << endl;

  return 0;
}

int mate(int argc, char* argv[]) {
  string filename = "../tests/mates.epd";
  if (argc > 1) filename = argv[1];
  ifstream epd(filename);
  Board board1;
  AI ai(board1);
  auto& b = ai.board;
  ai.search_type = Mate;

  string line;
  int k = 2000;
  double total = 0;

  string PASS, FAIL;
  if (isatty(STDOUT_FILENO)) {
    PASS = "\e[32mPASS\e[0m";
    FAIL = "\e[31mFAIL\e[0m";
  } else {
    PASS = "PASS";
    FAIL = "FAIL";
  }
  int i = 0;
  while (getline(epd, line) && k--) {
    if (!b.load_fen(line)) cout << "Unable to parse FEN" << endl;
    // int ans = stoi(line.substr(line.find("; M") + 3));
    // cout << "mate in " << ans << endl;
    cout << "[" << i++ << "] in: " << line << endl;
    // cout.setstate(ios_base::failbit);
    auto begint = chrono::high_resolution_clock::now();
    Move bestmove = ai.search(transpositions).first;
    auto endt = chrono::high_resolution_clock::now();

    auto elapsed =
        chrono::duration_cast<chrono::milliseconds>(endt - begint).count() *
        1e-3;
    total += elapsed;

    cout.clear();
    cout << "out: " << b.to_san(bestmove) << " mate " << ai.debug << endl;
    cout << (stoi(ai.debug) > 0 ? PASS : FAIL) << " [" << elapsed << "s]"
         << endl;
  }

  cout << "total time: " << total << "s" << endl;
  return 0;
}

int main(int argc, char* argv[]) {
  if (argc > 0) {
    ++argv;
    --argc;
    if (!strcmp(argv[0], "perft")) perft(argc, argv);
    if (!strcmp(argv[0], "bestmove")) bestmove(argc, argv);
    if (!strcmp(argv[0], "mate")) mate(argc, argv);  // TODO: fix
  }
}