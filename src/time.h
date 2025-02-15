#ifndef TIME
#define TIME

#include "common.h"
#include "misc.h"

#include <chrono>

struct LimitsType;

class TimeManagement {
	TimePoint startTime;
	TimePoint optimumTime;
	TimePoint maximumTime;

public:
	void init(LimitsType& limits,
		Piece               us,
		int                 movestogo);

	TimePoint optimum() const;
	TimePoint maximum() const;
	TimePoint elapsed_time() const { return now() - startTime; };
};


#endif


