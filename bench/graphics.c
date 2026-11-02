/* Reuse the shipped tile/sprite artwork and native reference renderer verbatim.
   Redirection only affects this separate executable, never the game build. */
#include "bench.h"
#include <gint/display.h>
#include <gint/prof.h>
#ifndef BENCH_HOST
#include <gint/drivers/r61523.h>
#include <gint/dma.h>
#include <gint/mpu/dma.h>
#else
static uint16_t host_xram[8*SCREEN_W],host_yram[8*SCREEN_W];
static uint16_t host_fragment[16*SCREEN_W];
void bench_host_send(const uint16_t *,unsigned,unsigned);
void bench_host_begin(void);
#endif
#include <azur/gint/render.h>
#include <string.h>
#define render_invalidate frozen_render_invalidate
#define render_world frozen_render_world
#define render_game frozen_render_game
#define render_inventory frozen_render_inventory
#define render_help frozen_render_help
#define render_results frozen_render_results
#define render_error frozen_render_error
#define render_menu frozen_render_menu
#define render_progress frozen_render_progress
#define render_slots frozen_render_slots
#define render_confirm frozen_render_confirm
#define render_message frozen_render_message
static void capture_rect(int,int,int,int,int);
static void capture_pixel(int,int,int);
static void capture_text(int,int,int,const char *);
#define drect capture_rect
#define dpixel capture_pixel
#define dtext capture_text
#include "../benchmarks/1.0.0/render-reference.c"
#undef drect
#undef dpixel
#undef dtext
#undef render_invalidate
#undef render_world
#undef render_game
#undef render_inventory
#undef render_help
#undef render_results
#undef render_error
#undef render_menu
#undef render_progress
#undef render_slots
#undef render_confirm
#undef render_message
void render_world(const Game *,const RenderInfo *);

enum {CMD_RECT,CMD_TILE,CMD_SHADE};
typedef struct {int16_t x0,y0,x1,y1;uint16_t kind,arg,variant,pad;} Draw;
static Draw commands[COMMAND_LIMIT];
static unsigned command_count;
static bool capturing,overflow;
static const Game *drawing_game;
static uint16_t ram_lines[8*SCREEN_W] __attribute__((aligned(32)));
static uint32_t exclamation[24];
static int custom_shader;
static unsigned active_scale;
static bool checking;
static uint32_t image_hash;
static const uint16_t *screen_copy;
static unsigned fragment_checks;

static void emit(Draw d)
{
    if(d.x1<=0 || d.x0>=SCREEN_W || d.y1<=VIEW_TOP || d.y0>=VIEW_TOP+VIEW_H) return;
    if(command_count==COMMAND_LIMIT) {overflow=true;return;}
    commands[command_count++]=d;
}
static void capture_rect(int x0,int y0,int x1,int y1,int color)
{
    if(!capturing) {drect(x0,y0,x1,y1,color);return;}
    if(x1<x0 || y1<y0) return;
    emit((Draw){(int16_t)x0,(int16_t)y0,(int16_t)(x1+1),(int16_t)(y1+1),CMD_RECT,(uint16_t)color,0,0});
}
static void capture_pixel(int x,int y,int color) {capture_rect(x,y,x,y,color);}
static void capture_text(int x,int y,int color,const char *text)
{
    if(!capturing) {dtext(x,y,color,text);return;}
    /* The deterministic workload only uses the game's native warning glyph. */
    if(strcmp(text,"!")!=0) {overflow=true;return;}
    for(int py=0;py<24;++py) for(int px=0;px<24;++px)
        if(exclamation[py]&(1u<<px)) capture_pixel(x+px,y+py,color);
}
static void capture_scene(const Game *g,unsigned scene)
{
    command_count=0;overflow=false;drawing_game=g;prepare_atlas();prepare_background(g);
    capturing=true;
    int first_x=g->camera_x/16,first_y=g->camera_y/16;
    for(int ty=first_y;ty<=(g->camera_y+VIEW_H-1)/16;++ty)
        for(int tx=first_x;tx<=(g->camera_x+SCREEN_W-1)/16;++tx) {
            Tile t=game_tile(g,tx,ty);int x=tx*16-g->camera_x,y=ty*16-g->camera_y+VIEW_TOP;
            emit((Draw){(int16_t)x,(int16_t)y,(int16_t)(x+16),(int16_t)(y+16),CMD_TILE,(uint16_t)t,(uint16_t)((tx+ty)%3),0});
            if(t==TILE_SEAL) {outline(x+3,y+2,10,12,C_RGB(25,16,29));capture_rect(x+6,y+5,x+9,y+10,C_RGB(30,26,13));}
        }
    if(scene>=3) {
        for(unsigned i=0;i<MOB_LIMIT;++i) if(g->mobs[i].active) draw_mob(g,&g->mobs[i]);
    }
    player_draw(g);
    if(scene>=3) {
        player_equipment(g);
        for(int y=0;y<27;++y) for(int x=0;x<22;++x) {
            int tx=first_x+x;
            block_light[y][x]=(uint8_t)(tx<WORLD_W && first_y+y<=g->surface[tx]?2:0);
        }
        int px=(g->player.x/FIXED_ONE+5)/16;
        int py=(g->player.y/FIXED_ONE+game_player_height(&g->player)/2)/16;
        bool held=g->slots[g->selected].item==ITEM_TORCH && g->slots[g->selected].count;
        light_source(px,py,held?4:2,2,first_x,first_y);
        for(unsigned i=0;i<TORCH_LIMIT;++i) if(g->torches[i].active)
            light_source(g->torches[i].x,g->torches[i].y,6,3,first_x,first_y);
        for(int y=0;y<27;++y) for(int x=0;x<22;++x) if(block_light[y][x]<2) {
            int sx=(first_x+x)*16-g->camera_x,sy=(first_y+y)*16-g->camera_y+VIEW_TOP;
            emit((Draw){(int16_t)sx,(int16_t)sy,(int16_t)(sx+16),(int16_t)(sy+16),CMD_SHADE,(uint16_t)(block_light[y][x]?1:2),0,0});
        }
        int tx,ty;game_target(g,&tx,&ty);outline(tx*16-g->camera_x,ty*16-g->camera_y+VIEW_TOP,16,16,C_WHITE);
    }
    capturing=false;
}

/* Both RAM and ILRAM versions compile exactly the same source. */
#define ROW_NAME row_ram
#define ROW_ATTR __attribute__((noinline))
#include "row.inc"
#undef ROW_NAME
#undef ROW_ATTR
#define ROW_NAME row_il
#ifdef BENCH_HOST
#define ROW_ATTR __attribute__((noinline))
#else
#define ROW_ATTR __attribute__((noinline,section(".ilram")))
#endif
#include "row.inc"
#undef ROW_NAME
#undef ROW_ATTR
typedef void (*Row)(const Draw *,uint16_t *,int,unsigned);

static void draw_region(const Draw *c,uint16_t *buffer,int start,int rows,unsigned scale,Row row)
{
    int top=c->y0>start?c->y0:start,bottom=c->y1<start+rows*(int)scale?c->y1:start+rows*(int)scale;
    if(top<VIEW_TOP) top=VIEW_TOP;
    if(bottom>VIEW_TOP+VIEW_H) bottom=VIEW_TOP+VIEW_H;
    top+=(int)((scale-(unsigned)(top-start)%scale)%scale);
    for(int y=top;y<bottom;y+=(int)scale) row(c,buffer+(y-start)/(int)scale*(SCREEN_W/(int)scale),y,scale);
}
static void render_region(uint16_t *buffer,int start,int rows,unsigned scale,Row row)
{
    for(unsigned i=0;i<command_count;++i) draw_region(&commands[i],buffer,start,rows,scale,row);
}
#ifndef BENCH_HOST
static void lcd_select(unsigned reg)
{
    volatile uint8_t *pr=(void *)0xa405013c;
    volatile uint16_t *lcd=(void *)0xb4000000;
    *pr&=(uint8_t)~0x10;__asm__ volatile("synco" ::: "memory");
    *lcd=(uint16_t)reg;__asm__ volatile("synco" ::: "memory");
    *pr|=0x10;__asm__ volatile("synco" ::: "memory");
}
void bench_lcd_begin(void) {r61523_win_set(0,319,0,527);lcd_select(0x2c);}
static void send_pixels(const uint16_t *src,unsigned n)
{volatile uint16_t *lcd=(void *)0xb4000000;for(unsigned i=0;i<n;++i) *lcd=src[i];}
static void send_twice(const uint16_t *src,unsigned n)
{volatile uint16_t *lcd=(void *)0xb4000000;for(unsigned i=0;i<n;++i) {*lcd=src[i];*lcd=src[i];}}
static bool send_dma(const uint16_t *src,unsigned n,bool wide)
{
    uintptr_t first=(uintptr_t)src&~(uintptr_t)31,last=(uintptr_t)(src+n);
    for(uintptr_t p=first;p<last;p+=32) __asm__ volatile("ocbwb @%0" :: "r"(p) : "memory");
    __asm__ volatile("synco" ::: "memory");
    if(!dma_transfer_async(0,wide?DMA_32B:DMA_2B,wide?n/16:n,src,DMA_INC,(void *)0xb4000000,DMA_FIXED,GINT_CALL_NULL)) return false;
    uint32_t start=bench_now();
    while(SH7305_DMA.DMA0.CHCR.DE) {
        if(bench_us(start,bench_now())>500000) {
            SH7305_DMA.DMA0.CHCR.DE=0;SH7305_DMA.DMA0.CHCR.IE=0;SH7305_DMA.DMA0.CHCR.TE=0;
            dma_transfer_wait(0);return false;
        }
    }
    dma_transfer_wait(0);return true;
}
#else
void bench_lcd_begin(void) {bench_host_begin();}
static void send_pixels(const uint16_t *p,unsigned n) {bench_host_send(p,n,1);}
static void send_twice(const uint16_t *p,unsigned n) {bench_host_send(p,n,2);}
static bool send_dma(const uint16_t *p,unsigned n,bool wide) {(void)wide;send_pixels(p,n);return true;}
#endif
static uint16_t *chunk_buffer(Variant v)
{
#ifdef BENCH_HOST
    return v==V_YRAM?host_yram:(v==V_XRAM || v==V_X_IL)?host_xram:ram_lines;
#else
    return v==V_YRAM?(void *)0xe5017020:(v==V_XRAM || v==V_X_IL)?(void *)0xe5007020:ram_lines;
#endif
}
static uint32_t hash_pixel(uint32_t h,uint16_t pixel)
{h^=pixel>>8;h*=16777619u;h^=pixel&255u;return h*16777619u;}
static uint32_t hash_view(const uint16_t *p,unsigned scale,unsigned stride)
{
    uint32_t h=2166136261u;
    for(unsigned y=0;y<VIEW_H/scale;++y) for(unsigned x=0;x<SCREEN_W/scale;++x)
        h=hash_pixel(h,p[y*stride+x]);
    return h;
}
static void send_ui_half(int first,int end)
{
    for(int y=first;y<end;y+=2) {
        for(unsigned x=0;x<SCREEN_W/2;++x) ram_lines[x]=gint_vram[y*SCREEN_W+x*2];
        send_twice(ram_lines,SCREEN_W/2);send_twice(ram_lines,SCREEN_W/2);
    }
}
typedef struct {uint8_t shader,pad;uint16_t fragment,index,unused;} AzCommand;
static void az_shader(void *uniform,void *command,void *fragment)
{
    (void)uniform;AzCommand *a=command;
    int start=(int)a->fragment*16*(int)active_scale;
    draw_region(&commands[a->index],fragment,start,16,active_scale,row_ram);++a->fragment;
}
static void az_background(void *uniform,void *command,void *fragment)
{
    (void)uniform;AzCommand *a=command;
    int start=(int)a->fragment*16*(int)active_scale;
    uint16_t *dest=fragment;
    for(unsigned y=0;y<16;++y) {
        unsigned sy=(unsigned)start+y*active_scale;
        /* Tiles completely cover the world; only retain the static UI strips. */
        if(sy>=VIEW_TOP && sy<VIEW_TOP+VIEW_H) continue;
        for(unsigned x=0;x<SCREEN_W/active_scale;++x)
            dest[y*(SCREEN_W/active_scale)+x]=sy<SCREEN_H?screen_copy[sy*SCREEN_W+x*active_scale]:0;
    }
    ++a->fragment;
}
static int background_shader;
static void check_fragment(int id,void *fragment,int bytes)
{
    if(!checking) return;
    const uint16_t *p=fragment;unsigned width=SCREEN_W/active_scale;
    unsigned rows=(unsigned)bytes/(width*2);
    for(unsigned y=0;y<rows;++y) {
        unsigned sy=((unsigned)id*16+y)*active_scale;
        if(sy<VIEW_TOP || sy>=VIEW_TOP+VIEW_H) continue;
        for(unsigned x=0;x<width;++x) image_hash=hash_pixel(image_hash,p[y*width+x]);
    }
    ++fragment_checks;
}
bool graphics_init(void)
{
#ifdef BENCH_HOST
    azrp_frag=host_fragment;
#endif
    /* Capture the actual gint font once, outside all measurements. */
    dclear(0);dtext(0,VIEW_TOP,C_WHITE,"!");
    for(unsigned y=0;y<24;++y) for(unsigned x=0;x<24;++x)
        if(gint_vram[(VIEW_TOP+y)*SCREEN_W+x]) exclamation[y]|=1u<<x;
    prepare_atlas();
    if(!azrp_cmdq_setup(8192,32768)) return false;
    background_shader=azrp_register_shader(az_background,NULL);
    custom_shader=azrp_register_shader(az_shader,NULL);
    azrp_hook_set_prefrag(check_fragment);
    return custom_shader>=0 && background_shader>=0;
}
static bool queue_azur(void)
{
    azrp_clear_commands();
    AzCommand *base=azrp_cmdq_command(sizeof(*base),0,azrp_frag_count);
    if(!base) return false;
    *base=(AzCommand){(uint8_t)background_shader,0,0,0,0};
    unsigned indices=(unsigned)azrp_frag_count;
    for(unsigned i=0;i<command_count;++i) {
        const Draw *c=&commands[i];int top=c->y0<VIEW_TOP?VIEW_TOP:c->y0;
        int bottom=c->y1>VIEW_TOP+VIEW_H?VIEW_TOP+VIEW_H:c->y1;
        int first=(top+(int)active_scale-1)/(int)active_scale/16;
        int last=(bottom-1)/(int)active_scale/16;
        if(bottom<=top || first>last) continue;
        indices+=(unsigned)(last-first+1);
        /* The upstream queue leaves allocation failure unchecked internally. */
        if(indices>=8192 || (i+2)*sizeof(AzCommand)>32768) return false;
        AzCommand *a=azrp_cmdq_command(sizeof(*a),first,last-first+1);
        if(!a) return false;
        *a=(AzCommand){(uint8_t)custom_shader,0,(uint16_t)first,(uint16_t)i,0};
    }
    return true;
}
bool graphics_frame(const Game *g,unsigned scene,Variant v,Measurement *m)
{
    uint32_t start=bench_now();active_scale=variants[v].scale;screen_copy=gint_vram;
    command_count=0;
    bool ok=true;uint32_t lcd=0;
    if(v==V_REFERENCE || v==V_DMA_FULL || v==V_DMA_32 ||
       v==V_OPTIMIZED || v==V_REFERENCE_CPU || v==V_OPTIMIZED_CPU || v==V_ROW_LIGHT) {
        RenderInfo info={.benchmark=scene<3};
        if(v==V_ROW_LIGHT) render_world(g,&info);
        else if(v==V_OPTIMIZED || v==V_OPTIMIZED_CPU) render_world_legacy_light(g,&info);
        else frozen_render_world(g,&info);
        uint32_t before=bench_now();
        if(v==V_REFERENCE || v==V_OPTIMIZED || v==V_ROW_LIGHT) dupdate();
        else if(v==V_REFERENCE_CPU || v==V_OPTIMIZED_CPU) {bench_lcd_begin();send_pixels(gint_vram,SCREEN_W*SCREEN_H);}
        else {bench_lcd_begin();ok=send_dma(gint_vram,SCREEN_W*SCREEN_H,v==V_DMA_32);}
        lcd=bench_us(before,bench_now());
    } else {
        capture_scene(g,scene);if(overflow) return false;
        if(variants[v].azur) {
            azrp_config_scale((int)active_scale);azrp_config_frag_offset(0);azrp_perf_clear();
            if(!queue_azur()) return false;
            azrp_update();lcd=prof_time(azrp_perf_lcd);
            m->sort=prof_time(azrp_perf_sort);m->shader=prof_time(azrp_perf_shaders);
        } else if(v==V_HALF) {
            uint16_t *small=gint_vram+VIEW_TOP*SCREEN_W;
            render_region(small,VIEW_TOP,VIEW_H/2,2,row_ram);
            uint32_t before=bench_now();bench_lcd_begin();
            send_ui_half(0,VIEW_TOP);
            for(unsigned y=0;y<VIEW_H/2;++y) {send_twice(small+y*160,160);send_twice(small+y*160,160);}
            send_ui_half(VIEW_TOP+VIEW_H,SCREEN_H);
            lcd=bench_us(before,bench_now());
        } else {
            uint16_t *buffer=chunk_buffer(v);
            Row row=(v==V_ILRAM || v==V_X_IL)?row_il:row_ram;
            uint32_t before=bench_now();bench_lcd_begin();lcd+=bench_us(before,bench_now());
            for(int y=0;y<SCREEN_H;y+=8) {
                if(y<VIEW_TOP || y>=VIEW_TOP+VIEW_H) memcpy(buffer,gint_vram+y*SCREEN_W,sizeof(ram_lines));
                else render_region(buffer,y,8,1,row);
                before=bench_now();
                if(v==V_DMA_LINES) {if(!send_dma(buffer,8*SCREEN_W,false)) {ok=false;break;}}
                else send_pixels(buffer,8*SCREEN_W);
                lcd+=bench_us(before,bench_now());
            }
        }
    }
    m->total=bench_us(start,bench_now());m->lcd=lcd;
    m->draw=m->total>=lcd?m->total-lcd:0;m->commands=command_count;
    return ok;
}
ImageCheck graphics_check(const Game *g,unsigned scene,Variant v)
{
    RenderInfo info={.benchmark=scene<3};frozen_render_world(g,&info);
    unsigned scale=variants[v].scale;uint32_t expected=2166136261u;
    for(unsigned y=VIEW_TOP;y<VIEW_TOP+VIEW_H;y+=scale)
        for(unsigned x=0;x<SCREEN_W;x+=scale) expected=hash_pixel(expected,gint_vram[y*SCREEN_W+x]);
    if(v==V_REFERENCE || v==V_DMA_FULL || v==V_DMA_32 || v==V_REFERENCE_CPU)
        return (ImageCheck){expected,expected,0,true};
    if(v==V_OPTIMIZED || v==V_OPTIMIZED_CPU || v==V_ROW_LIGHT) {
        if(v==V_ROW_LIGHT) render_world(g,&info);else render_world_legacy_light(g,&info);
        uint32_t actual=hash_view(gint_vram+VIEW_TOP*SCREEN_W,1,SCREEN_W);
        return (ImageCheck){expected,actual,0,expected==actual};
    }
    capture_scene(g,scene);uint32_t actual=2166136261u;
    if(variants[v].azur) {
        active_scale=scale;screen_copy=gint_vram;checking=true;image_hash=2166136261u;fragment_checks=0;
        azrp_config_scale((int)scale);azrp_config_frag_offset(0);
        bool queued=queue_azur();if(queued) azrp_update();checking=false;actual=image_hash;
        if(!queued || fragment_checks!=(unsigned)azrp_frag_count) overflow=true;
    } else if(v==V_HALF) {
        render_region(gint_vram+VIEW_TOP*SCREEN_W,VIEW_TOP,VIEW_H/2,2,row_ram);
        actual=hash_view(gint_vram+VIEW_TOP*SCREEN_W,2,160);
    } else {
        uint16_t *buf=chunk_buffer(v);
        Row row=(v==V_ILRAM || v==V_X_IL)?row_il:row_ram;
        for(int y=VIEW_TOP;y<VIEW_TOP+VIEW_H;y+=8) {
            render_region(buf,y,8,1,row);
            for(unsigned i=0;i<8*SCREEN_W;++i) actual=hash_pixel(actual,buf[i]);
        }
    }
    return (ImageCheck){expected,actual,command_count,!overflow && expected==actual};
}
size_t graphics_bytes(void)
{return sizeof(atlas)+sizeof(masks)+sizeof(mob_pixels)+sizeof(mob_masks)+sizeof(block_light)+sizeof(hill_top)+sizeof(commands)+sizeof(ram_lines)+sizeof(exclamation)+8192*4+32768+16*SCREEN_W*2+render_graphics_bytes();}
size_t graphics_working_ram(Variant v)
{
    size_t n=sizeof(atlas)+sizeof(masks)+sizeof(mob_pixels)+sizeof(mob_masks)+sizeof(block_light)+sizeof(hill_top);
    if(v==V_OPTIMIZED || v==V_OPTIMIZED_CPU || v==V_ROW_LIGHT) return render_graphics_bytes();
    if(v==V_REFERENCE || v==V_DMA_FULL || v==V_DMA_32 || v==V_REFERENCE_CPU || v==V_OPTIMIZED || v==V_OPTIMIZED_CPU) return n;
    n+=sizeof(commands)+sizeof(exclamation);
    if(variants[v].azur) n+=8192*4+32768;
    else if(v==V_HALF) n+=SCREEN_W;
    else if(v!=V_XRAM && v!=V_YRAM && v!=V_X_IL) n+=sizeof(ram_lines);
    return n;
}
size_t graphics_onchip_data(Variant v)
{return variants[v].azur?(size_t)(16*SCREEN_W*2/variants[v].scale):
    (v==V_XRAM || v==V_YRAM || v==V_X_IL)?sizeof(ram_lines):0;}
