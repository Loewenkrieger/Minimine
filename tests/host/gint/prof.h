#ifndef HOST_PROF_H
#define HOST_PROF_H
#include <stdint.h>
typedef struct { uint32_t rec, elapsed; } prof_t;
#define prof_make() ((prof_t){0, 0})
extern volatile uint32_t *prof_tcnt;
int prof_init(void);
void prof_quit(void);
uint32_t prof_time(prof_t context);
#endif

