#include "position.h"
#include "misc.h"

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

void Position::or2Bits(PieceCode k, Square sq) {
	int x = sq.abx(), y = sq.aby();
	codeLR[x] |= k << 2 * y;
	codeUD[y] |= k << 2 * x;
	codeMain[x - y + BOARD_LENGTH - 1] |= k << 2 * y;
	codeVice[x + y] |= k << 2 * y;
}

void Position::xor2Bits(PieceCode k, Square sq) {
	int x = sq.abx(), y = sq.aby();
	codeLR[x] ^= k << 2 * y;
	codeUD[y] ^= k << 2 * x;
	codeMain[x - y + BOARD_LENGTH - 1] ^= k << 2 * y;
	codeVice[x + y] ^= k << 2 * y;
}

void Position::and2Bits(PieceCode k, Square sq) {
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

//set empty board
void Position::set(int gameSize) {
	_gameSize = gameSize;

	_cntMove = 0;
	_side_to_move = P1;

	delete[] unitsCache;
	unitsCache = new UnitsCache[_gameSize * _gameSize + 1] {};

	delete[] _st;
	_st = new StateInfo[_gameSize * _gameSize + 1] {};

	StateInfo& st = _st[_cntMove];

	for (Square sq = 0; sq < BOARD_SIZE; sq = sq + 1) {

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
	for (Square sq = 0; sq < BOARD_SIZE; sq = sq + 1) {

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
				v.line[i] = Eval::decodeLine(code[i] >> 8);			
			}
			v.updateCombPattern();
			
			st.valueP1 += v.cp[P1].value;
			st.cntT[v.cp[P1].type][P1]++;
			st.valueP1 -= v.cp[P2].value;
			st.cntT[v.cp[P2].type][P2]++;
	}
	}
}

//copy 
void Position::set(const Position& cpos) {

	std::memcpy(this, &cpos, sizeof(Position));

	_st = new StateInfo[_gameSize * _gameSize + 1];
	std::memcpy(_st, cpos._st, (_gameSize * _gameSize + 1) * sizeof(StateInfo));

	unitsCache = new UnitsCache[_gameSize * _gameSize + 1];
	std::memcpy(unitsCache, cpos.unitsCache, (_gameSize * _gameSize + 1) * sizeof(UnitsCache));
}

std::vector<Square> Position::seq() const {
	std::vector<Square> ret;

	for (int i = 1; i < _cntMove; i++) {
		ret.push_back(_st[_cntMove].move);
	}
	return ret;
}

constexpr static int imax(int a, int b) {
	int diff = a - b;
	int dsgn = diff >> 31;
	return a - (diff & dsgn);
}

constexpr static int imin(int a, int b) {
	int diff = a - b;
	int dsgn = diff >> 31;
	return b + (diff & dsgn);
}

constexpr static void updateRange(Range& rg, const Range& _rg, Square sq, int length) {
	int x = sq.x(), y = sq.y();
	rg.x1 = imin(_rg.x1, x - CAND_RANGE);
	rg.x1 = imax(0, rg.x1);
	rg.y1 = imin(_rg.y1, y - CAND_RANGE);
	rg.y1 = imax(0, rg.y1);
	rg.x2 = imax(_rg.x2, x + CAND_RANGE);
	rg.x2 = imin(rg.x2, length - 1);
	rg.y2 = imax(_rg.y2, y + CAND_RANGE);
	rg.y2 = imin(rg.y2, length - 1);
}

void Position::make_move(Square sq) {

	std::memcpy(_st + _cntMove + 1, _st + _cntMove, offsetof(StateInfo, move));

	_st[_cntMove + 1].key = _st[_cntMove].key ^ Zobrist[_side_to_move][sq];
	updateRange(_st[_cntMove + 1].range, _st[_cntMove].range, sq, _gameSize);

	Unit* ic = &unitsCache[_cntMove][0];

	_cntMove++;

	StateInfo& st = _st[_cntMove];
	st.move = sq;

	content[sq] = _side_to_move;
	or2Bits(static_cast<PieceCode>(_side_to_move + 1), sq);

	st.valueP1 -= units[sq].cp[P1].value;
	st.cntT[units[sq].cp[P1].type][P1]--;
	st.valueP1 += units[sq].cp[P2].value;
	st.cntT[units[sq].cp[P2].type][P2]--;

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

			Square npos = sq + j * D4[i];
			if (content[npos] != Empty)continue;

			Unit& v = units[npos];
			*ic++ = v;

			st.valueP1 -= v.cp[P1].value;
			st.cntT[v.cp[P1].type][P1]--;
			st.valueP1 += v.cp[P2].value;
			st.cntT[v.cp[P2].type][P2]--;

			_cand[npos]++;

			v.line[i] = Eval::decodeLine(code[i]);
			v.updateCombPattern();

			st.valueP1 += v.cp[P1].value;
			st.cntT[v.cp[P1].type][P1]++;
			st.valueP1 -= v.cp[P2].value;
			st.cntT[v.cp[P2].type][P2]++;

			st.T5Square = v.cp[_side_to_move].type == T5 ? npos : st.T5Square;
		}
	}
	_side_to_move = ~_side_to_move;
}

void Position::undo() {

	assert(_cntMove >= 1);

	_side_to_move = ~_side_to_move;
	Square sq = _st[_cntMove].move;

	content[sq] = Empty;
	xor2Bits(static_cast<PieceCode>(_side_to_move + 1), sq);

	_cntMove--;

	Unit* ic = &unitsCache[_cntMove][0];

	constexpr int d = Eval::HALF_LINE_LEN;

	for (int i = 0; i < 4; i++) {
		for (int j = -d; j <= d; j++) {

			if (j == 0)continue;

			Square npos = sq + j * D4[i];
			if (content[npos] != Empty)continue;

			units[npos] = *ic++;
			_cand[npos]--;
		}
	}

	
}


