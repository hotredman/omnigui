#include "core/ProcessStats.hpp"

#include <algorithm>
#include <thread>

#if defined(_WIN32)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
    #include <psapi.h>
#else
    #include <ctime>
    #include <fstream>
    #include <sys/resource.h>
    #include <unistd.h>
#endif

namespace ProcessStats {

namespace {

constexpr double kBytesPerMb = 1024.0 * 1024.0;

#if defined(_WIN32)
double FileTimeSeconds(const FILETIME& ft) {
    ULARGE_INTEGER v;
    v.LowPart = ft.dwLowDateTime;
    v.HighPart = ft.dwHighDateTime;
    return static_cast<double>(v.QuadPart) / 1.0e7;   // единицы по 100 нс
}
#else
double ClockSeconds(clockid_t clock) {
    timespec ts;
    if (clock_gettime(clock, &ts) != 0) return -1.0;
    return static_cast<double>(ts.tv_sec) + static_cast<double>(ts.tv_nsec) * 1.0e-9;
}
#endif

} // namespace

Memory GetMemory() {
    Memory m;
#if defined(_WIN32)
    PROCESS_MEMORY_COUNTERS_EX pmc{};
    if (GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
        m.workingSetMb = static_cast<double>(pmc.WorkingSetSize) / kBytesPerMb;
        m.peakWorkingSetMb = static_cast<double>(pmc.PeakWorkingSetSize) / kBytesPerMb;
    }
#else
    rusage usage{};
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
    #if defined(__APPLE__)
        m.peakWorkingSetMb = static_cast<double>(usage.ru_maxrss) / kBytesPerMb;          // байты
    #else
        m.peakWorkingSetMb = static_cast<double>(usage.ru_maxrss) / 1024.0;               // килобайты
    #endif
    }
    #if defined(__linux__)
    // Текущая резидентная память: второе поле /proc/self/statm, в страницах
    std::ifstream statm("/proc/self/statm");
    long long totalPages = 0, residentPages = 0;
    if (statm >> totalPages >> residentPages) {
        m.workingSetMb = static_cast<double>(residentPages) * static_cast<double>(sysconf(_SC_PAGESIZE)) / kBytesPerMb;
    }
    #else
    m.workingSetMb = m.peakWorkingSetMb;
    #endif
#endif
    return m;
}

double ProcessCpuSeconds() {
#if defined(_WIN32)
    FILETIME creation, exit, kernel, user;
    if (!GetProcessTimes(GetCurrentProcess(), &creation, &exit, &kernel, &user)) return -1.0;
    return FileTimeSeconds(kernel) + FileTimeSeconds(user);
#else
    return ClockSeconds(CLOCK_PROCESS_CPUTIME_ID);
#endif
}

double ThreadCpuSeconds() {
#if defined(_WIN32)
    FILETIME creation, exit, kernel, user;
    if (!GetThreadTimes(GetCurrentThread(), &creation, &exit, &kernel, &user)) return -1.0;
    return FileTimeSeconds(kernel) + FileTimeSeconds(user);
#else
    return ClockSeconds(CLOCK_THREAD_CPUTIME_ID);
#endif
}

int ProcessorCount() {
    return std::max(1, static_cast<int>(std::thread::hardware_concurrency()));
}

} // namespace ProcessStats