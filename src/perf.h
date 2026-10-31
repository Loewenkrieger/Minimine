#ifndef SANDBOX_PERF_H
#define SANDBOX_PERF_H

#include <stddef.h>
#include <stdint.h>

enum { PERF_CAPACITY = 150 };

typedef struct {
    uint32_t frame_us, logic_us, draw_us, lcd_us;
} PerfSample;

typedef struct {
    uint32_t count, fps_x10;
    uint32_t mean_us, p95_us, worst_us;
    uint32_t logic_us, draw_us, lcd_us;
} PerfSummary;

typedef struct {
    PerfSample samples[PERF_CAPACITY];
    unsigned count, next;
} PerfHistory;

void perf_reset(PerfHistory *history);
void perf_push(PerfHistory *history, PerfSample sample);
PerfSummary perf_summarize(const PerfSample *samples, size_t count);

#endif

