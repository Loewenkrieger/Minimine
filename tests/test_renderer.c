#include "render.h"
#include <gint/display.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
void legacy050_world(const Game *,const RenderInfo *);
static Game g,before;
static uint16_t expected[SCREEN_W*SCREEN_H];
static unsigned comparisons;
static void compare(void)
{
    RenderInfo info={0};before=g;
    dclear(0x1357);legacy050_world(&g,&info);
    memcpy(expected,gint_vram,sizeof(expected));
    dclear(0x1357);render_world(&g,&info);
    if(memcmp(expected,gint_vram,sizeof(expected))) {
        for(unsigned i=0;i<SCREEN_W*SCREEN_H;++i) if(expected[i]!=gint_vram[i]) {
            fprintf(stderr,"Renderer mismatch case=%u camera=%d,%d pixel=%u,%u old=%04x new=%04x\n",
                comparisons,g.camera_x,g.camera_y,i%SCREEN_W,i/SCREEN_W,expected[i],gint_vram[i]);break;
        }
        assert(0);
    }
    assert(!memcmp(&before,&g,sizeof(g)));host_check_guards();++comparisons;
#ifdef MINIMINE_BENCH_LIGHT
    dclear(0x1357);render_world_legacy_light(&g,&info);
    assert(!memcmp(expected,gint_vram,sizeof(expected)));
    assert(!memcmp(&before,&g,sizeof(g)));host_check_guards();++comparisons;
#endif
}
static void actors_and_edges(void)
{
    game_init(&g,WORLD_SEED);memset(g.mobs,0,sizeof(g.mobs));
    for(unsigned kind=0;kind<MOB_COUNT;++kind) for(unsigned facing=0;facing<2;++facing)
        for(unsigned xi=0;xi<8;++xi) for(unsigned yi=0;yi<8;++yi) {
            int w=(int)game_mob_width((MobKind)kind),h=(int)game_mob_height((MobKind)kind);
            int xs[]={-w,-w+1,-3,0,SCREEN_W-w,SCREEN_W-1,SCREEN_W,53};
            int ys[]={-h,-h+1,-3,0,VIEW_H-h,VIEW_H-1,VIEW_H,87};
            g.camera_x=16*20+(int)xi;g.camera_y=16*40+(int)yi;
            g.ticks=(xi+yi)%2?8:0;g.ruins=(xi+yi)%3!=0;
            g.player.sneaking=(xi+yi)%2!=0;
            g.mobs[0]=(Mob){.active=true,.kind=(MobKind)kind,
                .health=(uint16_t)((xi+yi)%2?game_mob_health((MobKind)kind):100),
                .body={.x=(g.camera_x+xs[xi])*FIXED_ONE,.y=(g.camera_y+ys[yi])*FIXED_ONE,
                    .vx=xi%2?256:0,.facing=facing?1:-1},
                .hurt_ticks=(xi+yi)%6,.state=(xi+yi)%3==0?1u:0u};
            g.boss.phase=xi%5;g.boss.shock_ticks=yi;g.boss.shock_x=(g.camera_x+160)*FIXED_ONE;
            compare();
        }
}
static void lights_and_tiles(void)
{
    game_init(&g,WORLD_SEED);memset(g.mobs,0,sizeof(g.mobs));
    memset(g.chests,0,sizeof(g.chests));memset(g.furnaces,0,sizeof(g.furnaces));
    memset(g.torches,0,sizeof(g.torches));
    for(int y=0;y<WORLD_H;++y) for(int x=0;x<WORLD_W;++x)
        g.tiles[y][x]=(uint8_t)((x+3*y)%TILE_COUNT);
    for(int x=0;x<WORLD_W;++x) g.surface[x]=(uint8_t)(x%40+20);
    for(unsigned i=0;i<TORCH_LIMIT;++i) g.torches[i]=(Torch){i%2==0,25+(int)i%30,38+(int)i%11};
    g.slots[0]=(ItemStack){ITEM_TORCH,32,0};g.selected=0;
    for(int y=0;y<16;++y) for(int x=0;x<16;++x) {
        g.camera_x=16*20+x;g.camera_y=16*35+y;
        g.player.x=(g.camera_x+135)*FIXED_ONE;g.player.y=(g.camera_y+187)*FIXED_ONE;
        g.player.sneaking=(x+y)%2!=0;g.player.facing=(x+y)%3?1:-1;
        g.ruins=true;g.mining=(Mining){.active=true,.x=27,.y=44,.elapsed=13,.required=21};
        g.notice_ticks=(unsigned)x;g.notice=NOTICE_FULL;
        compare();
    }
    int cx[]={0,WORLD_W*16-SCREEN_W},cy[]={0,WORLD_H*16-VIEW_H};
    for(unsigned x=0;x<2;++x) for(unsigned y=0;y<2;++y) {g.camera_x=cx[x];g.camera_y=cy[y];compare();}
}
int main(void)
{
    actors_and_edges();lights_and_tiles();
    assert(render_graphics_bytes()==20370 && render_graphics_bytes()<=20*1024);
    printf("PASS: %u pixel-identical old/new-light frames, all mob types/facings/animation/hurt states, clipping, lighting offsets, tile types, graphics budget and state preservation\n",comparisons);
    return 0;
}
