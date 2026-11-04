#include "game.h"

static bool intersects(int32_t a,int size,int32_t b,int other)
{return a<b+other*FIXED_ONE && a+size*FIXED_ONE>b;}
bool game_mob_touches_player(const Game *g,const Mob *m,int margin)
{
    return intersects(m->body.x-margin*FIXED_ONE,(int)game_mob_width(m->kind)+2*margin,
            g->player.x,PLAYER_W) &&
        intersects(m->body.y-margin*FIXED_ONE,(int)game_mob_height(m->kind)+2*margin,
            g->player.y,game_player_height(&g->player));
}
bool game_player_space_clear(const Game *g,int32_t x,int32_t y,int height)
{
    if(!game_rect_clear(g,x,y,PLAYER_W,height)) return false;
    if(g->survival) for(unsigned i=0;i<MOB_LIMIT;++i) {
        const Mob *m=&g->mobs[i];
        if(m->active && intersects(x,PLAYER_W,m->body.x,(int)game_mob_width(m->kind)) &&
           intersects(y,height,m->body.y,(int)game_mob_height(m->kind))) return false;
    }
    return true;
}
/* Clamp swept movement at the first contact; touching edges are permitted. */
static int32_t clip(int32_t old,int32_t next,int size,int32_t obstacle,int other)
{
    if(next>old && old+size*FIXED_ONE<=obstacle && next+size*FIXED_ONE>obstacle)
        return obstacle-size*FIXED_ONE;
    if(next<old && old>=obstacle+other*FIXED_ONE && next<obstacle+other*FIXED_ONE)
        return obstacle+other*FIXED_ONE;
    return next;
}
static int32_t horizontal(const Game *g,const Player *p,int width,int height,int32_t dx)
{
    Player probe=*p;probe.vx=dx;probe.vy=0;
    game_move_actor(g,&probe,width,height);return probe.x;
}
void game_move_player(Game *g)
{
    Player *p=&g->player;int h=game_player_height(p);
    int32_t old_x=p->x,old_y=p->y,wanted=p->vx;
    int32_t next=horizontal(g,p,PLAYER_W,h,wanted);
    /* First find the shared movement limit, without moving any mob. */
    for(unsigned i=0;i<MOB_LIMIT;++i) {
        const Mob *m=&g->mobs[i];if(!m->active) continue;
        int w=(int)game_mob_width(m->kind),mh=(int)game_mob_height(m->kind);
        if(!intersects(old_y,h,m->body.y,mh)) continue;
        int32_t contact=clip(old_x,next,PLAYER_W,m->body.x,w);
        if(contact==next) continue;
        if(m->kind==MOB_WARDEN) {next=contact;continue;}
        int32_t push=next>old_x?next+PLAYER_W*FIXED_ONE-m->body.x:next-m->body.x-w*FIXED_ONE;
        int32_t moved=horizontal(g,&m->body,w,mh,push);
        int32_t limit=next>old_x?moved-PLAYER_W*FIXED_ONE:moved+w*FIXED_ONE;
        if(next>old_x && limit<next) next=limit;
        if(next<old_x && limit>next) next=limit;
    }
    /* Commit only the pushes required by the final, terrain-safe player move. */
    for(unsigned i=0;i<MOB_LIMIT;++i) {
        Mob *m=&g->mobs[i];if(!m->active || m->kind==MOB_WARDEN) continue;
        int w=(int)game_mob_width(m->kind),mh=(int)game_mob_height(m->kind);
        if(!intersects(old_y,h,m->body.y,mh) || clip(old_x,next,PLAYER_W,m->body.x,w)==next) continue;
        m->body.x=next>old_x?next+PLAYER_W*FIXED_ONE:next-w*FIXED_ONE;
    }
    p->x=next;if(next!=old_x+wanted) p->vx=0;
    int32_t vx=p->vx,vy=p->vy;p->vx=0;
    game_move_actor(g,p,PLAYER_W,h);p->vx=vx;
    next=p->y;
    for(unsigned i=0;i<MOB_LIMIT;++i) {
        const Mob *m=&g->mobs[i];if(!m->active) continue;
        int w=(int)game_mob_width(m->kind),mh=(int)game_mob_height(m->kind);
        if(!intersects(p->x,PLAYER_W,m->body.x,w)) continue;
        int32_t limited=clip(old_y,next,h,m->body.y,mh);
        if(limited!=next) {next=limited;p->vy=0;p->grounded=vy>0;}
    }
    p->y=next;
}
void game_move_mob(Game *g,Mob *m)
{
    Player *p=&m->body;int w=(int)game_mob_width(m->kind),h=(int)game_mob_height(m->kind);
    int ph=game_player_height(&g->player);int32_t old=p->x;
    int32_t next=horizontal(g,p,w,h,p->vx);
    if(intersects(p->y,h,g->player.y,ph)) next=clip(old,next,w,g->player.x,PLAYER_W);
    if(next!=old+p->vx) p->vx=0;
    p->x=next;old=p->y;int32_t vx=p->vx,vy=p->vy;p->vx=0;
    game_move_actor(g,p,w,h);p->vx=vx;
    if(intersects(p->x,w,g->player.x,PLAYER_W)) {
        next=clip(old,p->y,h,g->player.y,ph);
        if(next!=p->y) {p->y=next;p->vy=0;p->grounded=vy>0;}
    }
}
bool game_repair_overlaps(Game *g)
{
    bool changed=false;
    for(unsigned i=0;i<MOB_LIMIT;++i) {
        Mob *m=&g->mobs[i];if(!m->active || !game_mob_touches_player(g,m,0)) continue;
        int cx=m->body.x/FIXED_ONE/TILE_SIZE,cy=m->body.y/FIXED_ONE/TILE_SIZE;
        int w=(int)game_mob_width(m->kind),h=(int)game_mob_height(m->kind);
        bool found=false;
        for(int radius=1;radius<WORLD_W+WORLD_H && !found;++radius)
            for(int dy=-radius;dy<=radius && !found;++dy) {
                int row=cy+dy;if(row<0 || row>=WORLD_H) continue;
                int dx=radius-(dy<0?-dy:dy);
                for(int side=0;side<2 && !found;++side) {
                    int col=cx+(side?-dx:dx);if(col<0 || col>=WORLD_W) continue;
                    Player candidate=m->body;candidate.x=col*TILE_SIZE*FIXED_ONE;candidate.y=row*TILE_SIZE*FIXED_ONE;
                    if(m->kind==MOB_WARDEN && !game_in_arena(&candidate)) continue;
                    Mob probe=*m;probe.body=candidate;
                    if(!game_rect_clear(g,candidate.x,candidate.y,w,h) || game_mob_touches_player(g,&probe,0)) continue;
                    m->body=candidate;m->body.vx=m->body.vy=0;m->body.grounded=false;found=true;
                }
            }
        if(!found) {
            if(m->kind==MOB_WARDEN) game_boss_reset(g);
            else *m=(Mob){0};
        }
        changed=true;
    }
    if(changed) ++g->revision;
    return changed;
}
