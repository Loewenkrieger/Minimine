#ifndef SANDBOX_GAME_H
#define SANDBOX_GAME_H

#include <stdbool.h>
#include <stdint.h>

enum {
    WORLD_W=256, WORLD_H=96, TILE_SIZE=16, SCREEN_W=320, SCREEN_H=528,
    VIEW_TOP=56, VIEW_H=400, PLAYER_W=10, PLAYER_H=22, PLAYER_CROUCH_H=14,
    FIXED_ONE=256, SIM_HZ=60, INVENTORY_LIMIT=64,
    HOTBAR_SLOTS=3, BAG_SLOTS=12, TOTAL_SLOTS=15,
    CRAFT_SLOTS=9, CRAFT_FIRST=15, CRAFT_RESULT=24, ARMOR_SLOTS=4,
    MOB_LIMIT=16, DROP_LIMIT=64, FURNACE_LIMIT=32,
    HEALTH_MAX=2000, HUNGER_MAX=20,
    BENCH_PHASES=3, BENCH_WARMUP=30, BENCH_SAMPLES=150,
    BENCH_PHASE_FRAMES=BENCH_WARMUP+BENCH_SAMPLES
};
#define WORLD_SEED UINT32_C(0x43503430)
#define SANDBOX_VERSION "0.4.2"

typedef enum {
    TILE_AIR, TILE_GRASS, TILE_DIRT, TILE_STONE, TILE_WOOD,
    TILE_TRUNK, TILE_LEAVES, TILE_ORE, TILE_BEDROCK, TILE_LOG, TILE_WORKBENCH,
    TILE_COAL, TILE_FURNACE, TILE_COUNT
} Tile;
#define TILE_IRON_ORE TILE_ORE

typedef enum {
    ITEM_NONE, ITEM_DIRT, ITEM_STONE, ITEM_PLANKS, ITEM_LOG, ITEM_STICK,
    ITEM_WORKBENCH, ITEM_WOOD_PICKAXE, ITEM_STONE_PICKAXE,
    ITEM_COAL, ITEM_RAW_IRON, ITEM_IRON_INGOT, ITEM_FURNACE,
    ITEM_WOOD_SWORD, ITEM_STONE_SWORD, ITEM_IRON_SWORD,
    ITEM_IRON_HELMET, ITEM_IRON_CHEST, ITEM_IRON_LEGS, ITEM_IRON_BOOTS,
    ITEM_RAW_PORK, ITEM_COOKED_PORK, ITEM_RAW_BEEF, ITEM_COOKED_BEEF,
    ITEM_RAW_CHICKEN, ITEM_COOKED_CHICKEN, ITEM_COUNT
} Item;
typedef enum { KIND_MATERIAL, KIND_BLOCK, KIND_PICKAXE, KIND_SWORD, KIND_ARMOR, KIND_FOOD } ItemKind;
typedef struct { Item item; uint16_t count, durability; } ItemStack;
typedef enum {
    NOTICE_NONE, NOTICE_FULL, NOTICE_NO_SPACE, NOTICE_TOOL_BROKEN,
    NOTICE_PICKAXE, NOTICE_FURNACE_LIMIT, NOTICE_WRONG_SLOT, NOTICE_RESPAWN
} Notice;
typedef enum { INTERACT_NONE, INTERACT_WORKBENCH, INTERACT_FURNACE } GameInteraction;
typedef enum { SLOT_INVENTORY, SLOT_CRAFT, SLOT_ARMOR, SLOT_FURNACE, SLOT_INVALID } SlotKind;
typedef struct { SlotKind kind; unsigned index, container; } SlotRef;
typedef enum { MOB_PIG, MOB_COW, MOB_CHICKEN, MOB_ZOMBIE, MOB_COUNT } MobKind;

typedef struct {
    int x,y; Tile tile; unsigned elapsed,required,tool_slot;
    Item tool; uint16_t tool_durability; bool active;
} Mining;
typedef struct {
    int32_t x,y,vx,vy; /* Q8 pixels and velocity per 60 Hz step. */
    int facing; bool grounded,sneaking;
} Player;
typedef struct {
    bool active; MobKind kind; Player body;
    uint16_t health; unsigned ai_ticks,flee_ticks,attack_ticks;
} Mob;
typedef struct {
    bool active; ItemStack stack; int32_t x,y,vy; unsigned ttl;
} Drop;
typedef struct {
    bool active; int x,y; ItemStack slots[3]; /* input, fuel, output */
    unsigned burn_ticks,burn_total,progress; Item recipe;
} Furnace;
typedef struct {
    int move,aim_x,aim_y;
    bool jump,toggle_sneak,mine,use_pressed,use_held;
} GameInput;
typedef struct {
    uint8_t tiles[WORLD_H][WORLD_W],surface[WORLD_W];
    Player player;
    ItemStack slots[TOTAL_SLOTS],crafting[CRAFT_SLOTS],armor[ARMOR_SLOTS];
    unsigned selected; int aim_x,aim_y; bool aim_manual;
    int camera_x,camera_y; Mining mining;
    unsigned place_cooldown,notice_ticks; Notice notice;
    uint32_t ticks,seed,revision,rng;
    bool survival;
    uint16_t health; uint8_t hunger;
    unsigned hurt_ticks,action_ticks,swing_ticks,hunger_ticks,exertion;
    unsigned regen_ticks,regen_cost,starve_ticks,spawn_ticks,respawns;
    int32_t spawn_x,spawn_y;
    Mob mobs[MOB_LIMIT]; Drop drops[DROP_LIMIT]; Furnace furnaces[FURNACE_LIMIT];
} Game;

void game_init(Game *game,uint32_t seed);
void game_init_benchmark(Game *game,uint32_t seed);
GameInteraction game_step(Game *game,GameInput input);
Tile game_tile(const Game *game,int x,int y);
bool game_set_tile(Game *game,int x,int y,Tile tile);
bool game_tile_solid(Tile tile);
void game_target(const Game *game,int *x,int *y);
bool game_place(Game *game);
int game_player_height(const Player *player);
bool game_toggle_sneak(Game *game);
void game_cancel_mining(Game *game);
unsigned game_mining_ticks(Tile tile);
void game_select_hotbar(Game *game,unsigned slot);
void game_follow_camera(Game *game);
void game_benchmark_step(Game *game,unsigned phase,unsigned frame);
const char *game_phase_name(unsigned phase);
bool game_rect_clear(const Game *game,int32_t x,int32_t y,int width,int height);
void game_move_actor(const Game *game,Player *p,int width,int height);
const char *game_notice_text(Notice notice);

const char *game_item_name(Item item);
const char *game_item_label(Item item,unsigned line);
Tile game_item_tile(Item item);
ItemKind game_item_kind(Item item);
unsigned game_tool_durability(Item item);
unsigned game_stack_limit(Item item);
unsigned game_armor_slot(Item item);
unsigned game_armor_points(Item item);
unsigned game_food_points(Item item);
unsigned game_sword_damage(Item item);
unsigned game_fuel_ticks(Item item);
Item game_smelt_result(Item item);
bool game_stack_valid(const ItemStack *stack);
bool game_insert_stack(ItemStack slots[TOTAL_SLOTS],ItemStack incoming);
bool game_add_stack(Game *game,ItemStack incoming);
bool game_add_item(Game *game,Item item);
bool game_can_return_crafting(const Game *game);
const ItemStack *game_ref_stack(const Game *game,SlotRef ref);
bool game_transfer(Game *game,SlotRef from,SlotRef to,bool one);
bool game_transfer_half(Game *game,SlotRef from,SlotRef to);
bool game_inventory_transfer(Game *game,unsigned from,unsigned to);
bool game_inventory_transfer_one(Game *game,unsigned from,unsigned to);
const ItemStack *game_stack_at(const Game *game,unsigned slot);
unsigned game_item_total(const Game *game,Item item);
ItemStack game_craft_preview(const Game *game,unsigned width);
bool game_craft(Game *game,unsigned width);
bool game_close_crafting(Game *game);
GameInteraction game_interaction(const Game *game);
int game_furnace_at(const Game *game,int x,int y);
void game_furnaces_step(Game *game);

void game_survival_step(Game *game);
void game_exert(Game *game);
bool game_attack(Game *game,bool require_target);
bool game_eat(Game *game);
void game_damage_player(Game *game,unsigned damage,bool mob);
unsigned game_mob_width(MobKind kind);
unsigned game_mob_height(MobKind kind);
unsigned game_mob_health(MobKind kind);
bool game_spawn_mob(Game *game,MobKind kind,int32_t x,int32_t y);
void game_drop(Game *game,ItemStack stack,int32_t x,int32_t y);
bool game_validate(const Game *game);
#endif
