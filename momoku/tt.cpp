#include <random>
#include <iostream>

#include "test.h"
#include "tt.h"

void HashEntry::store(uint64_t hash, bool pv, Sqare best, int val, int dep, int step, HashType type) {
	uint64_t key22 = hash & 0x3fffff;
	testData[Test::store]++;
	if (key() != key22 || dep >= _dep || type == B_Exact) {
		_PV = pv;
		if (val >= WIN_CRITICAL) val += step;
		else if (val <= -WIN_CRITICAL) val -= step;
		_value = val;
		_dep = dep;
		_gen = tt.generation();
		_type = type;
		_moveSqare = best;
		_hashLow16 = (uint16_t)key22;
		_hashMid6 = (key22 >> 16);
	}
}

HashTable::HashTable(int maxByteSize) {
	numClusters = maxByteSize / sizeof(Cluster);
	clusters = new Cluster[numClusters];
	clear();
}

HashTable::~HashTable() {
	delete[] clusters;
}

void HashTable::clear() {
	for (int i = 0; i < numClusters; i++) 
		clusters[i].Clear();
	_generation = 0;
}

bool HashTable::probe(uint64_t hash, HashEntry*& tte) {

	HashEntry* entry = firstEntry(hash);
	uint64_t key22 = hash & 0x3fffff;

	for (int i = 0; i < ClusterSize; i++) {
		if (entry[i].key() == key22) {
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

