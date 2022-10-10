#pragma once

#include <algorithm>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
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
enum Status { Undecided, White_wins, Black_wins, Draw };