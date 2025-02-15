#ifndef MISC
#define MISC

#include <xmmintrin.h>
#include <cstdint>
#include <sstream>
#include <string>
#include <mutex>
#include <iostream>

template<class T, int len>
constexpr int arrLen(T(&x)[len]) { return len; }

class PRNG {
    uint64_t s;

    uint64_t rand64() {

        s ^= s >> 12, s ^= s << 25, s ^= s >> 27;
        return s * 2685821657736338717LL;
    }

public:
    PRNG(uint64_t seed) :s(seed) {}

    template<typename T>
    T rand() {
        return T(rand64());
    }

    // Special generator used to fast init magic numbers.
    // Output values only have 1/8th of their bits set on average.
    template<typename T>
    T sparse_rand() {
        return T(rand64() & rand64() & rand64());
    }
};

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

enum SyncCout {
    IO_LOCK,
    IO_UNLOCK
};

#define sync_cout std::cout << IO_LOCK
#define sync_endl std::endl << IO_UNLOCK

std::ostream& operator<<(std::ostream& os, SyncCout sc);

void sync_cout_start();
void sync_cout_end();

using TimePoint = std::chrono::milliseconds::rep;  // A value in milliseconds
static_assert(sizeof(TimePoint) == sizeof(int64_t), "TimePoint should be 64 bits");
inline TimePoint now() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch())
        .count();
}


// aligned_large_pages_alloc() will return suitably aligned memory,
// if possible using large pages.

void* aligned_large_pages_alloc(size_t allocSize);

// aligned_large_pages_free() will free the previously memory allocated
// by aligned_large_pages_alloc(). The effect is a nop if mem == nullptr.

void aligned_large_pages_free(void* mem);


enum Test {
    exclude,
    TTcutoff,
    betacutoff,
    vcfTTcutoff,
    vcfbetacutoff,
    moveCntpruning,
    dispersedT,
    TDH3T4H3wincheck,
    razor,
    futility = 60,
};

extern size_t testData[128];

void PrintTest();

#endif
