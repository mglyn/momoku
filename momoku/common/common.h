#ifndef COMMON
#define COMMON

#include <vector>
#include <array>
#include <cmath>
#include <chrono>
#include <iostream>

using namespace std::chrono;

enum Piece :uint8_t {
	P1 = 0,
	P2 = 1,
	Empty = 2,
	Invalid = 3
};

enum PieceCode :uint64_t {
	C_Empty = 0,
	C_P1 = 1,
	C_P2 = 2,
	C_Invalid = 3
};

constexpr Piece operator~(Piece p) { return static_cast<Piece>(p ^ 1); }

constexpr int FTYPE_NUM = 7;
enum FType : int{
	TNone,
	TH3,
	TDH3,
	T4,
	T4H3,
	TH4,
	T5
};

enum HashType :uint8_t {
	B_Initial = 0,
	B_Upper = 1,
	B_Lower = 2,
	B_Exact = 3
};

constexpr int BOARD_LENGTH = 32;
constexpr int BOARD_SIZE = BOARD_LENGTH * BOARD_LENGTH;
constexpr int BOARD_BOUNDARY = 5;
constexpr int MAX_GAME_LENGTH = BOARD_LENGTH - 2 * BOARD_BOUNDARY;
constexpr int MAX_MOVE = MAX_GAME_LENGTH * MAX_GAME_LENGTH;

constexpr int WIN_CRITICAL = 20000;
constexpr int WIN_MAX = 30000;
constexpr int VAL_INF = 30001;

#endif



