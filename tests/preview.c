#include "game.h"
#include "render.h"
#include <gint/display.h>
#include <stdio.h>

static Game game;

int main(void)
{
    /* No synthetic FPS values: a host screenshot cannot measure the CP400. */
    RenderInfo info = {0};
    game_init(&game, WORLD_SEED);
    render_game(&game, &info);
    host_write_ppm("build/native/play.ppm");
    InventoryUI ui={.cursor=4,.source=0};
    render_inventory(&game,&ui);
    host_write_ppm("build/native/inventory.ppm");
    game_toggle_sneak(&game);
    game.aim_x=game.aim_y=1;
    game.mining=(Mining){.active=true,.required=60,.elapsed=30};
    game_target(&game,&game.mining.x,&game.mining.y);
    render_game(&game,&info);
    host_write_ppm("build/native/sneak-mining.ppm");
    game_init(&game,WORLD_SEED);
    render_help();
    host_write_ppm("build/native/help.ppm");
    for(unsigned phase = 0; phase < BENCH_PHASES; ++phase) {
        game_benchmark_step(&game, phase, 90);
        info.benchmark = true;
        info.phase = phase;
        info.phase_frame = 90;
        render_game(&game, &info);
        char path[80];
        snprintf(path, sizeof(path), "build/native/scene-%u.ppm", phase);
        host_write_ppm(path);
    }
    PerfSummary results[BENCH_PHASES] = {{0}};
    render_results(results);
    host_write_ppm("build/native/results.ppm");
    puts("Host previews rendered (no device performance measurement).");
    return 0;
}

