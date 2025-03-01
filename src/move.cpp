#include "move.h"
#include "search.h"
#include "misc.h"

template <typename Comparator>
void insertionSort(ExtMove* cur, ExtMove* end, Comparator comp) {

	for (ExtMove* sortedEnd = cur, *p = cur + 1; p < end; ++p) {
		ExtMove tmp = *p, * q;
		*p = *++sortedEnd;
		for (q = sortedEnd; q != cur && comp(*(q - 1), tmp); --q)
			*q = *(q - 1);
		*q = tmp;
	}
}

enum MoveStage {
	M_main_tt,
	M_main_init,
	M_main,

	M_VCF_tt,
	M_VCF_init,
	M_VCF,
};

MovePicker::MovePicker(
	PickerMod mod,
	const Position& pos,
	const MainHistory* mh,
	const CounterMoveHistory* ch,
	Square ttMove) :
	pos(pos),
	mainHistory(mh),
	counterMoveHistory(ch),
	ttMove(ttMove),
	cur(moves), end(moves) {

	stage = mod == P_main ? M_main_tt : M_VCF_tt;
	bool ttValid = ttMove.is_ok() && pos[ttMove] == Empty;
	stage += !ttValid;
}

Square MovePicker::nextMove() {
	switch (stage) {

	case M_main_tt:
	case M_VCF_tt:

		stage++;

		return ttMove;

	case M_main_init:

		genMove();
		extraScore();
		auto cmp = [](const ExtMove& a, const ExtMove& b) {
			return a.val < b.val;
			};
		//insertionSort(cur, end, cmp);

		stage++;
		[[fallthrough]];

	case M_main:

		if (cur < end) {
			std::swap(*std::max_element(cur, end, cmp), *cur);
			return (*cur++).sq;
		}
		/*if (cur < end)
			return (*cur++).sq;*/

		return Square::NONE;

	case M_VCF_init:

		genVCFMove();

		stage++;
		[[fallthrough]];

	case M_VCF:

		//select best move and sort
		if (cur < end) {
			std::swap(*std::max_element(cur, end, cmp), *cur);
			return (*cur++).sq;
		}

		return Square::NONE;
	}

	assert(0);
	return Square::NONE;
}

void MovePicker::genMove() {

	Piece us = pos.side_to_move(), op = ~us;
	const StateInfo& st = pos.st();

	[[unlikely]] if (pos.cntMove() == 0) { //first move
		if (Square sq(pos.gameSize() / 2, pos.gameSize() / 2); sq != ttMove)
			*end++ = ExtMove(sq);
	}

	else if (st.cntT[T5][us]) {
		for (int i = st.range.x1; i <= st.range.x2; i++) {
			for (int j = st.range.y1; j <= st.range.y2; j++) {
				if (Square sq(i, j); pos.type(us, sq) == T5 && pos[sq] == Empty && sq != ttMove)
					*end++ = ExtMove(sq);
			}
		}
	}

	else if (st.cntT[T5][op]) {
		if (pos.type(op, st.T5Square) == T5 && pos[st.T5Square] == Empty && st.T5Square != ttMove)
			*end++ = ExtMove(st.T5Square);
		else {
			for (int i = st.range.x1; i <= st.range.x2; i++) {
				for (int j = st.range.y1; j <= st.range.y2; j++) {
					if (Square sq(i, j); pos.type(op, sq) == T5 && pos[sq] == Empty && st.T5Square != ttMove) {
						*end++ = ExtMove(st.T5Square);
					}
				}
			}
		}
	}

	else if (st.cntT[TH4][us]) {
		for (int i = st.range.x1; i <= st.range.x2; i++) {
			for (int j = st.range.y1; j <= st.range.y2; j++) {
				if (Square sq(i, j); pos.type(us, sq) == TH4 && pos[sq] == Empty && sq != ttMove)
					*end++ = ExtMove(sq);
			}
		}
	}

	else if (st.cntT[TH4][op]) {
		for (int i = st.range.x1; i <= st.range.x2; i++) {
			for (int j = st.range.y1; j <= st.range.y2; j++) {
				if (Square sq(i, j); (pos.type(us, sq) >= T4 || pos.type(op, sq) >= T4) && pos[sq] == Empty && sq != ttMove)
					*end++ = ExtMove(sq, pos.value(us, sq) + pos.value(op, sq));
			}
		}
	}

	else {
		for (int i = st.range.x1; i <= st.range.x2; i++) {
			for (int j = st.range.y1; j <= st.range.y2; j++) {
				if (Square sq(i, j); pos.cand(sq) && pos[sq] == Empty && sq != ttMove)
					*end++ = ExtMove(sq, pos.value(us, sq) + pos.value(op, sq));
			}
		}
	}
}

void MovePicker::genVCFMove() {

	Piece us = pos.side_to_move(), op = ~us;
	const StateInfo& prevst = pos.prevst();
	const StateInfo& st = pos.st();

	Square vlastMove = prevst.move;
	Square lastMove = st.move;

	[[unlikely]] if (lastMove == Square::NONE)
		return;

	else if (st.cntT[T5][us]) {
		for (int i = st.range.x1; i <= st.range.x2; i++) {
			for (int j = st.range.y1; j <= st.range.y2; j++) {
				if (Square sq(i, j); pos.type(us, sq) == T5 && pos[sq] == Empty && sq != ttMove)
					*end++ = ExtMove(sq);
			}
		}
	}

	else if (st.cntT[T5][op]) {
		assert(pos[st.T5Square] == Empty && pos.type(op, st.T5Square) == T5);
		if (st.T5Square != ttMove)
			*end++ = ExtMove(st.T5Square);
	}

	else {
		constexpr int len = arrLen(EXPAND_L4);
		for (int i = 0; i < len; i++) {
			if (Square sq{ lastMove + EXPAND_L4[i] }; pos.type(us, sq) >= T4 && pos[sq] == Empty && sq != ttMove ) {
				*end++ = ExtMove(sq, pos.value(us, sq) + pos.value(op, sq));
			}
		}
	}
}

void MovePicker::extraScore() {

	Piece us = pos.side_to_move(), op = ~us;
	const StateInfo& st = pos.st();

	for (ExtMove* m = cur; m != end; m++) {

		if (mainHistory) {
			if (pos.type(us, m->sq) >= TH3)
				m->val += (*mainHistory)[us][m->sq][HIST_THREAT] / 128;
			else
				m->val += (*mainHistory)[us][m->sq][HIST_QUIET] / 256;
		}

		if (counterMoveHistory && !(st.cntT[TH4][op] + st.cntT[T4H3][op] + st.cntT[TH4][op])) {
			if (Square lastMove = pos.prevst().move; lastMove) {
				const int CounterMoveBonus = 20;
				auto [counterMove, counterMoveP4] = (*counterMoveHistory)[op][lastMove.moveIndex()];

				if (counterMove == m->sq && counterMoveP4 <= pos.type(us, m->sq))
					m->val += CounterMoveBonus;
			}
		}
	}
}



std::vector<Square> genRootMove(const Position& pos) { //?????

	std::vector<Square> moves;
	Piece us = pos.side_to_move(), op = ~us;
	const StateInfo& st = pos.st();

	if (pos.cntMove() == 0) { //first move
		moves.push_back(Square(pos.gameSize() / 2, pos.gameSize() / 2));
	}

	else if (st.cntT[T5][us]) {
		for (int i = st.range.x1; i <= st.range.x2; i++) {
			for (int j = st.range.y1; j <= st.range.y2; j++) {
				if (Square sq(i, j); pos.type(us, sq) == T5 && pos[sq] == Empty)
					moves.push_back(sq);
			}
		}
	}

	else if (st.cntT[T5][op]) {
		if (pos[st.T5Square] == Empty && pos.type(op, st.T5Square) == T5)
			moves.push_back(st.T5Square);
		else {
			for (int i = st.range.x1; i <= st.range.x2; i++) {
				for (int j = st.range.y1; j <= st.range.y2; j++) {
					if (Square sq(i, j); pos.type(op, sq) == T5 && pos[sq] == Empty)
						moves.push_back(sq);
				}
			}
		}
	}

	else if (st.cntT[TH4][us]) {
		for (int i = st.range.x1; i <= st.range.x2; i++) {
			for (int j = st.range.y1; j <= st.range.y2; j++) {
				if (Square sq(i, j); pos.type(us, sq) == TH4 && pos[sq] == Empty)
					moves.push_back(sq);
			}
		}
	}

	else if (st.cntT[TH4][op]) {
		for (int i = st.range.x1; i <= st.range.x2; i++) {
			for (int j = st.range.y1; j <= st.range.y2; j++) {
				if (Square sq(i, j); (pos.type(us, sq) >= T4 || pos.type(op, sq) >= T4) && pos[sq] == Empty)
					moves.push_back(sq);
			}
		}
	}

	else {
		for (int i = st.range.x1; i <= st.range.x2; i++) {
			for (int j = st.range.y1; j <= st.range.y2; j++) {
				if (Square sq(i, j); pos.cand(sq) && pos[sq] == Empty)
					moves.push_back(sq);
			}
		}
	}

	return moves;
}