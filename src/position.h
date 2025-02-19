#ifndef POSITION
#define POSITION

#include <iostream>
#include <queue>
#include <cassert>
#include <algorithm>
#include <array>

#include "eval.h"

struct Range {
	int x1 = 32, y1 = 32, x2 = -1, y2 = -1;
	constexpr int area() const { return (x2 - x1 + 1) * (y2 - y1 + 1); }
};

struct Unit {
	Eval::Line line[4] = {};
	Eval::CombPattern cp[SIDE_NUM] = {};

	inline void updateCombPattern() {
		cp[P1] = Eval::decodeComb(line[0].lineP1, line[1].lineP1, line[2].lineP1, line[3].lineP1);
		cp[P2] = Eval::decodeComb(line[0].lineP2, line[1].lineP2, line[2].lineP2, line[3].lineP2);
	}
};

struct StateInfo {

	// Copied when making a move //////////////////
	uint8_t cntT[FTYPE_NUM][SIDE_NUM] = {};
	int16_t valueP1 = 0;

	// Not copied when making a move (will be recomputed anyhow)
	Square move = Square::NONE;
	Square T5Square;
	Key key = 5211314;
	Range range;
};

class Position {
	int		_gameSize;
	int		_cntMove;
	Piece	_side_to_move;

	uint64_t codeLR[BOARD_LENGTH];				// L->R 低位->高位
	uint64_t codeUD[BOARD_LENGTH];				// U->D 
	uint64_t codeMain[2 * BOARD_LENGTH - 1];	// x = y
	uint64_t codeVice[2 * BOARD_LENGTH - 1];	// x = -y 
	void or2Bits(PieceCode p, Square sq);
	void xor2Bits(PieceCode p, Square sq);
	void and2Bits(PieceCode p, Square sq);

	Piece		content[BOARD_SIZE];
	uint8_t		_cand[BOARD_SIZE];

	Unit units[BOARD_SIZE];
	using UnitsCache = std::array<Unit, 4 * 2 * Eval::HALF_LINE_LEN>;  //used for eval
	UnitsCache* unitsCache;

	StateInfo* _st;

public:
	Position() = default;
	~Position() { 
		delete[] unitsCache; 
		delete[] _st;
	};
	Position(const Position&) = delete;
	Position& operator=(const Position&) = delete;
	constexpr const StateInfo& st()			const { return _st[_cntMove]; }
	const StateInfo& prevst()				const { return _st[(std::max)(_cntMove - 1, 0)]; }
	constexpr int gameSize()				const { return _gameSize; }
	constexpr int cntMove()					const { return _cntMove; }
	constexpr Piece side_to_move()			const { return _side_to_move; }
	constexpr uint64_t key()				const { return _st[_cntMove].key; }
	constexpr bool full()					const { return _gameSize * _gameSize == _cntMove; }
	Piece operator[](Square sq)				const { return content[sq]; }
	FType type(Piece p, Square sq)			const { return units[sq].cp[p].type; }
	int value(Piece p, Square sq)			const { return units[sq].cp[p].value; }
	bool cand(Square sq)					const { return _cand[sq]; }
	constexpr bool notin(Square sq)			const {
		int x = sq.x(), y = sq.y();
		return x < 0 || y < 0 || x >= _gameSize || y >= _gameSize;
	}
	//set initial pos
	std::vector<Square> seq()				const;
	void set(int gameSize);
	void set(const Position& cpos);
	void make_move(Square sq);
	void undo();
};

#endif
