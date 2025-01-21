#ifndef SEARCH
#define SEARCH

#include "common.h"
#include "position.h"
#include "move.h"
#include "time.h"

namespace Search {
	constexpr int threadNum = 1;

	constexpr int mate_in(int ply) { return WIN_MAX - ply; }

	constexpr int mated_in(int ply) { return -WIN_MAX + ply; }

	enum NType {
		NonPV,
		PV,
		Root
	};

	struct Stack {
		int ply;
		Sqare* pv;
		Sqare excludedMove;
		int eval;
		bool ttpv;
		FType moveFT[2];
		Sqare killer[2];
	};

	struct RootMove {
		Sqare sq = NULLSQARE;
		int val = -VAL_INF;
		int lastVal = -VAL_INF;
		int avg = -VAL_INF;
		int meanSquared = -VAL_INF * VAL_INF;
		std::vector<Sqare> pv;
		RootMove(Sqare sq, int val, int lastVal) :sq(sq), val(val), lastVal(lastVal) {}
	};

	struct Worker {
		Timer timer;
		Position rootPos;
		StateInfo rootSt;
		Stack ss;

		std::vector<RootMove> rootMoves;
		int pvIdx = 0;

		void iterative_deepening();
		int search(NType NT, Position& pos, Stack* ss, int alpha, int beta, float dep, bool cutNode);
		int VCFSearch(NType NT, Position& pos, Stack* ss, int alpha, int beta, float dep);

		Worker(Position& pos) : rootPos(pos) {}
	};

	std::tuple<std::vector<Sqare>, int> search(Position& pos); // return best PV sequence
}

#endif


