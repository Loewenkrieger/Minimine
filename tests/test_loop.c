#include <assert.h>
#include <stdio.h>
#define main sandbox_main
#include "../src/main.c"
#undef main

extern void (*host_present_hook)(void);
static uint32_t counter=50000;
volatile uint32_t *prof_tcnt=&counter;
static unsigned presentations,page_waits,stage_frames,bench_frames;
static bool profiler_quit,held[64];
static key_event_t queue[32];
static unsigned next_event,event_count;
static Game saved_play,paused;
typedef enum { START,INITIAL,INVENTORY,RESUMED,WORKBENCH_OPEN,AFTER_WORKBENCH,HELP_RESUME,BEFORE_BENCH,BENCH,
    AFTER_BENCH,EXIT_HELP } TestStage;
static TestStage stage=START;

static void key(unsigned type,int code)
{
    assert(event_count<32);
    queue[event_count++]=(key_event_t){.type=type,.key=code};
}
static void presented(void)
{
    ++presentations;counter-=20000;
    assert(presentations<700);
    switch(stage) {
    case INITIAL:
        ++stage_frames;
        if(stage_frames==1) {
            key(KEYEV_DOWN,KEY_DOWN);key(KEYEV_DOWN,KEY_UP);
            counter-=100000; /* Six catch-up steps must consume edges only once. */
        }
        if(stage_frames==2) {
            assert(play_game.player.sneaking && play_game.player.vy<0);
            key(KEYEV_HOLD,KEY_DOWN);key(KEYEV_HOLD,KEY_UP);
        }
        if(stage_frames==3) {
            assert(play_game.player.sneaking);
            assert(play_game.player.vy>-1536+6*64);
            int x,y;game_target(&play_game,&x,&y);
            game_set_tile(&play_game,x,y,TILE_DIRT);
            play_game.mining=(Mining){.active=true,.x=x,.y=y,.tile=TILE_DIRT,
                .elapsed=10,.required=21};
            key(KEYEV_UP,KEY_DOWN);key(KEYEV_UP,KEY_UP);
            key(KEYEV_DOWN,KEY_KBD);
            paused=play_game;stage=INVENTORY;stage_frames=0;
        }
        break;
    case RESUMED:
        ++stage_frames;
        assert(play_game.player.sneaking && !play_game.mining.active);
        assert(play_game.slots[1].count==32 && play_game.slots[3].count==32);
        assert(play_game.ticks<=paused.ticks+stage_frames*2);
        if(stage_frames==1) assert(play_game.ticks==paused.ticks);
        if(stage_frames==3) {
            key(KEYEV_UP,KEY_DEL);key(KEYEV_UP,KEY_EXE);
            key(KEYEV_DOWN,KEY_Y);
        }
        if(stage_frames==4) {
            assert(play_game.selected==1);
            Player *p=&play_game.player;
            p->x=195*FIXED_ONE;p->y=(480-PLAYER_CROUCH_H)*FIXED_ONE;
            p->vx=p->vy=0;p->grounded=true;
            play_game.aim_x=1;play_game.aim_y=0;
            game_follow_camera(&play_game);
            int x,y;game_target(&play_game,&x,&y);
            game_set_tile(&play_game,x,y,TILE_WORKBENCH);
            play_game.slots[4]=(ItemStack){.item=ITEM_STICK,.count=2};
            key(KEYEV_DOWN,KEY_EXE);stage=WORKBENCH_OPEN;stage_frames=0;
        }
        break;
    case AFTER_WORKBENCH:
        assert(play_game.player.sneaking && play_game.slots[1].count==32);
        assert(game_item_total(&play_game,ITEM_WOOD_PICKAXE)==1);
        assert(game_item_total(&play_game,ITEM_PLANKS)==29);
        if(++stage_frames==3) {key(KEYEV_DOWN,KEY_CLEAR);stage=HELP_RESUME;}
        break;
    case BEFORE_BENCH:
        if(++stage_frames==3) {
            saved_play=play_game;key(KEYEV_DOWN,KEY_0);
            stage=BENCH;
        }
        break;
    case BENCH:
        if(results[BENCH_PHASES-1].count == BENCH_SAMPLES) break; /* Results page. */
        ++bench_frames;assert(!bench_game.player.sneaking);
        assert(!memcmp(&saved_play,&play_game,sizeof(saved_play)));
        break;
    case AFTER_BENCH:
        if(++stage_frames==4) {key(KEYEV_DOWN,KEY_CLEAR);stage=EXIT_HELP;}
        break;
    default:break;
    }
}
int prof_init(void) {return 0;}
void prof_quit(void) {profiler_quit=true;}
uint32_t prof_time(prof_t p) {return p.elapsed;}
int keydown(int code) {assert(code>=0 && code<64);return held[code];}
key_event_t pollevent(void)
{
    counter-=100;
    if(next_event==event_count) {next_event=event_count=0;return (key_event_t){0};}
    key_event_t e=queue[next_event++];
    if(e.type==KEYEV_DOWN) held[e.key]=true;
    if(e.type==KEYEV_UP) held[e.key]=false;
    return e;
}
key_event_t waitevent(volatile int *timeout)
{
    (void)timeout;assert(!"unexpected blocking key wait");return (key_event_t){0};
}
key_event_t getkey_opt(int options,volatile int *timeout)
{
    (void)options;(void)timeout;++page_waits;
    counter-=30000000; /* Menus may remain open arbitrarily long. */
    memset(held,0,sizeof(held));
    int code=KEY_EXE;
    if(stage==START) {stage=INITIAL;stage_frames=0;}
    else if(stage==INVENTORY) {
        assert(play_game.ticks==paused.ticks);
        assert(!memcmp(&play_game.player,&paused.player,sizeof(Player)));
        assert(!memcmp(play_game.tiles,paused.tiles,sizeof(paused.tiles)));
        assert(!play_game.mining.active && play_game.player.sneaking);
        static const int keys[]={KEY_EXE,KEY_DOWN,KEY_EXE,KEY_UP,
            KEY_EXE,KEY_X,KEY_DOWN,KEY_EXE,KEY_Y,KEY_KBD};
        assert(stage_frames<sizeof(keys)/sizeof(keys[0]));
        code=keys[stage_frames++];
        if(code==KEY_KBD) {
            assert(!play_game.slots[0].count && play_game.slots[3].count==32);
            stage=RESUMED;stage_frames=0;
            held[KEY_EXE]=held[KEY_DEL]=true;
        }
    } else if(stage==WORKBENCH_OPEN) {
        if(!stage_frames) paused=play_game;
        assert(play_game.ticks==paused.ticks && play_game.player.sneaking);
        assert(!memcmp(&play_game.player,&paused.player,sizeof(Player)));
        static const int keys[]={
            KEY_Z,KEY_EXE,
            KEY_DOWN,KEY_DOWN,KEY_DOWN,KEY_DOWN,KEY_DOWN,
            KEY_5,KEY_LEFT,KEY_5,KEY_LEFT,KEY_5,
            KEY_Z,KEY_EXE,KEY_Y,KEY_DOWN,KEY_EXE,
            KEY_DOWN,KEY_DOWN,KEY_DOWN,KEY_DOWN,KEY_DOWN,
            KEY_5,KEY_DOWN,KEY_5,KEY_RIGHT,KEY_RIGHT,KEY_EXE,KEY_CLEAR
        };
        assert(stage_frames<sizeof(keys)/sizeof(keys[0]));
        code=keys[stage_frames++];
        if(code==KEY_CLEAR) {
            assert(game_item_total(&play_game,ITEM_WOOD_PICKAXE)==1);
            assert(play_game.slots[3].item==ITEM_DIRT);
            stage=AFTER_WORKBENCH;stage_frames=0;
            held[KEY_EXE]=true;
        }
    } else if(stage==HELP_RESUME) {
        assert(play_game.player.sneaking);
        stage=BEFORE_BENCH;stage_frames=0;
    } else if(stage==BENCH) {
        assert(bench_frames==540);
        assert(!memcmp(&saved_play,&play_game,sizeof(saved_play)));
        for(unsigned i=0;i<3;++i) {
            assert(results[i].count==150 && results[i].lcd_us==20000);
            assert(results[i].mean_us>=20000 && results[i].mean_us<30000);
        }
        stage=AFTER_BENCH;stage_frames=0;
    } else {
        assert(stage==EXIT_HELP);code=KEY_CLEAR;
    }
    /* Except the deliberate post-menu held actions, release on next frame. */
    held[code]=true;
    if(stage==INITIAL || stage==BEFORE_BENCH || stage==AFTER_BENCH)
        key(KEYEV_UP,code);
    return (key_event_t){.type=KEYEV_DOWN,.key=code};
}
static void test_controller(void)
{
    Game g;game_init(&g,WORLD_SEED);
    InventoryUI ui={.source=-1};
    uint32_t ticks=g.ticks;Player player=g.player;
    inventory_key(&g,&ui,KEY_UP);assert(ui.cursor==0);
    inventory_key(&g,&ui,KEY_LEFT);assert(ui.cursor==0);
    inventory_key(&g,&ui,KEY_Z);assert(ui.cursor==2 && g.selected==2);
    for(int i=0;i<10;++i) inventory_key(&g,&ui,KEY_DOWN);
    assert(ui.cursor==CRAFT_FIRST+3);
    inventory_key(&g,&ui,KEY_RIGHT);assert(ui.cursor==CRAFT_RESULT);
    inventory_key(&g,&ui,KEY_LEFT);assert(ui.cursor==CRAFT_FIRST+3);
    inventory_key(&g,&ui,KEY_X);
    inventory_key(&g,&ui,KEY_EXE);assert(ui.source==0);
    inventory_key(&g,&ui,KEY_EXE);assert(ui.source==-1 && g.slots[0].count==32);
    inventory_key(&g,&ui,KEY_EXE);
    assert(inventory_key(&g,&ui,KEY_CLEAR) && ui.source==-1);
    assert(g.ticks==ticks && !memcmp(&player,&g.player,sizeof(player)));
    /* Build a workbench through controller actions in the 2x2 grid. */
    inventory_key(&g,&ui,KEY_Z);inventory_key(&g,&ui,KEY_EXE);
    for(unsigned i=0;i<4;++i) {
        ui.cursor=CRAFT_FIRST+i;inventory_key(&g,&ui,KEY_5);
    }
    assert(g.slots[2].count==28 && game_craft_preview(&g,2).item==ITEM_WORKBENCH);
    ui.cursor=CRAFT_RESULT;inventory_key(&g,&ui,KEY_EXE);
    assert(game_item_total(&g,ITEM_WORKBENCH)==1 && ui.source==-1);
    assert(inventory_key(&g,&ui,KEY_KBD));
    /* Every arrow stays in a visible slot for both grid sizes. */
    for(unsigned width=2;width<=3;++width) {
        ui.craft_width=width;
        for(unsigned at=0;at<=CRAFT_RESULT;++at) {
            if(at>=CRAFT_FIRST+width*width && at!=CRAFT_RESULT) continue;
            const int directions[]={KEY_UP,KEY_DOWN,KEY_LEFT,KEY_RIGHT};
            for(unsigned d=0;d<4;++d) {
                ui.cursor=at;inventory_key(&g,&ui,directions[d]);
                assert(ui.cursor<TOTAL_SLOTS || ui.cursor==CRAFT_RESULT ||
                    (ui.cursor>=CRAFT_FIRST && ui.cursor<CRAFT_FIRST+width*width));
            }
        }
    }
    /* Both jump keys and held manual direction override movement input. */
    key(KEYEV_DOWN,KEY_SHIFT);key(KEYEV_HOLD,KEY_DOWN);
    Presses p=read_input();assert(p.jump && !p.sneak_toggles);
    key(KEYEV_DOWN,KEY_UP);key(KEYEV_DOWN,KEY_DOWN);
    p=read_input();assert(p.jump && p.sneak_toggles==1);
    key(KEYEV_UP,KEY_DOWN);key(KEYEV_DOWN,KEY_DOWN);
    key(KEYEV_UP,KEY_DOWN);key(KEYEV_DOWN,KEY_DOWN);
    p=read_input();assert(p.sneak_toggles==2);
    held[KEY_LEFT]=held[KEY_9]=true;
    GameInput input=held_input();assert(input.move==-1 && input.aim_x==1 && input.aim_y==-1);
    memset(held,0,sizeof(held));
}
int main(void)
{
    test_controller();host_present_hook=presented;
    assert(sandbox_main()==0 && profiler_quit);
    assert(stage==EXIT_HELP && bench_frames==540);
    puts("PASS: real loop, edge-only jump/Sneak through catch-up, paused inventory navigation");
    puts("PASS: 2x2 crafting, real EXE workbench interaction, 3x3 pickaxe crafting and no held-key reopen");
    puts("PASS: material conservation, 540 benchmark frames, timer wrap and exit");
    return 0;
}

