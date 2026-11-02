#include "bench.h"
#include <string.h>
const VariantInfo variants[VARIANTS]={
    {"reference_1x","Minimine Referenz",1,false,false},
    {"packed_2x","2x Upscaling",2,false,false},
    {"chunks_ram_1x","Zeilenbloecke / RAM",1,false,false},
    {"chunks_xram_1x","Zeilenbloecke / XRAM",1,false,false},
    {"chunks_yram_1x","Zeilenbloecke / YRAM",1,false,false},
    {"chunks_ilram_1x","Blitkernel / ILRAM",1,false,false},
    {"chunks_xram_ilram_1x","XRAM + ILRAM",1,false,false},
    {"dma_full_2b_1x","DMA Vollbild / 2 Byte",1,false,true},
    {"dma_full_32b_1x","DMA Vollbild / 32 Byte",1,false,true},
    {"dma_chunks_2b_1x","DMA Zeilen / 2 Byte",1,false,true},
    {"azur_1x","Azur / 1x",1,true,false},
    {"azur_2x","Azur / 2x",2,true,false},
    {"optimized_1x","Block+Mobs / altes Licht",1,false,false},
    {"reference_cpu_1x","Alt / CPU-Ausgabe",1,false,false},
    {"optimized_cpu_1x","Block+Mobs / CPU-Ausgabe",1,false,false},
    {"scanline_light_1x","Block+Mobs / Zeilenlicht",1,false,false}
};
const char *const scenes[SCENES]={"surface","caves","deep_stone","lighting","mobs_lighting"};
void scene_reset(Game *g,unsigned scene)
{
    if(scene<3) {game_init_benchmark(g,WORLD_SEED);return;}
    game_init(g,WORLD_SEED);
    for(int y=0;y<WORLD_H;++y) for(int x=0;x<WORLD_W;++x)
        if(g->tiles[y][x]==TILE_CHEST || g->tiles[y][x]==TILE_TORCH || g->tiles[y][x]==TILE_FURNACE)
            game_set_tile(g,x,y,TILE_AIR);
    memset(g->mobs,0,sizeof(g->mobs));memset(g->drops,0,sizeof(g->drops));
    memset(g->torches,0,sizeof(g->torches));memset(g->furnaces,0,sizeof(g->furnaces));
    memset(g->chests,0,sizeof(g->chests));memset(&g->boss,0,sizeof(g->boss));
    for(int y=45;y<=66;++y) for(int x=44;x<=91;++x)
        game_set_tile(g,x,y,y==62 || y==66?TILE_STONE:TILE_AIR);
    for(unsigned i=0;i<9;++i) {
        int x=46+(int)i*5;
        g->torches[i]=(Torch){true,x,60};game_set_tile(g,x,60,TILE_TORCH);
    }
    g->player.x=66*16*FIXED_ONE;g->player.y=(62*16-PLAYER_H)*FIXED_ONE;
    g->player.vx=g->player.vy=0;g->player.grounded=true;
    g->hurt_ticks=300;g->notice_ticks=0;g->selected=0;
    g->slots[0]=(ItemStack){ITEM_TORCH,32,0};
    if(scene==4) for(unsigned i=0;i<MOB_LIMIT;++i) {
        MobKind kind=i<8?(MobKind)(i%3):i%2?MOB_BAT:MOB_ZOMBIE;
        int x=(58+(int)i)*16;
        int y=62*16-(int)game_mob_height(kind);
        if(kind==MOB_BAT) y-=40;
        /* Fixed stress fixture, deliberately independent of natural spawn
           restrictions (animals underground and existing zombies near torches). */
        g->mobs[i]=(Mob){.active=true,.kind=kind,.health=(uint16_t)(game_mob_health(kind)/2),
            .body={.x=x*FIXED_ONE,.y=y*FIXED_ONE,.facing=i%2?1:-1,.grounded=kind!=MOB_BAT},.ai_ticks=120};
    }
    g->camera_x=56*16;g->camera_y=49*16;
}
void scene_step(Game *g,unsigned scene,unsigned frame)
{
    if(scene<3) {game_benchmark_step(g,scene,frame);return;}
    game_step(g,(GameInput){.move=frame%120<60?1:-1,.jump=frame%80==0});
    /* Identical simulated time and camera path, independent of measured FPS. */
    g->camera_x=55*16+(int)(frame%180)*2/3;
    g->camera_y=49*16;g->notice_ticks=0;
}
static uint32_t mix(uint32_t h,uint32_t v)
{
    for(unsigned i=0;i<4;++i) {h^=v&255;h*=16777619u;v>>=8;}return h;
}
uint32_t scene_hash(const Game *g)
{
    /* Hash values, never C padding bytes or pointer representations. */
    uint32_t h=2166136261u;
    for(unsigned y=0;y<WORLD_H;++y) for(unsigned x=0;x<WORLD_W;++x) {h^=g->tiles[y][x];h*=16777619u;}
#define H(v) h=mix(h,(uint32_t)(v))
    H(g->rng);H(g->ticks);H(g->health);H(g->hunger);H(g->camera_x);H(g->camera_y);
    H(g->player.x);H(g->player.y);H(g->player.vx);H(g->player.vy);H(g->player.facing);H(g->player.grounded);
    for(unsigned i=0;i<MOB_LIMIT;++i) {
        const Mob *m=&g->mobs[i];H(m->active);H(m->kind);H(m->health);
        H(m->body.x);H(m->body.y);H(m->body.vx);H(m->body.vy);H(m->body.facing);
        H(m->state);H(m->state_ticks);H(m->ai_ticks);H(m->attack_ticks);H(m->flee_ticks);H(m->hurt_ticks);
    }
#undef H
    return h;
}
