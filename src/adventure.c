#include "game.h"
#include <string.h>

static int absolute(int x) {return x<0?-x:x;}
static ItemStack stack(Item item,unsigned n)
{return (ItemStack){item,(uint16_t)n,(uint16_t)game_tool_durability(item)};}
static bool notice(Game *g,Notice n)
{g->notice=n;g->notice_ticks=120;return false;}
bool game_hostile(MobKind kind) {return kind>=MOB_ZOMBIE;}
int game_chest_at(const Game *g,int x,int y)
{
    for(int i=0;i<CHEST_LIMIT;++i) if(g->chests[i].active && g->chests[i].x==x && g->chests[i].y==y) return i;
    return -1;
}
static bool vacant(const Game *g,int x,int y)
{
    if(game_tile(g,x,y)!=TILE_AIR || game_protected_tile(g,x,y)) return false;
    int32_t bx=x*16*FIXED_ONE,by=y*16*FIXED_ONE;
    const Player *p=&g->player;
    if(p->x<bx+16*FIXED_ONE && p->x+PLAYER_W*FIXED_ONE>bx &&
       p->y<by+16*FIXED_ONE && p->y+game_player_height(p)*FIXED_ONE>by) return false;
    for(unsigned i=0;i<MOB_LIMIT;++i) if(g->mobs[i].active) {
        const Mob *m=&g->mobs[i];
        if(m->body.x<bx+16*FIXED_ONE && m->body.x+(int)game_mob_width(m->kind)*FIXED_ONE>bx &&
           m->body.y<by+16*FIXED_ONE && m->body.y+(int)game_mob_height(m->kind)*FIXED_ONE>by) return false;
    }
    return true;
}
bool game_adventure_place(Game *g,Tile tile,int x,int y)
{
    if(tile==TILE_TORCH) {
        for(unsigned i=0;i<TORCH_LIMIT;++i) if(!g->torches[i].active) {
            g->torches[i]=(Torch){true,x,y};return game_set_tile(g,x,y,tile);
        }
        return notice(g,NOTICE_CONTAINER_LIMIT);
    }
    if(tile==TILE_CHEST) {
        for(unsigned i=0;i<CHEST_LIMIT;++i) if(!g->chests[i].active) {
            g->chests[i]=(Chest){.active=true,.x=x,.y=y};return game_set_tile(g,x,y,tile);
        }
        return notice(g,NOTICE_CONTAINER_LIMIT);
    }
    if(tile==TILE_DOOR_BOTTOM) {
        if(!vacant(g,x,y-1) || !game_tile_solid(game_tile(g,x,y+1))) return notice(g,NOTICE_NO_SPACE);
        game_set_tile(g,x,y,TILE_DOOR_BOTTOM);game_set_tile(g,x,y-1,TILE_DOOR_TOP);return true;
    }
    if(tile==TILE_BED_LEFT) {
        if(!vacant(g,x+1,y) || !game_tile_solid(game_tile(g,x,y+1)) ||
           !game_tile_solid(game_tile(g,x+1,y+1))) return notice(g,NOTICE_NO_SPACE);
        game_set_tile(g,x,y,TILE_BED_LEFT);game_set_tile(g,x+1,y,TILE_BED_RIGHT);return true;
    }
    return false;
}
bool game_adventure_break(Game *g,int x,int y)
{
    Tile tile=game_tile(g,x,y);Item drop=ITEM_NONE;int ox=x,oy=y,chest=-1;
    switch(tile) {
    case TILE_TORCH:drop=ITEM_TORCH;break;
    case TILE_CHEST:drop=ITEM_CHEST;chest=game_chest_at(g,x,y);if(chest<0) return false;break;
    case TILE_DOOR_TOP:case TILE_OPEN_TOP:++y; /* canonical lower half */
        /* fall through */
    case TILE_DOOR_BOTTOM:case TILE_OPEN_BOTTOM:drop=ITEM_DOOR;ox=x;oy=y-1;break;
    case TILE_BED_RIGHT:--x;
        /* fall through */
    case TILE_BED_LEFT:drop=ITEM_BED;ox=x+1;oy=y;break;
    default:return false;
    }
    ItemStack probe[TOTAL_SLOTS];memcpy(probe,g->slots,sizeof(probe));
    if(!game_insert_stack(probe,stack(drop,1))) return notice(g,NOTICE_FULL);
    if(chest>=0) for(unsigned i=0;i<CHEST_SLOTS;++i)
        if(g->chests[chest].slots[i].count && !game_insert_stack(probe,g->chests[chest].slots[i]))
            return notice(g,NOTICE_FULL);
    memcpy(g->slots,probe,sizeof(probe));
    if(chest>=0) g->chests[chest]=(Chest){0};
    if(tile==TILE_TORCH) for(unsigned i=0;i<TORCH_LIMIT;++i)
        if(g->torches[i].active && g->torches[i].x==x && g->torches[i].y==y) g->torches[i]=(Torch){0};
    if(drop==ITEM_BED && g->bed_spawn && g->bed_x==x && g->bed_y==y) g->bed_spawn=false;
    game_set_tile(g,x,y,TILE_AIR);game_set_tile(g,ox,oy,TILE_AIR);
    game_find_relics(g);++g->revision;return true;
}
bool game_protected_tile(const Game *g,int x,int y)
{
    if(!g->ruins) return false;
    return x>=ARENA_LEFT && x<=ARENA_RIGHT && y>=ARENA_TOP && y<=ARENA_FLOOR &&
        (x==ARENA_LEFT || x==ARENA_RIGHT || y==ARENA_TOP || y==ARENA_FLOOR);
}
bool game_adventure_interact(Game *g,int x,int y)
{
    Tile tile=game_tile(g,x,y);
    if(tile>=TILE_DOOR_BOTTOM && tile<=TILE_OPEN_TOP) {
        bool open=tile==TILE_OPEN_BOTTOM || tile==TILE_OPEN_TOP;
        if(tile==TILE_DOOR_TOP || tile==TILE_OPEN_TOP) ++y;
        if(open) {
            /* Temporarily clear the transparent halves for the common overlap test. */
            game_set_tile(g,x,y,TILE_AIR);game_set_tile(g,x,y-1,TILE_AIR);
            bool clear=vacant(g,x,y) && vacant(g,x,y-1);
            game_set_tile(g,x,y,clear?TILE_DOOR_BOTTOM:TILE_OPEN_BOTTOM);
            game_set_tile(g,x,y-1,clear?TILE_DOOR_TOP:TILE_OPEN_TOP);
            if(!clear) notice(g,NOTICE_NO_SPACE);
        } else {game_set_tile(g,x,y,TILE_OPEN_BOTTOM);game_set_tile(g,x,y-1,TILE_OPEN_TOP);}
        ++g->revision;return true;
    }
    if(tile==TILE_BED_LEFT || tile==TILE_BED_RIGHT) {
        if(tile==TILE_BED_RIGHT) --x;
        if(!game_rect_clear(g,(x*16+3)*FIXED_ONE,(y*16-PLAYER_H)*FIXED_ONE,PLAYER_W,PLAYER_H))
            {notice(g,NOTICE_NO_SPACE);return true;}
        g->bed_spawn=true;g->bed_x=x;g->bed_y=y;notice(g,NOTICE_BED_SET);++g->revision;return true;
    }
    if(tile==TILE_SEAL && g->ruins && !g->boss.gate_open) {
        unsigned found=0;
        for(unsigned i=0;i<TOTAL_SLOTS;++i) if(g->slots[i].count &&
            g->slots[i].item>=ITEM_RELIC_A && g->slots[i].item<=ITEM_RELIC_C)
                found|=1u<<(unsigned)(g->slots[i].item-ITEM_RELIC_A);
        if(found!=7) {notice(g,NOTICE_RELICS);return true;}
        for(unsigned kind=ITEM_RELIC_A;kind<=ITEM_RELIC_C;++kind)
            for(unsigned i=0;i<TOTAL_SLOTS;++i) if(g->slots[i].item==(Item)kind)
                {g->slots[i]=(ItemStack){0};break;}
        g->relics_found=7;g->boss.gate_open=true;
        for(int row=GATE_TOP;row<ARENA_FLOOR;++row) game_set_tile(g,ARENA_LEFT,row,TILE_AIR);
        notice(g,NOTICE_GATE_OPEN);++g->revision;return true;
    }
    return false;
}
void game_find_relics(Game *g)
{
    unsigned previous=g->relics_found;
    for(unsigned i=0;i<TOTAL_SLOTS+CRAFT_SLOTS;++i) {
        const ItemStack *s=i<TOTAL_SLOTS?&g->slots[i]:&g->crafting[i-TOTAL_SLOTS];
        if(s->count && s->item>=ITEM_RELIC_A && s->item<=ITEM_RELIC_C)
            g->relics_found|=1u<<(unsigned)(s->item-ITEM_RELIC_A);
    }
    if(previous!=g->relics_found) notice(g,NOTICE_RELIC_FOUND);
}
unsigned game_relic_count(const Game *g)
{return (g->relics_found&1u)+((g->relics_found>>1)&1u)+((g->relics_found>>2)&1u);}
unsigned game_artificial_light(const Game *g,int x,int y)
{
    unsigned level=0;
    for(unsigned i=0;i<TORCH_LIMIT;++i) if(g->torches[i].active) {
        int d=absolute(x-g->torches[i].x)+absolute(y-g->torches[i].y);
        if(d<=3) return 2;
        if(d<=6) level=1;
    }
    return level;
}
const char *game_progress_text(const Game *g)
{
    if(!g->ruins) return "Neue Ruinen: Neues Spiel starten";
    if(g->boss.defeated) return "Steinwaechter besiegt - Geheimraeume suchen";
    if(g->boss.active) return "Steinwaechter: Angriff abwarten, ausweichen";
    if(g->boss.gate_open) return "Offene Kammer im Osten der Basaltschicht";
    if(g->relics_found==7) return "3 Relikte zur versiegelten Kammer bringen";
    if(game_relic_count(g)==2) return "Letztes Relikt in der Basaltruine suchen";
    if(game_relic_count(g)==1) return "Tiefere Ruinen und Eisen suchen";
    return "Fackeln herstellen und alte Minen erkunden";
}
static void torch(Game *g,int x,int y)
{
    for(unsigned i=0;i<TORCH_LIMIT;++i) if(!g->torches[i].active) {
        g->torches[i]=(Torch){true,x,y};g->tiles[y][x]=TILE_TORCH;return;
    }
}
static Chest *new_chest(Game *g,int x,int y)
{
    for(unsigned i=0;i<CHEST_LIMIT;++i) if(!g->chests[i].active) {
        g->chests[i]=(Chest){.active=true,.x=x,.y=y};g->tiles[y][x]=TILE_CHEST;return &g->chests[i];
    }
    return NULL;
}
bool game_deliver_reward(Game *g)
{
    int x=ARENA_RIGHT-3,y=ARENA_FLOOR-1,index=game_chest_at(g,x,y);
    Chest *c=index>=0?&g->chests[index]:NULL;
    if(!c && vacant(g,x,y)) c=new_chest(g,x,y);
    if(c) for(unsigned i=0;i<CHEST_SLOTS;++i) if(!c->slots[i].count) {
        c->slots[i]=stack(ITEM_JUMP_AMULET,1);g->boss.reward_pending=false;return true;
    }
    notice(g,NOTICE_REWARD_FULL);return false;
}
static void room(Game *g,int cx,int cy,unsigned kind)
{
    int left=cx-7,right=cx+7,top=cy-3,bottom=cy+4;
    for(int y=top;y<=bottom;++y) for(int x=left;x<=right;++x)
        g->tiles[y][x]=(uint8_t)(x==left || x==right || y==top || y==bottom?
            kind==3?TILE_BASALT:TILE_STONE:TILE_AIR);
    if(kind!=3) for(int y=bottom-3;y<bottom;++y) g->tiles[y][left]=g->tiles[y][right]=TILE_AIR;
    int chest_y=bottom-1;
    if(kind>=2) {
        chest_y=bottom-3;
        for(int y=bottom-2;y<bottom;++y) for(int x=cx+2;x<=cx+4;++x)
            g->tiles[y][x]=(uint8_t)(kind==3?TILE_BASALT:TILE_STONE);
    }
    Chest *c=new_chest(g,cx+3,chest_y);
    if(c) {
        c->slots[0]=stack(ITEM_COAL,4+(g->seed+(unsigned)cx)%9);
        c->slots[1]=stack(ITEM_COOKED_PORK,2);
        if(kind>=1 && kind<=3) c->slots[2]=stack((Item)(ITEM_RELIC_A+kind-1),1);
        if(kind==1) c->slots[3]=stack(ITEM_MINER_CHARM,1);
        if(kind==2) {c->slots[3]=stack(ITEM_STONE_PICKAXE,1);c->slots[3].durability=55;}
        if(kind==3) c->slots[3]=stack(ITEM_CRYSTAL,6);
        if(!kind) {c->slots[2]=stack(ITEM_PLANKS,8);c->slots[3]=stack(ITEM_IRON_INGOT,2);}
    }
    if(kind!=3) torch(g,left+2,bottom-1);
    if(kind==2) {
        g->tiles[bottom-1][cx-3]=TILE_WORKBENCH;
        for(unsigned i=0;i<FURNACE_LIMIT;++i) if(!g->furnaces[i].active) {
            g->furnaces[i]=(Furnace){.active=true,.x=cx-1,.y=bottom-1};
            g->tiles[bottom-1][cx-1]=TILE_FURNACE;break;
        }
    }
}
static void stair(Game *g,int start,int end,int y,bool down)
{
    for(int x=start;x<=end;++x) {
        int floor=y+(down?1:-1)*(x-start)/2;
        for(int row=floor-3;row<floor;++row) game_set_tile(g,x,row,TILE_AIR);
        game_set_tile(g,x,floor,TILE_STONE);
    }
}
void game_adventure_init(Game *g)
{
    g->ruins=true;
    for(int y=72;y<WORLD_H-2;++y) for(int x=0;x<WORLD_W;++x) if(g->tiles[y][x]==TILE_STONE)
        g->tiles[y][x]=(uint8_t)((g->seed^(unsigned)(x*193+y*7919))%31==0?TILE_CRYSTAL:TILE_BASALT);
    stair(g,42,60,53,false);stair(g,100,124,53,true);stair(g,148,204,53,true);
    room(g,60,43,1);room(g,124,62,2);room(g,204,81,3);
    room(g,29,33,0);room(g,92,53,0);
    for(int y=82;y<85;++y) for(int x=212;x<ARENA_LEFT;++x) g->tiles[y][x]=TILE_AIR;
    for(int x=212;x<ARENA_LEFT;++x) g->tiles[85][x]=TILE_BASALT;
    for(int y=ARENA_TOP;y<=ARENA_FLOOR;++y) for(int x=ARENA_LEFT;x<=ARENA_RIGHT;++x)
        g->tiles[y][x]=(uint8_t)(x==ARENA_LEFT || x==ARENA_RIGHT || y==ARENA_TOP || y==ARENA_FLOOR?
            TILE_BASALT:TILE_AIR);
    for(int y=GATE_TOP;y<ARENA_FLOOR;++y) g->tiles[y][ARENA_LEFT]=TILE_SEAL;
    new_chest(g,ARENA_RIGHT-3,ARENA_FLOOR-1);
    torch(g,ARENA_LEFT+2,ARENA_FLOOR-1);torch(g,ARENA_RIGHT-2,ARENA_FLOOR-1);
    torch(g,(ARENA_LEFT+ARENA_RIGHT)/2,ARENA_FLOOR-1);
    /* A post-boss secret pocket reachable with the extra jump, away from relic paths. */
    for(int y=76;y<79;++y) for(int x=218;x<222;++x) g->tiles[y][x]=TILE_AIR;
    for(int x=218;x<222;++x) g->tiles[79][x]=TILE_BASALT;
    Chest *secret=new_chest(g,220,78);
    if(secret) {secret->slots[0]=stack(ITEM_SPEED_BOOTS,1);secret->slots[1]=stack(ITEM_CRYSTAL,12);}
}
