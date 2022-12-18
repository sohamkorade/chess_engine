#include <chrono>
#include <fstream>

#include "ai.hpp"

multiset<string> transpositions;

int test(int argc, char* argv[]) {
  // cout.setstate(ios_base::failbit);
  string filename = "bestmovetest.epd";
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
    string found = b.to_san(ai.search_best_move(transpositions).first);
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
  cout << "Total time taken: " << total << endl;
  return 0;
}

int main(int argc, char* argv[]) {
  // return test(argc, argv);

  Board b;
  AI ai(b);
  ai.set_clock(0, 0, 0, 0);
  ai.max_depth = 3;
  auto [bestmove, bestscore] = ai.search_best_move(transpositions);
  cout << "bestmove " << b.to_uci(bestmove) << endl;
}