#include "time.h"
#include "search.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <string>

TimePoint TimeManagement::optimum() const { return optimumTime; }
TimePoint TimeManagement::maximum() const { return maximumTime; }

void TimeManagement::init(LimitsType& limits,
    Piece               us,
    int                 movestogo) {

    // If we have no time, we don't need to fully initialize TM.
    // startTime is used by movetime and useNodesTime is used in elapsed calls.
    startTime = limits.startTime;

    if (limits.globalTime[us] == 0) {
        return;
    }

    // These numbers are used where multiplications, divisions or comparisons
    // with constants are involved.
    int moveOverhead = 5;
    // Make sure timeLeft is > 0 since we may use it as a divisor
    TimePoint timeLeft = std::max(TimePoint(1), limits.globalTime[us] - moveOverhead * movestogo);

    // Limit the maximum possible time for this move
    optimumTime = TimePoint(timeLeft / movestogo);
    if (optimumTime < 30 * moveOverhead) 
        maximumTime = TimePoint(std::min(limits.time[us], optimumTime) - moveOverhead);
    else
        maximumTime = TimePoint(std::min(std::min(limits.time[us], TimePoint(1.3 * optimumTime)), timeLeft) - moveOverhead);
}