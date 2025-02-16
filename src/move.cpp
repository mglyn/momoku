﻿#include "move.h"
#include "search.h"
#include "misc.h"

static void insertion_sort(ExtMove* cur, ExtMove* end) {

	for (ExtMove* sortedEnd = cur, *p = cur + 1; p < end; ++p) {
		ExtMove tmp = *p, * q;
		*p = *++sortedEnd;
		for (q = sortedEnd; q != cur && (q - 1)->val < tmp.val; --q)
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

MovePicker::MovePicker(PickerMod mod, const Position& pos,
	Square ttMove) :
	pos(pos),
	ttMove(ttMove),
	cur(moves), end(moves) {

	stage = mod == P_main ? M_main_tt : M_VCF_tt;
	bool ttValid = ttMove.is_ok() && pos[ttMove] == Empty && 
		(mod == P_main || pos.type(pos.side_to_move(), ttMove) < T4);
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
		insertion_sort(cur, end);

		stage++;
		[[fallthrough]];

	case M_main:

		if (cur < end)
			return (*cur++).sq;

		return Square::NONE;

	case M_VCF_init:

		genPseudoVCFMove();
		insertion_sort(cur, end);

		stage++;
		[[fallthrough]];

	case M_VCF:

		//select best move and sort
		if (cur < end) {
			auto cmp = [](const ExtMove& a, const ExtMove& b) {
				return  a.val < b.val;
				};
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
		Square sq(pos.gameSize() / 2, pos.gameSize() / 2);
		if (sq != ttMove)
			*end++ = ExtMove(sq, 0);
		return;
	}

	for (int i = st.range.x1; i <= st.range.x2; i++) {
		for (int j = st.range.y1; j <= st.range.y2; j++) {

			Square sq(i, j);

			if (pos[sq] != Empty || !pos.cand(sq) || sq == ttMove)
				continue;   //skip ttMove

			if (st.cntT[TH4][op] && pos.type(us, sq) < T4 && pos.type(op, sq) < T4) 
				continue;   //排除必输走法

			int val = pos.value(us, sq) + pos.value(op, sq); //.............
			*end++ = ExtMove(sq, val);
		}
	}
}

void MovePicker::genPseudoVCFMove() {

	Piece us = pos.side_to_move(), op = ~us;
	const StateInfo& prevst = pos.prevst();
	const StateInfo& st = pos.st();

	Square lastMove = prevst.move;
	[[unlikely]] if (lastMove == Square::NONE) 
		return;

	if (st.cntT[T4][us] + st.cntT[T4H3][us] + st.cntT[TH4][us] == 0) return;

	constexpr int len = arrLen(EXPAND_L4);
	for (int i = 0; i < len; i++) {

		Square sq{ lastMove + EXPAND_L4[i] };

		if (pos[sq] != Empty || sq == ttMove)continue;   //skip ttMove

		if (pos.type(us, sq) >= T4) {

			int val = pos.value(us, sq) + pos.value(op, sq);

			*end++ = ExtMove(sq, val);
		}
	}
}

std::vector<Square> genRootMove(const Position& pos) { //?????

	std::vector<Square> moves;

	[[unlikely]] if (pos.cntMove() == 0) { //first move
		moves.emplace_back(Square(pos.gameSize() / 2, pos.gameSize() / 2));
		return moves;
	}

	Piece us = pos.side_to_move(), op = ~us;
	const StateInfo& st = pos.st();

	for (int i = st.range.x1; i <= st.range.x2; i++) {
		for (int j = st.range.y1; j <= st.range.y2; j++) {
			Square sq(i, j);
			if (pos[sq] == Empty && pos.cand(sq))
				moves.emplace_back(sq);
		}
	}

	return moves;
}


