#ifndef COMMON
#define COMMON

#include <cassert>
#include <cstdint>
#include <algorithm>

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
	Forbid,
	TH3,
	TDH3,
	T4,
	T4H3,
	TH4,
	T5,
};

enum Bound {
	BOUND_NONE,
	BOUND_UPPER,
	BOUND_LOWER,
	BOUND_EXACT = BOUND_UPPER | BOUND_LOWER
};

enum : int {
	// The following DEPTH_ constants are used for transposition table entries
	// and quiescence search move generation stages. In regular search, the
	// depth stored in the transposition table is literal: the search depth
	// (effort) used to make the corresponding transposition table value. In
	// quiescence search, however, the transposition table entries only store
	// the current quiescence move generation stage (which should thus compare
	// lower than any regular search depth).
	DEPTH_QS = 0,
	// For transposition table entries where no searching at all was done
	// (whether regular or qsearch) we use DEPTH_UNSEARCHED, which should thus
	// compare lower than any quiescence or regular depth. DEPTH_ENTRY_OFFSET
	// is used only for the transposition table entry occupancy check (see tt.cpp),
	// and should thus be lower than DEPTH_UNSEARCHED.
	DEPTH_UNSEARCHED = -2,
	DEPTH_ENTRY_OFFSET = -3
};

using Key = uint64_t;

constexpr int MAX_PLY = 246;

constexpr int BOARD_LENGTH = 32;
constexpr int BOARD_SIZE = BOARD_LENGTH * BOARD_LENGTH;
constexpr int BOARD_BOUNDARY = 5;
constexpr int MAX_GAME_LENGTH = 20;
constexpr int MAX_MOVE = MAX_GAME_LENGTH * MAX_GAME_LENGTH;

constexpr int CAND_RANGE = 2;

using Depth = int;
// Value is used as an alias for int, this is done to differentiate between a search
// value and any other integer value. The values used in search are always supposed
// to be in the range (-VALUE_NONE, VALUE_NONE] and should not exceed this range.
using Value = int;

constexpr Value VALUE_ZERO = 0;
constexpr Value VALUE_DRAW = 0;
constexpr Value VALUE_NONE = 32002;
constexpr Value VALUE_INFINITE = 32001;

constexpr Value VALUE_MATE = 32000;
constexpr Value VALUE_MATE_IN_MAX_PLY = VALUE_MATE - MAX_PLY;
constexpr Value VALUE_MATED_IN_MAX_PLY = -VALUE_MATE_IN_MAX_PLY;

constexpr bool is_valid(Value value) { return value != VALUE_NONE; }

constexpr bool is_win(Value value) {
	assert(is_valid(value));
	return value >= VALUE_MATE_IN_MAX_PLY;
}

constexpr bool is_loss(Value value) {
	assert(is_valid(value));
	return value <= VALUE_MATED_IN_MAX_PLY;
}

constexpr bool is_decisive(Value value) { return is_win(value) || is_loss(value); }

constexpr Value mate_in(int ply) { return VALUE_MATE - ply; }

constexpr Value mated_in(int ply) { return -VALUE_MATE + ply; }

struct Square {

	int _sq;

	Square() = default;
	Square(int x, int y) : _sq(((x + BOARD_BOUNDARY) << 5) | (y + BOARD_BOUNDARY)) {}
	constexpr explicit Square(int sq) : _sq(sq) {}
	constexpr operator int() { return _sq; }
	constexpr int x() const { return (_sq >> 5) - BOARD_BOUNDARY; }
	constexpr int abx() const { return _sq >> 5; }
	constexpr int y() const { return (_sq & 31) - BOARD_BOUNDARY; }
	constexpr int aby() const { return _sq & 31; }
	constexpr int is_ok() const { return _sq >= NONE._sq && _sq < BOARD_SIZE; }
	static int distance(Square u, Square v) {
		return (std::max)(std::abs(u.x() - v.x()), std::abs(u.y() - v.y()));
	}
	static int lineDistance(Square u, Square v) {

		int dx = u.x() - v.x();
		int dy = u.y() - v.y();

		if (dx == 0)
			return std::abs(dy);
		else if (dy == 0 || dx - dy == 0 || dx + dy == 0)
			return std::abs(dx);
		else
			return MAX_GAME_LENGTH;
	}
	bool operator ==(Square sq) const { return _sq == sq._sq; }
	bool operator !=(Square sq) const { return _sq != sq._sq; }

	static const Square NONE;
};

constexpr Square& operator++(Square& p){
	return p = Square(p + 1);
}
constexpr Square& operator--(Square& p){
	return p = Square(p - 1);
}

inline constexpr Square Square::NONE{ 0 };

enum Direction : int {
	D_U = -BOARD_LENGTH,
	D_L = -1,
	D_D = BOARD_LENGTH,
	D_R = 1,

	D_UL = D_U + D_L,
	D_UR = D_U + D_R,
	D_DL = D_D + D_L,
	D_DR = D_D + D_R,
};

constexpr int Direction4[4] = { D_R, D_D, D_DR, D_UR };

const int EXPAND_S2L3[] = { //S2L3
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
const int EXPAND_S2L4[] = { //S2L4
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
const int EXPAND_L4[] = { //L4
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



