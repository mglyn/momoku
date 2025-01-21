//#pragma once
//
//#include "../common/common.h"
//#include "../game/board.h"
//
//namespace Search {
//	constexpr int TTKBSIZE = 1024 * 1024 * 250;
//
//	extern uint64_t Zobrist[2][BOARD_SIZE];
//
//	class HashEntry {
//		uint16_t _hashKeyLow16;
//		int16_t _value;
//		uint16_t _movePos;
//		int8_t _dep;
//		uint8_t _gen : 5;
//		uint8_t _PV : 1;
//		uint8_t _type : 2;
//
//	public:
//		HashEntry() { clear(); }
//		Pos movePos() const { return _movePos; }
//		bool isPV() const { return _PV; }
//		uint16_t key() const { return _hashKeyLow16; }
//		HashType type() const { return static_cast<HashType>(_type); }
//		uint8_t genaration() const { return _gen; }
//		int value(int ply) const {
//			if (_value == -VAL_INF) return -VAL_INF;
//			if (_value >= WIN_CRITICAL) return _value - ply;
//			if (_value <= -WIN_CRITICAL) return _value + ply;
//			return _value;
//		}
//		int depth() const { return _dep; }
//		void setGeneration(uint8_t gen) { _gen = gen; }
//		void clear() { memset(this, 0, sizeof(HashEntry)); }
//		void store(uint64_t hashKey, Pos best, int score, int dep, int step, HashType type);
//	};
//
//	class HashTable {
//		static constexpr int ClusterSize = 4;
//		static constexpr uint8_t MODULO = 32;
//		uint64_t cntCluster;
//
//		struct Cluster {
//			HashEntry hashEntries[ClusterSize];
//			inline void Clear() { for (int i = 0; i < ClusterSize; i++) hashEntries[i].clear(); }
//		}*clusters;
//
//		uint8_t _generation;
//
//	public:
//		HashTable(int maxByteSize);
//
//		~HashTable();
//
//		void clear();
//
//		uint8_t generation() const { return _generation; }
//		void newGen() { _generation++; }
//
//		HashEntry* firstEntry(uint64_t hashKey);
//
//		bool probe(uint64_t hashKey, HashEntry*& tte);
//	};
//
//	extern HashTable tt;
//}
//
