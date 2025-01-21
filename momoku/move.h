#ifndef MOVE
#define MOVE

#include "common.h"
#include "position.h"

enum PickerMod {
	P_main,
	P_VCF
};

struct Move {
	Sqare sq = NULLSQARE;
	int16_t val = 0;
	Move() = default;
	Move(Sqare sq, int val) :sq(sq), val(val) {}
};

class MovePicker {
	const Position& pos;
	const StateInfo& st;

	Sqare ttMove, killer1, killer2, counter;
	int stage;
	Move moves[MAX_MOVE];
	Move* cur, *end;

	void genMove();
	void genPseudoVCFMove();

public:
	MovePicker(PickerMod mod, const Position &pos, 
		Sqare ttMove);

	MovePicker(PickerMod mod, const Position& pos, 
		Sqare ttMove, Sqare killer1, Sqare killer2);

	Sqare nextMove(bool skipQuiets = false);
};

std::vector<Sqare> genRootMove(const Position& pos);

#endif
