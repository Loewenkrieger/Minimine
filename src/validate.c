#include "game.h"

static bool body_valid(const Game *g,const Player *p,int w,int h)
{
    return (p->facing==-1 || p->facing==1) && p->x>=0 && p->y>=0 &&
        p->x<=(WORLD_W*TILE_SIZE-w)*FIXED_ONE &&
        p->y<=(WORLD_H*TILE_SIZE-h)*FIXED_ONE &&
        p->vx>=-2048 && p->vx<=2048 && p->vy>=-1536 && p->vy<=2048 &&
        game_rect_clear(g,p->x,p->y,w,h);
}
bool game_validate(const Game *g)
{
    if((unsigned)g->difficulty>=DIFFICULTY_COUNT) return false;
    const DifficultyRules *rules=game_difficulty_rules(g->difficulty);
    if(!g->survival || g->selected>=HOTBAR_SLOTS || g->aim_x < -1 || g->aim_x>1 ||
       g->aim_y < -1 || g->aim_y>1 || (!g->aim_x && !g->aim_y) ||
       !g->health || g->health>HEALTH_MAX || g->hunger>HUNGER_MAX ||
       !body_valid(g,&g->player,PLAYER_W,game_player_height(&g->player))) return false;
    if(g->spawn_x<0 || g->spawn_x>(WORLD_W*16-PLAYER_W)*FIXED_ONE ||
       g->spawn_y<0 || g->spawn_y>(WORLD_H*16-PLAYER_H)*FIXED_ONE ||
       g->camera_x<0 || g->camera_x>WORLD_W*16-SCREEN_W ||
       g->camera_y<0 || g->camera_y>WORLD_H*16-VIEW_H ||
       g->hurt_ticks>300 || g->action_ticks>60 || g->swing_ticks>10 ||
       g->hunger_ticks>=rules->hunger_ticks || g->exertion>=rules->exertion || g->regen_ticks>=240 ||
       g->regen_cost>=400 || g->starve_ticks>=240 || g->spawn_ticks>rules->spawn_ticks ||
       g->place_cooldown>9 || !g->rng) return false;
    unsigned furnace_tiles=0,chest_tiles=0,torch_tiles=0,zombies=0,animals=0,bosses=0;
    for(int y=0;y<WORLD_H;++y) for(int x=0;x<WORLD_W;++x) {
        if(g->tiles[y][x]>=TILE_COUNT) return false;
        if(g->tiles[y][x]==TILE_FURNACE) ++furnace_tiles;
        Tile t=(Tile)g->tiles[y][x];
        if(t==TILE_CHEST) ++chest_tiles;
        if(t==TILE_SEAL && !g->ruins) return false;
        if(t==TILE_TORCH) ++torch_tiles;
        if(t==TILE_DOOR_BOTTOM && game_tile(g,x,y-1)!=TILE_DOOR_TOP) return false;
        if(t==TILE_DOOR_TOP && game_tile(g,x,y+1)!=TILE_DOOR_BOTTOM) return false;
        if(t==TILE_OPEN_BOTTOM && game_tile(g,x,y-1)!=TILE_OPEN_TOP) return false;
        if(t==TILE_OPEN_TOP && game_tile(g,x,y+1)!=TILE_OPEN_BOTTOM) return false;
        if(t==TILE_BED_LEFT && game_tile(g,x+1,y)!=TILE_BED_RIGHT) return false;
        if(t==TILE_BED_RIGHT && game_tile(g,x-1,y)!=TILE_BED_LEFT) return false;
    }
    for(int x=0;x<WORLD_W;++x) if(g->surface[x]>=WORLD_H-2) return false;
    for(unsigned i=0;i<TOTAL_SLOTS;++i) if(!game_stack_valid(&g->slots[i])) return false;
    for(unsigned i=0;i<CRAFT_SLOTS;++i) if(!game_stack_valid(&g->crafting[i])) return false;
    for(unsigned i=0;i<ARMOR_SLOTS;++i)
        if(!game_stack_valid(&g->armor[i]) ||
           (g->armor[i].count && game_armor_slot(g->armor[i].item)!=i)) return false;
    if(!game_can_return_crafting(g)) return false;
    unsigned furnaces=0;
    for(unsigned i=0;i<FURNACE_LIMIT;++i) if(g->furnaces[i].active) {
        const Furnace *f=&g->furnaces[i];++furnaces;
        if(f->x<0 || f->x>=WORLD_W || f->y<0 || f->y>=WORLD_H ||
           game_tile(g,f->x,f->y)!=TILE_FURNACE || f->progress>=600 ||
           f->burn_ticks>f->burn_total || f->burn_total>4800 ||
           (unsigned)f->recipe>=ITEM_COUNT) return false;
        for(unsigned j=0;j<3;++j) if(!game_stack_valid(&f->slots[j])) return false;
        if(f->slots[0].count && !game_smelt_result(f->slots[0].item)) return false;
        if(f->slots[1].count && !game_fuel_ticks(f->slots[1].item)) return false;
        if(f->slots[2].count && f->slots[2].item!=ITEM_IRON_INGOT &&
           f->slots[2].item!=ITEM_COOKED_PORK && f->slots[2].item!=ITEM_COOKED_BEEF &&
           f->slots[2].item!=ITEM_COOKED_CHICKEN) return false;
        if(f->recipe!=game_smelt_result(f->slots[0].item) &&
           !(f->progress==0 && f->recipe==ITEM_NONE)) return false;
        for(unsigned j=0;j<i;++j) if(g->furnaces[j].active &&
            g->furnaces[j].x==f->x && g->furnaces[j].y==f->y) return false;
    }
    if(furnaces!=furnace_tiles) return false;
    for(unsigned i=0;i<ACCESSORY_SLOTS;++i) if(!game_stack_valid(&g->accessories[i]) ||
        (g->accessories[i].count && game_item_kind(g->accessories[i].item)!=KIND_ACCESSORY)) return false;
    if(g->accessories[0].count && g->accessories[1].count && g->accessories[0].item==g->accessories[1].item) return false;
    unsigned chests=0,torches=0;
    for(unsigned i=0;i<CHEST_LIMIT;++i) if(g->chests[i].active) {
        const Chest *c=&g->chests[i];++chests;
        if(c->x<0 || c->x>=WORLD_W || c->y<0 || c->y>=WORLD_H || game_tile(g,c->x,c->y)!=TILE_CHEST) return false;
        for(unsigned j=0;j<CHEST_SLOTS;++j) if(!game_stack_valid(&c->slots[j])) return false;
        for(unsigned j=0;j<i;++j) if(g->chests[j].active && g->chests[j].x==c->x && g->chests[j].y==c->y) return false;
    }
    for(unsigned i=0;i<TORCH_LIMIT;++i) if(g->torches[i].active) {
        const Torch *t=&g->torches[i];++torches;
        if(t->x<0 || t->x>=WORLD_W || t->y<0 || t->y>=WORLD_H || game_tile(g,t->x,t->y)!=TILE_TORCH) return false;
        for(unsigned j=0;j<i;++j) if(g->torches[j].active && g->torches[j].x==t->x && g->torches[j].y==t->y) return false;
    }
    if(chests!=chest_tiles || torches!=torch_tiles || g->relics_found>7 ||
       g->bed_x<0 || g->bed_x>=WORLD_W-1 || g->bed_y<0 || g->bed_y>=WORLD_H) return false;
    if(g->bed_spawn && (game_tile(g,g->bed_x,g->bed_y)!=TILE_BED_LEFT ||
        game_tile(g,g->bed_x+1,g->bed_y)!=TILE_BED_RIGHT)) return false;
    const Boss *b=&g->boss;
    if(b->phase>4 || b->timer>72 || b->pattern>1 || b->shock_ticks>24 ||
       b->direction< -1 || b->direction>1 || b->shock_x<0 || b->shock_x>WORLD_W*16*FIXED_ONE ||
       (b->gate_open && (!g->ruins || g->relics_found!=7)) ||
       (b->defeated && (!b->gate_open || b->active)) || (b->reward_pending && !b->defeated) ||
       (b->active && (!b->gate_open || b->defeated || !b->direction || !b->timer ||
           !game_in_arena(&g->player)))) return false;
    if(g->ruins) for(int y=ARENA_TOP;y<=ARENA_FLOOR;++y) for(int x=ARENA_LEFT;x<=ARENA_RIGHT;++x)
        if(game_protected_tile(g,x,y)) {
            Tile wanted=x==ARENA_LEFT && y>=GATE_TOP && y<ARENA_FLOOR?
                b->gate_open?TILE_AIR:TILE_SEAL:TILE_BASALT;
            if(game_tile(g,x,y)!=wanted) return false;
        }
    for(unsigned i=0;i<MOB_LIMIT;++i) if(g->mobs[i].active) {
        const Mob *m=&g->mobs[i];
        if((unsigned)m->kind>=MOB_COUNT || !m->health || m->health>game_mob_health(m->kind) ||
           m->body.sneaking || !body_valid(g,&m->body,(int)game_mob_width(m->kind),
               (int)game_mob_height(m->kind)) || m->ai_ticks>240 ||
           m->flee_ticks>120 || m->attack_ticks>60 || m->state>3 || m->state_ticks>60 || m->hurt_ticks>12) return false;
        if(game_hostile(m->kind)) ++zombies;else ++animals;
        if(m->kind==MOB_WARDEN) {++bosses;if(!game_in_arena(&m->body)) return false;}
    }
    if(zombies>8 || animals>8 || bosses>1 || bosses!=(unsigned)b->active) return false;
    for(unsigned i=0;i<DROP_LIMIT;++i) if(g->drops[i].active) {
        const Drop *d=&g->drops[i];
        if(!d->stack.count || !game_stack_valid(&d->stack) || !d->ttl || d->ttl>18000 ||
           d->x<0 || d->x>(WORLD_W*16-6)*FIXED_ONE || d->y<0 ||
           d->y>(WORLD_H*16-6)*FIXED_ONE || d->vy<0 || d->vy>2048) return false;
    }
    return true;
}
