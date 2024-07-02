#ifndef MISC
#define MISC

#include <xmmintrin.h>
#include <cstdint>

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

template<class T, int len>
constexpr int arrLen(T(&x)[len]) { return len; }

#endif
