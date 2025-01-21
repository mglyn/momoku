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

constexpr int FTYPE_NUM = 8;
enum FType :uint8_t {
	TNone,
	TH3,
	TDH3,
	T4,
	T4H3,
	TH4,
	T5,
	Forbid
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
constexpr int MAX_GAME_LENGTH = 20;
constexpr int MAX_MOVE = MAX_GAME_LENGTH * MAX_GAME_LENGTH;

constexpr int CAND_RANGE = 2;

constexpr int WIN_CRITICAL = 20000;
constexpr int WIN_MAX = 30000;
constexpr int VAL_INF = 30001;
constexpr int VAL_DRAW = 0;
constexpr int VAL_ZERO = 0;

constexpr int MAX_PLY = 128;
constexpr int END_DEP = 127;

struct Sqare {
	int16_t _sq;
	Sqare() { _sq = 0; }
	Sqare(int x, int y) : _sq(((x + BOARD_BOUNDARY) << 5) | (y + BOARD_BOUNDARY)) {}
	constexpr Sqare(int sq) : _sq(sq) {}
	operator int() { return _sq; }
	constexpr int x() const { return (_sq >> 5) - BOARD_BOUNDARY; }
	constexpr int abx() const { return _sq >> 5; }
	constexpr int y() const { return (_sq & 31) - BOARD_BOUNDARY; }
	constexpr int aby() const { return _sq & 31; }
	static int dis1(Sqare u, Sqare v) {
		return (std::max)(std::abs(u.x() - v.x()), std::abs(u.y() - v.y()));
	}
	static int dis2(Sqare u, Sqare v) {
		int dx = std::abs(u.x() - v.x()), dy = std::abs(u.y() - v.y());
		if (dy == 0 || dx - dy == 0 || dx + dy == 0)return std::abs(dx);
		else if (dx == 0)return abs(dy);
	}
	bool operator ==(Sqare sq) const { return _sq == sq._sq; }
	bool operator !=(Sqare sq) const { return _sq != sq._sq; }
};

constexpr Sqare NULLSQARE = 0;

enum Dir : int16_t {
	D_U = -BOARD_LENGTH,
	D_L = -1,
	D_D = BOARD_LENGTH,
	D_R = 1,

	D_UL = D_U + D_L,
	D_UR = D_U + D_R,
	D_DL = D_D + D_L,
	D_DR = D_D + D_R,
};

constexpr int16_t D4[4] = { D_R, D_D, D_DR, D_UR };

[[maybe_unused]] constexpr int16_t EXPAND_S2L3[] = { //S2L3
	D_U * 3 + D_L * 3,
	D_U * 3,
	D_U * 3 + D_R * 3,

	D_U * 2 + D_L * 2,
	D_U * 2 + D_L,
	D_U * 2,
	D_U * 2 + D_R,
	D_U * 2 + D_R * 2,

	D_U + D_L * 2,
	D_U + D_L,
	D_U,
	D_U + D_R,
	D_U + D_R * 2,

	D_L * 3,
	D_L * 2,
	D_L,
	D_R,
	D_R * 2,
	D_R * 3,

	D_D + D_L * 2,
	D_D + D_L,
	D_D,
	D_D + D_R,
	D_D + D_R * 2,

	D_D * 2 + D_L * 2,
	D_D * 2 + D_L,
	D_D * 2,
	D_D * 2 + D_R,
	D_D * 2 + D_R * 2,

	D_D * 3 + D_L * 3,
	D_D * 3,
	D_D * 3 + D_R * 3,
};

[[maybe_unused]] constexpr int16_t EXPAND_S2L4[] = { //S2L4
	D_U * 4 + D_L * 4,
	D_U * 4,
	D_U * 4 + D_R * 4,

	D_U * 3 + D_L * 3,
	D_U * 3,
	D_U * 3 + D_R * 3,

	D_U * 2 + D_L * 2,
	D_U * 2 + D_L,
	D_U * 2,
	D_U * 2 + D_R,
	D_U * 2 + D_R * 2,

	D_U + D_L * 2,
	D_U + D_L,
	D_U,
	D_U + D_R,
	D_U + D_R * 2,

	D_L * 4,
	D_L * 3,
	D_L * 2,
	D_L,
	D_R,
	D_R * 2,
	D_R * 3,
	D_R * 4,

	D_D + D_L * 2,
	D_D + D_L,
	D_D,
	D_D + D_R,
	D_D + D_R * 2,

	D_D * 2 + D_L * 2,
	D_D * 2 + D_L,
	D_D * 2,
	D_D * 2 + D_R,
	D_D * 2 + D_R * 2,

	D_D * 3 + D_L * 3,
	D_D * 3,
	D_D * 3 + D_R * 3,

	D_D * 4 + D_L * 4,
	D_D * 4,
	D_D * 4 + D_R * 4,
};

[[maybe_unused]] constexpr int16_t EXPAND_L4[] = { //L4
	D_U * 4 + D_L * 4,
	D_U * 4,
	D_U * 4 + D_R * 4,

	D_U * 3 + D_L * 3,
	D_U * 3,
	D_U * 3 + D_R * 3,

	D_U * 2 + D_L * 2,
	D_U * 2,
	D_U * 2 + D_R * 2,

	D_U + D_L,
	D_U,
	D_U + D_R,

	D_L * 4,
	D_L * 3,
	D_L * 2,
	D_L,
	D_R,
	D_R * 2,
	D_R * 3,
	D_R * 4,

	D_D + D_L,
	D_D,
	D_D + D_R,

	D_D * 2 + D_L * 2,
	D_D * 2,
	D_D * 2 + D_R * 2,

	D_D * 3 + D_L * 3,
	D_D * 3,
	D_D * 3 + D_R * 3,

	D_D * 4 + D_L * 4,
	D_D * 4,
	D_D * 4 + D_R * 4,
};

#endif



