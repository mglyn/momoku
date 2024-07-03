#ifndef POS
#define POS

#include "../common/common.h"

struct Pos {
	int16_t _pos;
	Pos() { _pos = 0; }
	Pos(int x, int y) : _pos(((x + BOARD_BOUNDARY) << 5) | (y + BOARD_BOUNDARY)) {}
	constexpr Pos(int pos) : _pos(pos) {}
	operator int() { return _pos; }
	constexpr int x() const { return (_pos >> 5) - BOARD_BOUNDARY; }
	constexpr int abx() const { return _pos >> 5; }
	constexpr int y() const { return (_pos & 31) - BOARD_BOUNDARY; }
	constexpr int aby() const { return _pos & 31; }
	static int dis1(Pos u, Pos v) {
		return (std::max)(std::abs(u.x() - v.x()), std::abs(u.y() - v.y()));
	}
	static int dis2(Pos u, Pos v) {
		int dx = std::abs(u.x() - v.x()), dy = std::abs(u.y() - v.y());
		if (dy == 0 || dx - dy == 0 || dx + dy == 0)return std::abs(dx);
		else if (dx == 0)return abs(dy);
	}
	bool operator ==(Pos pos) const { return _pos == pos._pos; }
	bool operator !=(Pos pos) const { return _pos != pos._pos; }
};

constexpr Pos NULLPOS = 0;

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

constexpr int16_t D8[8] = { D_R, D_D, D_DR, D_UR, D_L, D_U, D_UL, D_DL };

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

constexpr int16_t EXPAND_L4[] = { //L4
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
