#include "misc.h"

#ifdef _WIN32
#if _WIN32_WINNT < 0x0601
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0601  // Force to include needed API prototypes
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <ios>       // std::hex, std::dec
#include <iostream>  // std::cerr
#include <ostream>   // std::endl
#include <windows.h>

// The needed Windows API for processor groups could be missed from old Windows
// versions, so instead of calling them directly (forcing the linker to resolve
// the calls at compile time), try to load them at runtime. To do this we need
// first to define the corresponding function pointers.

extern "C" {
    using OpenProcessToken_t = bool (*)(HANDLE, DWORD, PHANDLE);
    using LookupPrivilegeValueA_t = bool (*)(LPCSTR, LPCSTR, PLUID);
    using AdjustTokenPrivileges_t =
        bool (*)(HANDLE, BOOL, PTOKEN_PRIVILEGES, DWORD, PTOKEN_PRIVILEGES, PDWORD);
}
#endif

#if __has_include("features.h")
#include <features.h>
#endif

#if defined(__linux__) && !defined(__ANDROID__)
#include <sys/mman.h>
#endif

#if defined(__APPLE__) || defined(__ANDROID__) || defined(__OpenBSD__) \
  || (defined(__GLIBCXX__) && !defined(_GLIBCXX_HAVE_ALIGNED_ALLOC) && !defined(_WIN32)) \
  || defined(__e2k__)
#define POSIXALIGNEDALLOC
#include <stdlib.h>
#endif

#ifdef _WIN32
#if _WIN32_WINNT < 0x0601
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0601  // Force to include needed API prototypes
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <ios>       // std::hex, std::dec
#include <iostream>  // std::cerr
#include <ostream>   // std::endl
#include <windows.h>

// The needed Windows API for processor groups could be missed from old Windows
// versions, so instead of calling them directly (forcing the linker to resolve
// the calls at compile time), try to load them at runtime. To do this we need
// first to define the corresponding function pointers.

extern "C" {
    using OpenProcessToken_t = bool (*)(HANDLE, DWORD, PHANDLE);
    using LookupPrivilegeValueA_t = bool (*)(LPCSTR, LPCSTR, PLUID);
    using AdjustTokenPrivileges_t =
        bool (*)(HANDLE, BOOL, PTOKEN_PRIVILEGES, DWORD, PTOKEN_PRIVILEGES, PDWORD);
}
#endif

std::ostream& operator<<(std::ostream& os, SyncCout sc) {

    static std::mutex m;

    if (sc == IO_LOCK)
        m.lock();

    if (sc == IO_UNLOCK)
        m.unlock();

    return os;
}

void sync_cout_start() { std::cout << IO_LOCK; }
void sync_cout_end() { std::cout << IO_UNLOCK; }

#if defined(_WIN32)

static void* aligned_large_pages_alloc_windows([[maybe_unused]] size_t allocSize) {

    #if !defined(_WIN64)
    return nullptr;
    #else

    HANDLE hProcessToken{};
    LUID   luid{};
    void*  mem = nullptr;

    const size_t largePageSize = GetLargePageMinimum();
    if (!largePageSize)
        return nullptr;

    // Dynamically link OpenProcessToken, LookupPrivilegeValue and AdjustTokenPrivileges

    HMODULE hAdvapi32 = GetModuleHandle(TEXT("advapi32.dll"));

    if (!hAdvapi32)
        hAdvapi32 = LoadLibrary(TEXT("advapi32.dll"));

    auto OpenProcessToken_f =
      OpenProcessToken_t((void (*)()) GetProcAddress(hAdvapi32, "OpenProcessToken"));
    if (!OpenProcessToken_f)
        return nullptr;
    auto LookupPrivilegeValueA_f =
      LookupPrivilegeValueA_t((void (*)()) GetProcAddress(hAdvapi32, "LookupPrivilegeValueA"));
    if (!LookupPrivilegeValueA_f)
        return nullptr;
    auto AdjustTokenPrivileges_f =
      AdjustTokenPrivileges_t((void (*)()) GetProcAddress(hAdvapi32, "AdjustTokenPrivileges"));
    if (!AdjustTokenPrivileges_f)
        return nullptr;

    // We need SeLockMemoryPrivilege, so try to enable it for the process

    if (!OpenProcessToken_f(  // OpenProcessToken()
          GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hProcessToken))
        return nullptr;

    if (LookupPrivilegeValueA_f(nullptr, "SeLockMemoryPrivilege", &luid))
    {
        TOKEN_PRIVILEGES tp{};
        TOKEN_PRIVILEGES prevTp{};
        DWORD            prevTpLen = 0;

        tp.PrivilegeCount           = 1;
        tp.Privileges[0].Luid       = luid;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        // Try to enable SeLockMemoryPrivilege. Note that even if AdjustTokenPrivileges()
        // succeeds, we still need to query GetLastError() to ensure that the privileges
        // were actually obtained.

        if (AdjustTokenPrivileges_f(hProcessToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), &prevTp,
                                    &prevTpLen)
            && GetLastError() == ERROR_SUCCESS)
        {
            // Round up size to full pages and allocate
            allocSize = (allocSize + largePageSize - 1) & ~size_t(largePageSize - 1);
            mem       = VirtualAlloc(nullptr, allocSize, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES,
                                     PAGE_READWRITE);

            // Privilege no longer needed, restore previous state
            AdjustTokenPrivileges_f(hProcessToken, FALSE, &prevTp, 0, nullptr, nullptr);
        }
    }

    CloseHandle(hProcessToken);

    return mem;

    #endif
}

void* aligned_large_pages_alloc(size_t allocSize) {

    // Try to allocate large pages
    void* mem = aligned_large_pages_alloc_windows(allocSize);

    // Fall back to regular, page-aligned, allocation if necessary
    if (!mem)
        mem = VirtualAlloc(nullptr, allocSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    return mem;
}

#else

void* aligned_large_pages_alloc(size_t allocSize) {

    #if defined(__linux__)
    constexpr size_t alignment = 2 * 1024 * 1024;  // 2MB page size assumed
    #else
    constexpr size_t alignment = 4096;  // small page size assumed
    #endif

    // Round up to multiples of alignment
    size_t size = ((allocSize + alignment - 1) / alignment) * alignment;
    void*  mem  = std_aligned_alloc(alignment, size);
    #if defined(MADV_HUGEPAGE)
    madvise(mem, size, MADV_HUGEPAGE);
    #endif
    return mem;
}

#endif

// aligned_large_pages_free() will free the previously memory allocated
// by aligned_large_pages_alloc(). The effect is a nop if mem == nullptr.

#if defined(_WIN32)

void aligned_large_pages_free(void* mem) {

    if (mem && !VirtualFree(mem, 0, MEM_RELEASE))
    {
        DWORD err = GetLastError();
        std::cerr << "Failed to free large page memory. Error code: 0x" << std::hex << err
            << std::dec << std::endl;
        exit(EXIT_FAILURE);
    }
}

#else

void aligned_large_pages_free(void* mem) { std_aligned_free(mem); }

#endif


size_t testData[128];

void PrintTest() {

    std::string testText;

    testText += "--------- TEST INFO ---------\n";
    testText += "main node:" + std::to_string(testData[mainn] / 1000.f) + "\n";
    testData[mainn] = 0;
    testText += "vcf node:" + std::to_string(testData[qn] / 1000.f) + "\n";
    testData[qn] = 0;
    testText += "tot node:" + std::to_string((testData[qn] + testData[mainn]) / 1000.f) + "\n";
    testText += "TT cutoff: " + std::to_string(testData[TTcutoff] / 1000.f) + "\n";
    testData[TTcutoff] = 0;
    testText += "beta cutoff: " + std::to_string(testData[betacutoff] / 1000.f) + "\n";
    testData[betacutoff] = 0;
    testText += "vcf TT cutoff: " + std::to_string(testData[vcfTTcutoff] / 1000.f) + "\n";
    testData[vcfTTcutoff] = 0;
    testText += "vcf beta cutoff: " + std::to_string(testData[vcfbetacutoff] / 1000.f) + "\n";
    testData[vcfbetacutoff] = 0;
    testText += "moveCnt pruning: " + std::to_string(testData[moveCntpruning] / 1000.f) + "\n";
    testData[moveCntpruning] = 0;
    testText += "dispersed T: " + std::to_string(testData[dispersedT] / 1000.f) + "\n";
    testData[dispersedT] = 0;

    testText += "razor:\n";
    for (int i = 0; i < 32; i++) {
        testText += std::to_string(testData[razor + i]) + " ";
        testData[razor + i] = 0;
    }
    testText += "\n";

    std::cout << "futility:\n";
    for (int i = 0; i < 32; i++) {
        testText += std::to_string(testData[futility + i]) + " ";
        testData[futility + i] = 0;
    }
    testText += "\n";

    sync_cout << testText << sync_endl;
}