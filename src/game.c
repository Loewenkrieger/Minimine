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
    return tile != TILE_AIR && tile != TILE_TRUNK && tile != TILE_LEAVES &&
        tile!=TILE_TORCH && tile!=TILE_OPEN_BOTTOM && tile!=TILE_OPEN_TOP;
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

static void init(Game *game, uint32_t seed, bool survival,bool ruins)
{
    memset(game, 0, sizeof(*game));
    game->seed = seed;
    game->difficulty=DIFFICULTY_MEDIUM;
    game->survival=survival;game->health=HEALTH_MAX;game->hunger=HUNGER_MAX;
    game->rng=seed^UINT32_C(0x951e248a);
    if(!game->rng) game->rng=1;
    game->spawn_ticks=300;
    generate_world(game);
    if(survival) for(int y=0;y<WORLD_H;++y) for(int x=0;x<WORLD_W;++x)
        if(game->tiles[y][x]==TILE_STONE && hash(seed^UINT32_C(0x17816)^(uint32_t)(x+y*WORLD_W))%19==0)
            game->tiles[y][x]=TILE_COAL;
    game->player.x = (12 * TILE_SIZE + 3) * FIXED_ONE;
    game->player.y = (30 * TILE_SIZE - PLAYER_H) * FIXED_ONE;
    game->spawn_x=game->player.x;game->spawn_y=game->player.y;
    game->player.facing = 1;
    game->player.grounded = true;
    game->aim_x = 1;
    for(unsigned i = 0; i < 3; ++i)
        game->slots[i] = (ItemStack){ .item = (Item)(i + 1), .count = 32 };
    if(ruins) game_adventure_init(game);
    if(survival) game_populate_diamonds(game);
    game_follow_camera(game);
}

void game_init(Game *game,uint32_t seed) {init(game,seed,true,true);}
void game_init_classic(Game *game,uint32_t seed) {init(game,seed,true,false);}
void game_init_benchmark(Game *game,uint32_t seed) {init(game,seed,false,false);}

bool game_rect_clear(const Game *game,int32_t x,int32_t y,int width,int height)
{
    int left=floor_div(x,TILE_SIZE*FIXED_ONE);
    int right=floor_div(x+width*FIXED_ONE-1,TILE_SIZE*FIXED_ONE);
    int top=floor_div(y,TILE_SIZE*FIXED_ONE);
    int bottom=floor_div(y+height*FIXED_ONE-1,TILE_SIZE*FIXED_ONE);
    for(int ty=top;ty<=bottom;++ty) for(int tx=left;tx<=right;++tx)
        if(game_tile_solid(game_tile(game,tx,ty))) return false;
    return true;
}
bool game_toggle_sneak(Game *game)
{
    Player *p=&game->player;
    int difference=(PLAYER_H-PLAYER_CROUCH_H)*FIXED_ONE;
    if(p->sneaking) {
        if(!game_player_space_clear(game,p->x,p->y-difference,PLAYER_H)) {
            game->notice=NOTICE_NO_SPACE;game->notice_ticks=SIM_HZ;return false;
        }
        p->y-=difference;
    } else p->y+=difference;
    p->sneaking=!p->sneaking;++game->revision;game_follow_camera(game);return true;
}
static bool supported_at(const Game *game,const Player *p,int32_t x,int width,int height)
{
    int row=floor_div(p->y+height*FIXED_ONE,TILE_SIZE*FIXED_ONE);
    int left=floor_div(x,TILE_SIZE*FIXED_ONE);
    int right=floor_div(x+width*FIXED_ONE-1,TILE_SIZE*FIXED_ONE);
    for(int tx=left;tx<=right;++tx) if(game_tile_solid(game_tile(game,tx,row))) return true;
    return false;
}
void game_move_actor(const Game *game,Player *p,int width,int height)
{
    int32_t old=p->x,next=old+p->vx;
    int top=floor_div(p->y,TILE_SIZE*FIXED_ONE);
    int bottom=floor_div(p->y+height*FIXED_ONE-1,TILE_SIZE*FIXED_ONE);
    int edge=floor_div(next+(p->vx>0?width*FIXED_ONE-1:0),TILE_SIZE*FIXED_ONE);
    if(p->vx) for(int y=top;y<=bottom;++y) {
        if(!game_tile_solid(game_tile(game,edge,y))) continue;
        next=(p->vx>0?edge*TILE_SIZE-width:(edge+1)*TILE_SIZE)*FIXED_ONE;p->vx=0;break;
    }
    if(p->sneaking && p->grounded && supported_at(game,p,old,width,height) &&
       !supported_at(game,p,next,width,height)) {
        int sign=next>old?1:-1,low=0,high=next>old?next-old:old-next;
        while(low<high) {
            int mid=low+(high-low+1)/2;
            if(supported_at(game,p,old+sign*mid,width,height)) low=mid;else high=mid-1;
        }
        next=old+sign*low;p->vx=0;
    }
    p->x=next;next=p->y+p->vy;
    int left=floor_div(p->x,TILE_SIZE*FIXED_ONE);
    int right=floor_div(p->x+width*FIXED_ONE-1,TILE_SIZE*FIXED_ONE);
    edge=floor_div(next+(p->vy>0?height*FIXED_ONE-1:0),TILE_SIZE*FIXED_ONE);
    p->grounded=false;
    if(p->vy) for(int x=left;x<=right;++x) {
        if(!game_tile_solid(game_tile(game,x,edge))) continue;
        p->grounded=p->vy>0;
        p->y=(p->vy>0?edge*TILE_SIZE-height:(edge+1)*TILE_SIZE)*FIXED_ONE;
        p->vy=0;return;
    }
    p->y=next;
}
void game_target(const Game *g,int *x,int *y)
{
    const Player *p=&g->player;
    int size=TILE_SIZE*FIXED_ONE,h=game_player_height(p);
    if(g->aim_x>0) *x=floor_div(p->x+PLAYER_W*FIXED_ONE-1,size)+1;
    else if(g->aim_x<0) *x=floor_div(p->x,size)-1;
    else *x=floor_div(p->x+PLAYER_W*FIXED_ONE/2,size);
    if(g->aim_y<0) *y=floor_div(p->y,size)-1;
    else if(g->aim_y>0) {
        int feet=p->y+h*FIXED_ONE;
        /* Directly below: round up in Q8 so the complete tile clears the body.
           Sideways at foot height still targets the last occupied body row. */
        *y=g->aim_x?floor_div(feet-1,size):floor_div(feet+size-1,size);
    }
    else *y=floor_div(p->y+(g->aim_manual?(p->sneaking?3:5):h/2)*FIXED_ONE,size);
}


static const ItemStack *active_pickaxe(const Game *game)
{
    if(game->selected >= HOTBAR_SLOTS) return NULL;
    const ItemStack *stack = &game->slots[game->selected];
    return game_item_kind(stack->item)==KIND_PICKAXE && game_stack_valid(stack) ? stack : NULL;
}

static Item dropped_item(Tile tile, Item pickaxe)
{
    switch(tile) {
    case TILE_GRASS: case TILE_DIRT: return ITEM_DIRT;
    case TILE_STONE: return pickaxe?ITEM_STONE:ITEM_NONE;
    case TILE_COAL: return pickaxe?ITEM_COAL:ITEM_NONE;
    case TILE_ORE: return game_pick_tier(pickaxe)>=2?ITEM_RAW_IRON:ITEM_NONE;
    case TILE_BASALT:return ITEM_BASALT;case TILE_CRYSTAL:return ITEM_CRYSTAL;
    case TILE_DIAMOND_ORE:return ITEM_DIAMOND;
    case TILE_FURNACE: return ITEM_FURNACE;
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
    case TILE_STONE: case TILE_COAL: case TILE_FURNACE: return 60;
    case TILE_ORE: return 120;
    case TILE_LEAVES: return 9;
    case TILE_TORCH:return 3;
    case TILE_CHEST:case TILE_DOOR_BOTTOM:case TILE_DOOR_TOP:case TILE_OPEN_BOTTOM:case TILE_OPEN_TOP:
    case TILE_BED_LEFT:case TILE_BED_RIGHT:return 36;
    case TILE_BASALT:return 75;case TILE_CRYSTAL:return 60;
    case TILE_DIAMOND_ORE:return 90;
    default: return 0;
    }
}

void game_cancel_mining(Game *game)
{
    game->mining = (Mining){0};
}

static void wear_pickaxe(Game *g)
{
    ++g->mined_blocks;
    if(game_has_accessory(g,ITEM_MINER_CHARM) && g->mined_blocks%5==0) return;
    ItemStack *used=&g->slots[g->selected];
    if(!--used->durability) {*used=(ItemStack){0};g->notice=NOTICE_TOOL_BROKEN;g->notice_ticks=60;}
}

static void mining_step(Game *game)
{
    int x, y;
    game_target(game, &x, &y);
    Tile tile = game_tile(game, x, y);
    const ItemStack *pickaxe = active_pickaxe(game);
    unsigned required = game_mining_ticks(tile);
    unsigned tier=pickaxe?game_pick_tier(pickaxe->item):0;
    if(game_protected_tile(game,x,y) || tile==TILE_SEAL) {game_cancel_mining(game);return;}
    if((tile==TILE_BASALT || tile==TILE_CRYSTAL || tile==TILE_DIAMOND_ORE) && tier<3) {
        game_cancel_mining(game);game->notice=NOTICE_IRON_PICKAXE;game->notice_ticks=60;return;
    }
    if(tier>=2 &&
       (tile==TILE_STONE || tile==TILE_COAL || tile==TILE_ORE || tile==TILE_FURNACE)) required /= tier>=3?4u:2u;
    if(tier==3 && tile==TILE_ORE) required=30;
    if(tier==4) {
        if(tile==TILE_STONE || tile==TILE_COAL) required=10;
        else if(tile==TILE_ORE) required=20;
        else if(tile==TILE_BASALT) required=45;
        else if(tile==TILE_CRYSTAL) required=30;
        else if(tile==TILE_DIAMOND_ORE) required=60;
    }
    Item drop=dropped_item(tile,pickaxe?pickaxe->item:ITEM_NONE);
    int furnace=tile==TILE_FURNACE?game_furnace_at(game,x,y):-1;
    if(tile==TILE_FURNACE && (!pickaxe || furnace<0)) {
        game_cancel_mining(game);game->notice=NOTICE_PICKAXE;game->notice_ticks=SIM_HZ;return;
    }
    if(!required) { game_cancel_mining(game); return; }
    /* Capacity probe does not change the inventory or charge tool durability. */
    if(drop != ITEM_NONE) {
        ItemStack probe[TOTAL_SLOTS];
        memcpy(probe, game->slots, sizeof(probe));
        bool fits=game_insert_stack(probe,(ItemStack){.item=drop,.count=1});
        if(furnace>=0) for(unsigned i=0;i<3 && fits;++i)
            if(game->furnaces[furnace].slots[i].count)
                fits=game_insert_stack(probe,game->furnaces[furnace].slots[i]);
        if(!fits) {
            game_cancel_mining(game);
            game->notice=NOTICE_FULL;game->notice_ticks=SIM_HZ;
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
    if(tile>=TILE_TORCH && tile<=TILE_BED_RIGHT) {
        if(game_adventure_break(game,x,y) && pickaxe) wear_pickaxe(game);
        game_cancel_mining(game);return;
    }
    if(drop == ITEM_NONE || game_add_item(game, drop)) {
        if(furnace>=0) {
            Furnace *f=&game->furnaces[furnace];
            for(unsigned i=0;i<3;++i) if(f->slots[i].count) game_add_stack(game,f->slots[i]);
            *f=(Furnace){0};
        }
        game_set_tile(game, x, y, TILE_AIR);
        if(pickaxe) wear_pickaxe(game);
    }
    game_cancel_mining(game);
}

GameInteraction game_interaction(const Game *game)
{
    int x, y;
    game_target(game, &x, &y);
    Tile t=game_tile(game,x,y);
    return t==TILE_WORKBENCH?INTERACT_WORKBENCH:
        t==TILE_FURNACE && game_furnace_at(game,x,y)>=0?INTERACT_FURNACE:
        t==TILE_CHEST && game_chest_at(game,x,y)>=0?INTERACT_CHEST:INTERACT_NONE;
}

bool game_place(Game *game)
{
    if(game->selected >= HOTBAR_SLOTS) return false;
    ItemStack *stack = &game->slots[game->selected];
    Tile tile = game_item_tile(stack->item);
    if(!game_stack_valid(stack) || tile == TILE_AIR) return false;
    int x, y;
    game_target(game, &x, &y);
    if(game_tile(game, x, y) != TILE_AIR || game_protected_tile(game,x,y)) return false;
    const Player *p = &game->player;
    int32_t bx = x * TILE_SIZE * FIXED_ONE;
    int32_t by = y * TILE_SIZE * FIXED_ONE;
    if(p->x < bx + TILE_SIZE * FIXED_ONE && p->x + PLAYER_W * FIXED_ONE > bx &&
       p->y < by + TILE_SIZE * FIXED_ONE &&
       p->y + game_player_height(p) * FIXED_ONE > by) return false;
    for(unsigned i=0;i<MOB_LIMIT;++i) if(game->mobs[i].active) {
        const Mob *m=&game->mobs[i];
        if(m->body.x<bx+TILE_SIZE*FIXED_ONE && m->body.x+(int)game_mob_width(m->kind)*FIXED_ONE>bx &&
           m->body.y<by+TILE_SIZE*FIXED_ONE && m->body.y+(int)game_mob_height(m->kind)*FIXED_ONE>by) return false;
    }
    int furnace=-1;
    if(tile==TILE_FURNACE) {
        for(int i=0;i<FURNACE_LIMIT;++i) if(!game->furnaces[i].active) {furnace=i;break;}
        if(furnace<0) {game->notice=NOTICE_FURNACE_LIMIT;game->notice_ticks=SIM_HZ;return false;}
    }
    if(tile>=TILE_TORCH && tile<=TILE_BED_RIGHT) {
        if(!game_adventure_place(game,tile,x,y)) return false;
    } else if(!game_set_tile(game,x,y,tile)) return false;
    if(furnace>=0) game->furnaces[furnace]=(Furnace){.active=true,.x=x,.y=y};
    ++game->revision;
    if(!--stack->count) *stack=(ItemStack){0};
    return true;
}


static bool special_interaction(Game *g)
{int x,y;game_target(g,&x,&y);return game_adventure_interact(g,x,y);}

GameInteraction game_step(Game *game, GameInput input)
{
    if(game->survival) game_repair_overlaps(game);
    Player *p = &game->player;
    GameInteraction interaction = INTERACT_NONE;
    if(game->notice_ticks && !--game->notice_ticks) game->notice = NOTICE_NONE;
    if(game->action_ticks) --game->action_ticks;
    if(game->hurt_ticks) --game->hurt_ticks;
    if(game->swing_ticks) --game->swing_ticks;
    unsigned respawns=game->respawns;
    if(input.toggle_sneak) game_toggle_sneak(game);
    int limit = p->sneaking ? 240 : 480;
    if(game_has_accessory(game,ITEM_SPEED_BOOTS)) limit=limit*115/100;
    int acceleration = p->sneaking ? 40 : 80;
    int friction = p->sneaking ? 50 : 100;
    int move = clamp(input.move, -1, 1);
    p->vx = clamp(p->vx, -limit, limit);
    if(move) {
        p->vx = clamp(p->vx + move * acceleration, -limit, limit);
        p->facing = move;
        game->aim_x = move;
        game->aim_y = 0;game->aim_manual=false;
    } else if(p->vx > 0) p->vx = p->vx > friction ? p->vx - friction : 0;
    else if(p->vx < 0) p->vx = p->vx < -friction ? p->vx + friction : 0;
    if(input.aim_x || input.aim_y) {
        game->aim_manual=true;
        game->aim_x = clamp(input.aim_x, -1, 1);
        game->aim_y = clamp(input.aim_y, -1, 1);
    }
    if(input.jump && (p->grounded || (game_has_accessory(game,ITEM_JUMP_AMULET) && !game->double_jump_used))) {
        if(!p->grounded) game->double_jump_used=true;
        p->vy = -1536;
        p->grounded = false;
        game_exert(game);
    }
    p->vy = clamp(p->vy + 64, -1536, 2048);
    if(game->survival) game_move_player(game);
    else game_move_actor(game,p,PLAYER_W,game_player_height(p));
    if(p->grounded) game->double_jump_used=false;
    if(game->place_cooldown) --game->place_cooldown;
    if(input.mine) mining_step(game);
    else {
        game_cancel_mining(game);
        bool use=input.use_pressed || input.use_held;
        Item active=game->selected<HOTBAR_SLOTS?game->slots[game->selected].item:ITEM_NONE;
        bool fighter=active==ITEM_NONE || game_item_kind(active)==KIND_SWORD;
        if(use && game->survival && fighter && game_attack(game,true)) { /* Attack owns this use. */ }
        else if(input.use_pressed && special_interaction(game)) { }
        else if(input.use_pressed && game_interaction(game)!=INTERACT_NONE)
            interaction=game_interaction(game);
        else if(use && game->survival && game_item_kind(active)==KIND_FOOD) game_eat(game);
        else if(use && !game->place_cooldown && game_place(game)) game->place_cooldown=9;
        else if(use && game->survival && fighter) game_attack(game,false);
    }
    if(game->survival) {game_furnaces_step(game);game_survival_step(game);}
    if(respawns!=game->respawns) interaction=INTERACT_NONE;
    ++game->ticks;++game->revision;
    game_follow_camera(game);
    return interaction;
}

void game_benchmark_step(Game *game, unsigned phase, unsigned frame)
{
    if(phase==0 && frame==0 && game->survival) game_init_benchmark(game,game->seed);
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
