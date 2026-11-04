#include "game.h"
#include <stddef.h>

static int absolute(int x) {return x<0?-x:x;}
bool game_in_arena(const Player *p)
{
    return p->x>=(ARENA_LEFT+1)*16*FIXED_ONE && p->x<ARENA_RIGHT*16*FIXED_ONE &&
        p->y>=(ARENA_TOP+1)*16*FIXED_ONE && p->y<ARENA_FLOOR*16*FIXED_ONE;
}
void game_boss_reset(Game *g)
{
    for(unsigned i=0;i<MOB_LIMIT;++i) if(g->mobs[i].active && g->mobs[i].kind==MOB_WARDEN)
        g->mobs[i]=(Mob){0};
    g->boss.active=false;g->boss.phase=g->boss.timer=g->boss.pattern=g->boss.shock_ticks=0;
    g->boss.direction=0;g->boss.shock_x=0;
}
void game_boss_defeated(Game *g)
{
    game_boss_reset(g);g->boss.defeated=true;g->boss.reward_pending=true;
    game_deliver_reward(g);g->notice=NOTICE_BOSS_WIN;g->notice_ticks=240;
}
static Mob *warden(Game *g)
{
    for(unsigned i=0;i<MOB_LIMIT;++i) if(g->mobs[i].active && g->mobs[i].kind==MOB_WARDEN) return &g->mobs[i];
    return NULL;
}
static void begin(Game *g)
{
    for(unsigned i=0;i<MOB_LIMIT;++i) if(g->mobs[i].active && game_in_arena(&g->mobs[i].body)) g->mobs[i]=(Mob){0};
    unsigned hostiles=0;int replace=-1;
    for(int i=0;i<MOB_LIMIT;++i) if(g->mobs[i].active && game_hostile(g->mobs[i].kind))
        {++hostiles;replace=i;}
    if(hostiles>=8 && replace>=0) g->mobs[replace]=(Mob){0};
    int slot=-1;
    for(int i=0;i<MOB_LIMIT;++i) if(!g->mobs[i].active) {slot=i;break;}
    if(slot<0) slot=MOB_LIMIT-1;
    int32_t y=(ARENA_FLOOR*16-24)*FIXED_ONE;
    for(int col=ARENA_LEFT+12;col>ARENA_LEFT+1;--col) {
        int32_t x=col*16*FIXED_ONE;
        if(!game_rect_clear(g,x,y,16,24) || absolute((g->player.x-x)/FIXED_ONE)<32) continue;
        g->mobs[slot]=(Mob){.active=true,.kind=MOB_WARDEN,.health=10000,
            .body={.x=x,.y=y,.facing=-1,.grounded=true}};
        g->boss.active=true;g->boss.phase=0;g->boss.timer=42;
        g->boss.direction=g->player.x<x?-1:1;return;
    }
}
void game_boss_step(Game *g)
{
    if(!g->ruins) return;
    if(g->boss.reward_pending) game_deliver_reward(g);
    if(!game_in_arena(&g->player)) {if(g->boss.active) game_boss_reset(g);return;}
    if(!g->boss.gate_open || g->boss.defeated) return;
    if(!g->boss.active) begin(g);
    Mob *m=warden(g);if(!m) return;
    Boss *b=&g->boss;Player *p=&m->body;
    if(m->hurt_ticks) --m->hurt_ticks;
    if(m->attack_ticks) --m->attack_ticks;
    unsigned warning=m->health<=5000?27u:42u,recovery=m->health<=5000?48u:72u;
    if(b->timer) --b->timer;
    if(!b->timer) {
        switch(b->phase) {
        case 0:b->phase=1;b->timer=45;break;
        case 1:b->phase=2;b->timer=recovery;break;
        case 2:b->phase=3;b->timer=warning;break;
        case 3:
            b->phase=4;b->timer=recovery;b->shock_ticks=24;b->shock_x=p->x+8*FIXED_ONE;break;
        default:
            b->phase=0;b->timer=warning;b->direction=g->player.x<p->x?-1:1;break;
        }
    }
    p->vx=b->phase==1?b->direction*768:0;p->facing=b->direction;
    if(p->grounded && p->vx && !game_rect_clear(g,p->x+p->vx,p->y,16,24) &&
       game_rect_clear(g,p->x+p->vx,p->y-16*FIXED_ONE,16,24)) {p->vy=-1536;p->grounded=false;}
    p->vy+=64;if(p->vy>2048) p->vy=2048;
    game_move_mob(g,m);
    if(!m->attack_ticks && game_mob_touches_player(g,m,1) && !g->hurt_ticks) {
        game_damage_player(g,b->phase==1?300u:200u,true);m->attack_ticks=60;
        if(!b->active) return;
    }
    if(b->shock_ticks) {
        int distance=(int)(24-b->shock_ticks)*3;
        int dx=absolute((g->player.x+5*FIXED_ONE-b->shock_x)/FIXED_ONE);
        int feet=(g->player.y+game_player_height(&g->player)*FIXED_ONE)/FIXED_ONE;
        if(dx>=distance-PLAYER_W && dx<=distance+3 && feet>=ARENA_FLOOR*16-3)
            game_damage_player(g,300,true);
        if(b->shock_ticks) --b->shock_ticks;
    }
}
