
/*** Start of inlined file: data.cpp ***/

/*** Start of inlined file: data.h ***/
#ifndef DATA
#define DATA

#include <vector>

/*** Start of inlined file: common.h ***/
#ifndef COMMON
#define COMMON

#include <vector>
#include <array>
#include <cmath>
#include <chrono>
#include <iostream>

using namespace std::chrono;

enum Piece :uint8_t {
	P1 = 0,
	P2 = 1,
	Empty = 2,
	Invalid = 3
};

enum PieceCode :uint64_t {
	C_Empty = 0,
	C_P1 = 1,
	C_P2 = 2,
	C_Invalid = 3
};

constexpr Piece operator~(Piece p) { return static_cast<Piece>(p ^ 1); }

constexpr int FTYPE_NUM = 7;
enum FType : int{
	TNone,
	TH3,
	TDH3,
	T4,
	T4H3,
	TH4,
	T5
};

enum HashType :uint8_t {
	B_Initial = 0,
	B_Upper = 1,
	B_Lower = 2,
	B_Exact = 3
};

constexpr int BOARD_LENGTH = 32;
constexpr int BOARD_SIZE = BOARD_LENGTH * BOARD_LENGTH;
constexpr int BOARD_BOUNDARY = 5;
constexpr int MAX_GAME_LENGTH = BOARD_LENGTH - 2 * BOARD_BOUNDARY;
constexpr int MAX_MOVE = MAX_GAME_LENGTH * MAX_GAME_LENGTH;

constexpr int WIN_CRITICAL = 20000;
constexpr int WIN_MAX = 30000;
constexpr int VAL_INF = 30001;

#endif

/*** End of inlined file: common.h ***/


extern uint64_t Zobrist[2][BOARD_LENGTH];

namespace Eval {
	constexpr int HALF_LINE_LEN = 4;
	constexpr int NUMCODE = 1 << 16;
	constexpr int NUMLINE4DIR = 1 << 16;

	struct Line2 {
		uint8_t lineP1 : 4;
		uint8_t lineP2 : 4;
	};

	struct Flower {
		uint8_t value;
		uint8_t type;
	};

	Line2 decode1(uint64_t code);
	Flower decode2(int l1, int l2, int l3, int l4);
	int threatP1Eval(std::array<std::array<int, 2>, FTYPE_NUM>& cntT);

	struct trie_node {
		int father;
		int fail;
		int edge[4];
		int lineType;
	};

	class ACautomation {      //AC自动机多串匹配
		trie_node nodes[256];
		int root;
		int cnt_node;

	public:
		void insertPattern(const std::vector<int>& pattern, int lineID);
		void calc_fail();
		ACautomation();
		int query(std::vector<int>& arr) const;
	};
}

#endif

/*** End of inlined file: data.h ***/

#include <vector>
#include <queue>
#include <cassert>

/*** Start of inlined file: misc.h ***/
#ifndef MISC
#define MISC

#include <xmmintrin.h>
#include <cstdint>

inline uint64_t mulhi64(uint64_t a, uint64_t b){

	uint64_t aL = (uint32_t)a, aH = a >> 32;
	uint64_t bL = (uint32_t)b, bH = b >> 32;
	uint64_t c1 = (aL * bL) >> 32;
	uint64_t c2 = aH * bL + c1;
	uint64_t c3 = aL * bH + (uint32_t)c2;
	return aH * bH + (c2 >> 32) + (c3 >> 32);
}

inline void prefetch(void* addr) {
#if defined(_MSC_VER)
	_mm_prefetch((char*)addr, _MM_HINT_T0);
#else
	__builtin_prefetch(addr);
#endif
}

template<class T, int len>
constexpr int arrLen(T(&x)[len]) { return len; }

#endif

/*** End of inlined file: misc.h ***/


extern uint64_t Zobrist[2][BOARD_LENGTH] = {
		{
			14514284786278117030, 4620546740167642908, 13109570281517897720, 17462938647148434322,
			355488278567739596, 7469126240319926998, 4635995468481642529, 418970542659199878,
			9604170989252516556, 6358044926049913402, 5058016125798318033, 10349215569089701407,
			2583272014892537200, 10032373690199166667, 9627645531742285868, 15810285301089087632,
			9219209713614924562, 7736011505917826031, 13729552270962724157, 4596340717661012313,
			4413874586873285858, 5904155143473820934, 16795776195466785825, 3040631852046752166,
			4529279813148173111, 3658352497551999605, 13205889818278417278, 17853215078830450730,
			14193508720503142180, 1488787817663097441, 8484116316263611556, 4745643133208116498
		},
		{
			14333959900198994173, 10770733876927207790, 17529942701849009476, 8081518017574486547,
			5945178879512507902, 9821139136195250096, 4728986788662773602, 840062144447779464,
			9315169977352719788, 12843335216705846126, 1682692516156909696, 16733405176195045732,
			570275675392078508, 2804578118555336986, 18105853946332827420, 11444576169427052165,
			5511269538150904327, 6665263661402689669, 8872308438533970361, 5494304472256329401,
			5260777597240341458, 17048363385688465216, 11601203342555724204, 13927871433293278342,
			13168989862813642697, 13332527631701716084, 1288265801825883165, 8980511589347843149,
			1639193574298669424, 14012553476551396225, 7818048564976445173, 11012385938523194722
		}
};

namespace Data {
	constexpr int LINE_NUM = 15;
	const uint8_t eval[LINE_NUM] = {
		0,
		2,3,4,			4,5,6,	//2
		16,18,20,		30,34,	//3
		45,				60,	//4
		100,
	};
	const int lineID[] = {
		1,1,1,2,2,2,3,3,3,3,			4,5,5,6,6,6,//2
		7,7,7,7,7,8,8,9,9,9,			10,10,11,11,//3
		12,12,12,12,12,					13,			//4
		14,											//5
	};
	const std::vector<std::vector<int>> patterns = {
	{1,0,0,0,1},
	{0,1,0,0,1},
	{1,0,0,1,0},

	{0,0,1,0,1},
	{0,1,0,1,0},
	{1,0,1,0,0},

	{0,0,0,1,1},
	{0,0,1,1,0},
	{0,1,1,0,0},
	{1,1,0,0,0},//2

	{ 0,1,0,0,1,0 },

	{ 0,0,1,0,1,0 },
	{ 0,1,0,1,0,0 },

	{ 0,1,1,0,0,0 },
	{ 0,0,1,1,0,0 },
	{ 0,0,0,1,1,0 },//2

	{1,0,1,0,1},
	{1,0,0,1,1},
	{1,1,0,0,1},
	{0,1,0,1,1},
	{1,1,0,1,0},

	{0,1,1,0,1},
	{1,0,1,1,0},

	{0,0,1,1,1},
	{0,1,1,1,0},
	{1,1,1,0,0},//3

	{ 0,1,0,1,1,0 },
	{ 0,1,1,0,1,0 },

	{ 0,0,1,1,1,0 },
	{ 0,1,1,1,0,0 },//3

	{1,1,1,1,0},
	{1,1,1,0,1},
	{1,1,0,1,1},
	{1,0,1,1,1},
	{0,1,1,1,1},//4

	{ 0,1,1,1,1,0 },//4

	{1,1,1,1,1},//5
	};
}

namespace Eval {
	Line2 codeToLineID2[NUMCODE];
	Flower lineToFlower[NUMLINE4DIR];
	uint8_t threatStateToP1Eval[1 << 2 * (FTYPE_NUM - 1)];

	Line2 decode1(uint64_t code) {
		return codeToLineID2[(code >> 2) & 0b1111111100000000 | code & 0b11111111];
	}
	Flower decode2(int l1, int l2, int l3, int l4) {
		return lineToFlower[(l1 << 12) + (l2 << 8) + (l3 << 4) + l4];
	}

	int getThreatStat(std::array<std::array<int, 2>, FTYPE_NUM>& cntT) {
		int threatStat = 0;
		for (int i = 1; i < FTYPE_NUM; i++) {
			threatStat |= int(bool(cntT[i][P1])) << (2 * i - 2);
			threatStat |= int(bool(cntT[i][P2])) << (2 * i - 1);
		}
		return threatStat;
	}

	int threatP1Eval(std::array<std::array<int, 2>, FTYPE_NUM>& cntT) {
		return threatStateToP1Eval[getThreatStat(cntT)];
	}

	void ACautomation::insertPattern(const std::vector<int>& pattern, int lineID) {
		int now = root;
		for (int c : pattern) {
			if (nodes[now].edge[c] != 0)
				now = nodes[now].edge[c];
			else {
				nodes[now].edge[c] = ++cnt_node;
				nodes[nodes[now].edge[c]].father = now;
				now = nodes[now].edge[c];
			}
		}
		nodes[now].lineType = lineID;
	}

	void ACautomation::calc_fail() {
		nodes[root].fail = 0;
		std::queue<int> q;
		for (int i = 0; i < 4; i++) {
			if (nodes[root].edge[i] != 0) {
				nodes[nodes[root].edge[i]].fail = root;
				q.push(nodes[root].edge[i]);
			}
		}
		while (!q.empty()) {
			int fa = q.front();
			int fafail = nodes[fa].fail;
			q.pop();
			for (int i = 0; i < 4; i++) {
				int now = nodes[fa].edge[i];
				if (now == 0) continue;
				if (nodes[fafail].edge[i] != 0) {
					nodes[now].fail = nodes[fafail].edge[i];
				}
				else nodes[now].fail = root;
				q.push(now);
			}
		}
	}

	ACautomation::ACautomation() {
		memset(nodes, 0, sizeof(nodes));
		root = cnt_node = 1;
	}

	int ACautomation::query(std::vector<int>& arr) const {
		int lineType = 0;
		for (int i = 0, now = root; i < 9; i++) {
			int ch = arr[i];
			while (nodes[now].edge[ch] == 0 && nodes[now].fail != 0) //不能匹配时能跳则跳fail
				now = nodes[now].fail;
			if (nodes[now].edge[ch] != 0) {        //能匹配时记录答案
				now = nodes[now].edge[ch];
				lineType = std::max(lineType, nodes[now].lineType);
			}
		}
		return lineType;
	}

	static void Init() {
		ACautomation ac;

		assert(arrLen(Data::lineID) == Data::patterns.size());
		for (int i = 0; i < Data::patterns.size(); i++) {

			auto pattern = Data::patterns[i];
			ac.insertPattern(pattern, Data::lineID[i]);

			for (int& c : pattern)
				if (c == C_P1)
					c = C_P2;
			ac.insertPattern(pattern, Data::lineID[i]);
		}
		ac.calc_fail();

		//枚举code
		for (int code = 0; code < NUMCODE; code++) {

			auto simulate = [](int code) {
				std::vector<int> arr(9, 0);
				for (int i = 0; i <= 3; i++)
					arr[i] = (code >> 2 * i) & 0b11;
				for (int i = 4; i <= 7; i++)
					arr[i + 1] = (code >> 2 * i) & 0b11;
				return arr;
				};
			std::vector<int> arr = simulate(code);

			arr[4] = 1;
			int typeP1 = ac.query(arr);

			arr[4] = 2;
			int typeP2 = ac.query(arr);

			codeToLineID2[code].lineP1 = typeP1;
			codeToLineID2[code].lineP2 = typeP2;
		}

		//枚举line
		for (int a = 0; a < Data::LINE_NUM; a++) {
			for (int b = a; b < Data::LINE_NUM; b++) {
				for (int c = b; c < Data::LINE_NUM; c++) {
					for (int d = c; d < Data::LINE_NUM; d++) {

						int p[4] = { a,b,c,d };

						Flower f;

						f.value = Data::eval[a] + Data::eval[b] + Data::eval[c] + Data::eval[d];

						int n[20] = {};
						n[a]++, n[b]++, n[c]++, n[d]++;
						//4:活三 5:冲四 6:活四 7:连五 8:禁手

						int ft = TNone;

						if (n[10] || n[11]) ft = TH3;
						if (n[10] + n[11] >= 2) ft = TDH3;
						if ((ft == TH3 || ft == TDH3) && n[12]) ft = T4H3;
						else if (n[12])ft = T4;
						if (n[13] || n[12] >= 2)ft = TH4;
						if (n[14])ft = T5;
						f.type = (FType)ft;

						do {
							lineToFlower[(p[0] << 12) + (p[1] << 8) + (p[2] << 4) + p[3]] = f;
						} while (std::next_permutation(p, p + 4));
					}
				}
			}
		}

		//枚举threatStat
		for (int threatStat = 0; threatStat < 1 << 2 * (FTYPE_NUM - 1); threatStat++) {

			bool hasT[FTYPE_NUM][2] = {};

			for (int i = 1; i < FTYPE_NUM; i++) {
				hasT[i][P1] = threatStat & (1 << (2 * i - 2));
				hasT[i][P2] = threatStat & (1 << (2 * i - 1));
			}

			int evalP1 = 0;
			int evalP2 = 0;

			if (hasT[TH3][P1])evalP1 += 14;
			if (hasT[TDH3][P1])evalP1 += 34;
			if (hasT[T4][P1])evalP1 += 17;
			if (hasT[T4H3][P1])evalP1 += 39;
			if (hasT[TH4][P1])evalP1 += 49;
			if (hasT[T5][P1])evalP1 += 59;

			if (hasT[TH3][P2])evalP2 += 14;
			if (hasT[TDH3][P2])evalP2 += 34;
			if (hasT[T4][P2])evalP2 += 17;
			if (hasT[T4H3][P2])evalP2 += 39;
			if (hasT[TH4][P2])evalP2 += 49;
			if (hasT[T5][P2])evalP2 += 59;

			threatStateToP1Eval[threatStat] = evalP1 - evalP2;
		}

	}

	const auto init = []() {
		Init();
		return true;
		}();
}

/*** End of inlined file: data.cpp ***/


/*** Start of inlined file: board.cpp ***/

/*** Start of inlined file: board.h ***/
#ifndef BOARD
#define BOARD

#include <iostream>
#include <queue>
#include <cassert>
#include <algorithm>
#include <bitset>


/*** Start of inlined file: data.h ***/
#ifndef DATA
#define DATA

#include <vector>

extern uint64_t Zobrist[2][BOARD_LENGTH];

namespace Eval {
	constexpr int HALF_LINE_LEN = 4;
	constexpr int NUMCODE = 1 << 16;
	constexpr int NUMLINE4DIR = 1 << 16;

	struct Line2 {
		uint8_t lineP1 : 4;
		uint8_t lineP2 : 4;
	};

	struct Flower {
		uint8_t value;
		uint8_t type;
	};

	Line2 decode1(uint64_t code);
	Flower decode2(int l1, int l2, int l3, int l4);
	int threatP1Eval(std::array<std::array<int, 2>, FTYPE_NUM>& cntT);

	struct trie_node {
		int father;
		int fail;
		int edge[4];
		int lineType;
	};

	class ACautomation {      //AC自动机多串匹配
		trie_node nodes[256];
		int root;
		int cnt_node;

	public:
		void insertPattern(const std::vector<int>& pattern, int lineID);
		void calc_fail();
		ACautomation();
		int query(std::vector<int>& arr) const;
	};
}

#endif

/*** End of inlined file: data.h ***/


/*** Start of inlined file: pos.h ***/
#ifndef POS
#define POS

struct Pos {
	int16_t _pos;
	Pos() { _pos = 0; }
	Pos(int x, int y) : _pos(((x + BOARD_BOUNDARY) << 5) | (y + BOARD_BOUNDARY)) {}
	constexpr Pos(int pos) : _pos(pos) {}
	operator int() { return _pos; }
	constexpr int x() const { return (_pos >> 5) - BOARD_BOUNDARY; }
	constexpr int abx() const { return _pos >> 5; }
	constexpr int y() const { return (_pos & 31) - BOARD_BOUNDARY; }
	constexpr int aby() const { return _pos & 31; }
	static int dis1(Pos u, Pos v) {
		return (std::max)(std::abs(u.x() - v.x()), std::abs(u.y() - v.y()));
	}
	static int dis2(Pos u, Pos v) {
		int dx = std::abs(u.x() - v.x()), dy = std::abs(u.y() - v.y());
		if (dy == 0 || dx - dy == 0 || dx + dy == 0)return std::abs(dx);
		else if (dx == 0)return abs(dy);
	}
	bool operator ==(Pos pos) { return _pos == pos._pos; }
	bool operator !=(Pos pos) { return _pos != pos._pos; }
};

constexpr Pos NULLPOS = 0;

enum Dir : int16_t {
	D_U = -BOARD_LENGTH,
	D_L = -1,
	D_D = BOARD_LENGTH,
	D_R = 1,

	D_UL = D_U + D_L,
	D_UR = D_U + D_R,
	D_DL = D_D + D_L,
	D_DR = D_D + D_R,
};

constexpr int16_t D4[4] = { D_R, D_D, D_DR, D_UR };

[[maybe_unused]] constexpr int16_t EXPAND_S2L3[] = { //S2L3
	D_U * 3 + D_L * 3,
	D_U * 3,
	D_U * 3 + D_R * 3,

	D_U * 2 + D_L * 2,
	D_U * 2 + D_L,
	D_U * 2,
	D_U * 2 + D_R,
	D_U * 2 + D_R * 2,

	D_U + D_L * 2,
	D_U + D_L,
	D_U,
	D_U + D_R,
	D_U + D_R * 2,

	D_L * 3,
	D_L * 2,
	D_L,
	D_R,
	D_R * 2,
	D_R * 3,

	D_D + D_L * 2,
	D_D + D_L,
	D_D,
	D_D + D_R,
	D_D + D_R * 2,

	D_D * 2 + D_L * 2,
	D_D * 2 + D_L,
	D_D * 2,
	D_D * 2 + D_R,
	D_D * 2 + D_R * 2,

	D_D * 3 + D_L * 3,
	D_D * 3,
	D_D * 3 + D_R * 3,
};

[[maybe_unused]] constexpr int16_t EXPAND_S2L4[] = { //S2L4
	D_U * 4 + D_L * 4,
	D_U * 4,
	D_U * 4 + D_R * 4,

	D_U * 3 + D_L * 3,
	D_U * 3,
	D_U * 3 + D_R * 3,

	D_U * 2 + D_L * 2,
	D_U * 2 + D_L,
	D_U * 2,
	D_U * 2 + D_R,
	D_U * 2 + D_R * 2,

	D_U + D_L * 2,
	D_U + D_L,
	D_U,
	D_U + D_R,
	D_U + D_R * 2,

	D_L * 4,
	D_L * 3,
	D_L * 2,
	D_L,
	D_R,
	D_R * 2,
	D_R * 3,
	D_R * 4,

	D_D + D_L * 2,
	D_D + D_L,
	D_D,
	D_D + D_R,
	D_D + D_R * 2,

	D_D * 2 + D_L * 2,
	D_D * 2 + D_L,
	D_D * 2,
	D_D * 2 + D_R,
	D_D * 2 + D_R * 2,

	D_D * 3 + D_L * 3,
	D_D * 3,
	D_D * 3 + D_R * 3,

	D_D * 4 + D_L * 4,
	D_D * 4,
	D_D * 4 + D_R * 4,
};

#endif

/*** End of inlined file: pos.h ***/

constexpr int CAND_RANGE = 4;

struct Range {
	int x1 = 15, y1 = 15, x2 = -1, y2 = -1;

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
	int _length;
	int _cntMove;
	Piece _self;
	uint64_t hash;

	std::vector<BoardInfo> info;

	uint64_t codeLR[BOARD_LENGTH];				// L->R 低位->高位
	uint64_t codeUD[BOARD_LENGTH];				// U->D
	uint64_t codeMain[2 * BOARD_LENGTH - 1];	// x = y
	uint64_t codeVice[2 * BOARD_LENGTH - 1];	// x = -y
	void or2Bits(PieceCode p, Pos pos);
	void xor2Bits(PieceCode p, Pos pos);
	void and2Bits(PieceCode p, Pos pos);

	std::array<Piece, BOARD_SIZE> content;
	std::array<uint8_t, BOARD_SIZE> _cand;
	std::array<Unit, BOARD_SIZE> units;
	std::vector<std::array<Unit, 8 * Eval::HALF_LINE_LEN>> evalCache;

public:
	Board() { reset(); }

	bool notin(Pos pos) const {
		int x = pos.x(), y = pos.y();
		return x >= _length || y >= _length || x < 0 || y < 0;
	}
	int length() const { return _length; }
	int cntMove() const { return _cntMove; }

	Piece operator[](Pos pos) { return content[pos]; }
	FType type(Piece p, Pos pos) { return (FType)units[pos].fType[p]; }
	int value(Piece p, Pos pos) { return units[pos].fValue[p]; }
	bool cand(Pos pos) { return _cand[pos]; }

	Piece self() const { return _self; }
	int64_t hashKey() const { return hash; }
	Pos cost() const { return info[_cntMove].T4cost; }
	int cntFT(FType t, Piece p) const { return info[_cntMove].cntT[t][p]; }
	Range& candRange() { return info[_cntMove].range; }

	int staticEval() {
		return lastEval(0);
	}
	int lastEval(int r) {
		int id = (std::max)(1, _cntMove - r);

		int basicVal = (info[id].valueP1 + info[id - 1].valueP1) / 2;

		int threatVal = Eval::threatP1Eval(info[id].cntT);

		int val = basicVal + threatVal;
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

	void sssw(Piece p) { _self = p; }
};

#endif

/*** End of inlined file: board.h ***/


/*** Start of inlined file: tt.h ***/
#ifndef TT
#define TT

constexpr int TTBYTESIZE = 1024 * 1024 * 250;

class HashEntry {
	uint16_t _hashLow16;
	int16_t _value;
	uint16_t _movePos;
	int8_t _dep;
	uint8_t _gen : 5;
	uint8_t _PV : 1;
	uint8_t _type : 2;

public:
	HashEntry() { clear(); }
	Pos movePos() const { return _movePos; }
	bool isPV() const { return _PV; }
	uint16_t key() const { return _hashLow16; }
	HashType type() const { return static_cast<HashType>(_type); }
	uint8_t genaration() const { return _gen; }
	int value(int ply) const {
		if (_value == -VAL_INF) return -VAL_INF;
		if (_value >= WIN_CRITICAL) return _value - ply;
		if (_value <= -WIN_CRITICAL) return _value + ply;
		return _value;
	}
	int depth() const { return _dep; }
	void setGeneration(uint8_t gen) { _gen = gen; }
	void clear() { memset(this, 0, sizeof(HashEntry)); }
	void store(uint64_t hash, bool pv, Pos best, int score, int dep, int step, HashType type);
};

class HashTable {
	static constexpr int ClusterSize = 4;
	uint64_t numClusters;

	struct Cluster {
		HashEntry hashEntries[ClusterSize];
		inline void Clear() { for (int i = 0; i < ClusterSize; i++) hashEntries[i].clear(); }
	}*clusters;

	uint8_t _generation;

public:
	HashTable(int maxByteSize);

	~HashTable();

	void clear();

	uint8_t generation() const { return _generation; }
	void newGen() { _generation++; }

	HashEntry* firstEntry(uint64_t hashKey) {
		return clusters[mulhi64(hashKey, numClusters)].hashEntries;
	}

	bool probe(uint64_t hashKey, HashEntry*& tte);

	void prefetch(uint64_t hash) { ::prefetch(firstEntry(hash)); }
};

extern HashTable tt;

#endif

/*** End of inlined file: tt.h ***/


/*** Start of inlined file: test.h ***/
#ifndef TEST
#define TEST
#include <unordered_map>

enum Test {
	node,
	vcfnode,
	TTcutoff,
	betacutoff,
	vcfTTcutoff,
	vcfbetacutoff,
	moveCntpruning,
	dispersedT,
	TDH3T4H3wincheck,
	tothasherror,
	razor,
	futility = 60
};

extern double te[128];
void PrintTest();

#endif

/*** End of inlined file: test.h ***/

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

/*** End of inlined file: board.cpp ***/


/*** Start of inlined file: move.cpp ***/

/*** Start of inlined file: move.h ***/
#ifndef MOVE
#define MOVE

extern uint64_t mainHist[2][BOARD_SIZE];
extern Pos counterMove[2][BOARD_SIZE];

void clearStat();
void updateStat(Piece self, Pos pos, Pos lastPos, int dep);

enum GenMoveMod {
	G_threat,
	G_quiet,
	G_pseudoVCF,
	G_VCF,
	G_all
};

enum PickerMod {
	P_main,
	P_VCF
};

struct Move {
	Pos pos;
	int val;
	Move(Pos pos, int val) :pos(pos), val(val) {}
	bool operator < (const Move& a)const {
		return val < a.val;
	}
	bool operator > (const Move& a)const {
		return val > a.val;
	}
};

class MovePicker {
	Board& bd;
	int dep;

	Pos ttMove;
	int stage;
	std::vector<Move> moves;
	int cur;
public:
	MovePicker(PickerMod mod, Board &bd, Pos ttMove, int dep = 0);
	Pos nextMove(bool skipQuiets = false);
};

template<GenMoveMod mod>
void genMove(Board& bd, std::vector<Move>& moves);

#endif

/*** End of inlined file: move.h ***/


/*** Start of inlined file: search.h ***/
#ifndef SEARCH
#define SEARCH


/*** Start of inlined file: time.h ***/
#ifndef TIME
#define TIME

#include <chrono>

namespace Search {
	constexpr int MaxMsTime = 990;
	using namespace std::chrono;
	class Timer {
		const int Cycle = 64;
		int cntCheckClock;
		bool timeOut;
		time_point<steady_clock> start;
	public:

		Timer() {
			Reset();
		}
		void Reset() {
			cntCheckClock = 0;
			timeOut = false;
			start = steady_clock::now();
		}
		int time_out() {
			//return false;
			if (timeOut) return true;
			cntCheckClock++;
			if (cntCheckClock == Cycle) {
				cntCheckClock = 0;
				return timeOut = duration_cast<milliseconds>(steady_clock::now() - start).count() > MaxMsTime;
			}
			return false;
		}
	};
}

#endif

/*** End of inlined file: time.h ***/

namespace Search {

	constexpr int MAX_PLY = 128;
	constexpr int END_DEP = 127;

	constexpr int win(int ply) { return WIN_MAX - ply; }

	constexpr int loss(int ply) { return -WIN_MAX + ply; }

	enum NType {
		NonPV,
		PV,
		Root
	};

	struct Stack {
		int ply;
		Pos* pv;
		Pos excludedMove;
		int eval;
		bool ttpv;
		FType moveFT[2];
	};

	struct RootMove {
		Pos pos = NULLPOS;
		int val = -VAL_INF;
		int lastVal = -VAL_INF;
		int avg = -VAL_INF;
		std::vector<Pos> pv;
		RootMove(Pos pos, int val, int lastVal) :pos(pos), val(val), lastVal(lastVal) {}
	};
	extern std::vector<RootMove> rootMoves;

	class Worker {
		Timer timer;
		Board& bd;
		Stack ss;

		//std::vector<RootMove> rootMoves;
		int pvIdx = 0;
		void iterative_deepening();
		int search(NType NT, Stack* ss, int alpha, int beta, int dep, bool cutNode);
		int VCFSearch(NType NT, Stack* ss, int alpha, int beta, int dep);
		int quickEnd(Stack* ss);
	public:
		Worker(Board& bd) : bd(bd) {}
		void start();
	};
}

#endif

/*** End of inlined file: search.h ***/

uint64_t mainHist[2][BOARD_SIZE];
Pos counterMove[2][BOARD_SIZE];

void clearStat() {
	for (int i = 0; i < BOARD_SIZE; i++) {
		mainHist[P1][i] = mainHist[P2][i] = 0;
		counterMove[P1][i] = counterMove[P2][i] = NULLPOS;
	}
}

void updateStat(Piece self, Pos pos, Pos lastPos, int dep) {
	mainHist[self][pos] += 1ULL << std::clamp(dep, 0, 48);
	counterMove[self][lastPos] = pos;
}

enum MoveStage {

	M_main_tt,
	M_threat_init,
	M_threat,
	M_quiet_init,
	M_quiet,

	M_VCF_tt,
	M_VCF_init,
	M_VCF,

	M_all
};

MovePicker::MovePicker(PickerMod mod, Board& bd, Pos ttMove, int dep) :
	bd(bd),
	ttMove(ttMove),
	dep(dep),
	cur(0) {
	stage = (mod == P_main ? M_main_tt : M_VCF_tt) + (ttMove == NULLPOS);
	moves.reserve(bd.candRange().area() - bd.cntMove());
}

Pos MovePicker::nextMove(bool skipQuiets) {
	switch (stage) {

	case M_main_tt:
	case M_VCF_tt:

		stage++;
		return ttMove;

	case M_threat_init:

		genMove<G_threat>(bd, moves);
		stage++;
		[[fallthrough]];

	case M_threat:

		for (; cur < moves.size(); cur++) {
			std::swap(*std::max_element(moves.begin() + cur, moves.end()), moves[cur]);

			if (moves[cur].pos != ttMove) return moves[cur++].pos;
		}

		stage++;
		[[fallthrough]];

	case M_quiet_init:

		if (!skipQuiets) {
			genMove<G_quiet>(bd, moves);

			//insertion sort
			for (int i = cur; i < moves.size();) {

				//skip trivial move
				while (i < moves.size() &&
					dep <= 4 && moves[i].val < 5) {
					moves[i] = moves.back();
					moves.pop_back();
				}

				if (i < moves.size()) {
					int j = i;
					Move tmp = moves[i];
					for (; j >= cur + 1 && moves[j - 1] < tmp; j--) {
						std::swap(moves[j], moves[j - 1]);
					}
					moves[j] = tmp;
					i++;
				}
			}

		}

		stage++;
		[[fallthrough]];

	case M_quiet:

		for (; cur < moves.size(); cur++) {
			if (moves[cur].pos != ttMove) return moves[cur++].pos;
		}

		return NULLPOS;

	case M_VCF_init:

		genMove<G_pseudoVCF>(bd, moves);
		stage++;
		[[fallthrough]];

	case M_VCF:

		for (; cur < moves.size(); cur++) {
			std::swap(*std::max_element(moves.begin() + cur, moves.end()), moves[cur]);

			if (moves[cur].pos != ttMove) return moves[cur++].pos;
		}

		return NULLPOS;
	}
	assert(0);
	return 0;
}

template<GenMoveMod mod>
void genMove(Board& bd, std::vector<Move>& moves) {
	Piece self = bd.self(), oppo = ~self;

	auto score = [&self, &oppo, &bd](Pos pos) {
		int ret = 2 * bd.value(self, pos) + bd.value(oppo, pos);
		//ret += 7 * logf(mainHist[self][pos] + 1);
		//if (counterMove[self][bd.lastMove(1)] == pos) ret += 15;
		return ret;
		};

	if constexpr (mod == G_threat) {
		if (bd.cntFT(TH4, oppo)) {				//对方有成双4
			for (int i = bd.candRange().x1; i <= bd.candRange().x2; i++) {
				for (int j = bd.candRange().y1; j <= bd.candRange().y2; j++) {
					Pos pos(i, j);

					if (bd[pos] == Empty && bd.cand(pos) &&
						(bd.type(self, pos) >= T4 || bd.type(oppo, pos) >= T4)) {
						moves.emplace_back(pos, score(pos));
					}
				}
			}
		}
		else {
			for (int i = bd.candRange().x1; i <= bd.candRange().x2; i++) {
				for (int j = bd.candRange().y1; j <= bd.candRange().y2; j++) {
					Pos pos(i, j);

					if (bd[pos] == Empty && bd.cand(pos) &&
						(bd.type(self, pos) >= TH3 || bd.type(oppo, pos) >= TH3)) {
						moves.emplace_back(pos, score(pos));
					}
				}
			}
		}
	}
	if constexpr (mod == G_quiet) {
		for (int i = bd.candRange().x1; i <= bd.candRange().x2; i++) {
			for (int j = bd.candRange().y1; j <= bd.candRange().y2; j++) {
				Pos pos(i, j);

				if (bd[pos] == Empty && bd.cand(pos) &&
					bd.type(self, pos) == TNone && bd.type(oppo, pos) == TNone) {
					moves.emplace_back(pos, score(pos));
				}
			}
		}
	}
	if constexpr (mod == G_VCF) {
		assert(0);
	}
	if constexpr (mod == G_pseudoVCF) {
		Pos lastPos = bd.lastMove(1);
		constexpr int len = arrLen(EXPAND_S2L4);
		for (int i = 0; i < len; i++) {
			Pos pos = lastPos + EXPAND_S2L4[i];
			if (bd[pos] == Empty && bd.type(self, pos) >= T4) {
				moves.emplace_back(pos, score(pos));
			}
		}
	}
	if constexpr (mod == G_all) {
		for (int i = bd.candRange().x1; i <= bd.candRange().x2; i++) {
			for (int j = bd.candRange().y1; j <= bd.candRange().y2; j++) {
				Pos pos(i, j);
				if (bd[pos] == Empty && bd.cand(pos))
					moves.emplace_back(pos, 2 * bd.value(self, pos) + bd.value(oppo, pos));
			}
		}
	}
}

template void genMove<G_threat>(Board& bd, std::vector<Move>& moves);
template void genMove<G_quiet>(Board& bd, std::vector<Move>& moves);
template void genMove<G_pseudoVCF>(Board& bd, std::vector<Move>& moves);
template void genMove<G_VCF>(Board& bd, std::vector<Move>& moves);
template void genMove<G_all>(Board& bd, std::vector<Move>& moves);
/*** End of inlined file: move.cpp ***/


/*** Start of inlined file: search.cpp ***/
#include <iostream>

namespace Search {
	std::vector<RootMove> rootMoves;

	void updatePV(Pos* pv, Pos pos, Pos* childPV) {
		for (*pv++ = pos; childPV && *childPV;)
			*pv++ = *childPV++;
		*pv = NULLPOS;
	}

	int Worker::quickEnd(Stack* ss) {
		Piece self = bd.self(), oppo = ~self;

		if (bd.cntFT(T5, self)) return win(ss->ply);

		int oppo5 = bd.cntFT(T5, oppo);
		if (oppo5) {
			return oppo5 > 1 ? loss(ss->ply + 1) : 0;
		}

		if (bd.cntFT(TH4, self))return win(ss->ply + 2);

		if (bd.cntFT(TDH3, self) + bd.cntFT(T4H3, self) && !bd.cntFT(T4, oppo)) {
			te[TDH3T4H3wincheck]++;
			return win(ss->ply + 4);
		}

		return 0;
	}

	int Worker::VCFSearch(NType NT, Stack* ss, int alpha, int beta, int dep) {

		te[vcfnode]++;

		bool pvNode = NT != NonPV;
		Pos pv[MAX_PLY + 1];

		Piece self = bd.self(), oppo = ~self;
		int oppoT5 = bd.cntFT(T5, oppo);
		int val;

		assert(-WIN_MAX <= alpha && alpha < beta && beta <= WIN_MAX);
		assert(pvNode || (alpha == beta - 1));

		//检查平局
		if (bd.candRange().area() - bd.cntMove() == 0) return 0;

		//max ply
		if (ss->ply > MAX_PLY)return bd.staticEval();

		//检查赢/输棋
		if (int q = quickEnd(ss); q) return q;

		//mate distance pruning
		alpha = std::max(loss(ss->ply), alpha);
		beta = std::min(win(ss->ply + 1), beta);
		if (alpha >= beta) return alpha;

		//T5跳过节点
		if (oppoT5) {
			Pos move = bd.cost();

			bd.update(move);
			tt.prefetch(bd.hashKey());

			if (pvNode) {
				pv[0] = NULLPOS;
				(ss + 1)->pv = pv;

				val = -VCFSearch(PV, ss + 1, -beta, -alpha, dep);
			}
			else val = -VCFSearch(NonPV, ss + 1, -beta, -alpha, dep);

			bd.undo();

			if (pvNode) updatePV(ss->pv, move, (ss + 1)->pv);

			return val;
		}

		int bestVal = -VAL_INF;
		Pos bestMove = NULLPOS;

		uint64_t key = bd.hashKey();
		HashEntry* tte;
		int ttVal = -VAL_INF;
		Pos ttMove = NULLPOS;
		int ttDep = -VAL_INF;
		bool pvHit = false;
		HashType ttBound = B_Initial;

		//访问置换表
		bool ttHit = tt.probe(key, tte);
		if (ttHit) {
			ttVal = tte->value(ss->ply);

			ttMove = tte->movePos();
			if (ttMove != NULLPOS && bd[tte->movePos()] != Empty) {
				ttMove = NULLPOS;
				te[tothasherror]++;
			}

			ttDep = tte->depth();
			pvHit = tte->isPV();
			ttBound = tte->type();
		}

		//使用置换表中的值截断
		if (ttDep >= dep && ttVal != -VAL_INF) {
			if (ttBound & B_Lower)
				alpha = std::max(alpha, ttVal);
			if (ttBound & B_Upper)
				beta = std::min(beta, ttVal);
			if (alpha >= beta) {
				te[vcfTTcutoff]++;
				return ttVal;
			}
		}

		bestVal = bd.staticEval();
		if (bestVal >= beta) {
			return bestVal;
		}

		if (pvNode) alpha = std::max(alpha, bestVal);

		MovePicker mp(P_VCF, bd, NULLPOS);
		for (Pos move; move = mp.nextMove(); ) {

			bd.update(move);

			if (pvNode) {
				pv[0] = NULLPOS;
				(ss + 1)->pv = pv;
			}

			val = -VCFSearch(NT, ss + 1, -beta, -alpha, dep - 2);

			bd.undo();

			if (timer.time_out())return 0;

			if (val > bestVal) {
				bestVal = val;

				if (val > alpha) {
					bestMove = move;

					// Update pv even in fail-high case
					if (pvNode) updatePV(ss->pv, move, (ss + 1)->pv);

					if (pvNode && val < beta)  // Update alpha
						alpha = val;
					else {
						te[vcfbetacutoff]++;
						break;
					}
				}
			}
		}

		HashType bound = bestVal >= beta ? B_Lower : B_Upper;
		tte->store(key, pvHit, bestMove, bestVal, dep, ss->ply, bound);

		return bestVal;
	}

	int Worker::search(NType NT, Stack* ss, int alpha, int beta, int dep, bool cutNode) {

		bool pvNode = NT != NonPV;
		bool rootNode = NT == Root;
		Pos pv[MAX_PLY + 1];

		Piece self = bd.self(), oppo = ~self;
		int oppoT5 = bd.cntFT(T5, oppo);
		int oppoTH4 = bd.cntFT(TH4, oppo);
		int val;

		//深度小于零搜索VCF
		if (dep <= 0)
			return VCFSearch(NT, ss, alpha, beta, 0);

		assert(-WIN_MAX <= alpha && alpha < beta && beta <= WIN_MAX);
		assert(pvNode || (alpha == beta - 1));

		te[node]++;

		if (!rootNode) {
			//检查平局
			if (bd.candRange().area() - bd.cntMove() == 0)
				return 0;

			//max ply
			if (ss->ply > MAX_PLY)
				return bd.staticEval();

			//检查赢/输棋
			if (val = quickEnd(ss))
				return val;

			//mate distance pruning
			alpha = std::max(loss(ss->ply), alpha);
			beta = std::min(win(ss->ply + 1), beta);
			if (alpha >= beta)
				return alpha;

			//T5跳过节点
			if (oppoT5) {
				Pos move = bd.cost();

				bd.update(move);
				tt.prefetch(bd.hashKey());

				if (pvNode) {
					pv[0] = NULLPOS;
					(ss + 1)->pv = pv;

					val = -search(PV, ss + 1, -beta, -alpha, dep, false);
				}
				else val = -search(NonPV, ss + 1, -beta, -alpha, dep, !cutNode);

				bd.undo();

				if (pvNode && !rootNode)
					updatePV(ss->pv, move, (ss + 1)->pv);

				return val;
			}
		}

		int bestVal = -VAL_INF;
		Pos bestMove = NULLPOS;
		int moveCnt = 0;

		uint64_t key = bd.hashKey();
		int ttVal = -VAL_INF;
		Pos ttMove = NULLPOS;
		int ttDep = -VAL_INF;
		HashType ttBound = B_Initial;
		HashEntry* tte = nullptr;

		//估值
		int eval = rootNode ? -VAL_INF : bd.staticEval();
		bool improving = eval - bd.lastEval(2) > 0;

		//访问置换表
		bool ttHit = tt.probe(key, tte);
		if (ttHit) {
			ttVal = tte->value(ss->ply);
			ttMove = tte->movePos();
			if (ttMove != NULLPOS && bd[tte->movePos()] != Empty) {
				ttMove = NULLPOS;
				te[tothasherror]++;
			}
			ttDep = tte->depth();
			ttBound = tte->type();

			if (!ss->excludedMove)
				ss->ttpv = pvNode || tte->isPV();
		}

		if (!pvNode && !ss->excludedMove && ttDep > dep &&
			std::abs(ttVal) < WIN_CRITICAL &&
			(ttVal >= beta ? (ttBound & B_Lower) : (ttBound & B_Upper))) {
			te[TTcutoff]++;
			return ttVal;
		}

		//razoring
		if (dep <= 4 && eval + 36 * dep * dep + 55 < alpha) {
			te[razor + dep]++;
			return VCFSearch(NonPV, ss, alpha, alpha + 1, 0);
		}

		//futility pruning
		if (!ss->ttpv && !oppoTH4 && beta > -WIN_CRITICAL &&
			eval - (5 * dep * dep + (30 - 4 * improving - 4 * (!ttHit && cutNode)) * dep + 70) > beta) {
			te[futility + dep]++;
			return eval;
		}

		//内部迭代加深
		if (!rootNode && pvNode && !ttMove)
			dep -= 3;

		if (dep <= 0)
			return VCFSearch(NT, ss, alpha, beta, 0);

		if (dep >= 9 && cutNode && !ttMove)
			dep -= 3;

		//尝试所有着法直到产生beta截断
		MovePicker mp(P_main, bd, ttMove);
		bool skipQuietMove = false;

		for (Pos move; move = mp.nextMove(skipQuietMove); ) {

			if (rootNode && !std::count_if(rootMoves.begin() + pvIdx, rootMoves.end(), [&move](auto& a) {return move == a.pos;}))
				continue;

			if (move == ss->excludedMove)continue;

			assert(bd[move] == Empty);

			moveCnt++;
			ss->moveFT[P1] = bd.type(P1, move);
			ss->moveFT[P2] = bd.type(P2, move);

			int disSelf = Pos::dis1(bd.lastMove(1), move);
			int disOppo = Pos::dis1(bd.lastMove(2), move);
			bool dispersed = disSelf > 4 && disOppo > 4;

			//pruning at shallow depth
			if (!rootNode && bestVal > -WIN_CRITICAL) {
				//moveCount pruning
				if (!skipQuietMove && moveCnt > (improving ? 0.8f : .6f) * dep * dep + 10) {
					te[moveCntpruning]++;
					skipQuietMove = true;
				}

				//prune the dispersed move
				if (dispersed && oppoTH4 && ss->moveFT[oppo] < T4 && dep <= 4) {
					te[dispersedT]++;
					continue;
				}
			}

			//extensions
			int extension = 0;

			//extent when oppo threats
			if(oppoTH4)
				extension += 1;

			// Singular extension search. If all moves but one fail low on a
			// search of (alpha-s, beta-s), and just one fails high on (alpha, beta),
			// then that move is singular and should be extended. To verify this we do
			// a reduced search on the position excluding the ttMove and if the result
			// is lower than ttValue minus a margin, then we will extend the ttMove.
			if (!rootNode && move == ttMove && !ss->excludedMove &&
				dep >= 8 + 2 * ss->ttpv &&
				std::abs(ttVal) < WIN_CRITICAL &&
				(ttBound & B_Lower) &&
				ttDep >= dep - 3) {

				int singularBeta = ttVal - (3 + (ss->ttpv && !pvNode)) * dep;
				int singularDep = dep / 2;

				ss->excludedMove = move;
				val = search(NonPV, ss, singularBeta - 1, singularBeta, singularDep, cutNode);
				ss->excludedMove = NULLPOS;

				if (val < singularBeta) {
					extension += 3;
				}
				// Multi-cut pruning
				// Our ttMove is assumed to fail high based on the bound of the TT entry,
				// and if after excluding the ttMove with a reduced search we fail high over the original beta,
				// we assume this expected cut-node is not singular (multiple moves fail high),
				// and we can prune the whole subtree by returning a softbound.
				else if (singularBeta >= beta)
					return singularBeta;
				// Negative extensions
				// If other moves failed high over (ttValue - margin) without the ttMove on a reduced search,
				// but we cannot do multi-cut because (ttValue - margin) is lower than the original beta,
				// we do not know if the ttMove is singular or can do a multi-cut,
				// so we reduce the ttMove in favor of other moves based on some conditions:
				else if (ttVal >= beta)
					extension -= 2;
			}

			int newDep = dep + extension - 2;

			bd.update(move);
			if (ss->moveFT[self] != T4)
				tt.prefetch(bd.hashKey());

			//late move reduction
			int reduction = (logf(dep) * logf(moveCnt) + 1.5 * !improving) / 3.5;

			//Decrease reduction if position is or has been on the PV
			if (ss->ttpv)
				reduction -= 2;

			//Increase reduction for cut nodes
			if (cutNode)
				reduction += 2;

			//Increase reduction for useless defend move
			if (oppoTH4 && ss->moveFT[self] < T4)
				reduction += disSelf > 4 + 2 * (disOppo > 4);

			//Decrease reduction for continous attack
			if (!oppoTH4 && (ss - 2)->moveFT[self] >= TH3 && ss->moveFT[self] >= TH3)
				reduction -= 1;

			bool fullDep;
			if (reduction > 0 &&
				newDep > reduction &&
				moveCnt > 1 + rootNode) {
				val = -search(NonPV, ss + 1, -alpha - 1, -alpha, newDep - reduction, true);
				fullDep = val > alpha;
			}
			else fullDep = !pvNode || moveCnt > 1;

			//full depth seach
			if (fullDep) {
				val = -search(NonPV, ss + 1, -alpha - 1, -alpha, newDep, !cutNode);
			}

			if (pvNode && (moveCnt == 1 || val > alpha)) {

				pv[0] = NULLPOS;
				(ss + 1)->pv = pv;

				val = -search(PV, ss + 1, -beta, -alpha, newDep, false);
			}

			bd.undo();

			if (timer.time_out()) return 0;

			if (rootNode) {
				RootMove& rm = *std::find_if(rootMoves.begin(), rootMoves.end(), [&move](auto& a) {return a.pos == move; });

				rm.avg = rm.avg == -VAL_INF ? val : (2 * val + rm.avg) / 3;

				if (moveCnt == 1 || val > alpha) {

					rm.val = val;

					rm.pv.clear();
					for (Pos* p = (ss + 1)->pv; *p; p++)
						rm.pv.push_back(*p);
				}
				else rm.val = -VAL_INF;
			}

			if (val > bestVal) {
				bestVal = val;

				if (val > alpha) {
					bestMove = move;

					// Update pv even in fail-high case
					if (pvNode && !rootNode) updatePV(ss->pv, move, (ss + 1)->pv);

					if (pvNode && val < beta)  // Update alpha
						alpha = val;
					else {
						te[betacutoff]++;
						break;
					}
				}
			}
		}

		if (bestMove)updateStat(self, bestMove, bd.lastMove(1), dep);

		HashType bound = bestVal >= beta ? B_Lower :
			pvNode && bestMove ? B_Exact : B_Upper;
		tte->store(key, ss->ttpv, bestMove, bestVal, dep, ss->ply, bound);

		return bestVal;
	}

	void Worker::iterative_deepening() {

		int bestVal = -VAL_INF;
		std::vector<Move> moves;
		genMove<G_all>(bd, moves);
		int multiPV = 1;
		bool stop = false;

		Stack stack[MAX_PLY + 10] = {};
		Stack* ss = stack + 5;

		for (int i = 0; i < MAX_PLY + 5; i++)
			(ss + i)->ply = i;

		for (auto& move : moves) {
			rootMoves.emplace_back(move.pos, -VAL_INF, -VAL_INF);
		}
		auto cmpRootMove = [](const RootMove& a, const RootMove& b) {
			return a.val == b.val ? a.lastVal > b.lastVal : a.val > b.val;
		};
		multiPV = std::min(multiPV, (int)rootMoves.size());

		for (int dep = 5, endDep = VAL_INF; !stop && dep <= std::min(END_DEP, endDep); dep++) {

			for (auto& rm : rootMoves)
				rm.lastVal = rm.val;

			for (pvIdx = 0; !stop && pvIdx < multiPV; pvIdx++) {

				//aspiration search
				auto& rm = rootMoves[pvIdx];
				int delta = std::min(std::abs(rm.avg) / 14 + 11, VAL_INF);
				int alpha = std::max(rm.avg - delta, -WIN_MAX);
				int beta = std::min(rm.avg + delta, WIN_MAX);
				int failHighCnt = 0;

				while (true) {

					bestVal = search(Root, ss, alpha, beta, dep - failHighCnt / 4, false);
					printf("d: %d   ab: [%d, %d]   v: %d\n", dep, alpha, beta, bestVal);

					std::stable_sort(rootMoves.begin() + pvIdx, rootMoves.end(), cmpRootMove);

					if (timer.time_out()) {
						stop = true;
						break;
					}
					//After finding the checkmate, search slightly deeper in an attempt to find a better solution
					if (bestVal >= WIN_CRITICAL && endDep == VAL_INF)
						endDep = dep + 3;

					if (bestVal <= alpha) {
						beta = (alpha + beta) / 2;
						alpha = std::max(bestVal - delta, -WIN_MAX);
						failHighCnt = 0;
					}
					else if (bestVal >= beta) {
						beta = std::min(bestVal + delta, WIN_MAX);
						failHighCnt++;
					}
					else break;

					delta = std::min(delta * 4 / 3, VAL_INF);

					assert(-WIN_MAX <= alpha && alpha < beta && beta <= WIN_MAX);
				}

				std::stable_sort(rootMoves.begin(), rootMoves.begin() + pvIdx + 1, cmpRootMove);
			}
		}
	}

	void Worker::start() {
		rootMoves.clear();

		if (bd.cntMove() == 0) {
			rootMoves.push_back(RootMove({ 7,7 }, 0, 0));
			return;
		}
		if (bd.cntFT(T5, bd.self())) {
			std::vector<Move> moves;
			genMove<G_all>(bd, moves);
			Pos pos = find_if(moves.begin(), moves.end(), [this](auto& a) {return bd.type(bd.self(), a.pos) == T5; })->pos;
			rootMoves.push_back(RootMove(pos, 0, 0));
			return;
		}
		iterative_deepening();
	}
}

/*** End of inlined file: search.cpp ***/


/*** Start of inlined file: tt.cpp ***/
#include <random>
#include <iostream>

void HashEntry::store(uint64_t hash, bool pv, Pos best, int val, int dep, int step, HashType type) {
	uint16_t key16 = static_cast<uint16_t>(hash);

	if (_hashLow16 != key16 || dep >= _dep || type == B_Exact) {
		_PV = pv;
		if (val >= WIN_CRITICAL) val += step;
		else if (val <= -WIN_CRITICAL) val -= step;
		_value = val;
		_dep = dep;
		_gen = tt.generation();
		_type = type;
		_movePos = best;
		_hashLow16 = key16;
	}
}

HashTable::HashTable(int maxByteSize) {
	numClusters = maxByteSize / sizeof(Cluster);
	clusters = new Cluster[numClusters];
	_generation = 0;
}

HashTable::~HashTable() {
	delete[] clusters;
}

void HashTable::clear() {
	for (int i = 0; i < numClusters; i++)
		clusters[i].Clear();
	_generation = 0;
}

bool HashTable::probe(uint64_t hashKey, HashEntry*& tte) {

	HashEntry* entry = firstEntry(hashKey);
	uint16_t key16 = static_cast<uint16_t>(hashKey);

	for (int i = 0; i < ClusterSize; i++) {
		if (entry[i].key() == key16) {
			entry[i].setGeneration(_generation);
			tte = entry;
			return true;
		}
		if (entry[i].type() == B_Initial) {
			entry[i].setGeneration(_generation);
			tte = entry;
			return false;
		}
	}

	HashEntry* replace = entry;
	for (int i = 1; i < ClusterSize; i++) {
		if (replace->depth() - (32 + _generation - replace->genaration()) >
			entry[i].depth() - (32 + _generation - entry[i].genaration()))
			replace = entry + i;
	}
	tte = replace;
	return false;
}

HashTable tt(TTBYTESIZE);

/*** End of inlined file: tt.cpp ***/


/*** Start of inlined file: test.cpp ***/
#include <string>
#include <iostream>
#include <algorithm>
#include <vector>

double te[128];

void PrintTest() {
	std::cout << "node: " << te[node] / 1000.f / 0.990 << " k nodes/s\n";
	std::cout << "vcfnode: " << te[vcfnode] / 1000.f / 0.990 << " k nodes/s\n";
	std::cout << "totnode: " << (te[vcfnode] + te[node]) / 1000.f / 0.990 << " k nodes/s\n";
	te[node] = te[vcfnode] = 0;
	std::cout << "TT cutoff: " << te[TTcutoff] << "\n";
	te[TTcutoff] = 0;
	std::cout << "beta cutoff: " << te[betacutoff] << "\n";
	te[betacutoff] = 0;
	std::cout << "vcf TT cutoff: " << te[vcfTTcutoff] << "\n";
	te[vcfTTcutoff] = 0;
	std::cout << "vcf beta cutoff: " << te[vcfbetacutoff] << "\n";
	te[vcfbetacutoff] = 0;
	std::cout << "moveCnt pruning: " << te[moveCntpruning] << "\n";
	te[moveCntpruning] = 0;
	std::cout << "dispersed T: " << te[dispersedT] << "\n";
	te[dispersedT] = 0;
	std::cout << "TDH3 T4H3 win check: " << te[TDH3T4H3wincheck] << "\n";
	te[TDH3T4H3wincheck] = 0;
	std::cout << "tot hash error: " << te[tothasherror] << "\n";

	std::cout << "razor:\n";
	for (int i = 0; i < 32; i++) {
		std::cout << te[razor + i] << " ";
		te[razor + i] = 0;
	}
	std::cout << "\n";

	std::cout << "futility:\n";
	for (int i = 0; i < 32; i++) {
		std::cout << te[futility + i] << " ";
		te[futility + i] = 0;
	}
	std::cout << "\n";
}
//std::cout << "up\n";
//std::cout << " now:" << pos.first << " " << pos.second << "\n";
//for (int ii = 0; ii < 15; ii++) {
//	for (int jj = 0; jj < 15; jj++) {
//		if (board[ii][jj] == P1)
//			std::cout << "* ";
//		else if (board[ii][jj] == P2)
//			std::cout << "# ";
//		else std::cout << "- ";
//	}
//	std::cout << "\n";
//}

	/*void get1(int x, int y) {                   //debug
		std::cout << (codeLR[x] >> (2 * y) & 3);
	}
	void get2(int x, int y) {
		std::cout << (codeUD[y] >> (2 * x) & 3);
	}
	void get3(int x, int y) {
		std::cout << (codeMain[x - y + BOARD_LENGTH - 1] >> (2 * y) & 3);
	}
	void get4(int x, int y) {
		std::cout << (codeVice[x + y] >> (2 * y) & 3);
	}*/
/*** End of inlined file: test.cpp ***/


/*** Start of inlined file: pisqpipe.cpp ***/
/** don't modify this file */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

/*** Start of inlined file: pisqpipe.h ***/
#ifndef PISQPIPE
#define PISQPIPE
/** header with functions and variables for pipe AI */

namespace Piskvork {
	/* information about a game - you should use these variables */
	extern int width, height; /* the board size */
	extern int info_timeout_turn;  /* time for one turn in milliseconds */
	extern int info_timeout_match; /* total time for a game */
	extern int info_time_left;  /* remaining time for a game */
	extern int info_max_memory; /* maximum memory in bytes, zero if unlimited */
	extern int info_game_type;  /* 0:human opponent, 1:AI opponent, 2:tournament, 3:network tournament */
	extern int info_exact5;     /* 0:five or more stones win, 1:exactly five stones win */
	extern int info_renju;      /* 0:gomoku, 1:renju */
	extern int info_caro;       /* 0:gomoku, 1:Caro */
	extern int info_continuous; /* 0:single game, 1:continuous */
	extern int terminateAI; /* return from brain_turn when terminate>0 */
	extern unsigned start_time; /* tick count at the beginning of turn */
	extern char dataFolder[256]; /* folder for persistent files */

	/* you have to implement these functions */
	void brain_init(); /* create the board and call pipeOut("OK"); or pipeOut("ERROR Maximal board size is .."); */
	void brain_restart(); /* delete old board, create new board, call pipeOut("OK"); */
	void brain_turn(); /* choose your move and call do_mymove(x,y);
						  0<=x<width, 0<=y<height */
	void brain_my(int x, int y); /* put your move to the board */
	void brain_opponents(int x, int y); /* put opponent's move to the board */
	void brain_block(int x, int y); /* square [x,y] belongs to a winning line (when info_continuous is 1) */
	int brain_takeback(int x, int y); /* clear one square; return value: 0:success, 1:not supported, 2:error */
	void brain_end();  /* delete temporary files, free resources */

#ifdef DEBUG_EVAL
	void brain_eval(int x, int y); /* display evaluation of square [x,y] */
#endif

#ifndef ABOUT_FUNC
	extern const char* infotext; /* AI identification (copyright, version) */
#else
	void brain_about(); /* call pipeOut(" your AI info ") */
#endif

	/* these functions are in pisqpipe.cpp */
	int pipeOut(char* fmt, ...);
	void do_mymove(int x, int y);
	void suggest(int x, int y);
	void pisqpipe();
}

#endif

/*** End of inlined file: pisqpipe.h ***/


namespace Piskvork {
	int width, height; /* the board size */
	int info_timeout_turn = 1000000; /* time for one turn in milliseconds */
	int info_timeout_match = 1000000000; /* total time for a game */
	int info_time_left = 1000000000; /* left time for a game */
	int info_max_memory = 0; /* maximum memory in bytes, zero if unlimited */
	int info_game_type = 1; /* 0:human opponent, 1:AI opponent, 2:tournament, 3:network tournament */
	int info_exact5 = 0; /* 0:five or more stones win, 1:exactly five stones win */
	int info_renju = 0; /* 0:gomoku, 1:renju */
	int info_caro = 0; /* 0:gomoku, 1:Caro */
	int info_continuous = 0; /* 0:single game, 1:continuous */
	int terminateAI; /* return from brain_turn when terminateAI>0 */
	unsigned start_time; /* tick count at the beginning of turn */
	char dataFolder[256]; /* folder for persistent files */

	static char cmd[256];
	static HANDLE event1, event2;

	/** write a line to STDOUT */
	int pipeOut(char* fmt, ...)
	{
		int i;
		va_list va;
		va_start(va, fmt);
		i = vprintf(fmt, va);
		putchar('\n');
		fflush(stdout);
		va_end(va);
		return i;
	}

	/** read a line from STDIN */
	static void get_line()
	{
		int c, bytes;

		bytes = 0;
		do {
			c = getchar();
			if (c == EOF) exit(0);
			if (bytes < sizeof(cmd)) cmd[bytes++] = (char)c;
		} while (c != '\n');
		cmd[bytes - 1] = 0;
		if (cmd[bytes - 2] == '\r') cmd[bytes - 2] = 0;
	}

	/** parse coordinates x,y */
	static int parse_coord(const char* param, int* x, int* y)
	{
		if (sscanf(param, "%d,%d", x, y) != 2 ||
			*x < 0 || *y < 0 || *x >= width || *y >= height) {
			return 0;
		}
		return 1;
	}

	/** parse coordinates x,y and player number z */
	static void parse_3int_chk(const char* param, int* x, int* y, int* z)
	{
		if (sscanf(param, "%d,%d,%d", x, y, z) != 3 || *x < 0 || *y < 0 ||
			*x >= width || *y >= height) *z = 0;
	}

	/** return pointer to word after command if input starts with command, otherwise return NULL */
	static const char* get_cmd_param(const char* command, const char* input)
	{
		int n1, n2;
		n1 = (int)strlen(command);
		n2 = (int)strlen(input);
		if (n1 > n2 || _strnicmp(command, input, n1)) return NULL; /* it is not command */
		input += strlen(command);
		while (isspace(input[0])) input++;
		return input;
	}

	/** send suggest */
	void suggest(int x, int y)
	{
		pipeOut("SUGGEST %d,%d", x, y);
	}

	/** write move to the pipe and update internal data structures */
	void do_mymove(int x, int y)
	{
		brain_my(x, y);
		pipeOut("%d,%d", x, y);
	}

	/** main function for the working thread */
	static DWORD WINAPI threadLoop(LPVOID)
	{
		for (;;) {
			WaitForSingleObject(event1, INFINITE);
			brain_turn();
			SetEvent(event2);
		}
	}

	/** start thinking */
	static void turn()
	{
		terminateAI = 0;
		ResetEvent(event2);
		SetEvent(event1);
	}

	/** stop thinking */
	static void stop()
	{
		terminateAI = 1;
		WaitForSingleObject(event2, INFINITE);
	}

	static void start()
	{
		start_time = GetTickCount();
		stop();
		if (!width) {
			width = height = 20;
			brain_init();
		}
	}

	/** do command cmd */
	static void do_command()
	{
		const char* param;
		const char* info;
		char* t;
		int x, y, who, e;

		if ((param = get_cmd_param("info", cmd)) != 0) {
			if ((info = get_cmd_param("max_memory", param)) != 0) info_max_memory = atoi(info);
			if ((info = get_cmd_param("timeout_match", param)) != 0) info_timeout_match = atoi(info);
			if ((info = get_cmd_param("timeout_turn", param)) != 0) info_timeout_turn = atoi(info);
			if ((info = get_cmd_param("time_left", param)) != 0) info_time_left = atoi(info);
			if ((info = get_cmd_param("game_type", param)) != 0) info_game_type = atoi(info);
			if ((info = get_cmd_param("rule", param)) != 0) { e = atoi(info); info_exact5 = e & 1; info_continuous = (e >> 1) & 1; info_renju = (e >> 2) & 1; info_caro = (e >> 3) & 1; }
			if ((info = get_cmd_param("folder", param)) != 0) strncpy(dataFolder, info, sizeof(dataFolder) - 1);
#ifdef DEBUG_EVAL
			if ((info = get_cmd_param("evaluate", param)) != 0) { if (parse_coord(info, &x, &y)) brain_eval(x, y); }
#endif
			/* unknown info is ignored */
		}
		else if ((param = get_cmd_param("start", cmd)) != 0) {
			if (sscanf(param, "%d", &width) != 1 || width < 5) {
				width = 0;
				pipeOut("ERROR bad START parameter");
			}
			else {
				height = width;
				start();
				brain_init();
			}
		}
		else if ((param = get_cmd_param("rectstart", cmd)) != 0) {
			if (sscanf(param, "%d ,%d", &width, &height) != 2 || width < 5 || height < 5) {
				width = height = 0;
				pipeOut("ERROR bad RECTSTART parameters");
			}
			else {
				start();
				brain_init();
			}
		}
		else if ((param = get_cmd_param("restart", cmd)) != 0) {
			start();
			brain_restart();
		}
		else if ((param = get_cmd_param("turn", cmd)) != 0) {
			start();
			if (!parse_coord(param, &x, &y)) {
				pipeOut("ERROR bad coordinates");
			}
			else {
				brain_opponents(x, y);
				turn();
			}
		}
		else if ((param = get_cmd_param("play", cmd)) != 0) {
			start();
			if (!parse_coord(param, &x, &y)) {
				pipeOut("ERROR bad coordinates");
			}
			else {
				do_mymove(x, y);
			}
		}
		else if ((param = get_cmd_param("begin", cmd)) != 0) {
			start();
			turn();
		}
		else if ((param = get_cmd_param("about", cmd)) != 0) {
#ifdef ABOUT_FUNC
			brain_about();
#else
			pipeOut("%s", infotext);
#endif
		}
		else if ((param = get_cmd_param("end", cmd)) != 0) {
			stop();
			brain_end();
			exit(0);
		}
		else if ((param = get_cmd_param("board", cmd)) != 0) {
			start();
			for (;;) { /* fill the whole board */
				get_line();
				parse_3int_chk(cmd, &x, &y, &who);
				if (who == 1) brain_my(x, y);
				else if (who == 2) brain_opponents(x, y);
				else if (who == 3) brain_block(x, y);
				else {
					if (_stricmp(cmd, "done")) pipeOut("ERROR x,y,who or DONE expected after BOARD");
					break;
				}
			}
			turn();
		}
		else if ((param = get_cmd_param("takeback", cmd)) != 0) {
			start();
			t = "ERROR bad coordinates";
			if (parse_coord(param, &x, &y)) {
				e = brain_takeback(x, y);
				if (e == 0) t = "OK";
				else if (e == 1) t = "UNKNOWN";
			}
			pipeOut(t);
		}
		else {
			pipeOut("UNKNOWN command");
		}
	}

	void pisqpipe()
	{
		DWORD mode;
		if (GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode))
			puts("MESSAGE Gomoku AI should not be started directly. Please install gomoku manager (https://sourceforge.net/projects/piskvork). Then enter path to this exe file in players settings.");

#ifdef DEBUG
		SetErrorMode(0);
#endif
		DWORD tid;
		event1 = CreateEvent(0, FALSE, FALSE, 0);
		CreateThread(0, 0, threadLoop, 0, 0, &tid);
		event2 = CreateEvent(0, TRUE, TRUE, 0);
		for (;;) {
			get_line();
			do_command();
		}
	}

}

/*** End of inlined file: pisqpipe.cpp ***/


/*** Start of inlined file: example.cpp ***/
#include <windows.h>

namespace Piskvork {
	const char* infotext = "name=\"Random\", author=\"Petr Lastovicka\", version=\"3.2\", country=\"Czech Republic\", www=\"https://plastovicka.github.io\"";

	Board bd;

	void brain_init() {
		if (width > MAX_GAME_LENGTH || height > MAX_GAME_LENGTH) {
			width = height = 0;
			pipeOut("ERROR Maximal board size is %d", MAX_GAME_LENGTH);
			return;
		}
		bd.reset(width);
		pipeOut("OK");
	}

	void brain_restart(){
		bd.reset(width);
		pipeOut("OK");
	}

	int isFree(int x, int y) {
		return !bd.notin(Pos(x, y)) && bd[Pos(x, y)] == Empty;
	}

	void brain_my(int x, int y) {
		if (isFree(x, y)) bd.update(Pos(x, y));
		else pipeOut("ERROR my move [%d,%d]", x, y);
	}

	void brain_opponents(int x, int y) {
		if (isFree(x, y)) bd.update(Pos(x, y));
		else pipeOut("ERROR opponents's move [%d,%d]", x, y);
	}

	void brain_block(int x, int y) {
		if (isFree(x, y)) bd.update(Pos(x, y));
		else  pipeOut("ERROR winning move [%d,%d]", x, y);
	}

	int brain_takeback(int x, int y){
		if (bd.cntMove()) {
			bd.undo();
			return 0;
		}
		return 2;
	}

	void brain_turn() {
		Search::Worker search(bd);
		search.start();
		Pos pos = Search::rootMoves[0].pos;
		do_mymove(pos.x(), pos.y());
	}

	void brain_end() {}

#ifdef DEBUG_EVAL
#include <windows.h>

	void brain_eval(int x, int y)
	{
		HDC dc;
		HWND wnd;
		RECT rc;
		char c;
		wnd = GetForegroundWindow();
		dc = GetDC(wnd);
		GetClientRect(wnd, &rc);
		c = (char)(board[x][y] + '0');
		TextOut(dc, rc.right - 15, 3, &c, 1);
		ReleaseDC(wnd, dc);
	}

#endif
}

/*** End of inlined file: example.cpp ***/


/*** Start of inlined file: main.cpp ***/

/*** Start of inlined file: testGUI.h ***/
#ifndef TESTGUI
#define TESTGUI

#include<graphics.h>
#include<ctime>
#include <conio.h>
#include <gdiplus.h>
#include <vector>
#include <iostream>
#pragma comment(lib, "gdiplus.lib")

class button {
	char text[30] = {};
	bool isin = 0, isclick = 0;
	int mx = 0, my = 0, mw = 0, mh = 0, mtype = 0;

public:
	void set_text(char s[], int size) {
		memcpy(text, s, size);
	}
	void set(int x, int y, int w, int h, int type) {
		mx = x, my = y, mw = w, mh = h, mtype = type;
	}
	bool update(ExMessage& msg) {
		bool up = 0;
		isin = msg.x >= mx && msg.x <= mx + mw && msg.y >= my && msg.y <= my + mh;
		if (isin && msg.message == WM_LBUTTONDOWN) isclick = 1;
		if (!isin) isclick = 0;
		int oldone = isclick;
		if (isin && msg.message == WM_LBUTTONUP) {
			isclick = 0;
			up = oldone && !isclick;
		}
		return up;
	}
	bool show(ExMessage& msg) {
		bool play = !isclick;
		bool up = update(msg);
		setlinecolor(BLACK);
		setlinestyle(PS_SOLID, 1);
		if (mtype == 1) {
			Gdiplus::Graphics Graphics(GetImageHDC());
			Gdiplus::Pen Pen_1(Gdiplus::Color(160, 180, 210), 2.f);
			Gdiplus::Pen Pen_2(Gdiplus::Color(205, 225, 235), 2.f);
			Graphics.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighQuality);
			if (isin && !isclick) {
				Graphics.DrawEllipse(&Pen_1, Gdiplus::Rect{ mx - 1, my - 1, mw + 2,mw + 2 });
			}
			else if (isclick) {
				Graphics.DrawEllipse(&Pen_2, Gdiplus::Rect{ mx - 1, my - 1, mw + 2,mw + 2 });
			}
		}
		else if (mtype == 2) {
			setlinestyle(PS_NULL);
			if (isin && !isclick) {
				setfillcolor(RGB(255, 254, 215));
				fillroundrect(mx, my, mx + mw + 1, my + mh + 1, 10, 10);
			}
			else if (isclick) {
				setfillcolor(RGB(255, 255, 235));
				fillroundrect(mx, my, mx + mw + 1, my + mh + 1, 10, 10);
			}
		}
		else {
			setlinestyle(PS_NULL);
			if (isin && !isclick) {
				setfillcolor(RGB(148, 110, 20));
				fillroundrect(mx, my, mx + mw + 1, my + mh + 1, 10, 10);
				setfillcolor(RGB(200, 180, 120));
				fillroundrect(mx + 1, my + 1, mx + mw - 1, my + mh - 1, 10, 10);
			}
			else if (isclick) {
				int shift = 3;
				setfillcolor(RGB(148, 110, 20));
				fillroundrect(mx + 2 * shift, my + shift, (mx + mw) - 2 * shift, (my + mh) - shift, 10, 10);
				setfillcolor(RGB(170, 150, 100));
				fillroundrect(mx + 2 * shift + 1, my + shift + 1, mx + mw - 2 * shift - 1, my + mh - shift - 1, 10, 10);
			}
			else {
				setfillcolor(RGB(240, 220, 170));
				fillroundrect(mx + 1, my + 1, mx + mw - 1, my + mh - 1, 10, 10);
			}
			showTextInMiddle(20, 300);
		}
		return up;
	}
	void showText(char text[], int dx, int dy, int size, int thick) {
		LOGFONT f;
		gettextstyle(&f);
		f.lfHeight = size;
		f.lfWeight = thick;
		settextstyle(&f);
		outtextxy(dx + mx, dy + my, text);
	}
	void showTextInMiddle(int size = 20, int thick = 2000) {
		settextcolor(BLACK);
		settextstyle(size, 0, "微软雅黑");
		int width1 = mx + (mw - textwidth(text)) / 2;
		int height1 = my + (mh - textheight(text)) / 2;
		outtextxy(width1, height1, text);
	}
};

class GUI {
	static constexpr int width = 750;
	static constexpr int height = 800;
	static constexpr int row = 15;
	static constexpr int col = 15;
	static constexpr int grid_size = 36;
	static constexpr int start_x = (width - grid_size * (col - 1)) / 2;
	static constexpr int start_y = -height * 0.03 + (height - grid_size * (row - 1)) / 2;

	Board bd;
	bool PV = false;

	button btnplay[5];
	button infoo;
	button btnGrid[15][15];

	void draw_board() {
		Gdiplus::Graphics Graphics(GetImageHDC());
		Gdiplus::Pen Pen_1(Gdiplus::Color(0, 0, 0), 1.f);
		Gdiplus::Pen Pen_2(Gdiplus::Color(0, 0, 0), 2.f);
		// 设置绘图质量为高质量
		Graphics.SetSmoothingMode(Gdiplus::SmoothingMode::
			SmoothingModeHighQuality);
		//绘制棋盘位置标号
		setlinestyle(PS_SOLID, 1);
		settextcolor(BLACK);
		settextstyle(20, 0, "微软雅黑");
		char c[3] = "";
		for (int i = 1; i <= 9; i++) {
			sprintf_s(c, 3, "%d", i);
			::outtextxy(start_x + (i - 1) * grid_size - 4, start_y - grid_size, c);
		}
		for (int i = 10; i <= 15; i++) {
			sprintf_s(c, 3, "%d", i);
			::outtextxy(start_x + (i - 1) * grid_size - 8, start_y - grid_size, c);
		}
		for (int i = 1; i <= 9; i++) {
			sprintf_s(c, 3, "%c", i + 'A' - 1);
			::outtextxy(start_x - grid_size + 5, start_y + (i - 1) * grid_size - 12, c);
		}
		for (int i = 10; i <= 15; i++) {
			sprintf_s(c, 3, "%c", i + 'A' - 1);
			::outtextxy(start_x - grid_size + 5, start_y + (i - 1) * grid_size - 12, c);
		}
		//画线
		for (int i = 0; i < row; i++) {
			Graphics.DrawLine(&Pen_1, start_x, start_y + i * grid_size, start_x + (col - 1) * grid_size, start_y + i * grid_size);
		}
		for (int i = 0; i < col; i++) {
			Graphics.DrawLine(&Pen_1, start_x + i * grid_size, start_y, start_x + i * grid_size, start_y + (row - 1) * grid_size);
		}
		//外圈线加粗
		Graphics.DrawLine(&Pen_2, start_x, start_y, start_x + (col - 1) * grid_size, start_y);
		Graphics.DrawLine(&Pen_2, start_x, start_y + 14 * grid_size, start_x + (col - 1) * grid_size, start_y + 14 * grid_size);
		Graphics.DrawLine(&Pen_2, start_x, start_y, start_x, start_y + (row - 1) * grid_size);
		Graphics.DrawLine(&Pen_2, start_x + 14 * grid_size, start_y, start_x + 14 * grid_size, start_y + (row - 1) * grid_size);
		//绘制黑点
		setfillcolor(BLACK);
		solidcircle(start_x + 7 * grid_size, start_y + 7 * grid_size, 2);
		solidcircle(start_x + 3 * grid_size, start_y + 3 * grid_size, 2);
		solidcircle(start_x + 11 * grid_size, start_y + 11 * grid_size, 2);
		solidcircle(start_x + 3 * grid_size, start_y + 11 * grid_size, 2);
		solidcircle(start_x + 11 * grid_size, start_y + 3 * grid_size, 2);
	}

	void draw_piece(Piece p, Pos pos, bool vir, int label, int labelColor) {
		Gdiplus::Graphics Graphics(GetImageHDC());
		int t = 255 - vir * 30;
		Gdiplus::Pen Pen_W(Gdiplus::Color(t - 20, t - 20, t - 20), 17.f);
		Gdiplus::Pen Pen_w(Gdiplus::Color(t - vir * 20, t - vir * 20, t), 16.f);
		t = 0 + vir * 70;
		Gdiplus::Pen Pen_B(Gdiplus::Color(t + 80, t + 80, t + 80), 17.f);
		Gdiplus::Pen Pen_b(Gdiplus::Color(t, t, t + vir * 20), 16.f);
		Graphics.SetSmoothingMode(Gdiplus::SmoothingMode::
			SmoothingModeHighQuality);
		settextstyle(20, 0, "微软雅黑");
		int y = pos.x(), x = pos.y();

		if (p == P1) {
			int r = grid_size - 18, stx = start_x + x * grid_size - r / 2 - 1, sty = start_y + y * grid_size - r / 2 - 1;
			Graphics.DrawEllipse(&Pen_B, Gdiplus::Rect{ stx, sty, r, r });//绘制棋子外圈
			r = grid_size - 20, stx = start_x + x * grid_size - r / 2 - 1, sty = start_y + y * grid_size - r / 2 - 1;
			Graphics.DrawEllipse(&Pen_b, Gdiplus::Rect{ stx, sty, r, r });//绘制棋子内圈
		}
		else if (p == P2) {
			int r = grid_size - 18, stx = start_x + x * grid_size - r / 2 - 1, sty = start_y + y * grid_size - r / 2 - 1;
			Graphics.DrawEllipse(&Pen_W, Gdiplus::Rect{ stx, sty, r, r });//绘制棋子外圈
			r = grid_size - 20, stx = start_x + x * grid_size - r / 2 - 1, sty = start_y + y * grid_size - r / 2 - 1;
			Graphics.DrawEllipse(&Pen_w, Gdiplus::Rect{ stx, sty, r, r });//绘制棋子内圈
		}

		TCHAR s[8];
		_stprintf_s(s, _T("%d"), label);
		//标号居中
		int width = start_x + x * grid_size - grid_size / 2 - 1 + (grid_size - textwidth(s)) / 2;
		int height = start_y + y * grid_size - grid_size / 2 - 1 + (grid_size - textheight(s)) / 2;
		settextcolor(labelColor);
		outtextxy(width, height, s);
	}

	bool Isin(int x, int y) { return x < 15 && y < 15 && x >= 0 && y >= 0; }
public:
	GUI() {
		initgraph(width, height, SHOWCONSOLE);
		//initgraph(width, height);
		setbkcolor(RGB(255, 248, 196));
		LOGFONT f;
		gettextstyle(&f);
		_tcscpy_s(f.lfFaceName, _T("微软雅黑"));
		f.lfQuality = ANTIALIASED_QUALITY;
		settextstyle(&f);
		setbkmode(TRANSPARENT);
		cleardevice();
		Gdiplus::GdiplusStartupInput Input;
		ULONG_PTR Token;
		Gdiplus::GdiplusStartup(&Token, &Input, NULL);

		char strplay[5][30] = { "PV","计算","撤销","重置","" };
		int btnw = 90, btnh = 30;
		for (int i = 0; i < 5; i++) {
			btnplay[i].set(width / 2 - 5 * (btnw + 4) / 2 + i * (btnw + 4), height * 0.88, btnw, btnh, 3);
			btnplay[i].set_text(strplay[i], 30);
		}
		infoo.set(width / 2 - 14 * grid_size / 2, start_y + 14.5 * grid_size, 14 * grid_size, btnh, 2);
		for (int i = 0; i < row; i++) {
			for (int j = 0; j < col; j++) {
				btnGrid[i][j].set(start_x + i * grid_size - grid_size / 2, start_y + j * grid_size - grid_size / 2, grid_size - 2, grid_size - 2, 1);
			}
		}
		/*for (int i = 0; i < 7; i++) {
			for (int k = 0; k < 3; k++) {
				makeMove(2 * i, k * 5);
				makeMove(2 * i + 1, k * 5);
				makeMove(2 * i, 1 + k * 5);
				makeMove(2 * i + 1, 1 + k * 5);
				makeMove(2 * i, 2 + k * 5);
				makeMove(2 * i + 1, 2 + k * 5);
				makeMove(2 * i, 3 + k * 5);
				makeMove(2 * i + 1, 3 + k * 5);
			}
			makeMove(2 * i + 1, 4);
			makeMove(2 * i, 4);
			makeMove(2 * i + 1, 9);
			makeMove(2 * i, 9);
			makeMove(2 * i + 1, 14);
			makeMove(2 * i, 14);
		}*/
	}

	void Run() {

		while (1) {
			Sleep(5);
			BeginBatchDraw();
			cleardevice();
			draw_board();
			int op = -1;
			ExMessage msg;
			while(::peekmessage(&msg, EM_MOUSE));
			int y = (msg.x - start_x + grid_size / 2) / grid_size;
			int x = (msg.y - start_y + grid_size / 2) / grid_size;
			if (Isin(x, y))btnGrid[y][x].show(msg);

			for (int i = bd.cntMove(); i >= 1; i--) {
				Pos pos = bd.lastMove(i);
				Piece self = i & 1 ? ~bd.self() : bd.self();
				draw_piece(self, pos, false, bd.cntMove() - i + 1, self == P1 ? WHITE : BLACK);
			}

			if (PV) {
				for (int i = 0; Search::rootMoves.size() && i < Search::rootMoves[0].pv.size(); i++) {
					Pos pos = Search::rootMoves[0].pv[i];
					Piece self = i & 1 ? ~bd.self() : bd.self();
					draw_piece(self, pos, true, i + 1, GREEN);
				}
			}

			if (msg.message == WM_LBUTTONDOWN) {
				if (Isin(x, y) && bd[Pos(x, y)] == Empty) {
					bd.update(Pos(x, y));
					Search::rootMoves.clear();

					/*std::cout << bd.bdnf().valueP1 << '\n';
					std::cout << bd.staticEval() << '\n';
					for (int i = 0; i < 15; i++) {
						for (int j = 0; j < 15; j++) {
							if (bd[{i, j}] == Empty)
								std::cout << bd.value(P1, { i, j }) << "\t";
							else std::cout << "-\t";
						}
						std::cout << "\n";
					}
					std::cout << "\n";*/
					/*for (int i = 0; i < 15; i++) {
						for (int j = 0; j < 15; j++) {
							if (bd[{i, j}] == Empty)
								std::cout << bd.value(P2, { i, j }) << "\t";
							else std::cout << "-\t";
						}
						std::cout << "\n";
					}*/

					std::cout << "TH3:" << bd.cntFT(TH3, P1) << " ";
					std::cout << "TDH3:" << bd.cntFT(TDH3, P1) << " ";
					std::cout << "T4H3:" << bd.cntFT(T4H3, P1) << " ";
					std::cout << "T4:" << bd.cntFT(T4, P1) << " ";
					std::cout << "TH4:" << bd.cntFT(TH4, P1) << " ";
					std::cout << "T5:" << bd.cntFT(T5, P1) << " ";
					std::cout << "\n";

					std::cout << "TH3:" << bd.cntFT(TH3, P2) << " ";
					std::cout << "TDH3:" << bd.cntFT(TDH3, P2) << " ";
					std::cout << "T4H3:" << bd.cntFT(T4H3, P2) << " ";
					std::cout << "T4:" << bd.cntFT(T4, P2) << " ";
					std::cout << "TH4:" << bd.cntFT(TH4, P2) << " ";
					std::cout << "T5:" << bd.cntFT(T5, P2) << " ";
					std::cout << "\n";

					/*for (int j = 0; j < 7; j++)
						std::cout << bd.cntFT((FType)j, P2) << " ";
					std::cout << "\n";*/
				}
			}

			infoo.show(msg);
			for (int i = 0; i < 5; i++) {
				if (btnplay[i].show(msg)) {
					op = i;
				}
			}

			if (op == 0) {
				PV = !PV;
				msg.message = 0;
			}
			else if (op == 1) {//AI落子
				Search::Worker search(bd);
				search.start();
				Pos pos = Search::rootMoves[0].pos;
				bd.update(pos);

				PrintTest();

				msg.message = 0;
			}
			else if (op == 2) {//撤销
				if (bd.cntMove())bd.undo();
				for (int i = 0; i < 15; i++) {
					for (int j = 0; j < 15; j++) {
						if (bd[{i, j}] == Empty)
							std::cout << bd.value(P1, { i, j }) << "\t";
						else std::cout << "-\t";
					}
					std::cout << "\n";
				}
				msg.message = 0;
			}
			else if (op == 3) {//重置
				bd.reset();
				msg.message = 0;
			}
			else if (op == 4) {
				//bd.sssw(P2);
				msg.message = 0;
			}
			EndBatchDraw();
		}
	}
};

#endif

/*** End of inlined file: testGUI.h ***/

/** main function for AI console application  */
int main(){
	if (0) {
		Piskvork::pisqpipe();
	}
	else {
		GUI gui;
		gui.Run();
	}
}

/*** End of inlined file: main.cpp ***/

