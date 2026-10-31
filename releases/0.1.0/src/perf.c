#include "perf.h"

#include <string.h>

void perf_reset(PerfHistory *history)
{
    memset(history, 0, sizeof(*history));
}

void perf_push(PerfHistory *history, PerfSample sample)
{
    history->samples[history->next] = sample;
    history->next = (history->next + 1) % PERF_CAPACITY;
    if(history->count < PERF_CAPACITY) ++history->count;
}

PerfSummary perf_summarize(const PerfSample *samples, size_t count)
{
    PerfSummary result = {0};
    uint32_t sorted[PERF_CAPACITY];
    uint64_t frame = 0, logic = 0, draw = 0, lcd = 0;
    if(count > PERF_CAPACITY) count = PERF_CAPACITY;
    if(!count) return result;
    for(size_t i = 0; i < count; ++i) {
        uint32_t value = samples[i].frame_us;
        size_t j = i;
        while(j > 0 && sorted[j - 1] > value) {
            sorted[j] = sorted[j - 1];
            --j;
        }
        sorted[j] = value;
        frame += value;
        logic += samples[i].logic_us;
        draw += samples[i].draw_us;
        lcd += samples[i].lcd_us;
    }
    result.count = (uint32_t)count;
    result.fps_x10 = frame ? (uint32_t)((uint64_t)count * 10000000 / frame) : 0;
    result.mean_us = (uint32_t)(frame / count);
    result.logic_us = (uint32_t)(logic / count);
    result.draw_us = (uint32_t)(draw / count);
    result.lcd_us = (uint32_t)(lcd / count);
    result.worst_us = sorted[count - 1];
    /* Nearest-rank percentile, including the slowest five percent. */
    result.p95_us = sorted[(95 * count + 99) / 100 - 1];
    return result;
}
