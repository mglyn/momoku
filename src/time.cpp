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

    // startTime is used by movetime and useNodesTime is used in elapsed calls.
    startTime = limits.startTime;

    // Limit the maximum possible time for this move
    maximumTime = limits.time[us] - 1000;
}