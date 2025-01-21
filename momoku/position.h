#ifndef POSITION
#define POSITION

#include <iostream>
#include <queue>
#include <cassert>
#include <algorithm>
#include <bitset>

#include "eval.h"

struct Range {
	int x1 = MAX_GAME_LENGTH;
	int y1 = MAX_GAME_LENGTH;
	int x2 = -1;
	int y2 = -1;
	constexpr int area() const { return (x2 - x1 + 1) * (y2 - y1 + 1); }
};

struct Unit {
	Eval::Line2 line2[4];
	FType fType[2];
	uint8_t fValue[2];
};

struct StateInfo {

	// Copied when making a move
	int valueP1 = 0;
	uint8_t cntT[FTYPE_NUM][2] = {};

	// Not copied when making a move (will be recomputed anyhow)
	StateInfo* prev;
	StateInfo* next;
	uint64_t key;
	Range range;
	Sqare move;
	Sqare T4cost;
};

class Position {
	int		_gameSize;
	int		_cntMove;
	Piece	_side_to_move;

	StateInfo* st;

	uint64_t codeLR[BOARD_LENGTH];				// L->R 低位->高位
	uint64_t codeUD[BOARD_LENGTH];				// U->D 
	uint64_t codeMain[2 * BOARD_LENGTH - 1];	// x = y
	uint64_t codeVice[2 * BOARD_LENGTH - 1];	// x = -y 
	void or2Bits(PieceCode p, Sqare sq);
	void xor2Bits(PieceCode p, Sqare sq);
	void and2Bits(PieceCode p, Sqare sq);

	Piece		content[BOARD_SIZE];
	uint8_t		_cand[BOARD_SIZE];

	Unit		units[BOARD_SIZE];
	Unit evalCache[MAX_MOVE][4 * 2 * Eval::HALF_LINE_LEN];  //used for eval

public:
	Position(int len) : _gameSize(len) { }
	constexpr const StateInfo& stateInfo()	const { return *st; }
	constexpr int gameSize()				const { return _gameSize; }
	constexpr int cntMove()					const { return _cntMove; }
	constexpr Piece side_to_move()			const { return _side_to_move; }
	constexpr uint64_t key()				const { return st->key; }
	constexpr bool draw()					const { return _gameSize * _gameSize == _cntMove; }

	constexpr bool notin(Sqare sq) const {
		int x = sq.x(), y = sq.y();
		return (x | y) < 0 || x >= _gameSize || y >= _gameSize;
	}
	constexpr int staticEval() const {
		return _side_to_move == P1 ?
			(st->valueP1 + st->prev->valueP1) / 2 :
			-(st->valueP1 + st->prev->valueP1) / 2;
	}

	Piece operator[](Sqare sq)				const { return content[sq]; }
	FType type(Piece p, Sqare sq)			const { return units[sq].fType[p]; }
	int value(Piece p, Sqare sq)			const { return units[sq].fValue[p]; }
	bool cand(Sqare sq)						const { return _cand[sq]; }
	bool legal(Sqare sq)					const { return (*this)[sq] == Empty; }//.....//move from tt may be corrupted

	void set(StateInfo& st);
	void make_move(Sqare sq, StateInfo& newSt);
	void undo();
};

#endif
