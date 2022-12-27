#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>
#include <climits>
#include <fstream>
#include <future>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
using namespace std;

enum Direction {
  N = -8,
  S = +8,
  E = +1,
  W = -1,
  NE = -7,
  NW = -9,
  SE = +9,
  SW = +7
};
enum Player { White = 1, Black = -1 };
enum Status { Undecided, WhiteWins, BlackWins, Draw };

enum Piece {
  Empty = 0,
  wP = 1,
  wN = 2,
  wB = 3,
  wR = 4,
  wQ = 5,
  wK = 6,
  bP = -1,
  bN = -2,
  bB = -3,
  bR = -4,
  bQ = -5,
  bK = -6
};

enum SearchType {
  Infinite,
  Fixed_depth,
  Time_per_move,
  Time_per_game,
  Ponder,
  Mate
};

enum CheckType { CheckNotChecked = 0, WhiteChecked = 1, BlackChecked = -1 };

enum EvalType { Exact, LowerBound, UpperBound };