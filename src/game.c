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

int game_player_height(const Player *player)
{
    return player->sneaking ? PLAYER_CROUCH_H : PLAYER_H;
}

void game_follow_camera(Game *game)
{
    const Player *p = &game->player;
    int x = p->x / FIXED_ONE + PLAYER_W / 2 - SCREEN_W / 2;
    int feet = (p->y + game_player_height(p) * FIXED_ONE) / FIXED_ONE;
    int y = feet - PLAYER_H / 2 - VIEW_H / 2;
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
    for(unsigned i = 0; i < HOTBAR_SLOTS; ++i)
        game->slots[i] = (ItemStack){ .item = (Item)(i + 1), .count = 32 };
    game_follow_camera(game);
}

static bool box_clear(const Game *game, int32_t x, int32_t y, int height)
{
    int left = floor_div(x, TILE_SIZE * FIXED_ONE);
    int right = floor_div(x + PLAYER_W * FIXED_ONE - 1, TILE_SIZE * FIXED_ONE);
    int top = floor_div(y, TILE_SIZE * FIXED_ONE);
    int bottom = floor_div(y + height * FIXED_ONE - 1, TILE_SIZE * FIXED_ONE);
    for(int ty = top; ty <= bottom; ++ty)
        for(int tx = left; tx <= right; ++tx)
            if(game_tile_solid(game_tile(game, tx, ty))) return false;
    return true;
}

bool game_toggle_sneak(Game *game)
{
    Player *p = &game->player;
    int difference = (PLAYER_H - PLAYER_CROUCH_H) * FIXED_ONE;
    if(p->sneaking) {
        if(!box_clear(game, p->x, p->y - difference, PLAYER_H)) {
            game->notice = NOTICE_NO_SPACE;
            game->notice_ticks = SIM_HZ;
            return false;
        }
        p->y -= difference;
    } else p->y += difference;
    p->sneaking = !p->sneaking;
    game_follow_camera(game);
    return true;
}

static bool supported_at(const Game *game, int32_t x)
{
    const Player *p = &game->player;
    int row = floor_div(p->y + game_player_height(p) * FIXED_ONE,
        TILE_SIZE * FIXED_ONE);
    int left = floor_div(x, TILE_SIZE * FIXED_ONE);
    int right = floor_div(x + PLAYER_W * FIXED_ONE - 1, TILE_SIZE * FIXED_ONE);
    for(int tx = left; tx <= right; ++tx)
        if(game_tile_solid(game_tile(game, tx, row))) return true;
    return false;
}

static void move_horizontal(Game *game)
{
    Player *p = &game->player;
    int32_t old = p->x, next = old + p->vx;
    int top = floor_div(p->y, TILE_SIZE * FIXED_ONE);
    int bottom = floor_div(p->y + game_player_height(p) * FIXED_ONE - 1,
        TILE_SIZE * FIXED_ONE);
    int edge = floor_div(next + (p->vx > 0 ? PLAYER_W * FIXED_ONE - 1 : 0),
        TILE_SIZE * FIXED_ONE);
    if(p->vx) {
        for(int y = top; y <= bottom; ++y) {
            if(!game_tile_solid(game_tile(game, edge, y))) continue;
            next = (p->vx > 0 ? edge * TILE_SIZE - PLAYER_W :
                (edge + 1) * TILE_SIZE) * FIXED_ONE;
            p->vx = 0;
            break;
        }
    }
    if(p->sneaking && p->grounded && supported_at(game, old) &&
       !supported_at(game, next)) {
        int sign = next > old ? 1 : -1;
        int low = 0, high = next > old ? next - old : old - next;
        while(low < high) {
            int mid = low + (high - low + 1) / 2;
            if(supported_at(game, old + sign * mid)) low = mid;
            else high = mid - 1;
        }
        next = old + sign * low;
        p->vx = 0;
    }
    p->x = next;
}

static void move_vertical(Game *game)
{
    Player *p = &game->player;
    int height = game_player_height(p);
    int32_t next = p->y + p->vy;
    int left = floor_div(p->x, TILE_SIZE * FIXED_ONE);
    int right = floor_div(p->x + PLAYER_W * FIXED_ONE - 1,
        TILE_SIZE * FIXED_ONE);
    int edge = floor_div(next + (p->vy > 0 ? height * FIXED_ONE - 1 : 0),
        TILE_SIZE * FIXED_ONE);
    p->grounded = false;
    if(p->vy) {
        for(int x = left; x <= right; ++x) {
            if(!game_tile_solid(game_tile(game, x, edge))) continue;
            p->grounded = p->vy > 0;
            p->y = (p->vy > 0 ? edge * TILE_SIZE - height :
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
    int py = game->player.y / FIXED_ONE + game_player_height(&game->player) / 2;
    *x = floor_div(px + game->aim_x * 24, TILE_SIZE);
    *y = floor_div(py + game->aim_y * 24, TILE_SIZE);
}

const char *game_item_name(Item item)
{
    static const char *const names[] = { "Leer", "Erde", "Stein", "Holz" };
    return (unsigned)item < ITEM_COUNT ? names[item] : "Leer";
}

Tile game_item_tile(Item item)
{
    static const Tile tiles[] = { TILE_AIR, TILE_DIRT, TILE_STONE, TILE_WOOD };
    return (unsigned)item < ITEM_COUNT ? tiles[item] : TILE_AIR;
}

void game_select_hotbar(Game *game, unsigned slot)
{
    if(slot < HOTBAR_SLOTS) game->selected = slot;
}

unsigned game_item_total(const Game *game, Item item)
{
    unsigned total = 0;
    for(unsigned i = 0; i < TOTAL_SLOTS; ++i)
        if(game->slots[i].item == item) total += game->slots[i].count;
    return total;
}

static int pickup_slot(const Game *game, Item item)
{
    if(item <= ITEM_NONE || item >= ITEM_COUNT) return -1;
    /* Matching hotbar then matching bag; empty bag before empty hotbar. */
    for(int i = 0; i < TOTAL_SLOTS; ++i)
        if(game->slots[i].item == item && game->slots[i].count < INVENTORY_LIMIT)
            return i;
    for(int i = HOTBAR_SLOTS; i < TOTAL_SLOTS; ++i)
        if(game->slots[i].count == 0) return i;
    for(int i = 0; i < HOTBAR_SLOTS; ++i)
        if(game->slots[i].count == 0) return i;
    return -1;
}

bool game_add_item(Game *game, Item item)
{
    int index = pickup_slot(game, item);
    if(index < 0) return false;
    game->slots[index].item = item;
    ++game->slots[index].count;
    return true;
}

bool game_inventory_transfer(Game *game, unsigned from, unsigned to)
{
    if(from >= TOTAL_SLOTS || to >= TOTAL_SLOTS || from == to) return false;
    ItemStack *source = &game->slots[from], *target = &game->slots[to];
    if(!source->count) return false;
    if(target->count && source->item == target->item) {
        unsigned room = INVENTORY_LIMIT - target->count;
        unsigned amount = source->count < room ? source->count : room;
        target->count = (uint16_t)(target->count + amount);
        source->count = (uint16_t)(source->count - amount);
        if(!source->count) source->item = ITEM_NONE;
        return amount != 0;
    }
    ItemStack swap = *target;
    *target = *source;
    *source = swap;
    return true;
}

static Item dropped_item(Tile tile)
{
    switch(tile) {
    case TILE_GRASS: case TILE_DIRT: return ITEM_DIRT;
    case TILE_STONE: case TILE_ORE: return ITEM_STONE;
    case TILE_WOOD: case TILE_TRUNK: return ITEM_WOOD;
    default: return ITEM_NONE;
    }
}

unsigned game_mining_ticks(Tile tile)
{
    switch(tile) {
    case TILE_GRASS: case TILE_DIRT: return 21;
    case TILE_WOOD: case TILE_TRUNK: return 36;
    case TILE_STONE: case TILE_ORE: return 60;
    case TILE_LEAVES: return 9;
    default: return 0;
    }
}

void game_cancel_mining(Game *game)
{
    game->mining = (Mining){0};
}

static void mining_step(Game *game)
{
    int x, y;
    game_target(game, &x, &y);
    Tile tile = game_tile(game, x, y);
    unsigned required = game_mining_ticks(tile);
    Item drop = dropped_item(tile);
    if(!required) { game_cancel_mining(game); return; }
    if(drop != ITEM_NONE && pickup_slot(game, drop) < 0) {
        game_cancel_mining(game);
        game->notice = NOTICE_FULL;
        game->notice_ticks = SIM_HZ;
        return;
    }
    Mining *mining = &game->mining;
    if(!mining->active || mining->x != x || mining->y != y || mining->tile != tile)
        *mining = (Mining){ .x = x, .y = y, .tile = tile,
            .required = required, .active = true };
    if(++mining->elapsed < required) return;
    if(drop == ITEM_NONE || game_add_item(game, drop))
        game_set_tile(game, x, y, TILE_AIR);
    game_cancel_mining(game);
}

bool game_place(Game *game)
{
    if(game->selected >= HOTBAR_SLOTS) return false;
    ItemStack *stack = &game->slots[game->selected];
    Tile tile = game_item_tile(stack->item);
    if(!stack->count || tile == TILE_AIR) return false;
    int x, y;
    game_target(game, &x, &y);
    if(game_tile(game, x, y) != TILE_AIR) return false;
    const Player *p = &game->player;
    int32_t bx = x * TILE_SIZE * FIXED_ONE;
    int32_t by = y * TILE_SIZE * FIXED_ONE;
    if(p->x < bx + TILE_SIZE * FIXED_ONE && p->x + PLAYER_W * FIXED_ONE > bx &&
       p->y < by + TILE_SIZE * FIXED_ONE &&
       p->y + game_player_height(p) * FIXED_ONE > by) return false;
    if(!game_set_tile(game, x, y, tile)) return false;
    if(!--stack->count) stack->item = ITEM_NONE;
    return true;
}

void game_step(Game *game, GameInput input)
{
    Player *p = &game->player;
    if(game->notice_ticks && !--game->notice_ticks) game->notice = NOTICE_NONE;
    if(input.toggle_sneak) game_toggle_sneak(game);
    int limit = p->sneaking ? 240 : 480;
    int acceleration = p->sneaking ? 40 : 80;
    int friction = p->sneaking ? 50 : 100;
    int move = clamp(input.move, -1, 1);
    p->vx = clamp(p->vx, -limit, limit);
    if(move) {
        p->vx = clamp(p->vx + move * acceleration, -limit, limit);
        p->facing = move;
        game->aim_x = move;
        game->aim_y = 0;
    } else if(p->vx > 0) p->vx = p->vx > friction ? p->vx - friction : 0;
    else if(p->vx < 0) p->vx = p->vx < -friction ? p->vx + friction : 0;
    if(input.aim_x || input.aim_y) {
        game->aim_x = clamp(input.aim_x, -1, 1);
        game->aim_y = clamp(input.aim_y, -1, 1);
    }
    if(input.jump && p->grounded) {
        p->vy = -1536;
        p->grounded = false;
    }
    p->vy = clamp(p->vy + 64, -1536, 2048);
    move_horizontal(game);
    move_vertical(game);
    if(game->place_cooldown) --game->place_cooldown;
    if(input.mine) mining_step(game);
    else {
        game_cancel_mining(game);
        if(input.place && !game->place_cooldown && game_place(game))
            game->place_cooldown = 9;
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
