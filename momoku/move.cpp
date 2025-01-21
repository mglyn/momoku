#include "move.h"
#include "search.h"
#include "misc.h"
#include "test.h"

enum MoveStage {
	M_main_tt,
	M_threat_init,
	M_threat,
	M_refutation_init,
	M_refutation,
	M_quiet_init,
	M_quiet,

	M_VCF_tt,
	M_VCF_init,
	M_VCF,

	M_all
};

MovePicker::MovePicker(PickerMod mod, const Position& pos, 
	Sqare ttMove) :
	pos(pos), 
	st(pos.stateInfo()),
	ttMove(ttMove),
	cur(moves), end(moves) {
	stage = (mod == P_main ? M_main_tt : M_VCF_tt) + 
		(ttMove == NULLSQARE || !pos.legal(ttMove));
}

MovePicker::MovePicker(PickerMod mod, const Position& pos, 
	Sqare ttMove, Sqare killer1, Sqare killer2) :
	pos(pos),
	st(pos.stateInfo()),
	ttMove(ttMove), killer1(killer1), killer2(killer2),
	cur(moves), end(moves) {
	stage = (mod == P_main ? M_main_tt : M_VCF_tt) + 
		(ttMove == NULLSQARE || !pos.legal(ttMove));
}

Sqare MovePicker::nextMove(bool skipQuiets) {
	switch (stage) {

	case M_main_tt:
	case M_VCF_tt:

		stage++;
		return ttMove;

	case M_threat_init:

		genMove();

		stage++;
		[[fallthrough]];

	case M_threat:

		// find best move and sort
		if (cur < end) {

			Piece us = pos.side_to_move();
			auto cmp = [this, us](const Move& a, const Move& b) {
				bool threatA = pos.type(us, a.sq) >= TH3;
				bool threatB = pos.type(us, b.sq) >= TH3;
				return threatA == threatB ? a.val < b.val : threatA < threatB;
				};

			std::swap(*std::max_element(cur, end, cmp), *cur);

			if (pos.type(us, cur->sq) >= TH3)
				return (*cur++).sq;
		}

		stage++;
		[[fallthrough]];

	case M_refutation_init:         //to be tested

		/*if (pos[killer1] == Empty && killer1 != ttMove && pos.type(killer1) == TNone)
			*end++ = Move(killer1, 0);
		if (pos[killer2] == Empty && killer2 != ttMove && pos.type(killer2) == TNone)
			*end++ = Move(killer2, 0);
		if (pos[counter] == Empty && counter != ttMove &&
			counter != killer1 && counter != killer2 && pos.type(counter) == TNone)
			*end++ = Move(counter, 0);*/

		stage++;
		[[fallthrough]];

	case M_refutation:

		if (cur < end)
			return (*cur++).sq;

		stage++;
		[[fallthrough]];

	case M_quiet_init:

		if (skipQuiets)
			return NULLSQARE;

		//insertion sort
		for (Move* i = cur; i < end; i++) {

			Move* j = i;
			for (; j >= cur + 1 && (j - 1)->val < i->val; j--)
				std::swap(*j, *(j - 1));
			*j = *i;
		}

		stage++;
		[[fallthrough]];

	case M_quiet:

		if (cur < end)
			return (*cur++).sq;

		return NULLSQARE;

	case M_VCF_init:

		genPseudoVCFMove();

		stage++;
		[[fallthrough]];

	case M_VCF:

		//select best move and sort
		if (cur < end) {
			auto cmp = [](const Move& a, const Move& b) {
				return  a.val < b.val;
				};
			std::swap(*std::max_element(cur, end, cmp), *cur);
			return (*cur++).sq;
		}

		return NULLSQARE;
	}

	assert(0);
	return 0;
}

void MovePicker::genMove() {
	Piece self = pos.side_to_move(), oppo = ~self;
	for (int i = st.range.x1; i <= st.range.x2; i++) {
		for (int j = st.range.y1; j <= st.range.y2; j++) {

			Sqare sq(i, j);

			if (pos[sq] != Empty || !pos.cand(sq) || sq == ttMove)continue;   //skip ttMove

			if (st.cntT[TH4][oppo] && pos.type(self, sq) < T4 && pos.type(oppo, sq) < T4) continue; //排除必输走法

			int val = 2 * pos.value(self, sq) + pos.value(oppo, sq); //.............
			*end++ = Move(sq, val);
			
		}
	}
}

void MovePicker::genPseudoVCFMove() {

	Piece self = pos.side_to_move(), oppo = ~self;

	Sqare lastMove = st.prev->move;

	constexpr int len = arrLen(EXPAND_L4);
	for (int i = 0; i < len; i++) {

		Sqare sq = lastMove + EXPAND_L4[i];

		if (pos[sq] != Empty || sq == ttMove)continue;   //skip ttMove

		if (pos.type(self, sq) >= T4) {

			int val = pos.value(self, sq) + pos.value(oppo, sq);

			*end++ = Move(sq, val);
		}

	}
}

std::vector<Sqare> genRootMove(const Position& pos) { //?????
	Piece us = pos.side_to_move(), op = ~us;
	const StateInfo& st = pos.stateInfo();

	std::vector<Sqare> moves;

	for (int i = st.range.x1; i <= st.range.x2; i++) {
		for (int j = st.range.y1; j <= st.range.y2; j++) {
			Sqare sq(i, j);
			if (pos[sq] == Empty && pos.cand(sq))
				moves.emplace_back(sq);
		}
	}

	return moves;
}
