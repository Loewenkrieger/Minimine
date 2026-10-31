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
    static const char *const names[] = {
        "Leer", "Erde", "Stein", "Holzbretter", "Baumstamm",
        "Stock", "Werkbank", "Holzspitzhacke", "Steinspitzhacke"
    };
    return (unsigned)item < ITEM_COUNT ? names[item] : "Leer";
}

Tile game_item_tile(Item item)
{
    static const Tile tiles[] = {
        TILE_AIR, TILE_DIRT, TILE_STONE, TILE_WOOD, TILE_LOG,
        TILE_AIR, TILE_WORKBENCH, TILE_AIR, TILE_AIR
    };
    return (unsigned)item < ITEM_COUNT ? tiles[item] : TILE_AIR;
}

unsigned game_tool_durability(Item item)
{
    return item == ITEM_WOOD_PICKAXE ? 64 : item == ITEM_STONE_PICKAXE ? 128 : 0;
}

unsigned game_stack_limit(Item item)
{
    if(item <= ITEM_NONE || item >= ITEM_COUNT) return 0;
    return game_tool_durability(item) ? 1 : INVENTORY_LIMIT;
}

static bool stack_valid(const ItemStack *stack)
{
    unsigned limit = game_stack_limit(stack->item);
    unsigned durability = game_tool_durability(stack->item);
    return stack->count && stack->count <= limit &&
        (durability ? stack->durability && stack->durability <= durability :
            stack->durability == 0);
}

const ItemStack *game_stack_at(const Game *game, unsigned slot)
{
    if(slot < TOTAL_SLOTS) return &game->slots[slot];
    if(slot < CRAFT_RESULT) return &game->crafting[slot - CRAFT_FIRST];
    return NULL;
}

static ItemStack *mutable_stack(Game *game, unsigned slot)
{
    if(slot < TOTAL_SLOTS) return &game->slots[slot];
    if(slot < CRAFT_RESULT) return &game->crafting[slot - CRAFT_FIRST];
    return NULL;
}

static void consume_one(ItemStack *stack)
{
    if(!--stack->count) *stack = (ItemStack){0};
}

/* Atomic insertion: establish capacity first, then fill matching hotbar/bag,
   empty bag, and finally empty hotbar. Tools always occupy their own slot. */
static bool insert_stack(ItemStack slots[TOTAL_SLOTS], ItemStack incoming)
{
    if(!stack_valid(&incoming)) return false;
    unsigned limit = game_stack_limit(incoming.item), capacity = 0;
    bool tool = game_tool_durability(incoming.item) != 0;
    for(unsigned i = 0; i < TOTAL_SLOTS; ++i) {
        if(!slots[i].count) capacity += limit;
        else if(!tool && slots[i].item == incoming.item && slots[i].count < limit)
            capacity += limit - slots[i].count;
    }
    if(capacity < incoming.count) return false;
    unsigned remaining = incoming.count;
    if(!tool) for(unsigned i = 0; i < TOTAL_SLOTS && remaining; ++i) {
        if(slots[i].item != incoming.item || slots[i].count >= limit) continue;
        unsigned room = limit - slots[i].count;
        unsigned amount = remaining < room ? remaining : room;
        slots[i].count = (uint16_t)(slots[i].count + amount);
        remaining -= amount;
    }
    for(unsigned pass = 0; pass < 2 && remaining; ++pass) {
        unsigned begin = pass == 0 ? HOTBAR_SLOTS : 0;
        unsigned end = pass == 0 ? TOTAL_SLOTS : HOTBAR_SLOTS;
        for(unsigned i = begin; i < end && remaining; ++i) {
            if(slots[i].count) continue;
            unsigned amount = remaining < limit ? remaining : limit;
            slots[i] = incoming;
            slots[i].count = (uint16_t)amount;
            remaining -= amount;
        }
    }
    return true;
}

void game_select_hotbar(Game *game, unsigned slot)
{
    if(slot < HOTBAR_SLOTS && slot != game->selected) {
        game_cancel_mining(game);
        game->selected = slot;
    }
}

unsigned game_item_total(const Game *game, Item item)
{
    unsigned total = 0;
    for(unsigned i = 0; i < CRAFT_RESULT; ++i) {
        const ItemStack *stack = game_stack_at(game, i);
        if(stack->item == item) total += stack->count;
    }
    return total;
}

bool game_add_item(Game *game, Item item)
{
    return insert_stack(game->slots, (ItemStack){ .item = item, .count = 1,
        .durability = (uint16_t)game_tool_durability(item) });
}

bool game_inventory_transfer(Game *game, unsigned from, unsigned to)
{
    ItemStack *source = mutable_stack(game, from), *target = mutable_stack(game, to);
    if(!source || !target || source == target || !stack_valid(source)) return false;
    if(target->count && source->item == target->item &&
       !game_tool_durability(source->item)) {
        unsigned limit = game_stack_limit(target->item);
        if(target->count >= limit) return false;
        unsigned room = limit - target->count;
        unsigned amount = source->count < room ? source->count : room;
        target->count = (uint16_t)(target->count + amount);
        source->count = (uint16_t)(source->count - amount);
        if(!source->count) *source = (ItemStack){0};
        return true;
    }
    ItemStack swap = target->count ? *target : (ItemStack){0};
    *target = *source;
    *source = swap;
    return true;
}

bool game_inventory_transfer_one(Game *game, unsigned from, unsigned to)
{
    ItemStack *source = mutable_stack(game, from), *target = mutable_stack(game, to);
    if(!source || !target || source == target || !stack_valid(source)) return false;
    if(target->count) {
        if(target->item != source->item || game_tool_durability(source->item) ||
           target->count >= game_stack_limit(target->item)) return false;
        ++target->count;
    } else {
        *target = *source;
        target->count = 1;
    }
    consume_one(source);
    return true;
}

static ItemStack craft_result(Item item, unsigned count)
{
    return (ItemStack){ .item = item, .count = (uint16_t)count,
        .durability = (uint16_t)game_tool_durability(item) };
}

ItemStack game_craft_preview(const Game *game, unsigned width)
{
    ItemStack none = {0};
    if(width != 2 && width != 3) return none;
    int left = (int)width, top = (int)width, right = -1, bottom = -1;
    unsigned occupied = 0;
    for(unsigned i = 0; i < CRAFT_SLOTS; ++i) {
        const ItemStack *stack = &game->crafting[i];
        if(!stack->count) continue;
        if(i >= width * width || !stack_valid(stack)) return none;
        int x = (int)(i % width), y = (int)(i / width);
        if(x < left) left = x;
        if(x > right) right = x;
        if(y < top) top = y;
        if(y > bottom) bottom = y;
        ++occupied;
    }
    if(!occupied) return none;
    unsigned first = (unsigned)top * width + (unsigned)left;
    int w = right - left + 1, h = bottom - top + 1;
    if(occupied == 1 && game->crafting[first].item == ITEM_LOG)
        return craft_result(ITEM_PLANKS, 4);
    if(w == 1 && h == 2 && occupied == 2 &&
       game->crafting[first].item == ITEM_PLANKS &&
       game->crafting[first + width].item == ITEM_PLANKS)
        return craft_result(ITEM_STICK, 4);
    if(w == 2 && h == 2 && occupied == 4 &&
       game->crafting[first].item == ITEM_PLANKS &&
       game->crafting[first + 1].item == ITEM_PLANKS &&
       game->crafting[first + width].item == ITEM_PLANKS &&
       game->crafting[first + width + 1].item == ITEM_PLANKS)
        return craft_result(ITEM_WORKBENCH, 1);
    if(width == 3 && w == 3 && h == 3 && occupied == 5) {
        Item material = game->crafting[0].item;
        if((material == ITEM_PLANKS || material == ITEM_STONE) &&
           game->crafting[1].item == material && game->crafting[2].item == material &&
           game->crafting[4].item == ITEM_STICK && game->crafting[7].item == ITEM_STICK)
            return craft_result(material == ITEM_PLANKS ?
                ITEM_WOOD_PICKAXE : ITEM_STONE_PICKAXE, 1);
    }
    return none;
}

static bool inventory_full(Game *game)
{
    game->notice = NOTICE_FULL;
    game->notice_ticks = SIM_HZ;
    return false;
}

bool game_craft(Game *game, unsigned width)
{
    ItemStack result = game_craft_preview(game, width);
    if(!result.count) return false;
    ItemStack produced[TOTAL_SLOTS], returned[TOTAL_SLOTS];
    memcpy(produced, game->slots, sizeof(produced));
    if(!insert_stack(produced, result)) return inventory_full(game);
    /* Reserve room for every remaining ingredient too, so a successful craft
       can never trap the player in an inventory that cannot be closed. */
    memcpy(returned, produced, sizeof(returned));
    for(unsigned i = 0; i < width * width; ++i) {
        ItemStack remainder = game->crafting[i];
        if(remainder.count > 1) {
            --remainder.count;
            if(!insert_stack(returned, remainder)) return inventory_full(game);
        }
    }
    memcpy(game->slots, produced, sizeof(produced));
    for(unsigned i = 0; i < width * width; ++i)
        if(game->crafting[i].count) consume_one(&game->crafting[i]);
    game->notice = NOTICE_NONE;
    game->notice_ticks = 0;
    return true;
}

bool game_close_crafting(Game *game)
{
    /* Commit all returns together. A full bag leaves the menu and grid intact. */
    ItemStack returned[TOTAL_SLOTS];
    memcpy(returned, game->slots, sizeof(returned));
    for(unsigned i = 0; i < CRAFT_SLOTS; ++i)
        if(game->crafting[i].count && !insert_stack(returned, game->crafting[i]))
            return inventory_full(game);
    memcpy(game->slots, returned, sizeof(returned));
    memset(game->crafting, 0, sizeof(game->crafting));
    if(game->notice == NOTICE_FULL) {
        game->notice = NOTICE_NONE;
        game->notice_ticks = 0;
    }
    return true;
}

static const ItemStack *active_pickaxe(const Game *game)
{
    if(game->selected >= HOTBAR_SLOTS) return NULL;
    const ItemStack *stack = &game->slots[game->selected];
    return game_tool_durability(stack->item) && stack_valid(stack) ? stack : NULL;
}

static Item dropped_item(Tile tile, bool pickaxe)
{
    switch(tile) {
    case TILE_GRASS: case TILE_DIRT: return ITEM_DIRT;
    case TILE_STONE: case TILE_ORE: return pickaxe ? ITEM_STONE : ITEM_NONE;
    case TILE_WOOD: return ITEM_PLANKS;
    case TILE_TRUNK: case TILE_LOG: return ITEM_LOG;
    case TILE_WORKBENCH: return ITEM_WORKBENCH;
    default: return ITEM_NONE;
    }
}

unsigned game_mining_ticks(Tile tile)
{
    switch(tile) {
    case TILE_GRASS: case TILE_DIRT: return 21;
    case TILE_WOOD: case TILE_TRUNK: case TILE_LOG: case TILE_WORKBENCH: return 36;
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
    const ItemStack *pickaxe = active_pickaxe(game);
    unsigned required = game_mining_ticks(tile);
    if(pickaxe && pickaxe->item == ITEM_STONE_PICKAXE &&
       (tile == TILE_STONE || tile == TILE_ORE)) required /= 2;
    Item drop = dropped_item(tile, pickaxe != NULL);
    if(!required) { game_cancel_mining(game); return; }
    /* Capacity probe does not change the inventory or charge tool durability. */
    if(drop != ITEM_NONE) {
        ItemStack probe[TOTAL_SLOTS];
        memcpy(probe, game->slots, sizeof(probe));
        if(!insert_stack(probe, craft_result(drop, 1))) {
            game_cancel_mining(game);
            inventory_full(game);
            return;
        }
    }
    Item tool = pickaxe ? pickaxe->item : ITEM_NONE;
    uint16_t durability = pickaxe ? pickaxe->durability : 0;
    Mining *mining = &game->mining;
    if(!mining->active || mining->x != x || mining->y != y || mining->tile != tile ||
       mining->tool_slot != game->selected || mining->tool != tool ||
       mining->tool_durability != durability)
        *mining = (Mining){ .x = x, .y = y, .tile = tile, .required = required,
            .tool_slot = game->selected, .tool = tool, .tool_durability = durability,
            .active = true };
    if(++mining->elapsed < required) return;
    if(drop == ITEM_NONE || game_add_item(game, drop)) {
        game_set_tile(game, x, y, TILE_AIR);
        if(pickaxe) {
            ItemStack *used = &game->slots[game->selected];
            if(!--used->durability) {
                *used = (ItemStack){0};
                game->notice = NOTICE_TOOL_BROKEN;
                game->notice_ticks = SIM_HZ;
            }
        }
    }
    game_cancel_mining(game);
}

GameInteraction game_interaction(const Game *game)
{
    int x, y;
    game_target(game, &x, &y);
    return game_tile(game, x, y) == TILE_WORKBENCH ? INTERACT_WORKBENCH : INTERACT_NONE;
}

bool game_place(Game *game)
{
    if(game->selected >= HOTBAR_SLOTS) return false;
    ItemStack *stack = &game->slots[game->selected];
    Tile tile = game_item_tile(stack->item);
    if(!stack_valid(stack) || tile == TILE_AIR) return false;
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
    consume_one(stack);
    return true;
}


GameInteraction game_step(Game *game, GameInput input)
{
    Player *p = &game->player;
    GameInteraction interaction = INTERACT_NONE;
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
        if(input.interact) interaction = game_interaction(game);
        if(interaction == INTERACT_NONE && input.place && !game->place_cooldown && game_place(game))
            game->place_cooldown = 9;
    }
    ++game->ticks;
    game_follow_camera(game);
    return interaction;
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
