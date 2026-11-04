#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#define main minimine_main
#include "../src/main.c"
#undef main

extern void (*host_present_hook)(void);
static uint32_t counter=50000;
volatile uint32_t *prof_tcnt=&counter;
static bool held[64],quit;
static key_event_t queue[32];static unsigned next_event,event_count;
static unsigned stage,frames,bench_frames,presentations,page_keys;
static Game paused,saved_play;
static void key(unsigned type,int code)
{assert(event_count<32);queue[event_count++]=(key_event_t){.type=type,.key=code};}
int prof_init(void) {return 0;}
void prof_quit(void) {quit=true;}
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
    static bool pending;static key_event_t release;
    if(pending) {pending=false;return release;}
    key_event_t e=getkey_opt(GETKEY_NONE,timeout);UIRect r={0};bool touch=false;
    if((app.mode==MODE_START || app.mode==MODE_PAUSE || app.mode==MODE_DIFFICULTY) && e.key==KEY_EXE)
        {r=ui_menu_rect(app.cursor);touch=true;}
    else if((app.mode==MODE_HELP || app.mode==MODE_RESULTS) && e.key==KEY_EXE)
        {r=ui_play_rect();touch=true;}
    else if(app.mode==MODE_INVENTORY && e.key==KEY_SHIFT) {r=ui_armor_rect();touch=true;}
    else if(app.mode==MODE_INVENTORY && (e.key==KEY_KBD || e.key==KEY_CLEAR))
        {r=ui_close_rect();touch=true;}
    if(!touch) return e;
    e=(key_event_t){.type=KEYEV_TOUCH_DOWN,.x=(unsigned short)(r.x+r.w/2),.y=(unsigned short)(r.y+r.h/2)};
    release=e;release.type=KEYEV_TOUCH_UP;pending=true;return e;
}
static void presented(void)
{
    ++presentations;counter-=20000;assert(presentations<700);
    if(app.mode==MODE_BENCH) {
        ++bench_frames;assert(!bench_game.survival && !bench_game.player.sneaking);
        assert(!memcmp(&saved_play,&play_game,sizeof(play_game)));return;
    }
    if(app.mode!=MODE_PLAY) return;
    ++frames;
    if(stage==1) {
        if(frames==1) {key(KEYEV_DOWN,KEY_DOWN);key(KEYEV_DOWN,KEY_UP);counter-=100000;}
        if(frames==2) {assert(play_game.player.sneaking && play_game.player.vy<0);key(KEYEV_HOLD,KEY_DOWN);key(KEYEV_HOLD,KEY_UP);}
        if(frames==3) {
            assert(play_game.player.sneaking);
            key(KEYEV_UP,KEY_UP);key(KEYEV_UP,KEY_DOWN);key(KEYEV_DOWN,KEY_KBD);
            paused=play_game;stage=2;page_keys=0;
        }
    } else if(stage==3) {
        if(frames<=3) assert(play_game.player.sneaking && !play_game.mining.active);
        assert(play_game.ticks<=paused.ticks+frames*2);
        if(frames==1) assert(play_game.ticks==paused.ticks);
        if(frames==3) {
            key(KEYEV_UP,KEY_DEL);key(KEYEV_UP,KEY_EXE);
            /* Put a real interactive block beside a grounded player. */
            game_init(&play_game,WORLD_SEED);play_game.slots[0]=(ItemStack){0};
            int x,y;game_target(&play_game,&x,&y);game_set_tile(&play_game,x,y,TILE_WORKBENCH);
        }
        if(frames==4) {key(KEYEV_DOWN,KEY_EXE);stage=4;page_keys=0;}
    } else if(stage==5 && frames==3) {key(KEYEV_DOWN,KEY_CLEAR);stage=6;page_keys=0;}
    else if(stage==8 && frames==2) {saved_play=play_game;key(KEYEV_DOWN,KEY_0);stage=9;}
    else if(stage==10 && frames==2) {key(KEYEV_DOWN,KEY_CLEAR);stage=11;page_keys=0;}
}
key_event_t getkey_opt(int options,volatile int *timeout)
{
    (void)options;(void)timeout;counter-=30000000;memset(held,0,sizeof(held));int code=KEY_EXE;
    if(app.mode==MODE_DIFFICULTY) {assert(app.cursor==DIFFICULTY_MEDIUM);}
    else if(stage==0) {assert(app.mode==MODE_START);stage=1;frames=0;}
    else if(stage==2) {
        assert(app.mode==MODE_INVENTORY && play_game.ticks==paused.ticks && !play_game.mining.active);
        assert(!memcmp(&play_game.player,&paused.player,sizeof(Player)));
        static const int keys[]={KEY_SHIFT,KEY_UP,KEY_DOWN,KEY_SHIFT,KEY_KBD};
        assert(page_keys<sizeof(keys)/sizeof(keys[0]));code=keys[page_keys++];
        if(code==KEY_KBD) {stage=3;frames=0;held[KEY_EXE]=held[KEY_DEL]=true;}
    } else if(stage==4) {
        assert(app.mode==MODE_INVENTORY && app.inventory.craft_width==3 && !app.inventory.furnace);
        code=KEY_CLEAR;stage=5;frames=0;held[KEY_EXE]=true;
    } else if(stage==6) {
        assert(app.mode==MODE_PAUSE);code=page_keys++<3?KEY_DOWN:KEY_EXE;
        if(code==KEY_EXE) stage=7;
    } else if(stage==7) {assert(app.mode==MODE_HELP);stage=8;frames=0;}
    else if(stage==9) {
        assert(app.mode==MODE_RESULTS && bench_frames==540);
        for(unsigned i=0;i<3;++i) assert(results[i].count==150 && results[i].lcd_us==20000);
        assert(!memcmp(&saved_play,&play_game,sizeof(play_game)));stage=10;frames=0;
    } else {
        assert(stage==11);
        if(app.mode==MODE_PAUSE) code=page_keys++<5?KEY_DOWN:KEY_EXE;
        else assert(!"exit must not require confirmation");
    }
    held[code]=true;
    if(stage==1 || stage==8 || stage==10) key(KEYEV_UP,code);
    return (key_event_t){.type=KEYEV_DOWN,.key=code};
}
static void controller(void)
{
    game_init(&play_game,WORLD_SEED);InventoryUI ui={.source=-1,.craft_width=2};
    uint32_t ticks=play_game.ticks;Player player=play_game.player;
    ui.cursor=3;inventory_key(&play_game,&ui,KEY_X);
    assert(ui.cursor==3 && play_game.selected==0 && play_game.slots[3].count==32 && !play_game.slots[0].count);
    inventory_key(&play_game,&ui,KEY_SHIFT);assert(ui.armor);
    inventory_key(&play_game,&ui,KEY_UP);inventory_key(&play_game,&ui,KEY_DOWN);
    assert(play_game.ticks==ticks && !memcmp(&player,&play_game.player,sizeof(player)));
    inventory_key(&play_game,&ui,KEY_SHIFT);assert(!ui.armor && ui.cursor==3);
    ui.cursor=2;inventory_key(&play_game,&ui,KEY_EXE);
    for(unsigned i=0;i<4;++i) {ui.cursor=CRAFT_FIRST+i;inventory_key(&play_game,&ui,KEY_5);}
    assert(game_craft_preview(&play_game,2).item==ITEM_WORKBENCH);
    ui.cursor=UI_RESULT;inventory_key(&play_game,&ui,KEY_EXE);
    assert(game_item_total(&play_game,ITEM_WORKBENCH)==1 && inventory_key(&play_game,&ui,KEY_KBD));
    key(KEYEV_DOWN,KEY_SHIFT);key(KEYEV_HOLD,KEY_DOWN);Presses p=read_input();assert(p.jump && !p.sneak);
    key(KEYEV_DOWN,KEY_UP);key(KEYEV_DOWN,KEY_DOWN);p=read_input();assert(p.jump && p.sneak==1);
    key(KEYEV_UP,KEY_DOWN);key(KEYEV_DOWN,KEY_DOWN);key(KEYEV_UP,KEY_DOWN);key(KEYEV_DOWN,KEY_DOWN);
    p=read_input();assert(p.sneak==2);
    memset(held,0,sizeof(held));held[KEY_LEFT]=held[KEY_9]=true;
    GameInput input=held_input();assert(input.move==-1 && input.aim_x==1 && input.aim_y==-1);
    memset(held,0,sizeof(held));held[KEY_2]=held[KEY_EXE]=true;
    input=held_input();assert(!input.move && !input.aim_x && input.aim_y==1 && input.use_held);
    memset(held,0,sizeof(held));
    puts("PASS: real key mappings, single transfers, quickmove, armor view and jump/Sneak edges");
}
static void menus_and_saves(void)
{
    store.root="build/native/loop-save";
    for(unsigned s=1;s<=3;++s) for(unsigned c=0;c<2;++c) {
        char path[80];snprintf(path,sizeof(path),"%s/slot%u%c.sav",store.root,s,c?'b':'a');unlink(path);
    }
    app=(App){.mode=MODE_START,.running=true};handle_page(KEY_EXE);assert(app.mode==MODE_DIFFICULTY);
    handle_page(KEY_EXE);assert(app.mode==MODE_PLAY && dirty());
    page(MODE_PAUSE);handle_page(KEY_DOWN);handle_page(KEY_EXE);assert(app.mode==MODE_SAVE);
    handle_page(KEY_EXE);assert(app.mode==MODE_MESSAGE && !dirty());handle_page(KEY_EXE);
    assert(app.mode==MODE_PAUSE);
    handle_page(KEY_DOWN);handle_page(KEY_EXE);handle_page(KEY_EXE);
    assert(app.mode==MODE_CONFIRM && !app.yes && app.confirm==CONFIRM_SAVE);
    handle_page(KEY_EXE);assert(app.mode==MODE_SAVE); /* No is default. */
    handle_page(KEY_EXE);handle_page(KEY_RIGHT);handle_page(KEY_EXE);assert(app.mode==MODE_MESSAGE);
    handle_page(KEY_EXE);game_step(&play_game,(GameInput){0});assert(dirty());
    page(MODE_PAUSE);handle_page(KEY_DOWN);handle_page(KEY_DOWN);handle_page(KEY_EXE);
    assert(app.mode==MODE_LOAD);handle_page(KEY_EXE);assert(app.mode==MODE_CONFIRM && app.confirm==CONFIRM_LOAD);
    uint32_t current=play_game.ticks;handle_page(KEY_CLEAR);assert(play_game.ticks==current);
    handle_page(KEY_EXE);handle_page(KEY_RIGHT);handle_page(KEY_EXE);assert(app.mode==MODE_PLAY && !dirty() && !play_game.ticks);
    page(MODE_START);app.has_game=false;handle_page(KEY_DOWN);handle_page(KEY_EXE);handle_page(KEY_EXE);
    assert(app.mode==MODE_PLAY && app.has_game);
    memset(held,0,sizeof(held));next_event=event_count=0;
    puts("PASS: pause/start load, overwrite/discard confirmations, cancel and saving baseline");
}
static void touch_event(unsigned type,UIRect r)
{
    handle_menu_event((key_event_t){.type=type,
        .x=(unsigned short)(r.x+r.w/2),.y=(unsigned short)(r.y+r.h/2)});
}
static void tap_menu(UIRect r)
{touch_event(KEYEV_TOUCH_DOWN,r);touch_event(KEYEV_TOUCH_UP,r);}
static void touch_menus(void)
{
    app=(App){.mode=MODE_START,.running=true};
    touch_event(KEYEV_TOUCH_DOWN,ui_menu_rect(0));assert(!app.has_game);
    touch_event(KEYEV_TOUCH_UP,ui_menu_rect(0));assert(app.mode==MODE_DIFFICULTY && !app.has_game);
    tap_menu(ui_menu_rect(DIFFICULTY_MEDIUM));assert(app.mode==MODE_PLAY && app.has_game);
    page(MODE_INVENTORY);app.inventory=(InventoryUI){.source=-1,.craft_width=2};
    held[KEY_DIV]=true;handle_menu_event((key_event_t){.type=KEYEV_DOWN,.key=KEY_DIV});
    tap_menu((UIRect){16,85,92,30});assert(app.inventory.source_half);
    held[KEY_DIV]=false;tap_menu((UIRect){16,140,92,30});
    assert(play_game.slots[0].count==16 && play_game.slots[4].count==16);
    assert(!play_game.player.sneaking && !play_game.ticks);
    tap_menu(ui_armor_rect());assert(app.inventory.armor);
    tap_menu(ui_close_rect());assert(app.mode==MODE_PLAY);
    page(MODE_PAUSE);
    touch_event(KEYEV_TOUCH_DOWN,ui_menu_rect(1));touch_event(KEYEV_TOUCH_UP,ui_menu_rect(4));
    assert(app.mode==MODE_PAUSE && app.running); /* No slide-to-quit. */
    tap_menu(ui_menu_rect(1));assert(app.mode==MODE_SAVE);
    tap_menu(ui_save_rect(0));assert(app.mode==MODE_CONFIRM && !app.yes);
    touch_event(KEYEV_TOUCH_UP,ui_confirm_rect(true));assert(app.mode==MODE_CONFIRM); /* No fallthrough. */
    tap_menu(ui_confirm_rect(false));assert(app.mode==MODE_SAVE);
    tap_menu(ui_save_rect(0));tap_menu(ui_confirm_rect(true));assert(app.mode==MODE_MESSAGE && !dirty());
    touch_event(KEYEV_TOUCH_UP,ui_message_rect());assert(app.mode==MODE_MESSAGE);
    tap_menu(ui_message_rect());assert(app.mode==MODE_PAUSE);
    game_step(&play_game,(GameInput){0});uint32_t ticks=play_game.ticks;
    tap_menu(ui_menu_rect(2));tap_menu(ui_save_rect(0));assert(app.mode==MODE_CONFIRM);
    tap_menu(ui_confirm_rect(false));assert(app.mode==MODE_LOAD && play_game.ticks==ticks);
    tap_menu(ui_save_rect(0));tap_menu(ui_confirm_rect(true));assert(app.mode==MODE_PLAY && !dirty());
    page(MODE_PAUSE);tap_menu(ui_menu_rect(3));assert(app.mode==MODE_HELP);
    tap_menu(ui_back_rect());assert(app.mode==MODE_PAUSE);
    tap_menu(ui_menu_rect(3));tap_menu(ui_benchmark_rect());assert(app.mode==MODE_BENCH);
    page(MODE_RESULTS);tap_menu(ui_back_rect());assert(app.mode==MODE_PAUSE);
    play_game.revision++;assert(dirty());
    touch_event(KEYEV_TOUCH_DOWN,ui_menu_rect(5));assert(app.running && app.mode==MODE_PAUSE);
    touch_event(KEYEV_TOUCH_UP,ui_menu_rect(5));assert(!app.running && app.mode==MODE_PAUSE);
    /* Keyboard quitting is equally direct. */
    app.running=true;app.cursor=5;handle_page(KEY_EXE);assert(!app.running && app.mode==MODE_PAUSE);
    memset(held,0,sizeof(held));
    puts("PASS: touch menus, real divide modifier, save/load yes/no, release guards and immediate dirty-game exit");
}
static void queue_touch(unsigned type,UIRect r)
{
    assert(event_count<32);queue[event_count++]=(key_event_t){.type=type,
        .x=(unsigned short)(r.x+r.w/2),.y=(unsigned short)(r.y+r.h/2)};
}
static void new_controls(void)
{
    game_init(&play_game,WORLD_SEED);app=(App){.mode=MODE_PLAY,.has_game=true,.running=true};
    next_event=event_count=0;memset(held,0,sizeof(held));held[KEY_RIGHT]=true;
    for(unsigned i=0;i<HOTBAR_SLOTS;++i) {
        queue_touch(KEYEV_TOUCH_DOWN,ui_hotbar_rect(i,false));
        queue_touch(KEYEV_TOUCH_UP,ui_hotbar_rect(i,false));Presses p=read_input();
        assert(p.hotbar==(int)i && !p.use && !p.jump && !p.sneak && held_input().move==1);
        game_select_hotbar(&play_game,(unsigned)p.hotbar);assert(play_game.selected==i);
    }
    key(KEYEV_DOWN,KEY_POWER);Presses p=read_input();assert(p.hotbar==3);
    queue_touch(KEYEV_TOUCH_DOWN,ui_hotbar_rect(0,false));queue_touch(KEYEV_TOUCH_DRAG,ui_hotbar_rect(1,false));
    queue_touch(KEYEV_TOUCH_UP,ui_hotbar_rect(0,false));p=read_input();assert(p.hotbar==-1);
    queue_touch(KEYEV_TOUCH_UP,ui_hotbar_rect(2,false));p=read_input();assert(p.hotbar==-1);
    queue_touch(KEYEV_TOUCH_DOWN,ui_hotbar_rect(1,false));read_input();enter_play();
    queue_touch(KEYEV_TOUCH_UP,ui_hotbar_rect(1,false));p=read_input();assert(p.hotbar==-1);
    uint32_t ticks=play_game.ticks;Player player=play_game.player;
    page(MODE_PAUSE);tap_menu(ui_menu_rect(4));assert(app.mode==MODE_DIFFICULTY && app.cursor==DIFFICULTY_MEDIUM);
    handle_page(KEY_UP);assert(play_game.difficulty==DIFFICULTY_MEDIUM && app.cursor==DIFFICULTY_EASY);
    handle_page(KEY_CLEAR);assert(app.mode==MODE_PAUSE && play_game.difficulty==DIFFICULTY_MEDIUM);
    tap_menu(ui_menu_rect(4));tap_menu(ui_menu_rect(DIFFICULTY_HARD));
    assert(app.mode==MODE_PLAY && play_game.difficulty==DIFFICULTY_HARD && play_game.ticks==ticks);
    assert(!memcmp(&player,&play_game.player,sizeof(player)));
    app=(App){.mode=MODE_START,.running=true};Game previous=play_game;
    handle_page(KEY_EXE);assert(app.mode==MODE_DIFFICULTY && app.cursor==DIFFICULTY_MEDIUM);
    handle_page(KEY_DOWN);assert(!memcmp(&previous,&play_game,sizeof(previous)));
    handle_page(KEY_EXE);assert(app.mode==MODE_PLAY && play_game.difficulty==DIFFICULTY_HARD && !play_game.ticks);
    memset(held,0,sizeof(held));next_event=event_count=0;
    puts("PASS: touch hotbar while walking, fourth hardware key, cancelled/stale touches, difficulty preview/cancel/commit and deferred new-world creation");
}
int main(void)
{
    controller();menus_and_saves();touch_menus();new_controls();host_present_hook=presented;
    assert(minimine_main()==0 && quit && stage==11 && bench_frames==540);
    puts("PASS: real loop, catch-up edges, paused time, held-action suppression, workbench, 540-frame benchmark and exit");return 0;
}
