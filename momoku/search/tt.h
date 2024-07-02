#ifndef TT
#define TT

#include "../common/common.h"
#include "../common/misc.h"
#include "../game/board.h"

constexpr int TTBYTESIZE = 1024 * 1024 * 250;

class HashEntry {
	uint16_t _hashLow16;
	uint16_t _hashMid6 : 6;

	uint16_t _movePos : 10;
	int16_t _value;
	int8_t _dep;
	uint8_t _gen : 5;
	uint8_t _PV : 1;
	uint8_t _type : 2;

public:
	HashEntry() { clear(); }
	Pos movePos() const { return _movePos; }
	bool isPV() const { return _PV; }
	uint64_t key() const { return (uint64_t)_hashLow16 | ((uint64_t)_hashMid6 << 16); }
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

	float usage() {

		float tot = 0;
		for (int i = 0; i < 1000; i++) {
			for (int j = 0; j < ClusterSize; j++) {
				if (clusters[i].hashEntries[j].key() != 0)tot++;
			}
		}

		return tot / ClusterSize;
	}
};

extern HashTable tt;

#endif
