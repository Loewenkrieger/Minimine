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

/* Navigation belongs to the controller; rendering never changes stacks. */
typedef struct { unsigned cursor; int source; } InventoryUI;

void render_invalidate(void);
void render_world(const Game *game, const RenderInfo *info);
void render_game(const Game *game, const RenderInfo *info);
void render_inventory(const Game *game, const InventoryUI *ui);
void render_help(void);
void render_results(const PerfSummary results[BENCH_PHASES]);
void render_error(const char *message);

#endif

