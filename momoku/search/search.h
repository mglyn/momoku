#ifndef SEARCH
#define SEARCH

#include "../common/common.h"
#include "../game/board.h"
#include "../game/move.h"
#include "time.h"

namespace Search {

	constexpr int MAX_PLY = 128;
	constexpr int END_DEP = 127;
	constexpr int threadNum = 1;

	constexpr int win(int ply) { return WIN_MAX - ply; }

	constexpr int loss(int ply) { return -WIN_MAX + ply; }

	enum NType {
		NonPV,
		PV,
		Root
	};

	struct Stack {
		int ply;
		Pos* pv;
		Pos excludedMove;
		int eval;
		bool ttpv;
		FType moveFT[2];
		Pos killer[2];
	};

	struct RootMove {
		Pos pos = NULLPOS;
		int val = -VAL_INF;
		int lastVal = -VAL_INF;
		int avg = -VAL_INF;
		std::vector<Pos> pv;
		RootMove(Pos pos, int val, int lastVal) :pos(pos), val(val), lastVal(lastVal) {}
	};

	struct Worker {
		Timer timer;
		Board bd;
		Stack ss;

		std::vector<RootMove> rootMoves;
		int pvIdx = 0;

		MainHist mainHist = {};
		CounterMove counterMove = {};

		void iterative_deepening();
		int search(NType NT, Stack* ss, int alpha, int beta, int dep, bool cutNode);
		int VCFSearch(NType NT, Stack* ss, int alpha, int beta, int dep);
		int quickEnd(Stack* ss);
		void updateStats(Stack* ss, int dep, Piece self, Pos move, Pos lastMove);

		Worker(Board& bd) : bd(bd) {}
	};

	std::vector<Pos> search(Board& bd); // return best PV sequence
}

#endif


