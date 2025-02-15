#ifndef MOVE
#define MOVE

#include "common.h"
#include "position.h"

enum PickerMod {
	P_main,
	P_VCF,
};

struct ExtMove {
	Square sq = NULLSQUARE;
	int16_t val = 0;
	ExtMove() = default;
	ExtMove(Square sq, int val) :sq(sq), val(val) {}
};

class MovePicker {
	const Position& pos;

	Square ttMove;
	int stage;
	ExtMove moves[MAX_MOVE];
	ExtMove* cur, *end;

	void genMove();
	void genPseudoVCFMove();

public:

	MovePicker(PickerMod mod, const Position &pos, Square ttMove);

	Square nextMove();
};

std::vector<Square> genRootMove(const Position& pos);

#endif
