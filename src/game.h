#ifndef SANDBOX_GAME_H
#define SANDBOX_GAME_H

#include <stdbool.h>
#include <stdint.h>

enum {
    WORLD_W = 256, WORLD_H = 96, TILE_SIZE = 16,
    SCREEN_W = 320, SCREEN_H = 528,
    VIEW_TOP = 56, VIEW_H = 400,
    PLAYER_W = 10, PLAYER_H = 22, PLAYER_CROUCH_H = 14,
    FIXED_ONE = 256, SIM_HZ = 60,
    INVENTORY_LIMIT = 64, HOTBAR_SLOTS = 3, BAG_SLOTS = 12,
    TOTAL_SLOTS = HOTBAR_SLOTS + BAG_SLOTS,
    CRAFT_SLOTS = 9, CRAFT_FIRST = TOTAL_SLOTS,
    CRAFT_RESULT = TOTAL_SLOTS + CRAFT_SLOTS,
    BENCH_PHASES = 3, BENCH_WARMUP = 30, BENCH_SAMPLES = 150,
    BENCH_PHASE_FRAMES = BENCH_WARMUP + BENCH_SAMPLES
};

#define WORLD_SEED UINT32_C(0x43503430)
#define SANDBOX_VERSION "0.3.0"

typedef enum {
    TILE_AIR, TILE_GRASS, TILE_DIRT, TILE_STONE, TILE_WOOD,
    TILE_TRUNK, TILE_LEAVES, TILE_ORE, TILE_BEDROCK,
    TILE_LOG, TILE_WORKBENCH, TILE_COUNT
} Tile;

typedef enum {
    ITEM_NONE, ITEM_DIRT, ITEM_STONE, ITEM_PLANKS, ITEM_LOG,
    ITEM_STICK, ITEM_WORKBENCH, ITEM_WOOD_PICKAXE, ITEM_STONE_PICKAXE, ITEM_COUNT
} Item;

typedef struct { Item item; uint16_t count, durability; } ItemStack;
typedef enum { NOTICE_NONE, NOTICE_FULL, NOTICE_NO_SPACE, NOTICE_TOOL_BROKEN } Notice;
typedef enum { INTERACT_NONE, INTERACT_WORKBENCH } GameInteraction;

typedef struct {
    int x, y;
    Tile tile;
    unsigned elapsed, required;
    unsigned tool_slot;
    Item tool;
    uint16_t tool_durability;
    bool active;
} Mining;

typedef struct {
    int32_t x, y, vx, vy; /* 8 fractional bits; velocities per 60 Hz tick. */
    int facing;
    bool grounded, sneaking;
} Player;

typedef struct {
    int move;
    int aim_x, aim_y; /* Zero/zero leaves the current aim unchanged. */
    bool jump, toggle_sneak, mine, place, interact;
} GameInput;

typedef struct {
    uint8_t tiles[WORLD_H][WORLD_W];
    uint8_t surface[WORLD_W];
    Player player;
    ItemStack slots[TOTAL_SLOTS];
    ItemStack crafting[CRAFT_SLOTS];
    unsigned selected;
    int aim_x, aim_y;
    int camera_x, camera_y;
    Mining mining;
    unsigned place_cooldown, notice_ticks;
    Notice notice;
    uint32_t ticks;
    uint32_t seed;
} Game;

void game_init(Game *game, uint32_t seed);
GameInteraction game_step(Game *game, GameInput input);
Tile game_tile(const Game *game, int x, int y);
bool game_set_tile(Game *game, int x, int y, Tile tile);
bool game_tile_solid(Tile tile);
void game_target(const Game *game, int *x, int *y);
bool game_place(Game *game);
int game_player_height(const Player *player);
bool game_toggle_sneak(Game *game);
void game_cancel_mining(Game *game);
unsigned game_mining_ticks(Tile tile);
bool game_add_item(Game *game, Item item);
bool game_inventory_transfer(Game *game, unsigned from, unsigned to);
bool game_inventory_transfer_one(Game *game, unsigned from, unsigned to);
const ItemStack *game_stack_at(const Game *game, unsigned slot);
unsigned game_stack_limit(Item item);
unsigned game_tool_durability(Item item);
ItemStack game_craft_preview(const Game *game, unsigned width);
bool game_craft(Game *game, unsigned width);
bool game_close_crafting(Game *game);
GameInteraction game_interaction(const Game *game);
unsigned game_item_total(const Game *game, Item item);
void game_select_hotbar(Game *game, unsigned slot);
const char *game_item_name(Item item);
Tile game_item_tile(Item item);
void game_follow_camera(Game *game);
void game_benchmark_step(Game *game, unsigned phase, unsigned frame);
const char *game_phase_name(unsigned phase);

#endif

