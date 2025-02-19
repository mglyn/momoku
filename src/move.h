#ifndef MOVE
#define MOVE

#include "common.h"
#include "position.h"
#include "history.h"

enum PickerMod {
	P_main,
	P_VCF,
};

struct ExtMove {
	Square sq = Square::NONE;
	int16_t val = 0;
	ExtMove() = default;
	ExtMove(Square sq) :sq(sq), val(0) {};
	ExtMove(Square sq, int val) :sq(sq), val(val) {}
};

class MovePicker {
	const Position& pos;
	const MainHistory* mainHistory;
	const CounterMoveHistory* counterMoveHistory;

	Square ttMove;
	int stage;
	ExtMove moves[MAX_MOVE];
	ExtMove* cur, *end;

	void genMove();
	void extraScore();
	void genQsearchMove();
	void genVCFMove();

public:

	MovePicker(PickerMod mod,
		const Position& pos,
		const MainHistory* mh,
		const CounterMoveHistory* ch,
		Square ttMove);

	Square nextMove();
};

std::vector<Square> genRootMove(const Position& pos);

#endif
