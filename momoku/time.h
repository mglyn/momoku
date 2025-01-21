#ifndef TIME
#define TIME

#include <chrono>

#include "common.h"

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


