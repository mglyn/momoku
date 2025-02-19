#ifndef HISTORY
#define HISTORY

#include "common.h"

#include <array>

// StatsEntry is the container of various numerical statistics. We use a class
// instead of a naked value to directly call history update operator<<() on
// the entry. The first template parameter T is the base type of the array,
// and the second template parameter D limits the range of updates in [-D, D]
// when we update values with the << operator
template<typename T, int D>
class StatsEntry {

    static_assert(std::is_arithmetic<T>::value, "Not an arithmetic type");
    static_assert(D <= (std::numeric_limits<T>::max)(), "D overflows T");

    T entry;

public:
    StatsEntry& operator=(const T& v) {
        entry = v;
        return *this;
    }
    operator const T& () const { return entry; }

    void operator<<(int bonus) {
        // Make sure that bonus is in range [-D, D]
        int pr = entry;

        int clampedBonus = std::clamp(bonus, -D, D);
        entry += clampedBonus - entry * std::abs(clampedBonus) / D;

        assert(std::abs(entry) <= D);
    }
};

/// The type of a main history record.
enum MoveHistoryType { HIST_THREAT, HIST_QUIET, HIST_TYPE_NUM };

/// MainHistory records how often a certain type of move has been successful or unsuccessful
/// (causing a beta cutoff) during the current search. It is indexed by color of the move,
/// move's position, and the move's history type.
using MainHistory = std::array<std::array<std::array<StatsEntry<int16_t, 10000>, HIST_TYPE_NUM>, BOARD_SIZE>, SIDE_NUM>;

/// CounterMoveHistory records a natural response of moves irrespective of the actual position.
/// It is indexed by color of the previous move, previous move's position and current move's type.
using CounterMoveHistory = std::array<std::array<std::pair<Square, FType>, MAX_MOVE>, SIDE_NUM>;

#endif