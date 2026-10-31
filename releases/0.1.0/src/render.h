#ifndef SANDBOX_RENDER_H
#define SANDBOX_RENDER_H

#include "game.h"
#include "perf.h"

typedef struct {
    PerfSummary perf;
    bool benchmark;
    unsigned phase, phase_frame;
    uint32_t dropped_ticks;
} RenderInfo;

void render_game(const Game *game, const RenderInfo *info);
void render_help(void);
void render_results(const PerfSummary results[BENCH_PHASES]);
void render_error(const char *message);

#endif

