#include "game.h"
#include <string.h>
#include <limits.h>

static int absolute(int x) {return x<0?-x:x;}
static int sign(int x) {return (x>0)-(x<0);}
static uint32_t random_next(Game *g)
{g->rng^=g->rng<<13;g->rng^=g->rng>>17;g->rng^=g->rng<<5;return g->rng;}
unsigned game_mob_width(MobKind k)
{static const unsigned v[]={14,16,10,10};return (unsigned)k<MOB_COUNT?v[k]:0;}
unsigned game_mob_height(MobKind k)
{static const unsigned v[]={12,14,10,22};return (unsigned)k<MOB_COUNT?v[k]:0;}
unsigned game_mob_health(MobKind k)
{static const unsigned v[]={600,1000,400,1200};return (unsigned)k<MOB_COUNT?v[k]:0;}
static Item loot(MobKind k)
{return k==MOB_PIG?ITEM_RAW_PORK:k==MOB_COW?ITEM_RAW_BEEF:k==MOB_CHICKEN?ITEM_RAW_CHICKEN:ITEM_NONE;}
static bool overlaps(int32_t x,int32_t y,int w,int h,int32_t bx,int32_t by,int bw,int bh)
{return x<bx+bw*FIXED_ONE && x+w*FIXED_ONE>bx && y<by+bh*FIXED_ONE && y+h*FIXED_ONE>by;}
static bool visible_line(const Game *g,int x,int y,int bx,int by)
{
    int dx=bx-x,dy=by-y,n=absolute(dx)>absolute(dy)?absolute(dx):absolute(dy);
    n=n/4+1;
    for(int i=1;i<=n;++i) {
        int px=x+dx*i/n,py=y+dy*i/n;
        if(game_tile_solid(game_tile(g,px/TILE_SIZE,py/TILE_SIZE))) return false;
    }
    return true;
}
void game_exert(Game *g)
{
    if(!g->survival) return;
    if(++g->exertion>=20) {g->exertion-=20;if(g->hunger) --g->hunger;}
}
static void respawn(Game *g)
{
    Player *p=&g->player;int origin=g->spawn_x/FIXED_ONE/TILE_SIZE;
    bool found=false;
    for(int d=0;d<WORLD_W && !found;++d) for(int side=0;side<2 && !found;++side) {
        int col=origin+(side?-d:d);if(col<0 || col>=WORLD_W) continue;
        int32_t x=(col*TILE_SIZE+3)*FIXED_ONE;
        for(int y=2;y<WORLD_H;++y) if(game_tile_solid(game_tile(g,col,y))) {
            int32_t py=(y*TILE_SIZE-PLAYER_H)*FIXED_ONE;
            if(game_rect_clear(g,x,py,PLAYER_W,PLAYER_H)) {
                *p=(Player){.x=x,.y=py,.facing=1,.grounded=true};found=true;break;
            }
        }
    }
    if(!found) {p->vx=p->vy=0;p->sneaking=false;}
    g->health=HEALTH_MAX;g->hunger=HUNGER_MAX;g->hurt_ticks=300;
    g->hunger_ticks=g->regen_ticks=g->regen_cost=g->starve_ticks=g->exertion=0;
    g->action_ticks=30;g->swing_ticks=0;g->aim_x=1;g->aim_y=0;g->aim_manual=false;
    g->notice=NOTICE_RESPAWN;g->notice_ticks=180;++g->respawns;
    game_cancel_mining(g);
}
void game_damage_player(Game *g,unsigned damage,bool mob)
{
    if(!g->survival || !damage || (mob && g->hurt_ticks)) return;
    if(mob) {
        unsigned points=0;
        for(unsigned i=0;i<ARMOR_SLOTS;++i) points+=game_armor_points(g->armor[i].item);
        if(points>15) points=15;
        damage=damage*(100-points*4)/100;if(!damage) damage=1;
        for(unsigned i=0;i<ARMOR_SLOTS;++i) if(g->armor[i].count) {
            if(!--g->armor[i].durability) {
                g->armor[i]=(ItemStack){0};g->notice=NOTICE_TOOL_BROKEN;g->notice_ticks=60;
            }
        }
        g->hurt_ticks=45;
    }
    if(damage>=g->health) {g->health=0;respawn(g);}else g->health=(uint16_t)(g->health-damage);
}
void game_drop(Game *g,ItemStack in,int32_t x,int32_t y)
{
    if(!in.count || !game_stack_valid(&in)) return;
    for(unsigned i=0;i<DROP_LIMIT;++i) {
        Drop *d=&g->drops[i];
        if(!d->active || d->stack.item!=in.item || d->stack.durability!=in.durability ||
           game_stack_limit(in.item)==1 || absolute((d->x-x)/FIXED_ONE)>32 ||
           absolute((d->y-y)/FIXED_ONE)>32) continue;
        unsigned room=64-d->stack.count,take=in.count<room?in.count:room;
        d->stack.count=(uint16_t)(d->stack.count+take);in.count=(uint16_t)(in.count-take);
        d->ttl=18000;if(!in.count) return;
    }
    unsigned chosen=DROP_LIMIT,oldest=UINT_MAX;
    for(unsigned i=0;i<DROP_LIMIT;++i) {
        if(!g->drops[i].active) {chosen=i;break;}
        if(g->drops[i].ttl<oldest) {oldest=g->drops[i].ttl;chosen=i;}
    }
    g->drops[chosen]=(Drop){.active=true,.stack=in,.x=x,.y=y,.ttl=18000};
}
static int attack_target(const Game *g)
{
    const Player *p=&g->player;int h=game_player_height(p),dx=g->aim_x,dy=g->aim_y;
    int32_t x=p->x-8*FIXED_ONE,y=p->y-8*FIXED_ONE;int w=PLAYER_W+16,ah=h+16;
    if(dx) {
        x=dx>0?p->x+PLAYER_W*FIXED_ONE:p->x-24*FIXED_ONE;w=24;
        if(dy) y+=dy*16*FIXED_ONE;
    } else if(dy) {y=dy>0?p->y+h*FIXED_ONE:p->y-24*FIXED_ONE;ah=24;}
    else return -1;
    int px=(p->x+PLAYER_W*FIXED_ONE/2)/FIXED_ONE,py=(p->y+h*FIXED_ONE/2)/FIXED_ONE;
    int target=-1,best=INT_MAX;
    for(int i=0;i<MOB_LIMIT;++i) {
        const Mob *m=&g->mobs[i];if(!m->active) continue;
        int mw=(int)game_mob_width(m->kind),mh=(int)game_mob_height(m->kind);
        if(!overlaps(x,y,w,ah,m->body.x,m->body.y,mw,mh)) continue;
        int mx=m->body.x/FIXED_ONE+mw/2,my=m->body.y/FIXED_ONE+mh/2;
        int dist=(mx-px)*(mx-px)+(my-py)*(my-py);
        if(dist<best && visible_line(g,px,py,mx,my)) {best=dist;target=i;}
    }
    return target;
}
bool game_attack(Game *g,bool require_target)
{
    if(!g->survival) return false;
    ItemStack *held=g->selected<HOTBAR_SLOTS?&g->slots[g->selected]:NULL;
    Item item=held?held->item:ITEM_NONE;
    if(item!=ITEM_NONE && game_item_kind(item)!=KIND_SWORD) return false;
    int at=attack_target(g);if(require_target && at<0) return false;
    if(g->action_ticks) return true;
    g->action_ticks=item==ITEM_NONE?30:24;g->swing_ticks=10;game_exert(g);
    if(at>=0) {
        Mob *m=&g->mobs[at];unsigned damage=(item==ITEM_NONE?1:game_sword_damage(item))*100;
        if(damage>=m->health) {
            Item drop=loot(m->kind);
            if(drop) game_drop(g,(ItemStack){.item=drop,.count=1},
                m->body.x,m->body.y+((int)game_mob_height(m->kind)-6)*FIXED_ONE);
            *m=(Mob){0};
        } else {m->health=(uint16_t)(m->health-damage);m->flee_ticks=120;}
        if(item!=ITEM_NONE && !--held->durability) {
            *held=(ItemStack){0};g->notice=NOTICE_TOOL_BROKEN;g->notice_ticks=60;
        }
    }
    return true;
}
bool game_eat(Game *g)
{
    if(!g->survival || g->selected>=HOTBAR_SLOTS || g->hunger>=HUNGER_MAX || g->action_ticks)
        return false;
    ItemStack *s=&g->slots[g->selected];unsigned food=game_food_points(s->item);
    if(!s->count || !food) return false;
    unsigned n=g->hunger+food;g->hunger=(uint8_t)(n>HUNGER_MAX?HUNGER_MAX:n);
    if(!--s->count) *s=(ItemStack){0};
    g->action_ticks=60;return true;
}
bool game_spawn_mob(Game *g,MobKind kind,int32_t x,int32_t y)
{
    if((unsigned)kind>=MOB_COUNT) return false;
    unsigned w=game_mob_width(kind),h=game_mob_height(kind),count=0;
    if(!game_rect_clear(g,x,y,(int)w,(int)h)) return false;
    int tx=(x+(int)w*FIXED_ONE/2)/FIXED_ONE/TILE_SIZE;
    int ground=(y+(int)h*FIXED_ONE)/FIXED_ONE/TILE_SIZE;
    if(!game_tile_solid(game_tile(g,tx,ground))) return false;
    if(kind!=MOB_ZOMBIE && ground>g->surface[tx]+1) return false;
    if(kind==MOB_ZOMBIE) {
        int dx=(x-g->spawn_x)/FIXED_ONE,dy=(y-g->spawn_y)/FIXED_ONE;
        if(dx*dx+dy*dy<=192*192) return false;
    }
    if(overlaps(x,y,(int)w,(int)h,g->player.x,g->player.y,PLAYER_W,game_player_height(&g->player)))
        return false;
    for(unsigned i=0;i<MOB_LIMIT;++i) if(g->mobs[i].active) {
        const Mob *m=&g->mobs[i];
        count+=(kind==MOB_ZOMBIE)==(m->kind==MOB_ZOMBIE);
        if(overlaps(x,y,(int)w,(int)h,m->body.x,m->body.y,
            (int)game_mob_width(m->kind),(int)game_mob_height(m->kind))) return false;
    }
    if(count>=8) return false;
    for(unsigned i=0;i<MOB_LIMIT;++i) if(!g->mobs[i].active) {
        g->mobs[i]=(Mob){.active=true,.kind=kind,.health=(uint16_t)game_mob_health(kind),
            .body={.x=x,.y=y,.facing=random_next(g)&1?1:-1,.grounded=true},.ai_ticks=120};
        return true;
    }
    return false;
}
static void try_spawn(Game *g)
{
    MobKind kind=(MobKind)(random_next(g)%MOB_COUNT);
    int px=g->player.x/FIXED_ONE/TILE_SIZE,py=g->player.y/FIXED_ONE/TILE_SIZE;
    for(unsigned attempt=0;attempt<16;++attempt) {
        int col=px+(int)(random_next(g)%49)-24;
        if(col<1 || col>=WORLD_W-1) continue;
        int row=py+(int)(random_next(g)%25)-12;
        if(row<2 || row>=WORLD_H-2 || !game_tile_solid(game_tile(g,col,row))) continue;
        int x=col*16+1,y=row*16-(int)game_mob_height(kind);
        if(x+(int)game_mob_width(kind)>g->camera_x && x<g->camera_x+SCREEN_W &&
           y+(int)game_mob_height(kind)>g->camera_y && y<g->camera_y+VIEW_H) continue;
        if(game_spawn_mob(g,kind,x*FIXED_ONE,y*FIXED_ONE)) break;
    }
}
static void mobs_step(Game *g)
{
    for(unsigned i=0;i<MOB_LIMIT;++i) {
        Mob *m=&g->mobs[i];if(!m->active) continue;
        Player *p=&m->body;
        int dx=(g->player.x-p->x)/FIXED_ONE,dy=(g->player.y-p->y)/FIXED_ONE;
        if(dx*dx+dy*dy>512*512) {*m=(Mob){0};continue;}
        if(m->attack_ticks) --m->attack_ticks;
        if(m->flee_ticks) --m->flee_ticks;
        if(m->ai_ticks) --m->ai_ticks;
        int direction=p->facing;
        if(m->kind==MOB_ZOMBIE && absolute(dx)<=256) direction=absolute(dx)<2?0:sign(dx);
        else if(m->flee_ticks) direction=-sign(dx);
        else if(!m->ai_ticks) {
            direction=(int)(random_next(g)%3)-1;m->ai_ticks=60+random_next(g)%181;
        }
        if(direction) p->facing=direction;
        p->vx=direction*(m->kind==MOB_ZOMBIE?256:m->flee_ticks?320:96);
        int w=(int)game_mob_width(m->kind),h=(int)game_mob_height(m->kind);
        if(p->grounded && p->vx) {
            bool blocked=!game_rect_clear(g,p->x+p->vx,p->y,w,h);
            if(m->kind==MOB_ZOMBIE && blocked &&
               game_rect_clear(g,p->x+p->vx,p->y-16*FIXED_ONE,w,h))
                {p->vy=-1536;p->grounded=false;}
            if(m->kind!=MOB_ZOMBIE) {
                int edge=(p->x+p->vx+(p->vx>0?w*FIXED_ONE:0))/FIXED_ONE/TILE_SIZE;
                int below=(p->y+h*FIXED_ONE)/FIXED_ONE/TILE_SIZE;
                if(blocked || !game_tile_solid(game_tile(g,edge,below))) {
                    p->facing=-p->facing;p->vx=0;m->ai_ticks=60;
                }
            }
        }
        p->vy+=64;if(p->vy>2048) p->vy=2048;
        game_move_actor(g,p,w,h);
        if(m->kind==MOB_ZOMBIE && !m->attack_ticks &&
           overlaps(p->x,p->y,w,h,g->player.x,g->player.y,PLAYER_W,game_player_height(&g->player))) {
            if(!g->hurt_ticks) {game_damage_player(g,200,true);m->attack_ticks=60;}
        }
    }
}
static void drops_step(Game *g)
{
    for(unsigned i=0;i<DROP_LIMIT;++i) {
        Drop *d=&g->drops[i];if(!d->active) continue;
        if(!d->ttl || !--d->ttl) {*d=(Drop){0};continue;}
        Player body={.x=d->x,.y=d->y,.vy=d->vy+64,.facing=1};
        if(body.vy>2048) body.vy=2048;
        game_move_actor(g,&body,6,6);d->x=body.x;d->y=body.y;d->vy=body.vy;
        if(overlaps(d->x,d->y,6,6,g->player.x-12*FIXED_ONE,g->player.y-12*FIXED_ONE,
            PLAYER_W+24,game_player_height(&g->player)+24)) {
            ItemStack one=d->stack;one.count=1;
            while(d->stack.count && game_add_stack(g,one)) --d->stack.count;
            if(!d->stack.count) *d=(Drop){0};
        }
    }
}
void game_survival_step(Game *g)
{
    if(!g->survival) return;
    mobs_step(g);drops_step(g);
    if(g->spawn_ticks) --g->spawn_ticks;
    if(!g->spawn_ticks) {g->spawn_ticks=300;try_spawn(g);}
    if(++g->hunger_ticks>=3600) {g->hunger_ticks=0;if(g->hunger) --g->hunger;}
    if(g->hunger>=18 && g->health<HEALTH_MAX) {
        if(++g->regen_ticks>=240) {
            unsigned gain=HEALTH_MAX-g->health;if(gain>100) gain=100;
            g->health=(uint16_t)(g->health+gain);g->regen_ticks=0;g->regen_cost+=gain;
            if(g->regen_cost>=400) {g->regen_cost-=400;if(g->hunger) --g->hunger;}
        }
    } else g->regen_ticks=0;
    if(!g->hunger) {
        if(++g->starve_ticks>=240) {g->starve_ticks=0;game_damage_player(g,100,false);}
    } else g->starve_ticks=0;
    if(!g->health) respawn(g);
}
