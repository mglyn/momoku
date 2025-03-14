﻿#include "tt.h"
#include "misc.h"

// TTEntry struct is the 10 bytes transposition table entry, defined as below:
//
// key        16 bit
// depth       8 bit
// generation  5 bit
// pv node     1 bit
// bound type  2 bit
// move       16 bit
// value      16 bit
// evaluation 16 bit
//
// These fields are in the same order as accessed by TT::probe(), since memory is fastest sequentially.
// Equally, the store order in save() matches this order.

struct TTEntry {
private:
	friend class TranspositionTable;

	uint16_t key16;
	uint8_t  depth8;
	uint8_t  genBound8;
	int16_t  move16;
	int16_t  value16;
	int16_t  eval16;
public:
	// Convert internal bitfields to external types
	TTData read() const {
		return TTData{ Square(move16),			Value(value16),
					   Value(eval16),			int(depth8 + DEPTH_ENTRY_OFFSET),
					   Bound(genBound8 & 0x3),	bool(genBound8 & 0x4) };
	}

	bool is_occupied() const;
	void save(Key k, Value v, bool pv, Bound b, int d, Square m, Value ev, uint8_t generation8);
	// The returned age is a multiple of TranspositionTable::GENERATION_DELTA
	uint8_t relative_age(const uint8_t generation8) const;
};

// `genBound8` is where most of the details are. We use the following constants to manipulate 5 leading generation bits
// and 3 trailing miscellaneous bits.

// These bits are reserved for other things.
static constexpr unsigned GENERATION_BITS = 3;
// increment for generation field
static constexpr int GENERATION_DELTA = (1 << GENERATION_BITS);
// cycle length
static constexpr int GENERATION_CYCLE = 255 + GENERATION_DELTA;
// mask to pull out generation number
static constexpr int GENERATION_MASK = (0xFF << GENERATION_BITS) & 0xFF;

// DEPTH_ENTRY_OFFSET exists because 1) we use `bool(depth8)` as the occupancy check, but
// 2) we need to store negative depths for QS. (`depth8` is the only field with "spare bits":
// we sacrifice the ability to store depths greater than 1<<8 less the offset, as asserted in `save`.)
bool TTEntry::is_occupied() const { return bool(depth8); }

// Populates the TTEntry with a new node's data, possibly
// overwriting an old position. The update is not atomic and can be racy.
void TTEntry::save(
	Key k, Value v, bool pv, Bound b, int d, Square m, Value ev, uint8_t generation8) {

	// Preserve the old ttmove if we don't have a new one
	if (m || uint16_t(k) != key16)
		move16 = m;

	// Overwrite less valuable entries (cheapest checks first)
	if (b == BOUND_EXACT || uint16_t(k) != key16 || d - DEPTH_ENTRY_OFFSET + 2 >= depth8
		|| relative_age(generation8)) {

		assert(d > DEPTH_ENTRY_OFFSET);
		assert(d < 256 + DEPTH_ENTRY_OFFSET);

		key16 = uint16_t(k);
		depth8 = uint8_t(d - DEPTH_ENTRY_OFFSET);
		genBound8 = uint8_t(generation8 | uint8_t(pv) << 2 | b);
		value16 = int16_t(v);
		eval16 = int16_t(ev);
	}
}

uint8_t TTEntry::relative_age(const uint8_t generation8) const {
	// Due to our packed storage format for generation and its cyclic
	// nature we add GENERATION_CYCLE (256 is the modulus, plus what
	// is needed to keep the unrelated lowest n bits from affecting
	// the result) to calculate the entry age correctly even after
	// generation8 overflows into the next cycle.
	return (GENERATION_CYCLE + generation8 - genBound8) & GENERATION_MASK;
}

// TTWriter is but a very thin wrapper around the pointer
TTWriter::TTWriter(TTEntry* tte) :
	entry(tte) {
}

void TTWriter::write(
	Key k, Value v, bool pv, Bound b, int d, Square m, Value ev, uint8_t generation8) {
	entry->save(k, v, pv, b, d, m, ev, generation8);
}

// A TranspositionTable is an array of Cluster, of size clusterCount. Each cluster consists of ClusterSize number
// of TTEntry. Each non-empty TTEntry contains information on exactly one position. The size of a Cluster should
// divide the size of a cache line for best performance, as the cacheline is prefetched when possible.

static constexpr int ClusterSize = 3;

struct Cluster {
	TTEntry entry[ClusterSize];
	char    padding[2];  // Pad to 32 bytes
};

static_assert(sizeof(Cluster) == 32, "Suboptimal Cluster size");

// Sets the size of the transposition table,
// measured in megabytes. Transposition table consists
// of clusters and each cluster consists of ClusterSize number of TTEntry.
void TranspositionTable::resize(size_t bSize) {

	aligned_large_pages_free(table);

	clusterCount = bSize / sizeof(Cluster);

	table = static_cast<Cluster*>(aligned_large_pages_alloc(clusterCount * sizeof(Cluster)));

	if (!table) {
		std::cerr << "Failed to allocate " << bSize << "Byte for transposition table." << std::endl;
		exit(EXIT_FAILURE);
	}

	clear();
}

// Initializes the entire transposition table to zero,
// in a multi-threaded way.
void TranspositionTable::clear() {
	generation8 = 0;
	std::memset(table, 0, sizeof(table));
}

// Returns an approximation of the hashtable
// occupation during a search. The hash is x permill full, as per UCI protocol.
// Only counts entries which match the current generation.
int TranspositionTable::hashfull(int maxAge) const {
	int maxAgeInternal = maxAge << GENERATION_BITS;
	int cnt = 0;
	for (int i = 0; i < 1000; ++i)
		for (int j = 0; j < ClusterSize; ++j)
			cnt += table[i].entry[j].is_occupied()
			&& table[i].entry[j].relative_age(generation8) <= maxAgeInternal;

	return cnt / ClusterSize;
}

void TranspositionTable::new_search() {
	// increment by delta to keep lower bits as is
	generation8 += GENERATION_DELTA;
}

uint8_t TranspositionTable::generation() const { return generation8; }

// Looks up the current position in the transposition
// table. It returns true if the position is found.
// Otherwise, it returns false and a pointer to an empty or least valuable TTEntry
// to be replaced later. The replace value of an entry is calculated as its depth
// minus 8 times its relative age. TTEntry t1 is considered more valuable than
// TTEntry t2 if its replace value is greater than that of t2.
std::tuple<bool, TTData, TTWriter> TranspositionTable::probe(const Key key) const {

	TTEntry* const tte = first_entry(key);
	const uint16_t key16 = uint16_t(key);  // Use the low 16 bits as key inside the cluster

	for (int i = 0; i < ClusterSize; ++i)
		if (tte[i].key16 == key16)
			// This gap is the main place for read races.
			// After `read()` completes that copy is final, but may be self-inconsistent.
			return { tte[i].is_occupied(), tte[i].read(), TTWriter(&tte[i]) };

	// Find an entry to be replaced according to the replacement strategy
	TTEntry* replace = tte;
	for (int i = 1; i < ClusterSize; ++i)
		if (replace->depth8 - replace->relative_age(generation8) * 2
	> tte[i].depth8 - tte[i].relative_age(generation8) * 2)
			replace = &tte[i];

	return { false,
			TTData{Square::NONE, VALUE_NONE, VALUE_NONE, DEPTH_ENTRY_OFFSET, BOUND_NONE, false},
			TTWriter(replace) };
}


TTEntry* TranspositionTable::first_entry(const Key key) const {
	return &table[mulhi64(key, clusterCount)].entry[0];
}
