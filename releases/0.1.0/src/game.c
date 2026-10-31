#include "game.h"

#include <string.h>

static int clamp(int value, int low, int high)
{
    return value < low ? low : value > high ? high : value;
}

static int floor_div(int value, int divisor)
{
    return value >= 0 ? value / divisor : -((-value + divisor - 1) / divisor);
}

static uint32_t hash(uint32_t value)
{
    value ^= value >> 16;
    value *= UINT32_C(0x7feb352d);
    value ^= value >> 15;
    value *= UINT32_C(0x846ca68b);
    return value ^ (value >> 16);
}

static int terrain_noise(uint32_t seed, int x, int scale)
{
    unsigned cell = (unsigned)(x / scale);
    int a = (int)(hash(seed ^ cell) % 17) - 8;
    int b = (int)(hash(seed ^ (cell + 1)) % 17) - 8;
    return a + (b - a) * (x % scale) / scale;
}

Tile game_tile(const Game *game, int x, int y)
{
    if(x < 0 || y < 0 || x >= WORLD_W || y >= WORLD_H)
        return TILE_BEDROCK;
    return (Tile)game->tiles[y][x];
}

bool game_set_tile(Game *game, int x, int y, Tile tile)
{
    if(x < 0 || y < 0 || x >= WORLD_W || y >= WORLD_H ||
       (unsigned)tile >= TILE_COUNT)
        return false;
    game->tiles[y][x] = (uint8_t)tile;
    return true;
}

bool game_tile_solid(Tile tile)
{
    return tile != TILE_AIR && tile != TILE_TRUNK && tile != TILE_LEAVES;
}

static void generate_world(Game *game)
{
    for(int x = 0; x < WORLD_W; ++x) {
        int height = 30 + terrain_noise(game->seed, x, 32) +
            terrain_noise(game->seed ^ 91, x, 11) / 3;
        /* A flat clearing gives every run a useful starting location. */
        if(x >= 7 && x <= 19) height = 30;
        game->surface[x] = (uint8_t)height;
        for(int y = height; y < WORLD_H; ++y) {
            Tile tile = y == height ? TILE_GRASS :
                y < height + 5 ? TILE_DIRT : TILE_STONE;
            if(tile == TILE_STONE &&
               hash(game->seed ^ (uint32_t)(x + y * WORLD_W)) % 29 == 0)
                tile = TILE_ORE;
            if(y >= WORLD_H - 2) tile = TILE_BEDROCK;
            game->tiles[y][x] = (uint8_t)tile;
        }
    }
    /* Connected tunnels plus small chambers; no runtime world generation. */
    for(int x = 2; x < WORLD_W - 2; ++x) {
        int center = 53 + terrain_noise(game->seed ^ 33, x, 24) / 2;
        for(int y = center - 3; y <= center + 3; ++y)
            game->tiles[y][x] = TILE_AIR;
    }
    for(unsigned i = 0; i < 24; ++i) {
        uint32_t value = hash(game->seed + i * 7919);
        int cx = 6 + (int)(value % (WORLD_W - 12));
        int cy = 44 + (int)((value >> 8) % 40);
        int radius = 3 + (int)((value >> 16) % 4);
        for(int y = cy - radius; y <= cy + radius; ++y)
            for(int x = cx - radius; x <= cx + radius; ++x)
                if((x - cx) * (x - cx) + (y - cy) * (y - cy) <= radius * radius)
                    game_set_tile(game, x, y, TILE_AIR);
    }
    for(int x = 26; x < WORLD_W - 4; x += 13) {
        int ground = game->surface[x];
        for(int y = ground - 6; y < ground - 3; ++y)
            for(int dx = -2; dx <= 2; ++dx)
                if(dx != -2 || y != ground - 6)
                    game_set_tile(game, x + dx, y, TILE_LEAVES);
        for(int y = ground - 5; y < ground; ++y)
            game_set_tile(game, x, y, TILE_TRUNK);
    }
}

void game_follow_camera(Game *game)
{
    int x = game->player.x / FIXED_ONE + PLAYER_W / 2 - SCREEN_W / 2;
    int y = game->player.y / FIXED_ONE + PLAYER_H / 2 - VIEW_H / 2;
    game->camera_x = clamp(x, 0, WORLD_W * TILE_SIZE - SCREEN_W);
    game->camera_y = clamp(y, 0, WORLD_H * TILE_SIZE - VIEW_H);
}

void game_init(Game *game, uint32_t seed)
{
    memset(game, 0, sizeof(*game));
    game->seed = seed;
    generate_world(game);
    game->player.x = (12 * TILE_SIZE + 3) * FIXED_ONE;
    game->player.y = (30 * TILE_SIZE - PLAYER_H) * FIXED_ONE;
    game->player.facing = 1;
    game->player.grounded = true;
    game->aim_x = 1;
    for(int item = 0; item < ITEM_COUNT; ++item)
        game->inventory[item] = 32;
    game_follow_camera(game);
}

static void move_horizontal(Game *game)
{
    Player *p = &game->player;
    int32_t next = p->x + p->vx;
    int top = floor_div(p->y, TILE_SIZE * FIXED_ONE);
    int bottom = floor_div(p->y + PLAYER_H * FIXED_ONE - 1,
        TILE_SIZE * FIXED_ONE);
    int edge = floor_div(next + (p->vx > 0 ? PLAYER_W * FIXED_ONE - 1 : 0),
        TILE_SIZE * FIXED_ONE);
    if(p->vx) {
        for(int y = top; y <= bottom; ++y) {
            if(!game_tile_solid(game_tile(game, edge, y))) continue;
            p->x = (p->vx > 0 ? edge * TILE_SIZE - PLAYER_W :
                (edge + 1) * TILE_SIZE) * FIXED_ONE;
            p->vx = 0;
            return;
        }
    }
    p->x = next;
}

static void move_vertical(Game *game)
{
    Player *p = &game->player;
    int32_t next = p->y + p->vy;
    int left = floor_div(p->x, TILE_SIZE * FIXED_ONE);
    int right = floor_div(p->x + PLAYER_W * FIXED_ONE - 1,
        TILE_SIZE * FIXED_ONE);
    int edge = floor_div(next + (p->vy > 0 ? PLAYER_H * FIXED_ONE - 1 : 0),
        TILE_SIZE * FIXED_ONE);
    p->grounded = false;
    if(p->vy) {
        for(int x = left; x <= right; ++x) {
            if(!game_tile_solid(game_tile(game, x, edge))) continue;
            p->grounded = p->vy > 0;
            p->y = (p->vy > 0 ? edge * TILE_SIZE - PLAYER_H :
                (edge + 1) * TILE_SIZE) * FIXED_ONE;
            p->vy = 0;
            return;
        }
    }
    p->y = next;
}

void game_target(const Game *game, int *x, int *y)
{
    int px = game->player.x / FIXED_ONE + PLAYER_W / 2;
    int py = game->player.y / FIXED_ONE + PLAYER_H / 2;
    *x = floor_div(px + game->aim_x * 24, TILE_SIZE);
    *y = floor_div(py + game->aim_y * 24, TILE_SIZE);
}

static int dropped_item(Tile tile)
{
    switch(tile) {
    case TILE_GRASS: case TILE_DIRT: return ITEM_DIRT;
    case TILE_STONE: case TILE_ORE: return ITEM_STONE;
    case TILE_WOOD: case TILE_TRUNK: return ITEM_WOOD;
    default: return -1;
    }
}

bool game_mine(Game *game)
{
    int x, y;
    game_target(game, &x, &y);
    Tile tile = game_tile(game, x, y);
    if(tile == TILE_AIR || tile == TILE_BEDROCK) return false;
    int item = dropped_item(tile);
    if(item >= 0 && game->inventory[item] >= INVENTORY_LIMIT) return false;
    if(!game_set_tile(game, x, y, TILE_AIR)) return false;
    if(item >= 0) ++game->inventory[item];
    return true;
}

bool game_place(Game *game)
{
    static const Tile tiles[ITEM_COUNT] = { TILE_DIRT, TILE_STONE, TILE_WOOD };
    if((unsigned)game->selected >= ITEM_COUNT || !game->inventory[game->selected])
        return false;
    int x, y;
    game_target(game, &x, &y);
    if(game_tile(game, x, y) != TILE_AIR) return false;
    const Player *p = &game->player;
    int32_t bx = x * TILE_SIZE * FIXED_ONE;
    int32_t by = y * TILE_SIZE * FIXED_ONE;
    if(p->x < bx + TILE_SIZE * FIXED_ONE && p->x + PLAYER_W * FIXED_ONE > bx &&
       p->y < by + TILE_SIZE * FIXED_ONE && p->y + PLAYER_H * FIXED_ONE > by)
        return false;
    if(!game_set_tile(game, x, y, tiles[game->selected])) return false;
    --game->inventory[game->selected];
    return true;
}

void game_step(Game *game, GameInput input)
{
    Player *p = &game->player;
    int move = clamp(input.move, -1, 1);
    if(move) {
        p->vx = clamp(p->vx + move * 80, -480, 480);
        p->facing = move;
    } else if(p->vx > 0) p->vx = p->vx > 100 ? p->vx - 100 : 0;
    else if(p->vx < 0) p->vx = p->vx < -100 ? p->vx + 100 : 0;

    if(input.aim_x || input.aim_y) {
        game->aim_x = clamp(input.aim_x, -1, 1);
        game->aim_y = clamp(input.aim_y, -1, 1);
    }
    if(input.jump && p->grounded) p->vy = -1536;
    p->vy = clamp(p->vy + 64, -1536, 2048);
    move_horizontal(game);
    move_vertical(game);
    if(game->action_cooldown) --game->action_cooldown;
    if(!game->action_cooldown) {
        bool acted = input.mine ? game_mine(game) :
            input.place ? game_place(game) : false;
        if(acted) game->action_cooldown = 9;
    }
    ++game->ticks;
    game_follow_camera(game);
}

void game_benchmark_step(Game *game, unsigned phase, unsigned frame)
{
    GameInput input = { .move = 1, .jump = frame % 80 == 0 };
    game_step(game, input);
    int travel = WORLD_W * TILE_SIZE - SCREEN_W;
    game->camera_x = (int)(frame % BENCH_PHASE_FRAMES) * travel /
        (BENCH_PHASE_FRAMES - 1);
    int column = (game->camera_x + SCREEN_W / 2) / TILE_SIZE;
    int center_y = phase == 0 ? game->surface[column] * TILE_SIZE - 45 :
        phase == 1 ? 54 * TILE_SIZE : 80 * TILE_SIZE;
    game->camera_y = clamp(center_y - VIEW_H / 2, 0,
        WORLD_H * TILE_SIZE - VIEW_H);
}

const char *game_phase_name(unsigned phase)
{
    static const char *const names[] = { "Oberflaeche", "Hoehlen", "Tiefengestein" };
    return phase < BENCH_PHASES ? names[phase] : "Unbekannt";
}

