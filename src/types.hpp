#pragma once

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <climits>
#include <execution>
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

enum Direction : char {
  EmptyDirection = 0,
  N = -8,
  S = +8,
  E = +1,
  W = -1,
  NE = N + E,
  NW = N + W,
  SE = S + E,
  SW = S + W,
  NN = N + N,
  SS = S + S,
  NNW = N + NW,
  NNE = N + NE,
  WNW = W + NW,
  WSW = W + SW,
  ENE = E + NE,
  ESE = E + SE,
  SSW = S + SW,
  SSE = S + SE
};

enum Player { White = 1, Black = -1 };

enum Status { Undecided, WhiteWins, BlackWins, Draw };

enum DrawType {
  None,
  InsufficientMaterial,
  FiftyMoveRule,
  ThreefoldRepetition,
  FivefoldRepetition,
  SeventyFiveMoveRule,
  Stalemate,
  DeadPosition
};

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

typedef Piece Position[64];

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

enum MoveGenType {
  Evasions,
  NonEvasions,
  Captures,
};

// macros

// pieces relative to turn
// opp_<piece> is an opponent piece
// rel_<piece> is a friendly piece
#define GetOpponentPiece(piece, turn) \
  static_cast<Piece>(-static_cast<int>(turn) * static_cast<int>(piece))
#define GetFriendlyPiece(piece, turn) \
  static_cast<Piece>(static_cast<int>(turn) * static_cast<int>(piece))

// TODO: choose between these two

// #define opp_P (turn == Black ? wP : bP)
// #define opp_N (turn == Black ? wN : bN)
// #define opp_B (turn == Black ? wB : bB)
// #define opp_R (turn == Black ? wR : bR)
// #define opp_Q (turn == Black ? wQ : bQ)
// #define opp_K (turn == Black ? wK : bK)
// #define rel_P (turn == White ? wP : bP)
// #define rel_N (turn == White ? wN : bN)
// #define rel_B (turn == White ? wB : bB)
// #define rel_R (turn == White ? wR : bR)
// #define rel_Q (turn == White ? wQ : bQ)
// #define rel_K (turn == White ? wK : bK)

#define opp_P GetOpponentPiece(wP, turn)
#define opp_N GetOpponentPiece(wN, turn)
#define opp_B GetOpponentPiece(wB, turn)
#define opp_R GetOpponentPiece(wR, turn)
#define opp_Q GetOpponentPiece(wQ, turn)
#define opp_K GetOpponentPiece(wK, turn)
#define rel_P GetFriendlyPiece(wP, turn)
#define rel_N GetFriendlyPiece(wN, turn)
#define rel_B GetFriendlyPiece(wB, turn)
#define rel_R GetFriendlyPiece(wR, turn)
#define rel_Q GetFriendlyPiece(wQ, turn)
#define rel_K GetFriendlyPiece(wK, turn)

// directions relative to turn
#define opp_NW (turn == Black ? NW : SW)
#define opp_NE (turn == Black ? NE : SE)
#define rel_North (turn == White ? N : S)
#define rel_NW (turn == White ? NW : SW)
#define rel_NE (turn == White ? NE : SE)
#define rel_NN (turn == White ? NN : SS)
#define rel_South (turn == White ? S : N)
#define rel_SW (turn == White ? SW : NW)
#define rel_SE (turn == White ? SE : NE)
