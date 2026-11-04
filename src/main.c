#include "game.h"
#include "perf.h"
#include "render.h"
#include "storage.h"
#include "ui_layout.h"
#include <gint/config.h>
#include <gint/display.h>
#include <gint/keyboard.h>
#include <gint/prof.h>
#include <string.h>

HHK_NAME("Minimine")
HHK_DESCRIPTION("2D survival sandbox with crafting and three save slots")
HHK_AUTHOR("Minimine project")
HHK_VERSION(SANDBOX_VERSION)
_Static_assert((int)BENCH_SAMPLES<=(int)PERF_CAPACITY,"Benchmark history too small");

typedef enum {
    MODE_START,MODE_PLAY,MODE_PAUSE,MODE_HELP,MODE_INVENTORY,
    MODE_SAVE,MODE_LOAD,MODE_CONFIRM,MODE_MESSAGE,MODE_BENCH,MODE_RESULTS,MODE_DIFFICULTY
} Mode;
typedef enum { CONFIRM_SAVE,CONFIRM_LOAD } Confirmation;
typedef struct {
    Mode mode,message_return,confirm_return;
    bool has_game,baseline,resume,page_dirty,running,yes,difficulty_new;
    uint32_t saved_revision;
    unsigned cursor,phase,phase_frame;
    Confirmation confirm;
    const char *message;
    InventoryUI inventory;
    SaveInfo saves[3];
    bool touching;Mode touch_mode;int touch_target;
} App;
typedef struct {bool jump,use,zero,clear,inventory,mine_released;int hotbar;unsigned sneak;} Presses;

static Game play_game,bench_game;
static PerfHistory live_history,bench_history;
static PerfSummary results[BENCH_PHASES];
static SaveStore store={.root="/Minimine"};
static App app;

static uint32_t elapsed_us(uint32_t start,uint32_t end)
{prof_t context=prof_make();context.elapsed=start-end;return prof_time(context);}
static void page(Mode mode)
{app.mode=mode;app.page_dirty=true;app.cursor=0;app.touching=false;}
static void enter_play(void)
{app.mode=MODE_PLAY;app.resume=true;app.touching=false;render_invalidate();}
static bool dirty(void)
{return app.has_game && (!app.baseline || play_game.revision!=app.saved_revision);}
static void new_game(void)
{
    app.difficulty_new=true;page(MODE_DIFFICULTY);app.cursor=DIFFICULTY_MEDIUM;
}
static void start_benchmark(void)
{
    if(app.has_game) game_cancel_mining(&play_game);
    game_init_benchmark(&bench_game,WORLD_SEED);
    app.phase=app.phase_frame=0;memset(results,0,sizeof(results));
    perf_reset(&bench_history);app.mode=MODE_BENCH;app.resume=true;render_invalidate();
}
static void show_message(const char *message,Mode after)
{app.message=message;app.message_return=after;page(MODE_MESSAGE);}
static void slots_page(bool saving)
{
    page(saving?MODE_SAVE:MODE_LOAD);
    for(unsigned i=0;i<3;++i) save_slot_info(&store,i+1,&app.saves[i],&bench_game);
}
static void confirm(Confirmation action,Mode back)
{app.confirm=action;app.confirm_return=back;app.yes=false;app.mode=MODE_CONFIRM;app.page_dirty=true;}
static void save_current(void)
{
    render_message("Speichert ...");dupdate();
    SaveResult r=save_slot_write(&store,app.cursor+1,&play_game,&bench_game);
    if(r==SAVE_OK) {app.baseline=true;app.saved_revision=play_game.revision;}
    show_message(save_result_text(r),r==SAVE_OK?MODE_PAUSE:MODE_SAVE);
}
static void load_current(void)
{
    render_message("Laedt ...");dupdate();
    SaveInfo info;SaveResult r=save_slot_load(&store,app.cursor+1,&bench_game,&info);
    if(r!=SAVE_OK) {show_message(save_result_text(r),MODE_LOAD);return;}
    play_game=bench_game;app.has_game=true;app.baseline=!info.upgraded;
    app.saved_revision=play_game.revision;
    if(info.recovered) show_message("Sicherungsstand geladen",MODE_PLAY);
    else enter_play();
}
static UICommand menu_command(int key)
{
    switch(key) {
    case KEY_LEFT:return UI_LEFT;case KEY_RIGHT:return UI_RIGHT;
    case KEY_UP:return UI_UP;case KEY_DOWN:return UI_DOWN;
    case KEY_EXE:return UI_ACCEPT;case KEY_5:return UI_ONE;
    case KEY_CLEAR:case KEY_KBD:return UI_CLOSE;
    case KEY_SHIFT:return UI_TOGGLE_ARMOR;
    case KEY_X:return UI_HOTBAR_X;case KEY_Y:return UI_HOTBAR_Y;case KEY_Z:return UI_HOTBAR_Z;
    case KEY_POWER:return UI_HOTBAR_POWER;
    default:return UI_NONE;
    }
}
static bool inventory_key(Game *g,InventoryUI *ui,int key)
{return ui_inventory_key(g,ui,menu_command(key));}
static void handle_page(int key)
{
    if(app.mode==MODE_DIFFICULTY) {
        if(key==KEY_UP && app.cursor) {--app.cursor;app.page_dirty=true;}
        if(key==KEY_DOWN && app.cursor+1<DIFFICULTY_COUNT) {++app.cursor;app.page_dirty=true;}
        if(key==KEY_CLEAR) {page(app.difficulty_new?MODE_START:MODE_PAUSE);return;}
        if(key==KEY_EXE) {
            if(app.difficulty_new) {
                game_init(&play_game,WORLD_SEED);app.has_game=true;app.baseline=false;
            }
            game_set_difficulty(&play_game,(Difficulty)app.cursor);enter_play();
        }
        return;
    }
    if(app.mode==MODE_INVENTORY) {
        if(inventory_key(&play_game,&app.inventory,key)) enter_play();else app.page_dirty=true;
        return;
    }
    if(app.mode==MODE_MESSAGE) {
        if(key==KEY_EXE || key==KEY_CLEAR) {
            Mode after=app.message_return;
            if(after==MODE_PLAY) enter_play();
            else if(after==MODE_SAVE || after==MODE_LOAD) slots_page(after==MODE_SAVE);
            else page(after);
        }
        return;
    }
    if(app.mode==MODE_CONFIRM) {
        if(key==KEY_LEFT || key==KEY_RIGHT || key==KEY_UP || key==KEY_DOWN)
            {app.yes=!app.yes;app.page_dirty=true;}
        if(key==KEY_CLEAR || (key==KEY_EXE && !app.yes))
            {app.mode=app.confirm_return;app.page_dirty=true;return;}
        if(key==KEY_EXE && app.yes) {
            if(app.confirm==CONFIRM_SAVE) save_current();
            else if(app.confirm==CONFIRM_LOAD) load_current();
        }
        return;
    }
    if(app.mode==MODE_HELP) {
        if(key==KEY_0) start_benchmark();
        else if(key==KEY_EXE && app.has_game) enter_play();
        else if(key==KEY_EXE || key==KEY_CLEAR) page(app.has_game?MODE_PAUSE:MODE_START);
        return;
    }
    if(app.mode==MODE_RESULTS) {
        if(key==KEY_0) start_benchmark();
        else if(key==KEY_EXE) {if(app.has_game) enter_play();else new_game();}
        else if(key==KEY_CLEAR) page(app.has_game?MODE_PAUSE:MODE_START);
        return;
    }
    if(app.mode==MODE_SAVE || app.mode==MODE_LOAD) {
        if(key==KEY_UP && app.cursor) {--app.cursor;app.page_dirty=true;}
        if(key==KEY_DOWN && app.cursor<3) {++app.cursor;app.page_dirty=true;}
        if(key==KEY_CLEAR || (key==KEY_EXE && app.cursor==3))
            {page(app.has_game?MODE_PAUSE:MODE_START);return;}
        if(key==KEY_EXE) {
            if(app.mode==MODE_SAVE) {
                if(app.saves[app.cursor].status==SAVE_EMPTY) save_current();
                else confirm(CONFIRM_SAVE,MODE_SAVE);
            } else if(app.saves[app.cursor].status!=SAVE_OK)
                show_message(save_result_text(app.saves[app.cursor].status),MODE_LOAD);
            else if(dirty()) confirm(CONFIRM_LOAD,MODE_LOAD);else load_current();
        }
        return;
    }
    if(key==KEY_0) {start_benchmark();return;}
    if(key==KEY_UP && app.cursor) {--app.cursor;app.page_dirty=true;}
    if(key==KEY_DOWN && app.cursor<(app.mode==MODE_PAUSE?5u:4u)) {++app.cursor;app.page_dirty=true;}
    if(key==KEY_CLEAR) {if(app.mode==MODE_PAUSE) enter_play();return;}
    if(key!=KEY_EXE) return;
    if(app.mode==MODE_START) {
        if(app.cursor==0) new_game();
        else if(app.cursor==1) slots_page(false);
        else if(app.cursor==2) page(MODE_HELP);
        else if(app.cursor==3) start_benchmark();
        else app.running=false;
    } else {
        if(app.cursor==0) enter_play();
        else if(app.cursor==1) slots_page(true);
        else if(app.cursor==2) slots_page(false);
        else if(app.cursor==3) page(MODE_HELP);
        else if(app.cursor==4) {
            app.difficulty_new=false;page(MODE_DIFFICULTY);app.cursor=(unsigned)play_game.difficulty;
        }
        else app.running=false;
    }
}
static int page_hit(int x,int y)
{
    switch(app.mode) {
    case MODE_START:case MODE_PAUSE:case MODE_DIFFICULTY:
        for(unsigned i=0;i<(app.mode==MODE_DIFFICULTY?3u:app.mode==MODE_PAUSE?6u:5u);++i)
            if(ui_contains(ui_menu_rect(i),x,y)) return (int)i;
        break;
    case MODE_SAVE:case MODE_LOAD:
        for(unsigned i=0;i<4;++i) if(ui_contains(ui_save_rect(i),x,y)) return (int)i;
        break;
    case MODE_CONFIRM:
        if(ui_contains(ui_confirm_rect(false),x,y)) return 0;
        if(ui_contains(ui_confirm_rect(true),x,y)) return 1;
        break;
    case MODE_HELP:case MODE_RESULTS:
        if(ui_contains(ui_play_rect(),x,y)) return 0;
        if(ui_contains(ui_benchmark_rect(),x,y)) return 1;
        if(ui_contains(ui_back_rect(),x,y)) return 2;
        break;
    case MODE_MESSAGE:if(ui_contains(ui_message_rect(),x,y)) return 0;break;
    default:break;
    }
    return -1;
}
static void handle_menu_event(key_event_t e)
{
    if(app.mode==MODE_INVENTORY) {
        if(e.type==KEYEV_DOWN) {
            if(menu_command(e.key)==UI_NONE) return;
            if(inventory_key(&play_game,&app.inventory,e.key)) enter_play();else app.page_dirty=true;
        } else if(e.type==KEYEV_TOUCH_DOWN || e.type==KEYEV_TOUCH_DRAG || e.type==KEYEV_TOUCH_UP) {
            InventoryUI *ui=&app.inventory;
            unsigned cursor=ui->cursor;int source=ui->source;
            bool half=ui->source_half,drag=ui->dragging,armor=ui->armor;
            uint32_t revision=play_game.revision;Notice notice=play_game.notice;
            UITouch event=e.type==KEYEV_TOUCH_DOWN?UI_TOUCH_DOWN:
                e.type==KEYEV_TOUCH_DRAG?UI_TOUCH_DRAG:UI_TOUCH_UP;
            if(ui_inventory_touch(&play_game,ui,event,e.x,e.y,keydown(KEY_DIV))) enter_play();
            else if(cursor!=ui->cursor || source!=ui->source || half!=ui->source_half ||
                    drag!=ui->dragging || armor!=ui->armor || revision!=play_game.revision ||
                    notice!=play_game.notice) app.page_dirty=true;
        }
        return;
    }
    if(e.type==KEYEV_DOWN) {app.touching=false;handle_page(e.key);return;}
    if(e.type==KEYEV_TOUCH_DOWN) {
        app.touch_target=page_hit(e.x,e.y);app.touch_mode=app.mode;
        app.touching=app.touch_target>=0;return;
    }
    if(e.type!=KEYEV_TOUCH_UP) return;
    bool activate=app.touching && app.touch_mode==app.mode && app.touch_target==page_hit(e.x,e.y);
    app.touching=false;if(!activate) return;
    if(app.mode==MODE_START || app.mode==MODE_PAUSE || app.mode==MODE_SAVE || app.mode==MODE_LOAD || app.mode==MODE_DIFFICULTY) {
        app.cursor=(unsigned)app.touch_target;handle_page(KEY_EXE);
    } else if(app.mode==MODE_CONFIRM) {app.yes=app.touch_target==1;handle_page(KEY_EXE);}
    else if(app.mode==MODE_HELP || app.mode==MODE_RESULTS)
        handle_page(app.touch_target==0?KEY_EXE:app.touch_target==1?KEY_0:KEY_CLEAR);
    else handle_page(KEY_EXE);
}
static void draw_page(void)
{
    switch(app.mode) {
    case MODE_START:render_menu(false,app.cursor);break;
    case MODE_PAUSE:render_menu(true,app.cursor);render_progress(&play_game);break;
    case MODE_DIFFICULTY:render_difficulty((Difficulty)app.cursor,app.difficulty_new);break;
    case MODE_HELP:render_help();break;
    case MODE_INVENTORY:render_inventory(&play_game,&app.inventory);break;
    case MODE_SAVE:case MODE_LOAD:render_slots(app.mode==MODE_SAVE,app.saves,app.cursor);break;
    case MODE_CONFIRM:render_confirm(app.confirm==CONFIRM_SAVE?"Slot ueberschreiben?":
        "Aktuelles Spiel verwerfen?",app.yes);break;
    case MODE_MESSAGE:render_message(app.message);break;
    case MODE_RESULTS:render_results(results);break;
    default:break;
    }
    dupdate();app.page_dirty=false;
}
static Presses read_input(void)
{
    Presses p={.hotbar=-1};key_event_t e;
    while((e=pollevent()).type!=KEYEV_NONE) {
        if(e.type==KEYEV_TOUCH_DOWN) {
            app.touch_target=app.mode==MODE_PLAY?ui_hotbar_hit(e.x,e.y):-1;
            app.touching=app.touch_target>=0;app.touch_mode=app.mode;continue;
        }
        if(e.type==KEYEV_TOUCH_DRAG) {
            if(app.touch_target!=ui_hotbar_hit(e.x,e.y)) app.touching=false;
            continue;
        }
        if(e.type==KEYEV_TOUCH_UP) {
            if(app.touching && app.mode==MODE_PLAY && app.touch_mode==MODE_PLAY &&
               app.touch_target==ui_hotbar_hit(e.x,e.y)) p.hotbar=app.touch_target;
            app.touching=false;continue;
        }
        if(e.type==KEYEV_UP && e.key==KEY_DEL) p.mine_released=true;
        if(e.type!=KEYEV_DOWN) continue;
        switch(e.key) {
        case KEY_UP:case KEY_SHIFT:p.jump=true;break;
        case KEY_DOWN:++p.sneak;break;
        case KEY_EXE:p.use=true;break;
        case KEY_KBD:p.inventory=true;break;
        case KEY_CLEAR:p.clear=true;break;
        case KEY_0:p.zero=true;break;
        case KEY_X:p.hotbar=0;break;case KEY_Y:p.hotbar=1;break;case KEY_Z:p.hotbar=2;break;
        case KEY_POWER:p.hotbar=3;break;
        }
    }
    return p;
}
static GameInput held_input(void)
{
    return (GameInput){
        .move=!!keydown(KEY_RIGHT)-!!keydown(KEY_LEFT),
        .aim_x=!!(keydown(KEY_3)||keydown(KEY_6)||keydown(KEY_9))-
            !!(keydown(KEY_1)||keydown(KEY_4)||keydown(KEY_7)),
        .aim_y=!!(keydown(KEY_1)||keydown(KEY_2)||keydown(KEY_3))-
            !!(keydown(KEY_7)||keydown(KEY_8)||keydown(KEY_9)),
        .mine=keydown(KEY_DEL),.use_held=keydown(KEY_EXE)
    };
}
int main(void)
{
    if(prof_init()) {
        render_error("Kein freier Timer fuer die Messung.");dupdate();
        for(;;) {key_event_t e=getkey_opt(GETKEY_NONE,NULL);
            if(e.key==KEY_EXE || e.key==KEY_CLEAR) return 1;}
    }
    app=(App){.mode=MODE_START,.page_dirty=true,.running=true};
    uint32_t previous_start=*prof_tcnt,previous_end=previous_start,hud_elapsed=0,dropped=0;
    uint64_t accumulator=0;bool armed=false;GameInput pending={0};unsigned sneak=0;
    RenderInfo info={0};
    while(app.running) {
        if(app.mode!=MODE_PLAY && app.mode!=MODE_BENCH) {
            if(app.page_dirty) draw_page();
            handle_menu_event(waitevent(NULL));
            continue;
        }
        if(app.resume) {
            accumulator=0;hud_elapsed=0;pending=(GameInput){0};sneak=0;armed=false;
            previous_start=previous_end=*prof_tcnt;
            perf_reset(&live_history);info=(RenderInfo){0};app.resume=false;
        }
        uint32_t frame_start=*prof_tcnt,delta=elapsed_us(previous_start,frame_start);
        previous_start=frame_start;uint32_t logic_start=*prof_tcnt;
        Presses p=read_input();
        if(p.mine_released) game_cancel_mining(&play_game);
        if(p.clear) {
            if(app.has_game) game_cancel_mining(&play_game);
            page(app.has_game?MODE_PAUSE:MODE_START);continue;
        }
        if(app.mode==MODE_PLAY && p.inventory) {
            game_cancel_mining(&play_game);
            app.inventory=(InventoryUI){.cursor=play_game.selected,.source=-1,.craft_width=2};
            page(MODE_INVENTORY);continue;
        }
        if(app.mode==MODE_PLAY && p.zero) {start_benchmark();continue;}
        Game *g=app.mode==MODE_BENCH?&bench_game:&play_game;
        if(app.mode==MODE_PLAY) {
            GameInput input=held_input();
            if(!armed && !keydown(KEY_DEL) && !keydown(KEY_EXE)) armed=true;
            input.mine&=armed;input.use_held&=armed;
            pending.jump|=p.jump;if(armed) pending.use_pressed|=p.use;sneak+=p.sneak;
            if(p.hotbar>=0) game_select_hotbar(g,(unsigned)p.hotbar);
            accumulator+=(uint64_t)delta*SIM_HZ;unsigned steps=0;
            while(accumulator>=1000000 && steps<8) {
                input.jump=pending.jump;input.use_pressed=pending.use_pressed;
                while(sneak) {game_toggle_sneak(g);--sneak;}
                unsigned respawns=g->respawns;
                GameInteraction event=game_step(g,input);
                pending=(GameInput){0};input.jump=input.use_pressed=false;
                accumulator-=1000000;++steps;
                if(respawns!=g->respawns) {
                    armed=false;input.mine=input.use_held=false;pending=(GameInput){0};
                }
                if(event!=INTERACT_NONE) {
                    int x,y;game_target(g,&x,&y);
                    int furnace=event==INTERACT_FURNACE?game_furnace_at(g,x,y):-1;
                    int chest=event==INTERACT_CHEST?game_chest_at(g,x,y):-1;
                    app.inventory=(InventoryUI){.cursor=g->selected,.source=-1,
                        .craft_width=3,.furnace=furnace>=0?(unsigned)furnace+1:0,
                        .chest=chest>=0?(unsigned)chest+1:0};
                    page(MODE_INVENTORY);accumulator=0;break;
                }
            }
            if(app.mode==MODE_INVENTORY) continue;
            if(accumulator>=1000000) {dropped+=(uint32_t)(accumulator/1000000);accumulator%=1000000;}
        } else game_benchmark_step(g,app.phase,app.phase_frame);
        uint32_t draw_start=*prof_tcnt;
        info.benchmark=app.mode==MODE_BENCH;info.phase=app.phase;info.phase_frame=app.phase_frame;
        info.dropped_ticks=info.benchmark?0:dropped;
        render_game(g,&info);uint32_t lcd_start=*prof_tcnt;dupdate();
        uint32_t end=*prof_tcnt;
        PerfSample sample={.frame_us=elapsed_us(previous_end,end),.logic_us=elapsed_us(logic_start,draw_start),
            .draw_us=elapsed_us(draw_start,lcd_start),.lcd_us=elapsed_us(lcd_start,end)};
        previous_end=end;perf_push(&live_history,sample);hud_elapsed+=sample.frame_us;
        if(hud_elapsed>=500000) {info.perf=perf_summarize(live_history.samples,live_history.count);hud_elapsed=0;}
        if(app.mode==MODE_BENCH) {
            if(app.phase_frame>=BENCH_WARMUP) perf_push(&bench_history,sample);
            if(++app.phase_frame==BENCH_PHASE_FRAMES) {
                results[app.phase]=perf_summarize(bench_history.samples,bench_history.count);
                perf_reset(&bench_history);app.phase_frame=0;
                if(++app.phase==BENCH_PHASES) page(MODE_RESULTS);
            }
        }
    }
    prof_quit();return 0;
}
