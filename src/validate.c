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
    if(!g->survival || g->selected>=HOTBAR_SLOTS || g->aim_x < -1 || g->aim_x>1 ||
       g->aim_y < -1 || g->aim_y>1 || (!g->aim_x && !g->aim_y) ||
       !g->health || g->health>HEALTH_MAX || g->hunger>HUNGER_MAX ||
       !body_valid(g,&g->player,PLAYER_W,game_player_height(&g->player))) return false;
    if(g->spawn_x<0 || g->spawn_x>(WORLD_W*16-PLAYER_W)*FIXED_ONE ||
       g->spawn_y<0 || g->spawn_y>(WORLD_H*16-PLAYER_H)*FIXED_ONE ||
       g->camera_x<0 || g->camera_x>WORLD_W*16-SCREEN_W ||
       g->camera_y<0 || g->camera_y>WORLD_H*16-VIEW_H ||
       g->hurt_ticks>300 || g->action_ticks>60 || g->swing_ticks>10 ||
       g->hunger_ticks>=3600 || g->exertion>=20 || g->regen_ticks>=240 ||
       g->regen_cost>=400 || g->starve_ticks>=240 || g->spawn_ticks>300 ||
       g->place_cooldown>9 || !g->rng) return false;
    unsigned furnace_tiles=0,zombies=0,animals=0;
    for(int y=0;y<WORLD_H;++y) for(int x=0;x<WORLD_W;++x) {
        if(g->tiles[y][x]>=TILE_COUNT) return false;
        if(g->tiles[y][x]==TILE_FURNACE) ++furnace_tiles;
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
    for(unsigned i=0;i<MOB_LIMIT;++i) if(g->mobs[i].active) {
        const Mob *m=&g->mobs[i];
        if((unsigned)m->kind>=MOB_COUNT || !m->health || m->health>game_mob_health(m->kind) ||
           m->body.sneaking || !body_valid(g,&m->body,(int)game_mob_width(m->kind),
               (int)game_mob_height(m->kind)) || m->ai_ticks>240 ||
           m->flee_ticks>120 || m->attack_ticks>60) return false;
        if(m->kind==MOB_ZOMBIE) ++zombies;else ++animals;
    }
    if(zombies>8 || animals>8) return false;
    for(unsigned i=0;i<DROP_LIMIT;++i) if(g->drops[i].active) {
        const Drop *d=&g->drops[i];
        if(!d->stack.count || !game_stack_valid(&d->stack) || !d->ttl || d->ttl>18000 ||
           d->x<0 || d->x>(WORLD_W*16-6)*FIXED_ONE || d->y<0 ||
           d->y>(WORLD_H*16-6)*FIXED_ONE || d->vy<0 || d->vy>2048) return false;
    }
    return true;
}
