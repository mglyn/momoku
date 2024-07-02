#include "board.h"
#include "../search/tt.h"
#include "../test/test.h"

void Board::or2Bits(PieceCode k, Pos pos) {
	int x = pos.abx(), y = pos.aby();
	codeLR[x] |= k << 2 * y;
	codeUD[y] |= k << 2 * x;
	codeMain[x - y + BOARD_LENGTH - 1] |= k << 2 * y;
	codeVice[x + y] |= k << 2 * y;
}

void Board::xor2Bits(PieceCode k, Pos pos) {
	int x = pos.abx(), y = pos.aby();
	codeLR[x] ^= k << 2 * y;
	codeUD[y] ^= k << 2 * x;
	codeMain[x - y + BOARD_LENGTH - 1] ^= k << 2 * y;
	codeVice[x + y] ^= k << 2 * y;
}

void Board::and2Bits(PieceCode k, Pos pos) {
	int x = pos.abx(), y = pos.aby();
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

void Board::reset(int len) {
	_length = len;
	_cntMove = 0;
	_self = P1;
	hash = 0x5c20db8251649948ULL;

	info.resize(_length * _length + 1);
	evalCache.resize(_length * _length + 1);

	for (Pos pos = 0; pos < BOARD_SIZE; pos = pos + 1) {
		
		if (notin(pos)) {
			content[pos] = Invalid;
			or2Bits(C_Invalid, pos);
		} 
		else {
			and2Bits(C_Empty, pos);
			content[pos] = Empty;
		}

		_cand[pos] = 0;
		
		for (int i = 0; i < 4; i++) {
			units[pos].line2[i].lineP1 = 0;
			units[pos].line2[i].lineP2 = 0;
		}
		units[pos].fType[0] = units[pos].fType[1] = TNone;
		units[pos].fValue[0] = units[pos].fValue[1] = 0;
	}
}

void Board::update(Pos pos) {
	auto& nf = info[++_cntMove];

	assert(_cntMove - 1 >= 0);
	nf = info[_cntMove - 1];
	nf.lastMove = pos;
	nf.range.update(pos, _length);

	or2Bits(static_cast<PieceCode>(_self + 1), pos);
	content[pos] = _self;
	hash ^= Zobrist[_self][pos];

	nf.valueP1 -= units[pos].fValue[P1];
	nf.cntT[units[pos].fType[P1]][P1]--;
	nf.valueP1 += units[pos].fValue[P2];
	nf.cntT[units[pos].fType[P2]][P2]--;

	auto ic = evalCache[_cntMove].begin();

	constexpr int d = Eval::HALF_LINE_LEN;
	int x = pos.abx(), y = pos.aby();
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

			Pos npos = pos + j * D4[i];
			if (content[npos] != Empty)continue;

			Unit& v = units[npos];
			*ic++ = v;

			nf.valueP1 -= v.fValue[P1];
			nf.cntT[v.fType[P1]][P1]--;
			nf.valueP1 += v.fValue[P2];
			nf.cntT[v.fType[P2]][P2]--;

			_cand[npos]++;
			v.line2[i] = Eval::decode1(code[i]);
			Eval::Flower F1 = Eval::decode2(v.line2[0].lineP1, v.line2[1].lineP1, v.line2[2].lineP1, v.line2[3].lineP1);
			Eval::Flower F2 = Eval::decode2(v.line2[0].lineP2, v.line2[1].lineP2, v.line2[2].lineP2, v.line2[3].lineP2);
			v.fType[P1] = F1.type;
			v.fValue[P1] = F1.value;
			v.fType[P2] = F2.type;
			v.fValue[P2] = F2.value;

			nf.valueP1 += v.fValue[P1];
			nf.cntT[v.fType[P1]][P1]++;
			nf.valueP1 -= v.fValue[P2];
			nf.cntT[v.fType[P2]][P2]++;

			if (v.fType[_self] == T5) nf.T4cost = npos;
		}
	}

	_self = ~_self;
}

void Board::undo() {
	assert(_cntMove >= 1);

	auto ic = evalCache[_cntMove].begin();
	Pos pos = info[_cntMove].lastMove;
	_self = ~_self;

	hash ^= Zobrist[_self][pos];
	_cntMove--;

	xor2Bits(static_cast<PieceCode>(_self + 1), pos);
	content[pos] = Empty;

	constexpr int d = Eval::HALF_LINE_LEN;

	for (int i = 0; i < 4; i++) {
		for (int j = -d; j <= d; j++) {

			if (j == 0)continue;

			Pos npos = pos + j * D4[i];
			if (content[npos] != Empty)continue;

			units[npos] = *ic++;
			_cand[npos]--;
		}
	}
}

Pos Board::findPattern(Piece p, FType pat) {
	for (int i = 0; i < _length; i++) {
		for (int j = 0; j < _length; j++) {
			if (content[Pos(i, j)] != Empty)continue;
			if (type(p, Pos(i, j)) == pat)return Pos(i, j);
		}
	}
	assert(0);
	return NULLPOS;
}

