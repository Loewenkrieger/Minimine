/* Frozen 0.1.0 simulation, compiled only for regression tests. */
#define game_init old_init
#define game_tile old_tile
#define game_set_tile old_set_tile
#define game_tile_solid old_tile_solid
#define game_follow_camera old_follow_camera
#define game_target old_target
#define game_mine old_mine
#define game_place old_place
#define game_step old_step
#define game_benchmark_step old_benchmark_step
#define game_phase_name old_phase_name
#include "../releases/0.1.0/src/game.c"

void reference_route(uint8_t tiles[WORLD_H][WORLD_W], int32_t route[540][8])
{
    static Game old;
    game_init(&old,WORLD_SEED);
    memcpy(tiles,old.tiles,sizeof(old.tiles));
    unsigned i=0;
    for(unsigned p=0;p<BENCH_PHASES;++p) for(unsigned f=0;f<BENCH_PHASE_FRAMES;++f) {
        game_benchmark_step(&old,p,f);
        int32_t *row=route[i++];
        row[0]=old.player.x;row[1]=old.player.y;
        row[2]=old.player.vx;row[3]=old.player.vy;
        row[4]=old.player.facing;row[5]=old.player.grounded;
        row[6]=old.camera_x;row[7]=old.camera_y;
    }
}

