#ifndef SANDBOX_GAME_H
#define SANDBOX_GAME_H

#include <stdbool.h>
#include <stdint.h>

enum {
    WORLD_W = 256, WORLD_H = 96, TILE_SIZE = 16,
    SCREEN_W = 320, SCREEN_H = 528,
    VIEW_TOP = 56, VIEW_H = 400,
    PLAYER_W = 10, PLAYER_H = 22,
    FIXED_ONE = 256, SIM_HZ = 60,
    INVENTORY_LIMIT = 999,
    BENCH_PHASES = 3, BENCH_WARMUP = 30, BENCH_SAMPLES = 150,
    BENCH_PHASE_FRAMES = BENCH_WARMUP + BENCH_SAMPLES
};

#define WORLD_SEED UINT32_C(0x43503430)

typedef enum {
    TILE_AIR, TILE_GRASS, TILE_DIRT, TILE_STONE, TILE_WOOD,
    TILE_TRUNK, TILE_LEAVES, TILE_ORE, TILE_BEDROCK, TILE_COUNT
} Tile;

typedef enum { ITEM_DIRT, ITEM_STONE, ITEM_WOOD, ITEM_COUNT } Item;

typedef struct {
    int32_t x, y, vx, vy; /* 8 fractional bits; velocities per 60 Hz tick. */
    int facing;
    bool grounded;
} Player;

typedef struct {
    int move;
    int aim_x, aim_y; /* Zero/zero leaves the current aim unchanged. */
    bool jump, mine, place;
} GameInput;

typedef struct {
    uint8_t tiles[WORLD_H][WORLD_W];
    uint8_t surface[WORLD_W];
    Player player;
    uint16_t inventory[ITEM_COUNT];
    Item selected;
    int aim_x, aim_y;
    int camera_x, camera_y;
    unsigned action_cooldown;
    uint32_t ticks;
    uint32_t seed;
} Game;

void game_init(Game *game, uint32_t seed);
void game_step(Game *game, GameInput input);
Tile game_tile(const Game *game, int x, int y);
bool game_set_tile(Game *game, int x, int y, Tile tile);
bool game_tile_solid(Tile tile);
void game_target(const Game *game, int *x, int *y);
bool game_mine(Game *game);
bool game_place(Game *game);
void game_follow_camera(Game *game);
void game_benchmark_step(Game *game, unsigned phase, unsigned frame);
const char *game_phase_name(unsigned phase);

#endif

