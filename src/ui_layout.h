#ifndef MINIMINE_UI_LAYOUT_H
#define MINIMINE_UI_LAYOUT_H
#include <stdbool.h>
#include "game.h"

/* Drawing and touch hit tests use the same rectangles. Right/bottom excluded. */
typedef struct {int x,y,w,h;} UIRect;
static inline bool ui_contains(UIRect r,int x,int y)
{return x>=r.x && y>=r.y && x<r.x+r.w && y<r.y+r.h;}
static inline UIRect ui_menu_rect(unsigned i)
{return (UIRect){12,84+(int)i*53,296,34};}
static inline UIRect ui_hotbar_rect(unsigned i,bool inventory)
{return (UIRect){4+(int)i*79,inventory?85:VIEW_TOP+VIEW_H+3,75,inventory?30:47};}
static inline int ui_hotbar_hit(int x,int y)
{
    for(unsigned i=0;i<HOTBAR_SLOTS;++i) if(ui_contains(ui_hotbar_rect(i,false),x,y)) return (int)i;
    return -1;
}
static inline UIRect ui_save_rect(unsigned i)
{return i<3?(UIRect){16,88+(int)i*92,288,78}:(UIRect){16,370,288,34};}
static inline UIRect ui_confirm_rect(bool yes)
{return (UIRect){yes?172:16,184,132,42};}
static inline UIRect ui_play_rect(void) {return (UIRect){16,455,288,30};}
static inline UIRect ui_benchmark_rect(void) {return (UIRect){16,491,140,30};}
static inline UIRect ui_back_rect(void) {return (UIRect){164,491,140,30};}
static inline UIRect ui_message_rect(void) {return (UIRect){16,469,288,34};}
static inline UIRect ui_armor_rect(void) {return (UIRect){16,485,140,34};}
static inline UIRect ui_close_rect(void) {return (UIRect){164,485,140,34};}
#endif
