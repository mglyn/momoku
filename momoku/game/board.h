#ifndef BOARD
#define BOARD

#include <iostream>
#include <queue>
#include <cassert>
#include <algorithm>
#include <bitset>

#include "../common/data.h"
#include "pos.h"

constexpr int CAND_RANGE = 3;

struct Range {
	int x1 = MAX_GAME_LENGTH, y1 = MAX_GAME_LENGTH, x2 = -1, y2 = -1;

	void update(Pos pos, int length) {
		int x = pos.x(), y = pos.y();
		x1 = (std::min)(x1, x - CAND_RANGE);
		y1 = (std::min)(y1, y - CAND_RANGE);
		x2 = (std::max)(x2, x + CAND_RANGE);
		y2 = (std::max)(y2, y + CAND_RANGE);
		x1 = (std::max)(0, x1);
		y1 = (std::max)(0, y1);
		x2 = (std::min)(x2, length - 1);
		y2 = (std::min)(y2, length - 1);
	}

	int area() const { return (x2 - x1 + 1) * (y2 - y1 + 1); }
};

struct BoardInfo {
	int valueP1 = 0;
	std::array<std::array<int, 2>, FTYPE_NUM> cntT = {};
	Pos T4cost = NULLPOS;
	Pos lastMove = NULLPOS;
	Range range;
};

struct Unit {
	Eval::Line2 line2[4];
	uint8_t fType[2];
	uint8_t fValue[2];
};

class Board {
	int game_length;
	int _cntMove;
	Piece _self;
	uint64_t hash;

	std::unique_ptr<BoardInfo> _info;
	BoardInfo* info;

	std::array<uint64_t, BOARD_LENGTH> codeLR;				// L->R 低位->高位
	std::array<uint64_t, BOARD_LENGTH> codeUD;			// U->D 
	std::array<uint64_t, 2 * BOARD_LENGTH - 1> codeMain;// x = y
	std::array<uint64_t, 2 * BOARD_LENGTH - 1> codeVice;// x = -y 
	void or2Bits(PieceCode p, Pos pos);
	void xor2Bits(PieceCode p, Pos pos);
	void and2Bits(PieceCode p, Pos pos);

	std::array<Piece, BOARD_SIZE> content;
	std::array<uint8_t, BOARD_SIZE> _cand;
	std::array<Unit, BOARD_SIZE> units;

	std::unique_ptr<std::array<Unit, 8 * Eval::HALF_LINE_LEN>> _evalCache;
	std::array<Unit, 8 * Eval::HALF_LINE_LEN>* evalCache;

public:
	Board(int len);

	Board(const Board& bd);

	bool notin(Pos pos) const {
		int x = pos.x(), y = pos.y();
		return x >= game_length || y >= game_length || x < 0 || y < 0;
	}
	int length() const { return game_length; }
	int cntMove() const { return _cntMove; }

	Piece operator[](Pos pos) const { return content[pos]; }
	FType type(Piece p, Pos pos) const { return (FType)units[pos].fType[p]; }
	int value(Piece p, Pos pos) const { return units[pos].fValue[p]; }
	bool cand(Pos pos) const { return _cand[pos]; }

	Piece self() const { return _self; }
	int64_t hashKey() const { return hash; }
	Pos cost() const { return info[_cntMove].T4cost; }
	int cntFT(FType t, Piece p) const { return info[_cntMove].cntT[t][p]; }
	const Range& candRange() const { return info[_cntMove].range; }

	int staticEval() {
		return lastEval(0);
	}
	int lastEval(int r) {
		int id = (std::max)(1, _cntMove - r);
		int val = (info[id].valueP1 + info[id - 1].valueP1) / 2;
		return _self == P1 ? r & 1 ? -val : val : r & 1 ? val : -val;
	}
	Pos lastMove(int r) const { 
		int id = (std::max)(0, _cntMove - r + 1); 
		return info[id].lastMove; 
	}
	BoardInfo& bdnf() { return info[_cntMove]; }

	void reset(int len = 15);
	void update(Pos cd);
	void undo();
	Pos findPattern(Piece p, FType pat);

};

#endif
