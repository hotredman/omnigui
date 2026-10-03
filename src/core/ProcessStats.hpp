#pragma once

// Метрики процесса для профилирования: память и процессорное время.
// Реализации: Windows (psapi, GetProcessTimes), POSIX (getrusage, /proc, clock_gettime).
// Если платформа значение не поддерживает, память возвращается нулевой, время — отрицательным.
namespace ProcessStats {

struct Memory {
    double workingSetMb     = 0.0;   // текущая резидентная память, МБ
    double peakWorkingSetMb = 0.0;   // пиковая резидентная память, МБ
};

Memory GetMemory();

// Суммарное процессорное время (user + kernel) всего процесса, секунды; < 0 — не поддерживается
double ProcessCpuSeconds();

// То же для текущего потока
double ThreadCpuSeconds();

// Число логических процессоров (не менее 1)
int ProcessorCount();

} // namespace ProcessStats