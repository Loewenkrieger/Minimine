#include "render.h"
#include "ui_layout.h"

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
static void button(UIRect r,const char *text,int color)
{outline(r.x,r.y,r.w,r.h,color);dtext(r.x+8,r.y+11,color,text);}
static void play_buttons(void)
{
    button(ui_play_rect(),"EXE: Spielen / Fortsetzen",ACCENT);
    button(ui_benchmark_rect(),"0: Benchmark",MUTED);
    button(ui_back_rect(),"CLEAR: Menue",MUTED);
}

enum { TILE_GRAPHICS=TILE_COUNT+4 };
static uint16_t atlas[TILE_GRAPHICS][TILE_SIZE*TILE_SIZE];
static uint16_t masks[TILE_GRAPHICS][TILE_SIZE];
static uint16_t mob_pixels[MOB_COUNT][2][16*24];
static uint16_t mob_masks[MOB_COUNT][2][24];
static int hill_top[SCREEN_W];
_Static_assert(sizeof(atlas)+sizeof(masks)+sizeof(hill_top)+sizeof(mob_pixels)+sizeof(mob_masks) <= 20*1024,
    "Graphics cache exceeds 20 KiB");
static bool atlas_ready, hud_valid;
static RenderInfo old_info;
static ItemStack old_hotbar[HOTBAR_SLOTS];
static unsigned old_selected;
static bool old_sneak,old_survival;
static unsigned old_health,old_hunger;
static unsigned graphic(Tile tile,int variant)
{
    if(variant && (tile==TILE_GRASS || tile==TILE_DIRT))
        return (unsigned)TILE_COUNT+(tile==TILE_GRASS?0u:2u)+(unsigned)variant-1;
    return (unsigned)tile;
}

static void paint(Tile tile, int variant, int x1, int y1, int x2, int y2, int color)
{
    for(int y=y1; y<=y2; ++y) for(int x=x1; x<=x2; ++x) {
        atlas[graphic(tile,variant)][y*TILE_SIZE+x]=(uint16_t)color;
        masks[graphic(tile,variant)][y] |= (uint16_t)(1u<<x);
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
    case TILE_COAL:
        paint(tile,variant,0,0,15,15,C_RGB(12,14,17));
        paint(tile,variant,3,4,6,6,C_RGB(2,3,4));
        paint(tile,variant,10,9,13,12,C_RGB(2,3,4));
        paint(tile,variant,2,1,13,1,C_RGB(17,19,22));
        break;
    case TILE_FURNACE:
        paint(tile,variant,0,0,15,15,C_RGB(10,12,14));
        paint(tile,variant,1,1,14,2,C_RGB(21,22,23));
        paint(tile,variant,3,5,12,7,C_RGB(3,4,5));
        paint(tile,variant,3,10,12,14,C_RGB(2,3,4));
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

static void prepare_mobs(void);
static void prepare_atlas(void)
{
    if(atlas_ready) return;
    for(int t=0;t<TILE_COUNT;++t) make_tile((Tile)t,0);
    for(int v=1;v<3;++v) {make_tile(TILE_DIRT,v);make_tile(TILE_GRASS,v);}
    prepare_mobs();atlas_ready=true;
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
        const uint16_t *src=atlas[graphic(tile,variant)]+(py-y)*16;
        if(opaque) memcpy(dest,src+left-x,(size_t)(right-left)*sizeof(*dest));
        else if(tile == TILE_AIR) background_span(game, dest, left, right, py);
        else {
            uint16_t mask=masks[graphic(tile,variant)][py-y];
            for(int px=left;px<right;++px)
                *dest++=mask&(1u<<(px-x))?src[px-x]:background_pixel(game,px,py);
        }
    }
}
static void icon(Tile tile,int x,int y)
{
    for(int py=0;py<16;++py) for(int px=0;px<16;++px)
        if(masks[graphic(tile,0)][py]&(1u<<px))
            dpixel(x+px,y+py,atlas[graphic(tile,0)][py*16+px]);
}
static void item_icon(Item item,int x,int y)
{
    Tile tile=game_item_tile(item);
    if(tile!=TILE_AIR) {icon(tile,x,y);return;}
    ItemKind kind=game_item_kind(item);
    if(kind==KIND_FOOD) {
        int c=item==ITEM_RAW_PORK || item==ITEM_RAW_BEEF || item==ITEM_RAW_CHICKEN?
            C_RGB(28,12,12):C_RGB(18,9,4);
        drect(x+3,y+3,x+12,y+11,c);drect(x+1,y+10,x+5,y+13,C_RGB(28,27,21));return;
    }
    if(kind==KIND_ARMOR) {
        int c=C_RGB(23,25,27);unsigned slot=game_armor_slot(item);
        if(slot==0) {drect(x+2,y+3,x+13,y+8,c);drect(x+2,y+8,x+4,y+12,c);drect(x+11,y+8,x+13,y+12,c);}
        else if(slot==1) {drect(x+2,y+2,x+13,y+6,c);drect(x+4,y+6,x+11,y+14,c);}
        else {drect(x+3,y+3,x+12,y+5,c);drect(x+3,y+5,x+6,y+14,c);drect(x+9,y+5,x+12,y+14,c);}
        return;
    }
    if(item==ITEM_COAL) {drect(x+3,y+3,x+12,y+12,INK);drect(x+4,y+4,x+8,y+5,MUTED);return;}
    if(item==ITEM_RAW_IRON || item==ITEM_IRON_INGOT) {
        drect(x+2,y+5,x+13,y+11,item==ITEM_RAW_IRON?C_RGB(22,15,10):C_RGB(25,26,27));return;
    }
    if(item!=ITEM_STICK && kind!=KIND_PICKAXE && kind!=KIND_SWORD) return;
    for(int i=0;i<11;++i) {
        int c=kind==KIND_SWORD?(item==ITEM_WOOD_SWORD?C_RGB(23,16,8):C_RGB(24,25,27)):C_RGB(22,15,8);
        dpixel(x+2+i,y+13-i,c);dpixel(x+3+i,y+13-i,c);
    }
    if(kind==KIND_PICKAXE) {
        int c=item==ITEM_WOOD_PICKAXE?C_RGB(23,16,8):C_RGB(21,23,25);
        drect(x+3,y+2,x+12,y+4,c);drect(x+2,y+3,x+4,y+7,c);drect(x+11,y+3,x+13,y+6,c);
    } else if(kind==KIND_SWORD) drect(x+1,y+9,x+7,y+11,C_RGB(17,12,6));
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
static void status_symbol(int x,int y,unsigned amount,bool heart)
{
    static const uint8_t rows[]={54,127,127,62,28,8,0};
    for(int py=0;py<7;++py) for(int px=0;px<7;++px) {
        bool shape=heart?(rows[py]&(1u<<px))!=0:
            (py<5?(px>0 && px<6):(px>=4 && px<=5));
        if(shape) dpixel(x+px,y+py,(unsigned)(6-py)*200/7<amount?
            (heart?C_RGB(29,4,6):C_RGB(24,13,5)):C_RGB(8,9,11));
    }
}
static void header(const Game *g,const RenderInfo *info)
{
    const PerfSummary *p=&info->perf;
    drect(0,0,SCREEN_W-1,VIEW_TOP-1,PANEL);
    if(info->benchmark) {
        dtext(8,5,ACCENT,"CASIO SANDBOX / BENCHMARK");
        dprint(8,20,C_WHITE,"%u.%u FPS    Bild %u.%u ms",(unsigned)p->fps_x10/10,
            (unsigned)p->fps_x10%10,(unsigned)p->mean_us/1000,(unsigned)p->mean_us/100%10);
    } else {
        dtext(8,5,ACCENT,"MINIMINE " SANDBOX_VERSION);
        dprint(204,5,C_WHITE,"%u.%u FPS",(unsigned)p->fps_x10/10,(unsigned)p->fps_x10%10);
        for(unsigned i=0;i<10;++i) {
            unsigned health=g->health>i*200?g->health-i*200:0;
            unsigned hunger=(unsigned)g->hunger*100>i*200?(unsigned)g->hunger*100-i*200:0;
            status_symbol(8+(int)i*10,39,health,true);
            status_symbol(196+(int)i*10,39,hunger,false);
        }
    }
    int y=info->benchmark?36:21;
    dprint(8,y,MUTED,"L %u.%u  Z %u.%u  LCD %u.%u ms",
        (unsigned)p->logic_us/1000,(unsigned)p->logic_us/100%10,
        (unsigned)p->draw_us/1000,(unsigned)p->draw_us/100%10,
        (unsigned)p->lcd_us/1000,(unsigned)p->lcd_us/100%10);
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
        dtext(x+4,top+6,C_WHITE,game_item_label(stack->item,0));
        dtext(x+4,top+18,C_WHITE,game_item_label(stack->item,1));
        item_icon(stack->item,x+4,top+29);
        dprint(x+25,top+32,C_WHITE,"%u",(unsigned)stack->count);
        if(tool) {
            unsigned width=90u*stack->durability/game_tool_durability(stack->item);
            drect(x+4,top+46,x+93,top+47,INK);
            if(width) drect(x+4,top+46,x+3+(int)width,top+47,ACCENT);
        }
    }
    dtext(8,top+56,MUTED,game->player.sneaking?
        "SNEAK  Keyboard: Inventar":"Keyboard: Inventar  CLEAR: Menue");
}

static void mob_paint(unsigned kind,unsigned frame,int x,int y,int w,int h,int color)
{
    for(int py=y;py<y+h;++py) for(int px=x;px<x+w;++px) {
        mob_pixels[kind][frame][py*16+px]=(uint16_t)color;
        mob_masks[kind][frame][py]|=(uint16_t)(1u<<px);
    }
}
static void prepare_mobs(void)
{
    for(unsigned kind=0;kind<MOB_COUNT;++kind) for(unsigned frame=0;frame<2;++frame) {
        int stride=(int)frame;
        if(kind==MOB_ZOMBIE) {
            mob_paint(kind,frame,1,0,8,7,C_RGB(10,20,8));
            mob_paint(kind,frame,7,3,1,1,INK);
            mob_paint(kind,frame,0,8,10,8,C_RGB(6,16,17));
            mob_paint(kind,frame,1,16,3,6-stride,C_RGB(8,7,15));
            mob_paint(kind,frame,6,16,3,5+stride,C_RGB(8,7,15));
        } else if(kind==MOB_COW) {
            mob_paint(kind,frame,0,2,13,8,C_RGB(17,11,6));
            mob_paint(kind,frame,10,0,6,8,C_RGB(25,24,20));
            mob_paint(kind,frame,3,3,4,3,C_RGB(27,27,24));
            mob_paint(kind,frame,13,2,1,1,INK);
            mob_paint(kind,frame,2,10,3,4-stride,C_RGB(10,7,5));
            mob_paint(kind,frame,10,10,3,3+stride,C_RGB(10,7,5));
        } else if(kind==MOB_PIG) {
            mob_paint(kind,frame,0,2,11,7,C_RGB(29,17,17));
            mob_paint(kind,frame,8,0,6,7,C_RGB(31,20,20));
            mob_paint(kind,frame,11,2,1,1,INK);
            mob_paint(kind,frame,2,9,3,3-stride,C_RGB(21,11,12));
            mob_paint(kind,frame,9,9,3,2+stride,C_RGB(21,11,12));
        } else {
            mob_paint(kind,frame,1,3,7,5,C_RGB(29,29,27));
            mob_paint(kind,frame,6,0,4,5,C_RGB(31,31,29));
            mob_paint(kind,frame,8,1,1,1,INK);
            mob_paint(kind,frame,9,3,1,2,C_RGB(29,19,3));
            mob_paint(kind,frame,2,8,1,2-stride,C_RGB(25,16,2));
            mob_paint(kind,frame,6,8,1,1+stride,C_RGB(25,16,2));
        }
    }
}
static void draw_mob(const Game *g,const Mob *m)
{
    int w=(int)game_mob_width(m->kind),h=(int)game_mob_height(m->kind);
    int x=m->body.x/FIXED_ONE-g->camera_x,y=m->body.y/FIXED_ONE-g->camera_y+VIEW_TOP;
    if(x+w<=0 || x>=SCREEN_W || y+h<=VIEW_TOP || y>=VIEW_TOP+VIEW_H) return;
    unsigned frame=m->body.vx?(g->ticks/8)%2:0;
    for(int py=0;py<h;++py) for(int px=0;px<w;++px) {
        int source=m->body.facing>0?px:w-1-px;
        if(mob_masks[m->kind][frame][py]&(1u<<source))
            dpixel(x+px,y+py,mob_pixels[m->kind][frame][py*16+source]);
    }
    if(m->health<game_mob_health(m->kind)) {
        int barw=w<14?14:w,bx=x,by=y-5;
        if(bx<0) bx=0;
        if(bx>SCREEN_W-barw) bx=SCREEN_W-barw;
        if(by<VIEW_TOP) by=VIEW_TOP;
        if(by>VIEW_TOP+VIEW_H-4) by=VIEW_TOP+VIEW_H-4;
        drect(bx,by,bx+barw-1,by+3,PANEL);outline(bx,by,barw,4,C_WHITE);
        unsigned fill=(unsigned)(barw-2)*m->health/game_mob_health(m->kind);
        if(fill) drect(bx+1,by+1,bx+(int)fill,by+2,C_RED);
    }
}
static void player_equipment(const Game *g)
{
    int x=g->player.x/FIXED_ONE-g->camera_x;
    int y=g->player.y/FIXED_ONE-g->camera_y+VIEW_TOP,h=game_player_height(&g->player);
    int waist=g->player.sneaking?6:9,gray=C_RGB(24,26,28);
    if(g->armor[0].count) drect(x+1,y,x+8,y+2,gray);
    if(g->armor[1].count) drect(x+1,y+waist,x+8,y+h-7,gray);
    if(g->armor[2].count) {drect(x+1,y+h-6,x+4,y+h-3,gray);drect(x+6,y+h-6,x+8,y+h-3,gray);}
    if(g->armor[3].count) {drect(x+1,y+h-2,x+4,y+h-1,gray);drect(x+6,y+h-2,x+8,y+h-1,gray);}
    if(g->swing_ticks) {
        int sx=g->player.facing>0?x+PLAYER_W+2:x-13;
        drect(sx,y+4,sx+8,y+5,C_WHITE);
        drect(sx+7,y+5,sx+9,y+h-5,C_WHITE);
    }
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
    if(!info->benchmark && game->survival) {
        for(unsigned i=0;i<FURNACE_LIMIT;++i) {
            const Furnace *f=&game->furnaces[i];
            if(f->active && f->burn_ticks) {
                int fx=f->x*16-game->camera_x,fy=f->y*16-game->camera_y+VIEW_TOP;
                drect(fx+5,fy+11,fx+10,fy+13,C_RGB(31,13,2));
                dpixel(fx+7,fy+10,C_RGB(31,27,5));
            }
        }
        for(unsigned i=0;i<DROP_LIMIT;++i) if(game->drops[i].active)
            item_icon(game->drops[i].stack.item,game->drops[i].x/FIXED_ONE-game->camera_x-4,
                game->drops[i].y/FIXED_ONE-game->camera_y+VIEW_TOP-8);
        for(unsigned i=0;i<MOB_LIMIT;++i) if(game->mobs[i].active) draw_mob(game,&game->mobs[i]);
    }
    player_draw(game);
    if(!info->benchmark && game->survival) player_equipment(game);
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
        dtext(8,VIEW_TOP+8,C_WHITE,game_notice_text(game->notice));
    if(!info->benchmark && info->dropped_ticks)
        dprint(8,VIEW_TOP+26,C_RED,"Simulationsverlust: %u Ticks",(unsigned)info->dropped_ticks);
    dwindow_set(previous);
}

void render_game(const Game *game,const RenderInfo *info)
{
    render_world(game,info);
    if(!hud_valid || info->benchmark!=old_info.benchmark ||
       memcmp(&info->perf,&old_info.perf,sizeof(info->perf)) || old_health!=game->health ||
       old_hunger!=game->hunger || old_survival!=game->survival) header(game,info);
    if(!hud_valid || info->benchmark!=old_info.benchmark ||
       (info->benchmark && (info->phase!=old_info.phase || info->phase_frame!=old_info.phase_frame)) ||
       memcmp(game->slots,old_hotbar,sizeof(old_hotbar)) ||
       game->selected!=old_selected || game->player.sneaking!=old_sneak) footer(game,info);
    old_health=game->health;old_hunger=game->hunger;old_survival=game->survival;
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
    const char *title=ui->armor?"RUESTUNG / PAUSE":ui->furnace?"OFEN / PAUSE":
        width==3?"WERKBANK / PAUSE":"INVENTAR / PAUSE";
    page_title(title,"Touch / Pfeile   EXE: Quelle/Ziel");
    dtext(16,70,ACCENT,"Hotbar");dtext(16,122,ACCENT,"Inventar");
    if(ui->armor) {
        dtext(16,287,ACCENT,"Eisenruestung");
        dtext(16,299,MUTED,"Helm");dtext(161,299,MUTED,"Brustpanzer");
        dtext(16,354,MUTED,"Hose");dtext(161,354,MUTED,"Stiefel");
    } else if(ui->furnace) {
        dtext(16,287,ACCENT,"Ofen");dtext(16,298,MUTED,"Eingang");
        dtext(16,360,MUTED,"Brennstoff");dtext(218,324,MUTED,"Ausgang");
        if(ui->furnace<=FURNACE_LIMIT) {
            const Furnace *f=&game->furnaces[ui->furnace-1];
            dtext(110,347,MUTED,"Schmelzen");outline(110,362,92,6,MUTED);
            unsigned n=90*f->progress/600;
            if(n) drect(111,363,110+(int)n,366,ACCENT);
            dtext(110,382,MUTED,"Feuer");outline(110,397,92,6,MUTED);
            n=f->burn_total?90*f->burn_ticks/f->burn_total:0;
            if(n) drect(111,398,110+(int)n,401,C_RGB(31,15,3));
        }
    } else {dprint(16,287,ACCENT,"Crafting %ux%u",width,width);dtext(208,287,ACCENT,"Ergebnis");}
    ItemStack result=game_craft_preview(game,width);
    UISlot list[UI_MAX_SLOTS];unsigned count=ui_slots(ui,list);
    const ItemStack *selected=0;
    for(unsigned i=0;i<count;++i) {
        const UISlot *p=&list[i];
        const ItemStack *at=p->result?&result:game_ref_stack(game,p->ref);
        if(!at) continue;
        inventory_slot(at,p->id,ui,game->selected,p->x,p->y,p->w,p->h);
        if(p->id==ui->cursor) selected=at;
    }
    if(selected) {
        dprint(16,430,C_WHITE,"%s / %u",game_item_name(selected->item),(unsigned)selected->count);
        if(game_tool_durability(selected->item))
            dprint(16,449,MUTED,"Haltbarkeit: %u/%u",(unsigned)selected->durability,
                game_tool_durability(selected->item));
    }
    if(game->notice==NOTICE_FULL || game->notice==NOTICE_WRONG_SLOT)
        dtext(16,449,C_WHITE,game_notice_text(game->notice));
    dtext(16,467,MUTED,ui->dragging?"Loslassen: verschieben  DIV: halb":
        ui->source_half?"Halber Stapel ausgewaehlt":"Touch: Ziehen / Tippen  DIV: halb");
    button(ui_armor_rect(),ui->armor?"SHIFT: Zurueck":"SHIFT: Ruestung",ACCENT);
    button(ui_close_rect(),"Schliessen",ACCENT);
}

void render_help(void)
{
    page_title("MINIMINE","ClassPad II / Hollyhock 3 / " SANDBOX_VERSION);
    static const char *const lines[]={
        "Links / Rechts   Laufen + Zielen",
        "Hoch / SHIFT     Springen",
        "Runter           Sneak umschalten",
        "1-9 ohne 5       Zielrichtung",
        "X / Y / Z        Hotbar 1 / 2 / 3",
        "Keyboard         Inventar / Pause",
        "DEL halten       Block abbauen",
        "EXE halten       Kampf/Essen/Bauen",
        "0                Benchmark",
        "CLEAR            Menue / Speichern"
    };
    for(unsigned i=0;i<sizeof(lines)/sizeof(lines[0]);++i)
        dtext(16,80+(int)i*24,MUTED,lines[i]);
    dtext(16,335,C_WHITE,"Inventar: X/Y/Z holen den Hotbarstapel");
    dtext(16,359,MUTED,"Touch: Ziehen / Quelle, dann Ziel");
    dtext(16,383,MUTED,"DIV + Touch: halben Stapel bewegen");
    dtext(16,407,MUTED,"SHIFT im Inventar: Ruestung");
    dtext(16,431,MUTED,"Speichern/Laden: CLEAR-Menue");
    play_buttons();
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
    play_buttons();
}

void render_error(const char *message)
{
    page_title("START NICHT MOEGLICH", "Minimine");
    dtext(16, 100, C_WHITE, message);
    dtext(16, 140, MUTED, "EXE / CLEAR: Zurueck zum Launcher");
}


void render_menu(bool pause,unsigned cursor)
{
    page_title("MINIMINE " SANDBOX_VERSION,pause?"PAUSE / SPIELZEIT ANGEHALTEN":"ClassPad II / Hollyhock 3");
    static const char *const start[]={"Neues Spiel","Spielstand laden","Hilfe","Benchmark","Beenden"};
    static const char *const paused[]={"Fortsetzen","Spielstand speichern","Spielstand laden","Hilfe","Beenden (ohne Speichern)"};
    const char *const *labels=pause?paused:start;
    for(unsigned i=0;i<5;++i) {
        UIRect r=ui_menu_rect(i);
        button(r,labels[i],cursor==i?ACCENT:MUTED);
    }
    dtext(16,435,MUTED,"Antippen oder Pfeile / EXE");
    dtext(16,464,MUTED,pause?"CLEAR: Fortsetzen":"3 manuelle Spielstaende");
    dtext(16,493,MUTED,"0: Benchmark");
}
void render_slots(bool saving,const SaveInfo slots[3],unsigned cursor)
{
    page_title(saving?"SPIELSTAND SPEICHERN":"SPIELSTAND LADEN","Drei voneinander getrennte Slots");
    for(unsigned i=0;i<3;++i) {
        UIRect r=ui_save_rect(i);int y=r.y;
        outline(r.x,r.y,r.w,r.h,cursor==i?ACCENT:MUTED);
        dprint(26,y+9,C_WHITE,"Slot %u",i+1);
        const SaveInfo *s=&slots[i];
        if(s->status==SAVE_OK) {
            unsigned seconds=s->ticks/SIM_HZ;
            dprint(26,y+31,MUTED,"Spielzeit %u:%02u min",seconds/60,seconds%60);
            dprint(26,y+52,MUTED,"Leben %u.%u  Hunger %u",(unsigned)s->health/100,
                (unsigned)s->health/10%10,(unsigned)s->hunger);
        } else dtext(26,y+35,MUTED,save_result_text(s->status));
    }
    button(ui_save_rect(3),"Zurueck",cursor==3?ACCENT:MUTED);
    dtext(16,461,MUTED,"Antippen oder Pfeile / EXE");
    dtext(16,493,MUTED,"CLEAR: Zurueck");
}
void render_confirm(const char *question,bool yes)
{
    page_title("BESTAETIGEN","Das Spiel bleibt waehrenddessen pausiert");
    dtext(16,116,C_WHITE,question);
    button(ui_confirm_rect(false),"Nein",yes?MUTED:ACCENT);
    button(ui_confirm_rect(true),"Ja",yes?ACCENT:MUTED);
    dtext(16,461,MUTED,"Antippen oder Pfeile / EXE");
    dtext(16,493,MUTED,"CLEAR: Abbrechen");
}
void render_message(const char *message)
{
    page_title("MINIMINE","Spielstand");dtext(16,116,C_WHITE,message);
    button(ui_message_rect(),"EXE / CLEAR: Weiter",ACCENT);
}
