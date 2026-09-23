/*
 * q9_bogomips.c
 *
 * Calibrate loops_per_jiffy for a system with HZ=100 (Q9).
 * Uses doubling + binary search. Requires platform implementations of:
 *   unsigned long read_ticks(void);    // returns a monotonically increasing tick counter incremented at HZ (100Hz)
 *   void scheduler_lock(void);         // prevent context switches but keep timer IRQs running
 *   void scheduler_unlock(void);
 *
 * Compile for testing with: gcc -DTEST_MAIN -O2 q9_bogomips.c -o q9_bogomips
 */

#include <stdint.h>
#include <stdio.h>

#define HZ 100
#define JIFFY_US ((1000000UL) / HZ) /* microseconds per jiffy (100Hz -> 10000 us) */

/* Platform must provide these symbols. Keep them extern here. */
extern unsigned long read_ticks(void);   /* returns tick counter incremented at HZ */
extern void scheduler_lock(void);        /* prevent task switches; do NOT disable timer IRQs */
extern void scheduler_unlock(void);

static inline void cpu_relax(void) { asm volatile("nop" ::: "memory"); }

/* Busy-wait loop that the compiler must not optimize away */
static void loops_delay(unsigned long loops)
{
    volatile unsigned long i;
    for (i = 0; i < loops; ++i) {
        asm volatile("" ::: "memory");
    }
}

/* Return non-zero if performing 'loops' takes >= 1 jiffy */
static int too_many_loops(unsigned long loops)
{
    unsigned long start;

    /* wait for tick boundary */
    start = read_ticks();
    while (read_ticks() == start)
        cpu_relax();

    start = read_ticks();
    loops_delay(loops);
    return (read_ticks() != start);
}

unsigned long calibrate_loops_per_jiffy(void)
{
    unsigned long lo, hi, mid;

    scheduler_lock();

    /* find an upper bound by doubling */
    lo = 1UL;
    while (!too_many_loops(lo)) {
        lo <<= 1;
        /* safety: avoid infinite loop if too large */
        if (lo == 0UL) break;
    }

    hi = lo;
    lo >>= 1;

    /* binary search between lo (good) and hi (too many) */
    while (hi - lo > 1) {
        mid = lo + (hi - lo) / 2;
        if (too_many_loops(mid))
            hi = mid;
        else
            lo = mid;
    }

    scheduler_unlock();
    return lo; /* loops_per_jiffy (largest value that fits in one jiffy) */
}

/* BogoMIPS = (loops_per_jiffy * HZ) / 1_000_000 */
double compute_bogomips(unsigned long loops_per_jiffy)
{
    return (double)loops_per_jiffy * (double)HZ / 1000000.0;
}

#ifdef TEST_MAIN
/* Minimal test harness for native testing: provides a fake tick using usleep.
 * Note: This is only for host-side testing when -DTEST_MAIN is defined.
 */
#include <unistd.h>
#include <pthread.h>
#include <time.h>

static volatile unsigned long _tick_counter = 0;
static pthread_t _tick_thread;
static int _tick_thread_running = 0;

static void *tick_fn(void *arg)
{
    (void)arg;
    _tick_thread_running = 1;
    while (_tick_thread_running) {
        usleep(JIFFY_US);
        __sync_fetch_and_add(&_tick_counter, 1);
    }
    return NULL;
}

unsigned long read_ticks(void)
{
    return _tick_counter;
}

void scheduler_lock(void) { /* noop for host test */ }
void scheduler_unlock(void) { /* noop for host test */ }

int main(void)
{
    unsigned long loops;
    double bm;

    /* start fake tick thread */
    pthread_create(&_tick_thread, NULL, tick_fn, NULL);
    usleep(1000); /* let thread start */

    loops = calibrate_loops_per_jiffy();
    bm = compute_bogomips(loops);
    printf("loops_per_jiffy=%lu, BogoMIPS=%.3f\n", loops, bm);

    /* cleanup */
    _tick_thread_running = 0;
    pthread_join(_tick_thread, NULL);
    return 0;
}
#endif
