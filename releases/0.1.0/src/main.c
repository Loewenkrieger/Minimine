#include "game.h"
#include "perf.h"
#include "render.h"

#include <gint/config.h>
#include <gint/display.h>
#include <gint/keyboard.h>
#include <gint/prof.h>
#include <stdbool.h>
#include <string.h>

HHK_NAME("Casio Sandbox")
HHK_DESCRIPTION("2D block sandbox and repeatable performance test")
HHK_AUTHOR("Casio Sandbox project")
HHK_VERSION("0.1.0")

_Static_assert((int)BENCH_SAMPLES <= (int)PERF_CAPACITY, "Benchmark history is too small");

typedef enum { MODE_HELP, MODE_PLAY, MODE_BENCH, MODE_RESULTS } Mode;

typedef struct {
    bool jump, mine, place, material, zero, clear, exe;
} Presses;

/* About 50 KiB for both worlds, inside gint's fixed application region. */
static Game play_game, bench_game;
static PerfHistory live_history, bench_history;
static PerfSummary results[BENCH_PHASES];

static Presses read_input(void)
{
    Presses pressed = {0};
    key_event_t event;
    while((event = pollevent()).type != KEYEV_NONE) {
        if(event.type != KEYEV_DOWN) continue;
        switch(event.key) {
        case KEY_SHIFT: pressed.jump = true; break;
        case KEY_DEL: pressed.mine = true; break;
        case KEY_EXE: pressed.place = pressed.exe = true; break;
        case KEY_5: pressed.material = true; break;
        case KEY_0: pressed.zero = true; break;
        case KEY_CLEAR: pressed.clear = true; break;
        }
    }
    return pressed;
}

static GameInput held_input(void)
{
    GameInput input = {0};
    input.move = !!keydown(KEY_RIGHT) - !!keydown(KEY_LEFT);
    input.aim_x = !!(keydown(KEY_3) || keydown(KEY_6) || keydown(KEY_9)) -
        !!(keydown(KEY_1) || keydown(KEY_4) || keydown(KEY_7));
    input.aim_y = !!(keydown(KEY_1) || keydown(KEY_2) || keydown(KEY_3)) -
        !!(keydown(KEY_7) || keydown(KEY_8) || keydown(KEY_9));
    input.mine = keydown(KEY_DEL);
    input.place = keydown(KEY_EXE);
    return input;
}

static uint32_t elapsed_us(uint32_t start, uint32_t end)
{
    /* TMU counts down; unsigned subtraction also handles a counter wrap. */
    prof_t context = prof_make();
    context.elapsed = start - end;
    return prof_time(context);
}

static void wait_page_key(Presses *pressed)
{
    /* Page screens block instead of burning CPU or distorting game timings. */
    key_event_t event = getkey_opt(GETKEY_NONE, NULL);
    memset(pressed, 0, sizeof(*pressed));
    pressed->exe = event.key == KEY_EXE;
    pressed->zero = event.key == KEY_0;
    pressed->clear = event.key == KEY_CLEAR;
}

int main(void)
{
    if(prof_init()) {
        render_error("Kein freier Timer fuer die Messung.");
        dupdate();
        for(;;) {
            key_event_t event = getkey_opt(GETKEY_NONE, NULL);
            if(event.key == KEY_EXE || event.key == KEY_CLEAR) return 1;
        }
    }
    game_init(&play_game, WORLD_SEED);
    Mode mode = MODE_HELP;
    bool running = true, page_dirty = true;
    unsigned phase = 0, phase_frame = 0;
    uint32_t frame_start = *prof_tcnt, previous_start = frame_start;
    uint32_t previous_end = frame_start;
    uint32_t hud_elapsed = 0, dropped_ticks = 0;
    uint64_t accumulator = 0; /* Elapsed microseconds multiplied by SIM_HZ. */
    GameInput pending = {0};
    RenderInfo info = {0};

    while(running) {
        if(mode == MODE_HELP || mode == MODE_RESULTS) {
            if(page_dirty) {
                if(mode == MODE_HELP) render_help();
                else render_results(results);
                dupdate();
                page_dirty = false;
            }
            Presses pressed;
            wait_page_key(&pressed);
            if(pressed.clear) {
                if(mode == MODE_HELP) running = false;
                else { mode = MODE_HELP; page_dirty = true; }
                continue;
            }
            if(!pressed.zero && !pressed.exe) continue;
            if(pressed.zero) {
                game_init(&bench_game, WORLD_SEED);
                phase = phase_frame = 0;
                perf_reset(&bench_history);
                memset(results, 0, sizeof(results));
                mode = MODE_BENCH;
            } else mode = MODE_PLAY;
            perf_reset(&live_history);
            info = (RenderInfo){0};
            accumulator = hud_elapsed = 0;
            pending = (GameInput){0};
            /* Wait for EXE to be released before enabling block placement. */
            while(keydown(KEY_EXE) || keydown(KEY_0))
                (void)waitevent(NULL);
            previous_start = *prof_tcnt;
            previous_end = previous_start;
            continue;
        }

        frame_start = *prof_tcnt;
        uint32_t delta_us = elapsed_us(previous_start, frame_start);
        previous_start = frame_start;
        uint32_t logic_start = *prof_tcnt;
        Presses pressed = read_input();
        if(pressed.clear) {
            mode = MODE_HELP;
            page_dirty = true;
            continue;
        }
        if(pressed.zero && mode == MODE_PLAY) {
            game_init(&bench_game, WORLD_SEED);
            phase = phase_frame = 0;
            memset(results, 0, sizeof(results));
            perf_reset(&bench_history);
            perf_reset(&live_history);
            info = (RenderInfo){0};
            mode = MODE_BENCH;
            previous_start = *prof_tcnt;
            previous_end = previous_start;
            continue;
        }

        Game *game = mode == MODE_BENCH ? &bench_game : &play_game;
        if(mode == MODE_PLAY) {
            GameInput input = held_input();
            pending.jump |= pressed.jump;
            pending.mine |= pressed.mine;
            pending.place |= pressed.place;
            if(pressed.material)
                game->selected = (Item)((game->selected + 1) % ITEM_COUNT);
            accumulator += (uint64_t)delta_us * SIM_HZ;
            unsigned steps = 0;
            while(accumulator >= 1000000 && steps < 8) {
                input.jump = pending.jump;
                input.mine |= pending.mine;
                input.place |= pending.place;
                game_step(game, input);
                pending = (GameInput){0};
                input.jump = false;
                input.mine = keydown(KEY_DEL);
                input.place = keydown(KEY_EXE);
                accumulator -= 1000000;
                ++steps;
            }
            /* Keep input responsive during extreme stalls and expose loss. */
            if(accumulator >= 1000000) {
                dropped_ticks += (uint32_t)(accumulator / 1000000);
                accumulator %= 1000000;
            }
        } else game_benchmark_step(game, phase, phase_frame);

        uint32_t draw_start = *prof_tcnt;
        info.benchmark = mode == MODE_BENCH;
        info.phase = phase;
        info.phase_frame = phase_frame;
        info.dropped_ticks = mode == MODE_PLAY ? dropped_ticks : 0;
        render_game(game, &info);
        uint32_t lcd_start = *prof_tcnt;
        dupdate();
        uint32_t frame_end = *prof_tcnt;
        PerfSample sample = {
            /* Presentation-to-presentation includes measurement overhead. */
            .frame_us = elapsed_us(previous_end, frame_end),
            .logic_us = elapsed_us(logic_start, draw_start),
            .draw_us = elapsed_us(draw_start, lcd_start),
            .lcd_us = elapsed_us(lcd_start, frame_end)
        };
        previous_end = frame_end;
        perf_push(&live_history, sample);
        hud_elapsed += sample.frame_us;
        if(hud_elapsed >= 500000) {
            info.perf = perf_summarize(live_history.samples, live_history.count);
            hud_elapsed = 0;
        }
        if(mode == MODE_BENCH) {
            if(phase_frame >= BENCH_WARMUP) perf_push(&bench_history, sample);
            if(++phase_frame == BENCH_PHASE_FRAMES) {
                results[phase] = perf_summarize(bench_history.samples,
                    bench_history.count);
                perf_reset(&bench_history);
                phase_frame = 0;
                if(++phase == BENCH_PHASES) {
                    mode = MODE_RESULTS;
                    page_dirty = true;
                }
            }
        }
    }
    prof_quit();
    return 0;
}
