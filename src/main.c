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
HHK_VERSION(SANDBOX_VERSION)

_Static_assert((int)BENCH_SAMPLES <= (int)PERF_CAPACITY, "Benchmark history is too small");

typedef enum { MODE_HELP, MODE_PLAY, MODE_BENCH, MODE_RESULTS, MODE_INVENTORY } Mode;

typedef struct {
    bool jump, mine_released, place, zero, clear, exe, inventory;
    int hotbar;
    unsigned sneak_toggles;
} Presses;

/* About 50 KiB for both worlds, inside gint's fixed application region. */
static Game play_game, bench_game;
static PerfHistory live_history, bench_history;
static PerfSummary results[BENCH_PHASES];

static Presses read_input(void)
{
    Presses pressed = { .hotbar = -1 };
    key_event_t event;
    while((event = pollevent()).type != KEYEV_NONE) {
        if(event.type == KEYEV_UP && event.key == KEY_DEL) pressed.mine_released = true;
        if(event.type != KEYEV_DOWN) continue;
        switch(event.key) {
        case KEY_SHIFT: case KEY_UP: pressed.jump = true; break;
        case KEY_DOWN: ++pressed.sneak_toggles; break;
        case KEY_EXE: pressed.place = pressed.exe = true; break;
        case KEY_KBD: pressed.inventory = true; break;
        case KEY_X: pressed.hotbar = 0; break;
        case KEY_Y: pressed.hotbar = 1; break;
        case KEY_Z: pressed.hotbar = 2; break;
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

/* Cursor, source selection and menu dimensions belong to the controller. */
static bool inventory_key(Game *game, InventoryUI *ui, int key)
{
    unsigned width = ui->craft_width == 3 ? 3 : 2;
    if(key == KEY_CLEAR || key == KEY_KBD) {
        if(!game_close_crafting(game)) return false;
        ui->source = -1;
        return true;
    }
    if(ui->cursor < TOTAL_SLOTS) {
        unsigned col = ui->cursor % 3;
        if(key == KEY_LEFT && col) --ui->cursor;
        if(key == KEY_RIGHT && col < 2) ++ui->cursor;
        if(key == KEY_UP && ui->cursor >= 3) ui->cursor -= 3;
        if(key == KEY_DOWN) {
            if(ui->cursor + 3 < TOTAL_SLOTS) ui->cursor += 3;
            else ui->cursor = CRAFT_FIRST + (col < width ? col : width - 1);
        }
    } else if(ui->cursor < CRAFT_FIRST + width * width) {
        unsigned index = ui->cursor - CRAFT_FIRST, col = index % width;
        if(key == KEY_LEFT && col) --ui->cursor;
        if(key == KEY_RIGHT)
            ui->cursor = col + 1 < width ? ui->cursor + 1 : CRAFT_RESULT;
        if(key == KEY_UP)
            ui->cursor = index >= width ? ui->cursor - width : TOTAL_SLOTS - 3 + col;
        if(key == KEY_DOWN && index + width < width * width) ui->cursor += width;
    } else if(ui->cursor == CRAFT_RESULT) {
        if(key == KEY_LEFT) ui->cursor = CRAFT_FIRST + width * (width / 2) + width - 1;
        if(key == KEY_UP) ui->cursor = CRAFT_FIRST + width - 1;
        if(key == KEY_DOWN) ui->cursor = CRAFT_FIRST + width * width - 1;
    }
    int hotbar = key == KEY_X ? 0 : key == KEY_Y ? 1 : key == KEY_Z ? 2 : -1;
    if(hotbar >= 0) {
        ui->cursor = (unsigned)hotbar;
        game_select_hotbar(game, (unsigned)hotbar);
    }
    if(key != KEY_EXE && key != KEY_5) return false;
    if(ui->cursor == CRAFT_RESULT) {
        if(key == KEY_EXE) { game_craft(game, width); ui->source = -1; }
        return false;
    }
    const ItemStack *stack = game_stack_at(game, ui->cursor);
    if(ui->source < 0) {
        if(stack && stack->count) ui->source = (int)ui->cursor;
    } else if(key == KEY_5) {
        game_inventory_transfer_one(game, (unsigned)ui->source, ui->cursor);
        const ItemStack *source = game_stack_at(game, (unsigned)ui->source);
        if(!source || !source->count) ui->source = -1;
    } else {
        game_inventory_transfer(game, (unsigned)ui->source, ui->cursor);
        ui->source = -1;
    }
    return false;
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
    bool running = true, page_dirty = true, actions_armed = false;
    InventoryUI inventory = { .source = -1, .craft_width = 2 };
    unsigned phase = 0, phase_frame = 0;
    uint32_t frame_start = *prof_tcnt, previous_start = frame_start;
    uint32_t previous_end = frame_start;
    uint32_t hud_elapsed = 0, dropped_ticks = 0;
    uint64_t accumulator = 0; /* Elapsed microseconds multiplied by SIM_HZ. */
    GameInput pending = {0};
    unsigned pending_sneak = 0;
    RenderInfo info = {0};

    while(running) {
        if(mode == MODE_HELP || mode == MODE_RESULTS || mode == MODE_INVENTORY) {
            if(page_dirty) {
                if(mode == MODE_HELP) render_help();
                else if(mode == MODE_RESULTS) render_results(results);
                else render_inventory(&play_game, &inventory);
                dupdate();
                page_dirty = false;
            }
            key_event_t event = getkey_opt(GETKEY_NONE, NULL);
            if(event.type != KEYEV_DOWN) continue;
            if(mode == MODE_INVENTORY && !inventory_key(&play_game, &inventory, event.key)) {
                page_dirty = true;
                continue;
            }
            bool resume_inventory = mode == MODE_INVENTORY;
            Presses pressed = { .exe = event.key == KEY_EXE,
                .zero = event.key == KEY_0, .clear = event.key == KEY_CLEAR };
            if(resume_inventory) pressed.exe = true;
            if(pressed.clear && !resume_inventory) {
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
            pending_sneak = 0;
            /* Menu keys cannot leak into held gameplay actions. */
            actions_armed = false;
            render_invalidate();
            previous_start = *prof_tcnt;
            previous_end = previous_start;
            continue;
        }

        frame_start = *prof_tcnt;
        uint32_t delta_us = elapsed_us(previous_start, frame_start);
        previous_start = frame_start;
        uint32_t logic_start = *prof_tcnt;
        Presses pressed = read_input();
        if(pressed.mine_released) game_cancel_mining(&play_game);
        if(pressed.clear || (pressed.inventory && mode == MODE_PLAY)) {
            game_cancel_mining(&play_game);
            mode = pressed.clear ? MODE_HELP : MODE_INVENTORY;
            inventory = (InventoryUI){ .cursor = play_game.selected, .source = -1, .craft_width = 2 };
            page_dirty = true;
            continue;
        }
        if(pressed.zero && mode == MODE_PLAY) {
            game_cancel_mining(&play_game);
            render_invalidate();
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
            if(!actions_armed && !keydown(KEY_DEL) && !keydown(KEY_EXE)) actions_armed = true;
            input.mine &= actions_armed;
            input.place &= actions_armed;
            pending.jump |= pressed.jump;
            pending_sneak += pressed.sneak_toggles;
            if(actions_armed) pending.place |= pressed.place;
            if(pressed.hotbar >= 0) game_select_hotbar(game, (unsigned)pressed.hotbar);
            accumulator += (uint64_t)delta_us * SIM_HZ;
            unsigned steps = 0;
            while(accumulator >= 1000000 && steps < 8) {
                input.jump = pending.jump;
                while(pending_sneak) {
                    game_toggle_sneak(game);
                    --pending_sneak;
                }
                input.place |= pending.place;
                input.interact = pending.place;
                GameInteraction interaction = game_step(game, input);
                pending = (GameInput){0};
                input.jump = input.interact = false;
                input.mine = actions_armed && keydown(KEY_DEL);
                input.place = actions_armed && keydown(KEY_EXE);
                accumulator -= 1000000;
                ++steps;
                if(interaction == INTERACT_WORKBENCH) {
                    inventory = (InventoryUI){ .cursor = game->selected,
                        .source = -1, .craft_width = 3 };
                    mode = MODE_INVENTORY;
                    page_dirty = true;
                    accumulator = 0;
                    break;
                }
            }
            if(mode == MODE_INVENTORY) continue;
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
