#ifndef MOVE
#define MOVE

#include "../common/common.h"
#include "../game/board.h"

using MainHist = std::array<std::array<uint64_t, BOARD_SIZE>, 2>;
using CounterMove = std::array<std::array<Pos, BOARD_SIZE>, 2>;

enum PickerMod {
	P_main,
	P_VCF
};

struct Move {
	Pos pos = NULLPOS;
	float val = 0;
	Move() = default;
	Move(Pos pos, int val) :pos(pos), val(val) {}
	bool operator < (const Move& mv)const {
		return val < mv.val;
	}
};

class MovePicker {
	const Board& bd;
	const MainHist& mainHist;

	Pos ttMove;
	Move refutations[3];
	int stage;
	Move moves[MAX_MOVE + 1];
	Move* cur, *end;

	void genThreatMove();
	void genQuietMove();
	void genPseudoVCFMove();

public:
	MovePicker(PickerMod mod, const Board &bd, const MainHist& mainHist, Pos ttMove);

	MovePicker(PickerMod mod, const Board& bd, const MainHist& mainHist, 
		Pos ttMove, Pos killer1, Pos killer2, Pos counterMove);

	Pos nextMove(bool skipQuiets = false);
};

std::vector<Move> genRootMove(const Board& bd);

#endif
