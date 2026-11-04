#ifndef SANDBOX_RENDER_H
#define SANDBOX_RENDER_H

#include "game.h"
#include "perf.h"
#include "inventory_ui.h"
#include "storage.h"

typedef struct {
    PerfSummary perf;
    bool benchmark;
    unsigned phase, phase_frame;
    uint32_t dropped_ticks;
} RenderInfo;

void render_invalidate(void);
void render_world(const Game *game, const RenderInfo *info);
#ifdef MINIMINE_BENCH_LIGHT
void render_world_legacy_light(const Game *game,const RenderInfo *info);
#endif
void render_game(const Game *game, const RenderInfo *info);
void render_inventory(const Game *game, const InventoryUI *ui);
void render_help(void);
void render_results(const PerfSummary results[BENCH_PHASES]);
void render_error(const char *message);
void render_menu(bool pause,unsigned cursor);
void render_difficulty(Difficulty selected,bool new_game);
void render_progress(const Game *game);
void render_slots(bool saving,const SaveInfo slots[3],unsigned cursor);
void render_confirm(const char *question,bool yes);
void render_message(const char *message);
size_t render_graphics_bytes(void);

#endif
