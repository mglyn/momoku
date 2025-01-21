#include "position.h"
#include "tt.h"

uint64_t Zobrist[2][BOARD_LENGTH * BOARD_LENGTH];

bool init = []()->bool {
	PRNG rng(1070372);
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < BOARD_LENGTH * BOARD_LENGTH; j++) {
			Zobrist[i][j] = rng.rand<uint64_t>();
		}
	}
	return true;
}();

void Position::or2Bits(PieceCode k, Sqare sq) {
	int x = sq.abx(), y = sq.aby();
	codeLR[x] |= k << 2 * y;
	codeUD[y] |= k << 2 * x;
	codeMain[x - y + BOARD_LENGTH - 1] |= k << 2 * y;
	codeVice[x + y] |= k << 2 * y;
}

void Position::xor2Bits(PieceCode k, Sqare sq) {
	int x = sq.abx(), y = sq.aby();
	codeLR[x] ^= k << 2 * y;
	codeUD[y] ^= k << 2 * x;
	codeMain[x - y + BOARD_LENGTH - 1] ^= k << 2 * y;
	codeVice[x + y] ^= k << 2 * y;
}

void Position::and2Bits(PieceCode k, Sqare sq) {
	int x = sq.abx(), y = sq.aby();
	uint64_t t = k | (~0ULL - 3);
	uint64_t u = t << 2 * x;
	u |= (1ULL << (2 * x)) - 1;
	uint64_t v = t << 2 * y;
	v |= (1ULL << (2 * y)) - 1;
	codeLR[x] &= v;
	codeUD[y] &= u;
	codeMain[x - y + BOARD_LENGTH - 1] &= v;
	codeVice[x + y] &= v;
}

void Position::set(StateInfo& newSt) {
	_cntMove = 0;
	_side_to_move = P1;

	st = &newSt;

	for (Sqare sq = 0; sq < BOARD_SIZE; sq = sq + 1) {

		if (notin(sq)) {
			content[sq] = Invalid;
			or2Bits(C_Invalid, sq);
		}
		else {
			content[sq] = Empty;
			and2Bits(C_Empty, sq);
		}

		_cand[sq] = 0;
	}
	for (Sqare sq = 0; sq < BOARD_SIZE; sq = sq + 1) {

		if (!notin(sq)) {

			constexpr int d = Eval::HALF_LINE_LEN;
			int x = sq.abx(), y = sq.aby();
			uint64_t code[4] = {
				(y - 2 * d) < 0 ? codeLR[x] << 2 * (2 * d - y) : codeLR[x] >> 2 * (y - 2 * d),
				(x - 2 * d) < 0 ? codeUD[y] << 2 * (2 * d - x) : codeUD[y] >> 2 * (x - 2 * d),
				(y - 2 * d) < 0 ? codeMain[x - y + BOARD_LENGTH - 1] << 2 * (2 * d - y) :
								  codeMain[x - y + BOARD_LENGTH - 1] >> 2 * (y - 2 * d),
				(y - 2 * d) < 0 ? codeVice[x + y] << 2 * (2 * d - y) : codeVice[x + y] >> 2 * (y - 2 * d)
			};

			Unit& v = units[sq];

			for (int i = 0; i < 4; i++) {
				v.line2[i] = Eval::decode1(code[i] >> 8);
			}
			Eval::Flower F1 = Eval::decode2(v.line2[0].lineP1, v.line2[1].lineP1, v.line2[2].lineP1, v.line2[3].lineP1);
			Eval::Flower F2 = Eval::decode2(v.line2[0].lineP2, v.line2[1].lineP2, v.line2[2].lineP2, v.line2[3].lineP2);

			v.fType[P1] = F1.type;
			v.fValue[P1] = F1.value;
			v.fType[P2] = F2.type;
			v.fValue[P2] = F2.value;
			
			st->valueP1 += v.fValue[P1];
			st->cntT[v.fType[P1]][P1]++;
			st->valueP1 -= v.fValue[P2];
			st->cntT[v.fType[P2]][P2]++;
		}
	}
}

constexpr static void updateRange(const Range& _rg, Range& rg, Sqare sq, int length) {
	int x = sq.x(), y = sq.y();
	rg.x1 = (std::min)(_rg.x1, x - CAND_RANGE);
	rg.x1 = (std::max)(0, rg.x1);
	rg.y1 = (std::min)(_rg.y1, y - CAND_RANGE);
	rg.y1 = (std::max)(0, rg.y1);
	rg.x2 = (std::max)(_rg.x2, x + CAND_RANGE);
	rg.x2 = (std::min)(rg.x2, length - 1);
	rg.y2 = (std::max)(_rg.y2, y + CAND_RANGE);
	rg.y2 = (std::min)(rg.y2, length - 1);
}

void Position::make_move(Sqare sq, StateInfo& newSt) {

	std::memcpy(&newSt, st, offsetof(StateInfo, prev));
	newSt.prev = st;
	st->next = &newSt;

	newSt.key = st->key ^ Zobrist[_side_to_move][sq];
	updateRange(st->range, newSt.range, sq, _gameSize);
	newSt.move = sq;

	Unit* ic = evalCache[_cntMove];
	_cntMove++;
	st = &newSt;

	content[sq] = _side_to_move;
	or2Bits(static_cast<PieceCode>(_side_to_move + 1), sq);

	st->valueP1 -= units[sq].fValue[P1];
	st->cntT[units[sq].fType[P1]][P1]--;
	st->valueP1 += units[sq].fValue[P2];
	st->cntT[units[sq].fType[P2]][P2]--;

	constexpr int d = Eval::HALF_LINE_LEN;
	int x = sq.abx(), y = sq.aby();
	uint64_t code[4] = {
		(y - 2 * d) < 0 ? codeLR[x] << 2 * (2 * d - y) : codeLR[x] >> 2 * (y - 2 * d),
		(x - 2 * d) < 0 ? codeUD[y] << 2 * (2 * d - x) : codeUD[y] >> 2 * (x - 2 * d),
		(y - 2 * d) < 0 ? codeMain[x - y + BOARD_LENGTH - 1] << 2 * (2 * d - y) :
						  codeMain[x - y + BOARD_LENGTH - 1] >> 2 * (y - 2 * d),
		(y - 2 * d) < 0 ? codeVice[x + y] << 2 * (2 * d - y) : codeVice[x + y] >> 2 * (y - 2 * d)
	};

	for (int i = 0; i < 4; i++) {
		for (int j = -d; j <= d; code[i] >>= 2, j++) {

			if (j == 0)continue;

			Sqare npos = sq + j * D4[i];
			if ((*this)[npos] != Empty)continue;

			Unit& v = units[npos];
			*ic++ = v;

			st->valueP1 -= v.fValue[P1];
			st->cntT[v.fType[P1]][P1]--;
			st->valueP1 += v.fValue[P2];
			st->cntT[v.fType[P2]][P2]--;

			_cand[npos]++;
			v.line2[i] = Eval::decode1(code[i]);
			Eval::Flower F1 = Eval::decode2(v.line2[0].lineP1, v.line2[1].lineP1, v.line2[2].lineP1, v.line2[3].lineP1);
			Eval::Flower F2 = Eval::decode2(v.line2[0].lineP2, v.line2[1].lineP2, v.line2[2].lineP2, v.line2[3].lineP2);
			v.fType[P1] = F1.type;
			v.fValue[P1] = F1.value;
			v.fType[P2] = F2.type;
			v.fValue[P2] = F2.value;

			st->valueP1 += v.fValue[P1];
			st->cntT[v.fType[P1]][P1]++;
			st->valueP1 -= v.fValue[P2];
			st->cntT[v.fType[P2]][P2]++;

			if (v.fType[_side_to_move] == T5) st->T4cost = npos;
		}
	}
	_side_to_move = ~_side_to_move;
}

void Position::undo() {

	assert(_cntMove >= 1);

	_side_to_move = ~_side_to_move;
	Sqare sq = st->move;

	content[sq] = Empty;
	xor2Bits(static_cast<PieceCode>(_side_to_move + 1), sq);

	st = st->prev;
	_cntMove--;
	Unit* ic = evalCache[_cntMove];

	constexpr int d = Eval::HALF_LINE_LEN;

	for (int i = 0; i < 4; i++) {
		for (int j = -d; j <= d; j++) {

			if (j == 0)continue;

			Sqare npos = sq + j * D4[i];
			if ((*this)[npos] != Empty)continue;

			units[npos] = *ic++;
			_cand[npos]--;
		}
	}

	
}


