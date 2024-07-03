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
	moves(new Move[bd.length() * bd.length()]),
	cur(moves.get()),
	end(cur)
{
	stage = (mod == P_main ? M_main_tt : M_VCF_tt) + (ttMove == NULLPOS);
}

MovePicker::MovePicker(PickerMod mod, const Board& bd, const MainHist& mainHist, 
	Pos ttMove, Pos killer1, Pos killer2, Pos counterMove) :
	bd(bd),
	ttMove(ttMove), killer1(killer1), killer2(killer2), counterMove(counterMove),
	mainHist(mainHist),
	moves(new Move[bd.length() * bd.length()]),
	cur(moves.get()),
	end(cur)
{
	stage = (mod == P_main ? M_main_tt : M_VCF_tt) + (ttMove == NULLPOS);
}

Pos MovePicker::nextMove(bool skipQuiets) {
	switch (stage) {
	
	case M_main_tt:
	case M_VCF_tt:

		stage++;
		return ttMove;

	case M_threat_init:

		genMove(G_threat);

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

	{
		auto ok = [&](Pos pos) { return bd[pos] == Empty && bd.type(P1, pos) == TNone && bd.type(P2, pos) == TNone && pos != ttMove; };

		if (ok(killer1)) *end++ = Move(killer1, 0);
		if (ok(killer2)) *end++ = Move(killer2, 0);
		if (ok(counterMove)) *end++ = Move(counterMove, 0);
	}

		stage++; 
		[[fallthrough]];

	case M_refutation:

		if (cur < end) 
			return (*cur++).pos;

		stage++;
		[[fallthrough]];

	case M_quiet_init:

		if (skipQuiets) 
			return NULLPOS;
		
		genMove(G_quiet);

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

		if (cur < end) 
			return (*cur++).pos;
		
		return NULLPOS;

	case M_VCF_init:

		genMove(G_vcf);
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

void MovePicker::genMove(GenType type) {
	Piece self = bd.self(), oppo = ~self;

	switch (type) {
	case G_threat:

		for (int i = bd.candRange().x1; i <= bd.candRange().x2; i++) {
			for (int j = bd.candRange().y1; j <= bd.candRange().y2; j++) {

				Pos pos(i, j);

				if (bd[pos] != Empty || !bd.cand(pos)) continue;   
				if (pos == ttMove) continue;                    //skip ttMove

				if ((bd.cntFT(TH4, oppo) && (bd.type(self, pos) >= T4 || bd.type(oppo, pos) >= T4)) ||   //when oppo has dual 4 attack
					(bd.type(self, pos) >= TH3 || bd.type(oppo, pos) >= TH3)) {

					int score = 2 * bd.value(self, pos) + bd.value(oppo, pos);

					*end++ = Move(pos, score);
				}
			}
		}

		break;
	
	case G_quiet:

		for (int i = bd.candRange().x1; i <= bd.candRange().x2; i++) {
			for (int j = bd.candRange().y1; j <= bd.candRange().y2; j++) {

				Pos pos(i, j);

				if (bd[pos] != Empty || !bd.cand(pos))continue;   
				if (pos == ttMove) continue;                    //skip ttMove
				if (pos == killer1 || pos == killer2 || pos == counterMove) continue; //skip refutations

				if (bd.type(self, pos) == TNone && bd.type(oppo, pos) == TNone) {

					int score = bd.value(self, pos) + bd.value(oppo, pos);

					score += mainHist[self][pos];

					*end++ = Move(pos, score);
				}
			}
		}

		break;

	case G_vcf:

		Pos lastPos = bd.lastMove(1);

		for (int i = 0; i < 8; i++) {
			for (int j = 1; j <= 4; j++) {

				Pos pos = lastPos + j * D8[i];

				if (bd[pos] != Empty)continue;   //skip ttMove
				if (bd[pos] == oppo)break;
				if (pos == ttMove) continue;

				if (bd.type(self, pos) >= T4) {

					int score = 2 * bd.value(self, pos) + bd.value(oppo, pos);

					*end++ = Move(pos, score);
				}
			}
		}

		break;
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
