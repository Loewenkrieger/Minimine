#ifndef BENCH_HOST_PROF_H
#define BENCH_HOST_PROF_H
#include <stdint.h>
typedef struct {uint32_t rec,elapsed;} prof_t;
uint32_t bench_now(void);
#define prof_make() ((prof_t){0,0})
#define prof_enter_norec(p) ((p).rec=bench_now())
#define prof_leave_norec(p) ((p).elapsed+=(p).rec-bench_now())
#define prof_enter(p) prof_enter_norec(p)
#define prof_leave(p) prof_leave_norec(p)
extern volatile uint32_t *prof_tcnt;
int prof_init(void);
void prof_quit(void);
uint32_t prof_time(prof_t p);
#endif
