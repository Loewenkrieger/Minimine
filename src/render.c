#include "render.h"

#include <gint/display.h>
#include <string.h>

_Static_assert(DWIDTH == SCREEN_W && DHEIGHT == SCREEN_H,
    "This renderer requires the ClassPad II display");

#define INK C_RGB(3, 5, 8)
#define PANEL C_RGB(3, 5, 9)
#define MUTED C_RGB(17, 21, 23)
#define ACCENT C_RGB(13, 29, 19)

static void outline(int x, int y, int width, int height, int color)
{
    drect(x, y, x + width - 1, y, color);
    drect(x, y + height - 1, x + width - 1, y + height - 1, color);
    drect(x, y, x, y + height - 1, color);
    drect(x + width - 1, y, x + width - 1, y + height - 1, color);
}

static uint16_t atlas[TILE_COUNT][3][TILE_SIZE*TILE_SIZE];
static uint16_t masks[TILE_COUNT][3][TILE_SIZE];
static int hill_top[SCREEN_W];
_Static_assert(sizeof(atlas)+sizeof(masks)+sizeof(hill_top) <= 20*1024,
    "Graphics cache exceeds 20 KiB");
static bool atlas_ready, hud_valid;
static RenderInfo old_info;
static ItemStack old_hotbar[HOTBAR_SLOTS];
static unsigned old_selected;
static bool old_sneak;

static void paint(Tile tile, int variant, int x1, int y1, int x2, int y2, int color)
{
    for(int y=y1; y<=y2; ++y) for(int x=x1; x<=x2; ++x) {
        atlas[tile][variant][y*TILE_SIZE+x]=(uint16_t)color;
        masks[tile][variant][y] |= (uint16_t)(1u<<x);
    }
}
static void make_tile(Tile tile, int variant)
{
    int x = 0, y = 0;
    switch(tile) {
    case TILE_GRASS:
    case TILE_DIRT:
        paint(tile, variant, x, y, x + 15, y + 15, C_RGB(15, 10, 6));
        paint(tile, variant, x + 2 + variant, y + 7, x + 5 + variant, y + 8, C_RGB(20, 14, 8));
        paint(tile, variant, x + 10, y + 12, x + 12, y + 13, C_RGB(10, 7, 5));
        if(tile == TILE_GRASS) {
            paint(tile, variant, x, y, x + 15, y + 2, C_RGB(10, 24, 9));
            paint(tile, variant, x + 2, y + 3, x + 4, y + 4, C_RGB(7, 18, 7));
        }
        break;
    case TILE_STONE:
    case TILE_ORE:
    case TILE_BEDROCK:
        paint(tile, variant, x, y, x + 15, y + 15, tile == TILE_BEDROCK ?
            C_RGB(5, 6, 9) : C_RGB(12, 14, 17));
        paint(tile, variant, x + 1, y + 1, x + 13, y + 1, C_RGB(17, 19, 22));
        paint(tile, variant, x + 3, y + 8, x + 6, y + 8, C_RGB(8, 10, 13));
        paint(tile, variant, x + 11, y + 12, x + 14, y + 13, C_RGB(8, 10, 13));
        if(tile == TILE_ORE) {
            paint(tile, variant, x + 3, y + 4, x + 6, y + 6, C_RGB(27, 19, 9));
            paint(tile, variant, x + 9, y + 9, x + 11, y + 12, C_RGB(24, 15, 7));
        }
        break;
    case TILE_WOOD:
        paint(tile, variant, x, y, x + 15, y + 15, C_RGB(20, 13, 7));
        paint(tile, variant, x, y + 7, x + 15, y + 8, C_RGB(12, 8, 4));
        paint(tile, variant, x + 1, y + 1, x + 14, y + 1, C_RGB(25, 18, 10));
        break;
    case TILE_LOG:
        paint(tile, variant, 0, 0, 15, 15, C_RGB(12, 8, 5));
        paint(tile, variant, 2, 0, 3, 15, C_RGB(21, 14, 7));
        paint(tile, variant, 8, 0, 9, 15, C_RGB(18, 11, 6));
        paint(tile, variant, 13, 0, 14, 15, C_RGB(8, 5, 3));
        break;
    case TILE_WORKBENCH:
        paint(tile, variant, 0, 0, 15, 15, C_RGB(16, 10, 5));
        paint(tile, variant, 1, 1, 14, 5, C_RGB(24, 17, 9));
        paint(tile, variant, 5, 1, 5, 5, C_RGB(10, 6, 3));
        paint(tile, variant, 10, 1, 10, 5, C_RGB(10, 6, 3));
        paint(tile, variant, 1, 3, 14, 3, C_RGB(10, 6, 3));
        paint(tile, variant, 3, 8, 12, 12, C_RGB(23, 15, 7));
        paint(tile, variant, 7, 8, 8, 15, C_RGB(9, 6, 4));
        break;
    case TILE_TRUNK:
        paint(tile, variant, x + 5, y, x + 10, y + 15, C_RGB(12, 8, 5));
        paint(tile, variant, x + 6, y, x + 7, y + 15, C_RGB(21, 14, 7));
        break;
    case TILE_LEAVES:
        paint(tile, variant, x + 1, y + 1, x + 14, y + 14, C_RGB(5, 15, 8));
        paint(tile, variant, x + 3, y + 2, x + 11, y + 8, C_RGB(7, 21, 10));
        paint(tile, variant, x + 5, y + 3, x + 9, y + 4, C_RGB(11, 25, 12));
        break;
    default: break;
    }
}

static void prepare_atlas(void)
{
    if(atlas_ready) return;
    for(int t=0;t<TILE_COUNT;++t) for(int v=0;v<3;++v) make_tile((Tile)t,v);
    atlas_ready=true;
}
void render_invalidate(void) { hud_valid=false; }

static void prepare_background(const Game *game)
{
    int horizon=30*TILE_SIZE-game->camera_y+VIEW_TOP;
    for(int x=0;x<SCREEN_W;++x) hill_top[x]=horizon+81;
    for(int i=-1;i<12;++i) {
        int left=i*40-(game->camera_x/5)%40;
        int h=20+((i+game->camera_x/200+128)*17%45);
        int right=left+40;
        if(left<0) left=0;
        if(right>=SCREEN_W) right=SCREEN_W-1;
        for(int x=left;x<=right;++x)
            if(horizon-h<hill_top[x]) hill_top[x]=horizon-h;
    }
}
static uint16_t background_pixel(const Game *game,int x,int y)
{
    if(y>=36*TILE_SIZE-game->camera_y+VIEW_TOP) return C_RGB(6,7,10);
    int horizon=30*TILE_SIZE-game->camera_y+VIEW_TOP;
    if(y>=hill_top[x] && y<=horizon+80) return C_RGB(11,19,20);
    return C_RGB(15,23,28);
}
static void background_span(const Game *game, uint16_t *dest, int left, int right, int y)
{
    int horizon = 30 * TILE_SIZE - game->camera_y + VIEW_TOP;
    uint16_t color = C_RGB(15,23,28);
    if(y >= 36 * TILE_SIZE - game->camera_y + VIEW_TOP) color = C_RGB(6,7,10);
    else if(y >= horizon - 64 && y <= horizon + 80) {
        for(int x = left; x < right; ++x)
            *dest++ = y >= hill_top[x] ? C_RGB(11,19,20) : color;
        return;
    }
    for(int x = left; x < right; ++x) *dest++ = color;
}
static void world_tile(const Game *game,Tile tile,int x,int y,int variant)
{
    int left=x<0?0:x, right=x+16>SCREEN_W?SCREEN_W:x+16;
    int top=y<VIEW_TOP?VIEW_TOP:y;
    int bottom=y+16>VIEW_TOP+VIEW_H?VIEW_TOP+VIEW_H:y+16;
    bool opaque=tile!=TILE_AIR && tile!=TILE_TRUNK && tile!=TILE_LEAVES;
    for(int py=top;py<bottom;++py) {
        uint16_t *dest=gint_vram+py*SCREEN_W+left;
        const uint16_t *src=atlas[tile][variant]+(py-y)*16;
        if(opaque) memcpy(dest,src+left-x,(size_t)(right-left)*sizeof(*dest));
        else if(tile == TILE_AIR) background_span(game, dest, left, right, py);
        else {
            uint16_t mask=masks[tile][variant][py-y];
            for(int px=left;px<right;++px)
                *dest++=mask&(1u<<(px-x))?src[px-x]:background_pixel(game,px,py);
        }
    }
}
static void icon(Tile tile,int x,int y)
{
    for(int py=0;py<16;++py) for(int px=0;px<16;++px)
        if(masks[tile][0][py]&(1u<<px))
            dpixel(x+px,y+py,atlas[tile][0][py*16+px]);
}
static void item_icon(Item item, int x, int y)
{
    Tile tile = game_item_tile(item);
    if(tile != TILE_AIR) { icon(tile, x, y); return; }
    if(item != ITEM_STICK && !game_tool_durability(item)) return;
    for(int i = 0; i < 11; ++i) {
        dpixel(x + 2 + i, y + 13 - i, C_RGB(22,15,8));
        dpixel(x + 3 + i, y + 13 - i, C_RGB(12,8,4));
    }
    if(game_tool_durability(item)) {
        int color = item == ITEM_WOOD_PICKAXE ? C_RGB(23,16,8) : C_RGB(21,23,25);
        drect(x + 3, y + 2, x + 12, y + 4, color);
        drect(x + 2, y + 3, x + 4, y + 7, color);
        drect(x + 11, y + 3, x + 13, y + 6, color);
    }
}
static void player_draw(const Game *game)
{
    const Player *p = &game->player;
    int x = p->x / FIXED_ONE - game->camera_x;
    int y = p->y / FIXED_ONE - game->camera_y + VIEW_TOP;
    int stride = p->vx && p->grounded ? (int)(game->ticks / 6 % 2) : 0;
    if(p->sneaking) {
        int height=game_player_height(p);
        drect(x+1,y,x+8,y+2,C_RGB(9,6,4));
        drect(x+2,y+2,x+8,y+5,C_RGB(29,22,15));
        dpixel(x+(p->facing>0?7:3),y+3,INK);
        drect(x+1,y+6,x+8,y+9,C_RGB(28,14,6));
        drect(x,y+7,x+1,y+10,C_RGB(29,22,15));
        drect(x+8,y+7,x+9,y+10,C_RGB(29,22,15));
        drect(x+1,y+10,x+4,y+height-1-stride,C_RGB(5,9,17));
        drect(x+6,y+10,x+8,y+height-2+stride,C_RGB(5,9,17));
        return;
    }
    drect(x + 1, y, x + 8, y + 4, C_RGB(9, 6, 4));
    drect(x + 2, y + 3, x + 8, y + 8, C_RGB(29, 22, 15));
    dpixel(x + (p->facing > 0 ? 7 : 3), y + 5, INK);
    drect(x + 1, y + 9, x + 8, y + 15, C_RGB(28, 14, 6));
    drect(x, y + 10, x + 1, y + 16, C_RGB(29, 22, 15));
    drect(x + 8, y + 10, x + 9, y + 16, C_RGB(29, 22, 15));
    drect(x + 1, y + 16, x + 4, y + 21 - stride, C_RGB(5, 9, 17));
    drect(x + 6, y + 16, x + 8, y + 20 + stride, C_RGB(5, 9, 17));
}
static void header(const RenderInfo *info)
{
    const PerfSummary *p = &info->perf;
    drect(0, 0, SCREEN_W - 1, VIEW_TOP - 1, PANEL);
    dtext(8, 5, ACCENT, info->benchmark ? "CASIO SANDBOX / BENCHMARK" :
        "CASIO SANDBOX / " SANDBOX_VERSION);
    dprint(8, 20, C_WHITE, "%u.%u FPS    Bild %u.%u ms",
        (unsigned)p->fps_x10 / 10, (unsigned)p->fps_x10 % 10,
        (unsigned)p->mean_us / 1000, (unsigned)p->mean_us / 100 % 10);
    dprint(8, 36, MUTED, "L %u.%u  Z %u.%u  LCD %u.%u ms",
        (unsigned)p->logic_us / 1000, (unsigned)p->logic_us / 100 % 10,
        (unsigned)p->draw_us / 1000, (unsigned)p->draw_us / 100 % 10,
        (unsigned)p->lcd_us / 1000, (unsigned)p->lcd_us / 100 % 10);
}
static void footer(const Game *game, const RenderInfo *info)
{
    int top = VIEW_TOP + VIEW_H;
    drect(0, top, SCREEN_W - 1, SCREEN_H - 1, PANEL);
    if(info->benchmark) {
        dprint(8, top + 6, C_WHITE, "%u/3  %s", info->phase + 1,
            game_phase_name(info->phase));
        dprint(8, top + 22, MUTED, "Bild %u/%u / %s", info->phase_frame + 1,
            (unsigned)BENCH_PHASE_FRAMES,
            info->phase_frame < BENCH_WARMUP ? "Aufwaermen" : "Messung");
        dtext(8, top + 43, ACCENT, "CLEAR: Test abbrechen");
        return;
    }
    for(unsigned i=0;i<HOTBAR_SLOTS;++i) {
        int x=7+(int)i*104;
        const ItemStack *stack=&game->slots[i];
        outline(x,top+3,100,47,game->selected==i?ACCENT:MUTED);
        bool tool=game_tool_durability(stack->item)!=0;
        const char *name=tool?(stack->item==ITEM_WOOD_PICKAXE?"Holz-":"Stein-"):
            game_item_name(stack->item);
        dtext(x+4,top+6,C_WHITE,name);
        if(tool) dtext(x+4,top+18,C_WHITE,"spitzhacke");
        item_icon(stack->item,x+4,top+29);
        dprint(x+25,top+32,C_WHITE,"%u",(unsigned)stack->count);
        if(tool) {
            unsigned width=90u*stack->durability/game_tool_durability(stack->item);
            drect(x+4,top+46,x+93,top+47,INK);
            if(width) drect(x+4,top+46,x+3+(int)width,top+47,ACCENT);
        }
    }
    dtext(8,top+56,MUTED,game->player.sneaking?
        "SNEAK  Keyboard: Inventar":"Keyboard: Inventar  CLEAR: Hilfe");
}

void render_world(const Game *game, const RenderInfo *info)
{
    prepare_atlas();
    prepare_background(game);
    struct dwindow previous = dwindow_set((struct dwindow){
        0, VIEW_TOP, SCREEN_W, VIEW_TOP + VIEW_H });
    int first_x = game->camera_x / TILE_SIZE;
    int first_y = game->camera_y / TILE_SIZE;
    int last_x = (game->camera_x + SCREEN_W - 1) / TILE_SIZE;
    int last_y = (game->camera_y + VIEW_H - 1) / TILE_SIZE;
    for(int y = first_y; y <= last_y; ++y)
        for(int x = first_x; x <= last_x; ++x)
            world_tile(game, game_tile(game, x, y), x * TILE_SIZE - game->camera_x,
                y * TILE_SIZE - game->camera_y + VIEW_TOP, (x + y) % 3);
    player_draw(game);
    if(!info->benchmark) {
        int tx, ty;
        game_target(game, &tx, &ty);
        outline(tx * TILE_SIZE - game->camera_x,
            ty * TILE_SIZE - game->camera_y + VIEW_TOP,
            TILE_SIZE, TILE_SIZE, C_WHITE);
    }
    if(!info->benchmark && game->mining.active) {
        int x=game->mining.x*16-game->camera_x;
        int y=game->mining.y*16-game->camera_y+VIEW_TOP-7;
        if(x<0) x=0;
        if(x>SCREEN_W-20) x=SCREEN_W-20;
        if(y<VIEW_TOP) y=VIEW_TOP;
        if(y>VIEW_TOP+VIEW_H-5) y=VIEW_TOP+VIEW_H-5;
        drect(x,y,x+19,y+4,PANEL);
        outline(x,y,20,5,C_WHITE);
        unsigned width=18*game->mining.elapsed/game->mining.required;
        if(width) drect(x+1,y+1,x+(int)width,y+3,ACCENT);
    }
    if(!info->benchmark && game->notice_ticks)
        dtext(8,VIEW_TOP+8,C_WHITE,game->notice==NOTICE_FULL?"Inventar voll":
            game->notice==NOTICE_TOOL_BROKEN?"Spitzhacke zerbrochen":"Kein Platz");
    if(!info->benchmark && info->dropped_ticks)
        dprint(8,VIEW_TOP+26,C_RED,"Simulationsverlust: %u Ticks",(unsigned)info->dropped_ticks);
    dwindow_set(previous);
}

void render_game(const Game *game,const RenderInfo *info)
{
    render_world(game,info);
    if(!hud_valid || info->benchmark!=old_info.benchmark ||
       memcmp(&info->perf,&old_info.perf,sizeof(info->perf))) header(info);
    if(!hud_valid || info->benchmark!=old_info.benchmark ||
       (info->benchmark && (info->phase!=old_info.phase || info->phase_frame!=old_info.phase_frame)) ||
       memcmp(game->slots,old_hotbar,sizeof(old_hotbar)) ||
       game->selected!=old_selected || game->player.sneaking!=old_sneak) footer(game,info);
    old_info=*info;
    memcpy(old_hotbar,game->slots,sizeof(old_hotbar));
    old_selected=game->selected;
    old_sneak=game->player.sneaking;
    hud_valid=true;
}

static void page_title(const char *title, const char *subtitle)
{
    render_invalidate();
    dclear(PANEL);
    dtext(16, 20, ACCENT, title);
    dtext(16, 39, MUTED, subtitle);
    drect(16, 58, SCREEN_W - 17, 59, C_RGB(9, 13, 16));
}

static void inventory_slot(const ItemStack *stack, unsigned index,
    const InventoryUI *ui, unsigned selected, int x, int y, int w, int h)
{
    int border=index==ui->cursor?C_WHITE:MUTED;
    outline(x,y,w,h,border);
    if(index==selected) outline(x+1,y+1,w-2,h-2,ACCENT);
    if((int)index==ui->source) outline(x+3,y+3,w-6,h-6,ACCENT);
    item_icon(stack->item,x+5,y+6);
    if(stack->count) dprint(x+25,y+9,C_WHITE,"%u",(unsigned)stack->count);
    if(game_tool_durability(stack->item)) {
        unsigned length=(unsigned)(w-10)*stack->durability/game_tool_durability(stack->item);
        if(length) drect(x+5,y+h-4,x+4+(int)length,y+h-3,ACCENT);
    }
}

void render_inventory(const Game *game,const InventoryUI *ui)
{
    prepare_atlas();
    unsigned width=ui->craft_width==3?3:2;
    page_title(width==3?"WERKBANK / PAUSE":"INVENTAR / PAUSE",
        "Pfeile: Auswahl   EXE: Quelle/Ziel");
    dtext(16,70,ACCENT,"Hotbar");
    dtext(16,122,ACCENT,"Inventar");
    for(unsigned i=0;i<TOTAL_SLOTS;++i) {
        int row=(int)i/3,col=(int)i%3;
        int x=16+col*98,y=row==0?85:140+(row-1)*34;
        inventory_slot(&game->slots[i],i,ui,game->selected,x,y,92,30);
    }
    dprint(16,287,ACCENT,"Crafting %ux%u",width,width);
    dtext(208,287,ACCENT,"Ergebnis");
    for(unsigned i=0;i<width*width;++i) {
        int x=16+(int)(i%width)*49,y=309+(int)(i/width)*36;
        inventory_slot(&game->crafting[i],CRAFT_FIRST+i,ui,CRAFT_RESULT+1,x,y,45,32);
    }
    int output_y=width==3?345:327;
    dtext(177,output_y+12,ACCENT,"=");
    ItemStack result=game_craft_preview(game,width);
    inventory_slot(&result,CRAFT_RESULT,ui,CRAFT_RESULT+1,218,output_y,70,36);
    const ItemStack *selected=ui->cursor==CRAFT_RESULT?&result:game_stack_at(game,ui->cursor);
    if(selected) {
        dprint(16,430,C_WHITE,"%s / %u",game_item_name(selected->item),(unsigned)selected->count);
        if(game_tool_durability(selected->item))
            dprint(16,449,MUTED,"Haltbarkeit: %u/%u",(unsigned)selected->durability,
                game_tool_durability(selected->item));
    }
    if(game->notice==NOTICE_FULL) dtext(16,449,C_WHITE,"Inventar voll: Platz schaffen");
    dtext(16,470,MUTED,ui->source<0?"EXE: Quelle / Ergebnis herstellen":"EXE: Stapel   5: ein Item");
    dtext(16,499,ACCENT,"5: ein Item  Keyboard/CLEAR: Zu");
}

void render_help(void)
{
    page_title("CASIO SANDBOX","ClassPad II / Hollyhock 3 / " SANDBOX_VERSION);
    static const char *const lines[]={
        "Links / Rechts   Laufen + Zielen",
        "Hoch / SHIFT     Springen",
        "Runter           Sneak umschalten",
        "1-9 ohne 5       Zielrichtung",
        "X / Y / Z        Hotbar 1 / 2 / 3",
        "Keyboard         Inventar / Pause",
        "DEL halten       Block abbauen",
        "EXE              Bauen / Werkbank",
        "0                Benchmark",
        "CLEAR            Pause / Hilfe"
    };
    for(unsigned i=0;i<sizeof(lines)/sizeof(lines[0]);++i)
        dtext(16,80+(int)i*24,MUTED,lines[i]);
    dtext(16,335,C_WHITE,"Crafting: EXE Stapel, 5 einzelnes Item");
    dtext(16,359,MUTED,"L = Logik, Z = Zeichnen, LCD = Transfer");
    dtext(16,383,MUTED,"Physik 60 Hz. Benchmark: 3 Szenen.");
    dtext(16,407,MUTED,"Testwelt getrennt vom Spiel.");
    dtext(16,431,MUTED,"Noch keine Speicherfunktion.");
    dtext(16,477,ACCENT,"EXE: Spielen / Fortsetzen");
    dtext(16,499,MUTED,"0: Benchmark   CLEAR: Launcher");
}

void render_results(const PerfSummary results[BENCH_PHASES])
{
    page_title("BENCHMARK / ERGEBNIS", "Fester Seed / 150 Messbilder pro Szene");
    for(unsigned i = 0; i < BENCH_PHASES; ++i) {
        const PerfSummary *p = &results[i];
        int y = 78 + (int)i * 110;
        dtext(16, y, ACCENT, game_phase_name(i));
        dprint(16, y + 20, C_WHITE, "%u.%u FPS   Mittel %u.%u ms",
            (unsigned)p->fps_x10 / 10, (unsigned)p->fps_x10 % 10,
            (unsigned)p->mean_us / 1000, (unsigned)p->mean_us / 100 % 10);
        dprint(16, y + 40, MUTED, "P95 %u.%u ms   Max %u.%u ms",
            (unsigned)p->p95_us / 1000, (unsigned)p->p95_us / 100 % 10,
            (unsigned)p->worst_us / 1000, (unsigned)p->worst_us / 100 % 10);
        dprint(16, y + 60, MUTED, "L %u  Z %u  LCD %u us",
            (unsigned)p->logic_us, (unsigned)p->draw_us, (unsigned)p->lcd_us);
    }
    dtext(16, 419, MUTED, "P95: 95% der Bilder sind so schnell");
    dtext(16, 437, MUTED, "oder schneller. Werte bitte ablesen.");
    dtext(16, 477, ACCENT, "EXE: Spielen   0: Test wiederholen");
    dtext(16, 499, MUTED, "CLEAR: Hilfe");
}

void render_error(const char *message)
{
    page_title("START NICHT MOEGLICH", "Casio Sandbox");
    dtext(16, 100, C_WHITE, message);
    dtext(16, 140, MUTED, "EXE / CLEAR: Zurueck zum Launcher");
}

