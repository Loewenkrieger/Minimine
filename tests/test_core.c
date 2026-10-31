#include "game.h"
#include "perf.h"
#include "render.h"
#include <gint/display.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static Game a,b;
static uint16_t reference_pixels[SCREEN_W*VIEW_H];
void reference_world(const Game *,const RenderInfo *);

static void arena(Game *g)
{
    memset(g,0,sizeof(*g));
    for(int x=0;x<WORLD_W;++x) {
        g->tiles[20][x]=TILE_STONE;
        g->surface[x]=20;
    }
    g->player=(Player){ .x=160*FIXED_ONE, .y=(320-PLAYER_H)*FIXED_ONE,
        .facing=1,.grounded=true };
    g->aim_x=1;
    g->slots[0]=(ItemStack){.item=ITEM_DIRT,.count=32};
    game_follow_camera(g);
}
static void clear_body(const Game *g)
{
    const Player *p=&g->player;
    int left=p->x/FIXED_ONE/16, right=(p->x+PLAYER_W*FIXED_ONE-1)/FIXED_ONE/16;
    int top=p->y/FIXED_ONE/16;
    int bottom=(p->y+game_player_height(p)*FIXED_ONE-1)/FIXED_ONE/16;
    for(int y=top;y<=bottom;++y) for(int x=left;x<=right;++x)
        assert(!game_tile_solid(game_tile(g,x,y)));
}
static void test_generation_physics_aim(void)
{
    game_init(&a,WORLD_SEED);game_init(&b,WORLD_SEED);
    assert(!memcmp(&a,&b,sizeof(a)));clear_body(&a);
    assert(a.slots[0].item==ITEM_DIRT && a.slots[1].item==ITEM_STONE &&
        a.slots[2].item==ITEM_PLANKS);
    for(unsigned i=0;i<TOTAL_SLOTS;++i) assert(a.slots[i].count==(i<3?32:0));
    assert(game_tile(&a,-1,1)==TILE_BEDROCK);
    assert(!game_set_tile(&a,WORLD_W,0,TILE_DIRT));
    assert(!game_set_tile(&a,0,0,TILE_COUNT));
    for(int y=WORLD_H-2;y<WORLD_H;++y) for(int x=0;x<WORLD_W;++x)
        assert(a.tiles[y][x]==TILE_BEDROCK);
    game_init(&b,WORLD_SEED+1);assert(memcmp(a.tiles,b.tiles,sizeof(a.tiles)));
    arena(&a);int32_t y=a.player.y;
    for(int i=0;i<120;++i) game_step(&a,(GameInput){0});
    assert(a.player.y==y && a.player.grounded);
    game_step(&a,(GameInput){.jump=true});
    assert(a.player.y<y);int32_t vy=a.player.vy;
    game_step(&a,(GameInput){.jump=true});assert(a.player.vy==vy+64);
    for(int i=0;i<120;++i) game_step(&a,(GameInput){0});
    assert(a.player.y==y && a.player.grounded);
    for(int row=0;row<20;++row) a.tiles[row][13]=TILE_STONE;
    for(int i=0;i<120;++i) game_step(&a,(GameInput){.move=1});
    assert(a.player.x==(208-PLAYER_W)*FIXED_ONE);clear_body(&a);
    game_step(&a,(GameInput){.move=1,.aim_x=-1,.aim_y=-1});
    assert(a.aim_x==-1 && a.aim_y==-1);
    game_step(&a,(GameInput){.move=1});
    assert(a.aim_x==1 && a.aim_y==0); /* Even at a wall. */
    for(int ax=-1;ax<=1;++ax) for(int ay=-1;ay<=1;++ay) {
        if(!ax&&!ay) continue;
        game_step(&a,(GameInput){.aim_x=ax,.aim_y=ay});
        game_step(&a,(GameInput){0});
        assert(a.aim_x==ax && a.aim_y==ay);
    }
    for(int i=0;i<200;++i) game_step(&a,(GameInput){.move=-1});
    assert(a.player.x==0 && a.aim_x==-1);
    arena(&a);a.tiles[17][10]=TILE_STONE;
    for(int i=0;i<120;++i) {game_step(&a,(GameInput){.jump=i==0});clear_body(&a);}
    puts("PASS: deterministic world, bounds, collision, jump and eight aim directions");
}
static void test_sneak(void)
{
    arena(&a);int32_t feet=a.player.y+PLAYER_H*FIXED_ONE;int cam=a.camera_y;
    assert(game_toggle_sneak(&a));assert(game_player_height(&a.player)==14);
    assert(a.player.y+14*FIXED_ONE==feet && a.camera_y==cam);
    for(int i=0;i<30;++i) game_step(&a,(GameInput){.move=1});
    assert(a.player.vx==240);
    assert(game_toggle_sneak(&a));assert(a.player.y+22*FIXED_ONE==feet);
    for(int i=0;i<30;++i) game_step(&a,(GameInput){.move=1});
    assert(a.player.vx==480);
    arena(&a);game_toggle_sneak(&a);
    /* A one-tile-high passage: standing overlaps the ceiling. */
    for(int x=8;x<14;++x) a.tiles[18][x]=TILE_STONE;
    int32_t y=a.player.y;
    assert(!game_toggle_sneak(&a));
    assert(a.player.sneaking && a.player.y==y && a.notice==NOTICE_NO_SPACE);
    for(int i=0;i<90;++i) {game_step(&a,(GameInput){.move=1});clear_body(&a);}
    assert(a.player.x>14*16*FIXED_ONE && a.player.sneaking);
    assert(game_toggle_sneak(&a));clear_body(&a);

    for(int direction=-1;direction<=1;direction+=2) {
        arena(&a);
        for(int x=0;x<WORLD_W;++x) if(x!=10) a.tiles[20][x]=TILE_AIR;
        a.player.x=(10*16+3)*FIXED_ONE;
        game_toggle_sneak(&a);
        int32_t start_y=a.player.y;
        for(int i=0;i<100;++i) game_step(&a,(GameInput){.move=direction});
        assert(a.player.grounded && a.player.y==start_y);
        int32_t edge=a.player.x;
        assert(direction>0 ? edge==176*FIXED_ONE-1 : edge==150*FIXED_ONE+1);
        /* Jump bypasses the ledge guard but leaves Sneak active. */
        for(int i=0;i<20;++i) game_step(&a,(GameInput){.move=direction,.jump=i==0});
        assert(a.player.sneaking && !a.player.grounded);
        assert(direction>0 ? a.player.x>edge : a.player.x<edge);
    }
    arena(&a);game_toggle_sneak(&a);
    a.tiles[20][10]=TILE_AIR;
    y=a.player.y;
    for(int i=0;i<5;++i) game_step(&a,(GameInput){0});
    assert(a.player.y>y && !a.player.grounded);
    /* Standing can walk over the same unprotected ledge. */
    arena(&a);
    for(int x=11;x<WORLD_W;++x) a.tiles[20][x]=TILE_AIR;
    y=a.player.y;
    for(int i=0;i<40;++i) game_step(&a,(GameInput){.move=1});
    assert(a.player.y>y);
    puts("PASS: crouch height, feet/camera, half speed, low ceiling, both ledges, jump and falling");
}
static void test_inventory(void)
{
    game_init(&a,WORLD_SEED);
    assert(game_inventory_transfer(&a,0,3));
    assert(!a.slots[0].count && a.slots[3].count==32);
    assert(game_add_item(&a,ITEM_DIRT) && a.slots[3].count==33 && !a.slots[0].count);
    a.slots[0]=(ItemStack){.item=ITEM_DIRT,.count=63};
    assert(game_add_item(&a,ITEM_DIRT) && a.slots[0].count==64);
    assert(game_add_item(&a,ITEM_DIRT) && a.slots[3].count==34);
    a.slots[0].count=55;
    assert(game_inventory_transfer(&a,3,0));
    assert(a.slots[0].count==64 && a.slots[3].count==25);
    assert(!game_inventory_transfer(&a,3,0)); /* Full matching destination. */
    assert(game_inventory_transfer(&a,3,1)); /* Different materials swap. */
    assert(a.slots[3].item==ITEM_STONE && a.slots[1].count==25);
    assert(game_inventory_transfer(&a,1,4));
    assert(a.slots[1].item==ITEM_NONE);
    unsigned totals[ITEM_COUNT]={0};
    for(int item=1;item<ITEM_COUNT;++item) totals[item]=game_item_total(&a,(Item)item);
    uint32_t random=42;
    for(int i=0;i<5000;++i) {
        random=random*1664525u+1013904223u;
        game_inventory_transfer(&a,random%TOTAL_SLOTS,(random>>8)%TOTAL_SLOTS);
        for(int item=1;item<ITEM_COUNT;++item)
            assert(game_item_total(&a,(Item)item)==totals[item]);
        for(unsigned j=0;j<TOTAL_SLOTS;++j) {
            assert(a.slots[j].count<=64);
            assert((a.slots[j].item==ITEM_NONE)==(a.slots[j].count==0));
        }
    }
    memset(a.slots,0,sizeof(a.slots));
    assert(game_add_item(&a,ITEM_PLANKS));
    assert(a.slots[3].item==ITEM_PLANKS && !a.slots[0].count);
    for(int i=3;i<TOTAL_SLOTS;++i) a.slots[i]=(ItemStack){.item=ITEM_STONE,.count=64};
    assert(game_add_item(&a,ITEM_DIRT) && a.slots[0].count==1);
    for(int i=0;i<3;++i) a.slots[i]=(ItemStack){.item=ITEM_PLANKS,.count=64};
    assert(!game_add_item(&a,ITEM_DIRT));
    assert(!game_add_item(&a,ITEM_NONE));
    assert(!game_inventory_transfer(&a,99,0));
    assert(!game_inventory_transfer(&a,0,99));
    assert(!game_inventory_transfer(&a,0,0));
    arena(&a);game_select_hotbar(&a,2);assert(a.selected==2 && !game_place(&a));
    game_select_hotbar(&a,99);assert(a.selected==2);
    game_select_hotbar(&a,0);int x,y;game_target(&a,&x,&y);
    assert(game_place(&a) && a.tiles[y][x]==TILE_DIRT && a.slots[0].count==31);
    assert(!game_place(&a));
    arena(&a);a.aim_x=a.aim_y=0;assert(!game_place(&a));
    a.slots[0].count=1;a.aim_x=1;assert(game_place(&a));
    assert(a.slots[0].item==ITEM_NONE && !a.slots[0].count);
    puts("PASS: move, merge/remainder, swap, pickup order, empty slots and 5,000 conservation transactions");
}
static void test_mining(void)
{
    const Item drops[TILE_COUNT]={ITEM_NONE,ITEM_DIRT,ITEM_DIRT,ITEM_NONE,
        ITEM_PLANKS,ITEM_LOG,ITEM_NONE,ITEM_NONE,ITEM_NONE,ITEM_LOG,ITEM_WORKBENCH};
    for(int tile=TILE_GRASS;tile<TILE_COUNT;++tile) {
        arena(&a);int x,y;game_target(&a,&x,&y);a.tiles[y][x]=(uint8_t)tile;
        unsigned duration=game_mining_ticks((Tile)tile);
        unsigned before=drops[tile]?game_item_total(&a,drops[tile]):0;
        for(unsigned tick=1;tick<=duration+1;++tick) {
            game_step(&a,(GameInput){.mine=true});
            if(duration && tick<duration) assert(a.tiles[y][x]==tile && a.mining.elapsed==tick);
            else if(duration) assert(a.tiles[y][x]==TILE_AIR);
            else assert(a.tiles[y][x]==TILE_BEDROCK && !a.mining.active);
        }
        if(drops[tile]) assert(game_item_total(&a,drops[tile])==before+1);
        else assert(!game_item_total(&a,ITEM_STONE));
    }
    arena(&a);int x,y;game_target(&a,&x,&y);a.tiles[y][x]=TILE_DIRT;
    for(int i=0;i<10;++i) game_step(&a,(GameInput){.mine=true});
    game_step(&a,(GameInput){0});assert(!a.mining.active);
    game_step(&a,(GameInput){.mine=true});assert(a.mining.elapsed==1);
    game_step(&a,(GameInput){.mine=true,.aim_x=-1});assert(!a.mining.active);
    game_step(&a,(GameInput){.mine=true,.aim_x=1});assert(a.mining.elapsed==1);
    a.tiles[y][x]=TILE_STONE;
    game_step(&a,(GameInput){.mine=true});assert(a.mining.elapsed==1 && a.mining.required==60);
    game_cancel_mining(&a);assert(!a.mining.active);
    for(int i=0;i<TOTAL_SLOTS;++i) a.slots[i]=(ItemStack){.item=ITEM_DIRT,.count=64};
    a.tiles[y][x]=TILE_DIRT;
    for(int i=0;i<80;++i) game_step(&a,(GameInput){.mine=true});
    assert(a.tiles[y][x]==TILE_DIRT && a.notice==NOTICE_FULL);
    arena(&a);a.aim_x=0;a.aim_y=1;
    game_target(&a,&x,&y);assert(y==20);a.tiles[y][x]=TILE_DIRT;
    int32_t start=a.player.y;
    for(int i=0;i<21;++i) game_step(&a,(GameInput){.mine=true});
    assert(a.tiles[y][x]==TILE_AIR);
    game_step(&a,(GameInput){0});assert(a.player.y>start);
    puts("PASS: mining times, correct hand drops, reset rules, bedrock and full inventory");
}
static void test_tick_rates(void)
{
    const unsigned rates[]={10,15,30,60,120};
    for(unsigned r=0;r<sizeof(rates)/sizeof(rates[0]);++r) {
        arena(&a);int x,y;game_target(&a,&x,&y);a.tiles[y][x]=TILE_STONE;
        unsigned frames=0,ticks=0;uint64_t accumulator=0;
        while(a.tiles[y][x]!=TILE_AIR) {
            ++frames;accumulator+=60;
            while(accumulator>=rates[r]) {
                game_step(&a,(GameInput){.mine=true});
                accumulator-=rates[r];++ticks;
            }
            assert(frames<=rates[r]);
        }
        assert(ticks==60 && frames==rates[r]);
    }
    puts("PASS: one-second mining at 10, 15, 30, 60 and 120 rendered frames per second");
}
static void test_long_run(void)
{
    game_init(&a,WORLD_SEED);uint32_t r=12345;
    for(int i=0;i<20000;++i) {
        r=r*1664525u+1013904223u;
        game_step(&a,(GameInput){.move=(int)((r>>12)%3)-1,.jump=r%19==0,
            .toggle_sneak=r%101==0,.aim_x=(int)((r>>8)%3)-1,
            .aim_y=(int)((r>>10)%3)-1,.mine=r%3==0,.place=r%5==0});
        clear_body(&a);
        assert(a.camera_x>=0 && a.camera_x<=WORLD_W*16-SCREEN_W);
        assert(a.camera_y>=0 && a.camera_y<=WORLD_H*16-VIEW_H);
        for(int j=0;j<TOTAL_SLOTS;++j) assert(a.slots[j].count<=64);
    }
    static int32_t route[540][8];
    extern void reference_route(uint8_t [WORLD_H][WORLD_W],int32_t [540][8]);
    reference_route(b.tiles,route);
    game_init(&a,WORLD_SEED);assert(!memcmp(a.tiles,b.tiles,sizeof(a.tiles)));
    game_init(&b,WORLD_SEED);unsigned index=0;
    for(unsigned phase=0;phase<3;++phase) for(unsigned f=0;f<180;++f) {
        game_benchmark_step(&a,phase,f);game_benchmark_step(&b,phase,f);
        assert(!memcmp(&a,&b,sizeof(a)) && !a.player.sneaking);
        const int32_t *r0=route[index++];
        assert(a.player.x==r0[0] && a.player.y==r0[1] && a.player.vx==r0[2] && a.player.vy==r0[3]);
        assert(a.player.facing==r0[4] && a.player.grounded==r0[5]);
        assert(a.camera_x==r0[6] && a.camera_y==r0[7]);
    }
    puts("PASS: 20,000 mixed ticks and deterministic 540-frame benchmark");
}
static void test_metrics(void)
{
    PerfSummary summary = perf_summarize(NULL, 0);
    assert(summary.count == 0 && summary.fps_x10 == 0);
    PerfSample samples[100];
    for(unsigned i = 0; i < 100; ++i)
        samples[i] = (PerfSample){ .frame_us = (i + 1) * 1000,
            .logic_us = 100, .draw_us = 200, .lcd_us = 300 };
    summary = perf_summarize(samples, 100);
    assert(summary.mean_us == 50500 && summary.p95_us == 95000);
    assert(summary.worst_us == 100000 && summary.fps_x10 == 198);
    assert(summary.logic_us == 100 && summary.draw_us == 200 && summary.lcd_us == 300);
    PerfSample uneven[] = { { .frame_us = 10000 }, { .frame_us = 30000 } };
    assert(perf_summarize(uneven, 2).fps_x10 == 500); /* Not mean of instantaneous FPS. */
    PerfHistory history;
    perf_reset(&history);
    for(unsigned i = 0; i < PERF_CAPACITY + 10; ++i)
        perf_push(&history, (PerfSample){ .frame_us = i + 1 });
    assert(history.count == PERF_CAPACITY && history.next == 10);
    summary = perf_summarize(history.samples, history.count);
    assert(summary.worst_us == PERF_CAPACITY + 10 && summary.mean_us == 85);
    puts("PASS: weighted FPS, mean, P95, phase timings and bounded history");
}



static void compare_world(RenderInfo info)
{
    dclear(0);reference_world(&a,&info);
    memcpy(reference_pixels,gint_vram+VIEW_TOP*SCREEN_W,sizeof(reference_pixels));
    dclear(0);render_world(&a,&info);
    assert(!memcmp(reference_pixels,gint_vram+VIEW_TOP*SCREEN_W,sizeof(reference_pixels)));
    for(int i=0;i<VIEW_TOP*SCREEN_W;++i) assert(gint_vram[i]==0);
    for(int i=(VIEW_TOP+VIEW_H)*SCREEN_W;i<SCREEN_H*SCREEN_W;++i) assert(gint_vram[i]==0);
    host_check_guards();
}
static void test_rendering(void)
{
    RenderInfo info={0};
    game_init(&a,WORLD_SEED);
    for(unsigned phase=0;phase<3;++phase) for(unsigned f=0;f<180;f+=7) {
        game_benchmark_step(&a,phase,f);info.benchmark=true;compare_world(info);
    }
    /* All tile types and variants, including clipped tree masks at each edge. */
    for(int y=0;y<WORLD_H;++y) for(int x=0;x<WORLD_W;++x)
        a.tiles[y][x]=(uint8_t)((x+3*y)%TILE_LOG);
    for(int y=0;y<=16;++y) for(int x=0;x<=16;++x) {
        a.camera_x=96+x;a.camera_y=240+y;compare_world(info);
    }
    game_init(&a,WORLD_SEED);info=(RenderInfo){0};compare_world(info);
    render_invalidate();render_game(&a,&info);
    assert(host_nonzero_pixels()==SCREEN_W*SCREEN_H);
    render_help();render_game(&a,&info); /* Must restore both cached strips. */
    assert(host_nonzero_pixels()==SCREEN_W*SCREEN_H);
    InventoryUI ui={.source=-1};
    for(unsigned i=0;i<TOTAL_SLOTS;++i) {ui.cursor=i;render_inventory(&a,&ui);}
    for(unsigned width=2;width<=3;++width) {
        ui.craft_width=width;
        for(unsigned i=0;i<width*width;++i) {
            ui.cursor=CRAFT_FIRST+i;render_inventory(&a,&ui);
        }
        ui.cursor=CRAFT_RESULT;render_inventory(&a,&ui);
    }
    b=a;
    render_inventory(&a,&ui);assert(!memcmp(&a,&b,sizeof(a)));
    for(int y=0;y<WORLD_H;++y) for(int x=0;x<WORLD_W;++x)
        a.tiles[y][x]=(uint8_t)((x+y)%2?TILE_LOG:TILE_WORKBENCH);
    for(int offset=0;offset<16;++offset) {
        a.camera_x=offset;a.camera_y=offset;
        dclear(0);render_world(&a,&info);host_check_guards();
        for(int i=0;i<VIEW_TOP*SCREEN_W;++i) assert(gint_vram[i]==0);
    }
    game_init(&a,WORLD_SEED);
    a.mining=(Mining){.active=true,.required=60,.elapsed=30,.x=-1,.y=-1};
    render_game(&a,&info);game_toggle_sneak(&a);render_game(&a,&info);
    PerfSummary results[BENCH_PHASES]={{0}};render_results(results);render_error("Test");
    host_check_guards();
    puts("PASS: pixel-identical old world rendering, masks, clipping, HUD invalidation and all pages");
}
int main(void)
{
    test_generation_physics_aim();test_sneak();test_inventory();test_mining();
    test_tick_rates();test_long_run();test_metrics();test_rendering();
    printf("All native checks passed. Game state: %zu bytes per world.\n",sizeof(Game));
    return 0;
}

