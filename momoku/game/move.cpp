#include "move.h"
#include "../test/test.h"
#include "../search/search.h"
#include "../common/misc.h"

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

MovePicker::MovePicker(PickerMod mod, const Board& bd, const MainHist& mainHist, Pos ttMove) :
	bd(bd),
	ttMove(ttMove),
	mainHist(mainHist),
	cur(moves), end(moves) {
	stage = (mod == P_main ? M_main_tt : M_VCF_tt) + (ttMove == NULLPOS);
	added[ttMove] = true;
}

MovePicker::MovePicker(PickerMod mod, const Board& bd, const MainHist& mainHist, 
	Pos ttMove, Pos killer1, Pos killer2, Pos counterMove) :
	bd(bd),
	ttMove(ttMove), killer1(killer1), killer2(killer2), counterMove(counterMove),
	mainHist(mainHist),
	cur(moves), end(moves) {
	stage = (mod == P_main ? M_main_tt : M_VCF_tt) + (ttMove == NULLPOS);
	added[ttMove] = true;
}

Pos MovePicker::nextMove(bool skipQuiets) {
	switch (stage) {
	
	case M_main_tt:
	case M_VCF_tt:

		stage++;

		if (bd[ttMove] == Empty) return ttMove;
		
		[[fallthrough]];

	case M_threat_init:
		
		genThreatMove();
		stage++;
		[[fallthrough]];

	case M_threat:

		// find best move and sort
		if (cur < end) {
			std::swap(*std::max_element(cur, end), *cur);
			return (*cur++).pos;
		}

		stage++;
		[[fallthrough]];

	case M_refutation_init:

		if (bd[killer1] == Empty && !added[killer1]) {  //killers might be ilegal
			(*end++).pos = killer1;
			added[killer1] = true;
		}
		if (bd[killer2] == Empty && !added[killer2]) {
			(*end++).pos = killer2;
			added[killer2] = true;
		}
		if (bd[counterMove] == Empty && !added[counterMove]) {
			(*end++).pos = counterMove;
			added[counterMove] = true;
		}

		stage++; 
		[[fallthrough]];

	case M_refutation:

		if (cur < end) return (*cur++).pos;

		stage++;
		[[fallthrough]];

	case M_quiet_init:

		if (skipQuiets) 
			return NULLPOS;
		
		genQuietMove();

		//insertion sort
		for (Move* i = cur; i < end; i++) {

			Move* j = i;
			Move tmp = *i;
			for (; j >= cur + 1 && *(j - 1) < tmp; j--) 
				std::swap(*j, *(j - 1));
				
			*j = tmp;
		}
				
		
		stage++;
		[[fallthrough]];

	case M_quiet:

		if (cur < end) return (*cur++).pos;

		return NULLPOS;

	case M_VCF_init:

		genPseudoVCFMove();
		stage++;
		[[fallthrough]];

	case M_VCF:

		//select best move and sort
		if (cur < end) {
			std::swap(*std::max_element(cur, end), *cur);
			return (*cur++).pos;
		}

		return NULLPOS;
	}

	assert(0);
	return 0;
}

void MovePicker::genThreatMove() {

	Piece self = bd.self(), oppo = ~self;

	for (int i = bd.candRange().x1; i <= bd.candRange().x2; i++) {
		for (int j = bd.candRange().y1; j <= bd.candRange().y2; j++) {

			Pos pos(i, j);

			if (bd[pos] != Empty || !bd.cand(pos) || added[pos]) continue;   

			if ((bd.cntFT(TH4, oppo) && (bd.type(self, pos) >= T4 || bd.type(oppo, pos) >= T4)) ||   //when oppo has dual 4 attack
				(bd.type(self, pos) >= TH3 || bd.type(oppo, pos) >= TH3)) {

				int val = 2 * bd.value(self, pos) + bd.value(oppo, pos);

				*end++ = Move(pos, val);
				added[pos] = true;
			}
		}
	}
}

void MovePicker::genQuietMove() {

	Piece self = bd.self(), oppo = ~self;

	for (int i = bd.candRange().x1; i <= bd.candRange().x2; i++) {
		for (int j = bd.candRange().y1; j <= bd.candRange().y2; j++) {

			Pos pos(i, j);

			if (bd[pos] != Empty || !bd.cand(pos) || added[pos]) continue;   

			int val = 2 * bd.value(self, pos) + bd.value(oppo, pos);

			val += mainHist[self][pos];

			*end++ = Move(pos, val);
			added[pos] = true;
		}
	}
}

void MovePicker::genPseudoVCFMove() {

	Piece self = bd.self(), oppo = ~self;

	Pos lastPos = bd.lastMove(2);
	constexpr int len = arrLen(EXPAND_L4);
	for (int i = 0; i < len; i++) {

		Pos pos = lastPos + EXPAND_L4[i];

		if (bd[pos] != Empty || added[pos]) continue;   

		if (bd.type(self, pos) >= T4) {

			int val = 2 * bd.value(self, pos) + bd.value(oppo, pos);

			*end++ = Move(pos, val);
			added[pos] = true;
		}
	}
}

std::vector<Pos> genRootMove(const Board& bd) {

	Piece self = bd.self(), oppo = ~self;

	std::vector<Pos> moves;

	for (int i = bd.candRange().x1; i <= bd.candRange().x2; i++) {
		for (int j = bd.candRange().y1; j <= bd.candRange().y2; j++) {
			Pos pos(i, j);
			if (bd[pos] == Empty && bd.cand(pos))
				moves.push_back(pos);
		}
	}

	return moves;
}
