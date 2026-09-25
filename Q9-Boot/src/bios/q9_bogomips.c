/* q9_bogomips.c - Bogomips-style calibration for Q9
 * Added to q-boot BIOS sources. See q9_bogomips.c in repository root for test harness.
 */

#include "q9_bios.h"
#include <stdint.h>

#define HZ 100
#define JIFFY_US ((1000000UL) / HZ)

extern unsigned long read_ticks(void);
extern void scheduler_lock(void);
extern void scheduler_unlock(void);

static inline void cpu_relax(void) { asm volatile("nop" ::: "memory"); }

static void loops_delay(unsigned long loops)
{
    volatile unsigned long i;
    for (i = 0; i < loops; ++i) {
        asm volatile("");
    }
}

static int too_many_loops(unsigned long loops)
{
    unsigned long start = read_ticks();
    while (read_ticks() == start) cpu_relax();
    start = read_ticks();
    loops_delay(loops);
    return (read_ticks() != start);
}

unsigned long calibrate_loops_per_jiffy(void)
{
    unsigned long lo = 1UL, hi, mid;
    scheduler_lock();
    while (!too_many_loops(lo)) {
        lo <<= 1;
        if (lo == 0UL) break;
    }
    hi = lo; lo >>= 1;
    while (hi - lo > 1) {
        mid = lo + (hi - lo) / 2;
        if (too_many_loops(mid)) hi = mid; else lo = mid;
    }
    scheduler_unlock();
    return lo;
}
