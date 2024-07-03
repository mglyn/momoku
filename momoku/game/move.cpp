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
}

MovePicker::MovePicker(PickerMod mod, const Board& bd, const MainHist& mainHist, 
	Pos ttMove, Pos killer1, Pos killer2, Pos counterMove) :
	bd(bd),
	ttMove(ttMove),
	mainHist(mainHist),
	cur(moves), end(moves)
{
	stage = (mod == P_main ? M_main_tt : M_VCF_tt) + (ttMove == NULLPOS);
	refutations[0].pos = killer1;
	refutations[1].pos = killer2;
	refutations[2].pos = counterMove;
}

Pos MovePicker::nextMove(bool skipQuiets) {
	switch (stage) {
	
	case M_main_tt:
	case M_VCF_tt:

		stage++;
		return ttMove;

	case M_threat_init:
		
		genThreatMove();
		stage++;
		[[fallthrough]];

	case M_threat:

		// find best move and sort
		for (; cur < end;) {
			std::swap(*std::max_element(cur, end), *cur);
			return (*cur++).pos;
		}

		stage++;
		[[fallthrough]];

	case M_refutation_init:

		cur = refutations;
		end = std::unique(refutations, refutations + 3, [](const Move& mv1, const Move& mv2) {return mv1.pos == mv2.pos; });

		stage++; 
		[[fallthrough]];

	case M_refutation:

		for (; cur < end; cur++) {
			if ((*cur).pos == NULLPOS || bd[(*cur).pos] != Empty) continue;
			if (bd.type(P1, (*cur).pos) == TNone && bd.type(P2, (*cur).pos) == TNone) //skip the move already tried 
				return (*cur++).pos;
		}

		cur = moves, end = moves;

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

		for (; cur < end; cur++) {
			if ((*cur).pos != refutations[0].pos && 
				(*cur).pos != refutations[1].pos && 
				(*cur).pos != refutations[2].pos) //skip the move already tried 
				return (*cur++).pos;
		}

		return NULLPOS;




	case M_VCF_init:

		genPseudoVCFMove();
		stage++;
		[[fallthrough]];

	case M_VCF:

		//select best move and sort
		for (; cur < end; cur++) {
			std::swap(*std::max_element(cur, end), *cur);

			if ((*cur).pos != ttMove) return (*cur++).pos;
		}

		return NULLPOS;
	}

	assert(0);
	return 0;
}

void MovePicker::genThreatMove() {
	Piece self = bd.self(), oppo = ~self;

	// move scoring rule
	auto score = [this, self, oppo](Pos pos) {

		//Evaluate value through the formed pattern
		int ret = 2 * bd.value(self, pos) + bd.value(oppo, pos);

		//history huristic
		//ret += mainHist[self][1];

		return ret;
		};

	for (int i = bd.candRange().x1; i <= bd.candRange().x2; i++) {
		for (int j = bd.candRange().y1; j <= bd.candRange().y2; j++) {

			Pos pos(i, j);

			if (bd[pos] != Empty || !bd.cand(pos) || pos == ttMove)continue;   //skip ttMove

			if ((bd.cntFT(TH4, oppo) && (bd.type(self, pos) >= T4 || bd.type(oppo, pos) >= T4)) ||   //when oppo has dual 4 attack
				(bd.type(self, pos) >= TH3 || bd.type(oppo, pos) >= TH3)) 
				*end++ = Move(pos, score(pos)); 
		}
	}
}

void MovePicker::genQuietMove() {
	Piece self = bd.self(), oppo = ~self;

	// move scoring rule
	auto score = [this, self, oppo](Pos pos) {

		//Evaluate value through the formed pattern
		int ret = bd.value(self, pos) + bd.value(oppo, pos);

		//history huristic
		ret += mainHist[self][pos];

		return ret;
		};

	//int valMax = -VAL_INF, valMin = VAL_INF;

	for (int i = bd.candRange().x1; i <= bd.candRange().x2; i++) {
		for (int j = bd.candRange().y1; j <= bd.candRange().y2; j++) {

			Pos pos(i, j);

			if (bd[pos] != Empty || !bd.cand(pos) || pos == ttMove)continue;   //skip ttMove

			if (bd.type(self, pos) == TNone && bd.type(oppo, pos) == TNone) {
				int val = score(pos);
				*end++ = Move(pos, val);
				//valMax = std::max(valMax, val);
				//valMin = std::min(valMin, val);
			}
		}
	}

	//float len = std::max(valMax - valMin, 1);
	//for (Move* i = cur; i < end; i++) {                     
	//	
	//	float val = (float)((*i).val - valMin) / len;  //归一化

	//	int idx = 1 << int(5 * val); // 桶id

	//	int emptyPos = 0;
	//	while (bucket[emptyPos][idx] && emptyPos < 16)
	//		emptyPos++;
	//	bucket[emptyPos][idx] = (*i).pos;
	//}
}

void MovePicker::genPseudoVCFMove() {
	Piece self = bd.self(), oppo = ~self;

	// move scoring rule
	auto score = [this, self, oppo](Pos pos) {

		//Evaluate value through the formed pattern
		int ret = bd.value(self, pos) + bd.value(oppo, pos);

		//history huristic
		//ret += mainHist[self][1];

		return ret;
		};

	Pos lastPos = bd.lastMove(1);
	constexpr int len = arrLen(EXPAND_L4);
	for (int i = 0; i < len; i++) {

		Pos pos = lastPos + EXPAND_L4[i];

		if (bd[pos] != Empty || pos == ttMove)continue;   //skip ttMove

		if (bd.type(self, pos) >= T4) 
			*end++ = Move(pos, score(pos));
	}
}

std::vector<Move> genRootMove(const Board& bd) {
	Piece self = bd.self(), oppo = ~self;

	// move scoring rule
	auto score = [&self, &oppo, &bd](Pos pos) {

		//Evaluate value through the formed pattern
		int ret = bd.value(self, pos) + bd.value(oppo, pos);

		return ret;
		};

	std::vector<Move> moves;

	for (int i = bd.candRange().x1; i <= bd.candRange().x2; i++) {
		for (int j = bd.candRange().y1; j <= bd.candRange().y2; j++) {
			Pos pos(i, j);
			if (bd[pos] == Empty && bd.cand(pos))
				moves.emplace_back(pos, 2 * bd.value(self, pos) + bd.value(oppo, pos));
		}
	}

	return moves;
}
