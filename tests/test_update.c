#include "game.h"
#include "inventory_ui.h"
#include "ui_layout.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static Game g,before;
static ItemStack stack(Item item,unsigned count)
{return (ItemStack){item,(uint16_t)count,(uint16_t)game_tool_durability(item)};}
static void arena(void)
{
    game_init_classic(&g,WORLD_SEED);memset(g.tiles,0,sizeof(g.tiles));
    memset(g.slots,0,sizeof(g.slots));memset(g.mobs,0,sizeof(g.mobs));
    for(int x=0;x<WORLD_W;++x) {
        g.surface[x]=20;
        for(int y=20;y<WORLD_H;++y) g.tiles[y][x]=TILE_STONE;
    }
    g.player=(Player){.x=100*FIXED_ONE,.y=(320-PLAYER_H)*FIXED_ONE,.facing=1,.grounded=true};
    g.aim_x=1;g.aim_y=0;
}
static Mob *mob(unsigned index,MobKind kind,int x,int y)
{
    g.mobs[index]=(Mob){.active=true,.kind=kind,.health=(uint16_t)game_mob_health(kind),
        .body={.x=x*FIXED_ONE,.y=y*FIXED_ONE,.facing=-1,.grounded=true}};
    return &g.mobs[index];
}
static void separated(void)
{
    assert(game_rect_clear(&g,g.player.x,g.player.y,PLAYER_W,game_player_height(&g.player)));
    for(unsigned i=0;i<MOB_LIMIT;++i) if(g.mobs[i].active) {
        assert(!game_mob_touches_player(&g,&g.mobs[i],0));
        assert(game_rect_clear(&g,g.mobs[i].body.x,g.mobs[i].body.y,
            (int)game_mob_width(g.mobs[i].kind),(int)game_mob_height(g.mobs[i].kind)));
    }
}
static void collisions(void)
{
    arena();Mob *m=mob(0,MOB_PIG,110,308);
    g.player.vx=FIXED_ONE;g.player.vy=64;game_move_player(&g);
    assert(g.player.x==101*FIXED_ONE && m->body.x==111*FIXED_ONE);separated();
    m->body.vx=-FIXED_ONE;m->body.vy=64;game_move_mob(&g,m);
    assert(m->body.x==111*FIXED_ONE);separated();
    g.action_ticks=0;assert(game_attack(&g,true) && m->health==500);
    arena();g.player.x=104*FIXED_ONE;m=mob(0,MOB_PIG,114,308);g.tiles[19][8]=TILE_STONE;
    g.player.vx=480;g.player.vy=64;game_move_player(&g);
    assert(g.player.x==104*FIXED_ONE && m->body.x==114*FIXED_ONE);separated();
    arena();m=mob(0,MOB_WARDEN,110,296);g.player.vx=480;g.player.vy=64;
    game_move_player(&g);assert(g.player.x==100*FIXED_ONE && m->body.x==110*FIXED_ONE);separated();
    arena();for(unsigned i=0;i<MOB_LIMIT;++i) mob(i,i%2?MOB_COW:MOB_PIG,110,i%2?306:308);
    g.tiles[19][8]=TILE_STONE;g.player.vx=2048;g.player.vy=64;game_move_player(&g);
    assert(g.player.x==102*FIXED_ONE);separated();
    for(unsigned i=0;i<MOB_LIMIT;++i) assert(g.mobs[i].body.x==112*FIXED_ONE);
    /* Leftward pushes, vertical landing, low overhead mobs and repair. */
    arena();m=mob(0,MOB_PIG,86,308);g.player.vx=-256;g.player.vy=64;game_move_player(&g);
    assert(g.player.x==99*FIXED_ONE && m->body.x==85*FIXED_ONE);separated();
    arena();mob(0,MOB_CHICKEN,100,310);g.player.y=287*FIXED_ONE;g.player.vy=512;
    game_move_player(&g);assert(g.player.y==288*FIXED_ONE && g.player.grounded);separated();
    arena();assert(game_toggle_sneak(&g));mob(0,MOB_BAT,100,297);
    assert(!game_toggle_sneak(&g) && g.player.sneaking);separated();
    arena();m=mob(0,MOB_PIG,100,308);before=g;
    assert(game_repair_overlaps(&g) && m->active && !game_repair_overlaps(&g));
    assert(!memcmp(&g.player,&before.player,sizeof(Player)));separated();
    arena();m=mob(0,MOB_BAT,110,305);m->state=2;m->state_ticks=20;m->body.vx=-512;
    game_survival_step(&g);assert(g.health==1900 && m->attack_ticks==60);separated();
    /* Sneak still stops at a ledge while a mob can be pushed. */
    arena();g.player.x=118*FIXED_ONE;assert(game_toggle_sneak(&g));
    for(int y=20;y<WORLD_H;++y) for(int x=8;x<WORLD_W;++x) g.tiles[y][x]=TILE_AIR;
    mob(0,MOB_CHICKEN,128,310);
    for(unsigned i=0;i<30;++i) {g.player.vx=240;g.player.vy=64;game_move_player(&g);separated();}
    assert(g.player.x<128*FIXED_ONE);
    puts("PASS: contact hits, bidirectional pushing, walls, 16 stacked mobs, immovable boss, vertical contact, Sneak and overlap repair");
}
static unsigned count_ore(const Game *game)
{
    unsigned n=0;
    for(int y=0;y<WORLD_H;++y) for(int x=0;x<WORLD_W;++x)
        if(game->tiles[y][x]==TILE_DIAMOND_ORE) {assert(y>=72 && y<94 && !game_protected_tile(game,x,y));++n;}
    return n;
}
static void diamonds(void)
{
    for(uint32_t seed=0;seed<12;++seed) {
        game_init(&g,seed);assert(g.diamonds_generated && count_ore(&g)==96 && game_validate(&g));
        game_init(&before,seed);assert(!memcmp(g.tiles,before.tiles,sizeof(g.tiles)) && g.rng==before.rng);
        assert(!game_populate_diamonds(&g));
    }
    arena();g.diamonds_generated=false;g.tiles[75][10]=g.tiles[75][11]=g.tiles[75][12]=TILE_BASALT;
    before=g;assert(game_populate_diamonds(&g)==3 && count_ore(&g)==3);
    for(int y=0;y<WORLD_H;++y) for(int x=0;x<WORLD_W;++x)
        if(before.tiles[y][x]!=TILE_BASALT) assert(before.tiles[y][x]==g.tiles[y][x]);
    g.tiles[75][13]=TILE_BASALT;assert(!game_populate_diamonds(&g));
    arena();g.diamonds_generated=false;assert(!game_populate_diamonds(&g) && g.diamonds_generated);
    const Item outputs[]={ITEM_DIAMOND_HELMET,ITEM_DIAMOND_CHEST,ITEM_DIAMOND_LEGS,ITEM_DIAMOND_BOOTS,
        ITEM_DIAMOND_PICKAXE,ITEM_DIAMOND_SWORD};
    const char *patterns[]={"DDDD D   ","D DDDDDDD","DDDD DD D","D DD D   ","DDD S  S ","D  D  S  "};
    const unsigned durability[]={256,512,384,256,512,512};
    for(unsigned i=0;i<6;++i) {
        arena();for(unsigned j=0;j<9;++j) if(patterns[i][j]!=' ')
            g.crafting[j]=stack(patterns[i][j]=='D'?ITEM_DIAMOND:ITEM_STICK,1);
        ItemStack result=game_craft_preview(&g,3);assert(result.item==outputs[i] && result.durability==durability[i]);
        assert(game_craft(&g,3) && game_item_total(&g,outputs[i])==1 && !game_item_total(&g,ITEM_DIAMOND));
    }
    for(unsigned tier=0;tier<3;++tier) {
        arena();g.slots[3]=stack(tier==0?ITEM_STONE_PICKAXE:tier==1?ITEM_IRON_PICKAXE:ITEM_DIAMOND_PICKAXE,1);
        g.selected=3;g.tiles[19][7]=TILE_DIAMOND_ORE;unsigned required=tier==1?90:60;
        for(unsigned step=0;step<required-1;++step) game_step(&g,(GameInput){.mine=true});
        assert(g.tiles[19][7]==TILE_DIAMOND_ORE);
        game_step(&g,(GameInput){.mine=true});
        assert(tier==0?g.tiles[19][7]==TILE_DIAMOND_ORE:
            g.tiles[19][7]==TILE_AIR && game_item_total(&g,ITEM_DIAMOND)==1);
    }
    arena();g.slots[3]=stack(ITEM_DIAMOND_PICKAXE,1);g.slots[3].durability=1;g.selected=3;
    g.tiles[19][7]=TILE_DIAMOND_ORE;
    for(unsigned step=0;step<60;++step) game_step(&g,(GameInput){.mine=true});
    assert(!g.slots[3].count && game_item_total(&g,ITEM_DIAMOND)==1);
    arena();g.slots[3]=stack(ITEM_DIAMOND_SWORD,1);g.slots[3].durability=1;g.selected=3;
    Mob *m=mob(0,MOB_ZOMBIE,110,298);assert(game_attack(&g,true) && m->health==500 && !g.slots[3].count);
    puts("PASS: deterministic 96-ore worlds, once-only old basalt migration, no-stone conversion, six recipes, fourth-slot mining and last-use diamond tools");
}
static void difficulty(void)
{
    const unsigned damages[]={150,200,250};
    for(unsigned d=0;d<DIFFICULTY_COUNT;++d) {
        arena();game_set_difficulty(&g,(Difficulty)d);game_damage_player(&g,200,true);
        assert(g.health==HEALTH_MAX-damages[d]);
        arena();game_set_difficulty(&g,(Difficulty)d);
        for(unsigned i=0;i<ARMOR_SLOTS;++i) {g.armor[i]=stack((Item)(ITEM_DIAMOND_HELMET+i),1);g.armor[i].durability=1;}
        game_damage_player(&g,200,true);assert(g.health==HEALTH_MAX-damages[d]*30/100);
        for(unsigned i=0;i<ARMOR_SLOTS;++i) assert(!g.armor[i].count);
        g.hurt_ticks=0;uint16_t hp=g.health;game_damage_player(&g,100,false);assert(g.health==hp-100);
        arena();game_set_difficulty(&g,(Difficulty)d);
        const DifficultyRules *r=game_difficulty_rules(g.difficulty);
        for(unsigned i=0;i<r->exertion-1;++i) game_exert(&g);
        assert(g.hunger==20);game_exert(&g);assert(g.hunger==19);
        arena();game_set_difficulty(&g,(Difficulty)d);
        for(unsigned i=0;i<r->hunger_ticks-1;++i) game_survival_step(&g);
        assert(g.hunger==20);game_survival_step(&g);assert(g.hunger==19 && game_validate(&g));
    }
    arena();g.hunger_ticks=1800;g.exertion=10;g.spawn_ticks=150;
    before=g;game_set_difficulty(&g,DIFFICULTY_EASY);
    assert(g.hunger_ticks==2700 && g.exertion==15 && g.spawn_ticks==240 && g.health==before.health);
    game_set_difficulty(&g,DIFFICULTY_HARD);assert(g.hunger_ticks==1350 && g.spawn_ticks==90);
    arena();g.spawn_x=0;g.spawn_y=0;game_set_difficulty(&g,DIFFICULTY_EASY);
    for(unsigned i=0;i<4;++i) assert(game_spawn_mob(&g,MOB_ZOMBIE,(400+(int)i*32)*FIXED_ONE,298*FIXED_ONE));
    assert(!game_spawn_mob(&g,MOB_ZOMBIE,600*FIXED_ONE,298*FIXED_ONE));
    game_set_difficulty(&g,DIFFICULTY_HARD);
    for(unsigned i=4;i<8;++i) assert(game_spawn_mob(&g,MOB_ZOMBIE,(400+(int)i*32)*FIXED_ONE,298*FIXED_ONE));
    game_set_difficulty(&g,DIFFICULTY_EASY);assert(game_validate(&g));
    for(unsigned i=0;i<8;++i) assert(g.mobs[i].active);
    puts("PASS: difficulty damage before 70% armor, last-hit protection, unchanged starvation, exact hunger/action periods, proportional switches and spawn-only caps");
}
static void hotbar(void)
{
    arena();assert(HOTBAR_SLOTS==4 && TOTAL_SLOTS==16);
    for(unsigned i=0;i<HOTBAR_SLOTS;++i) {
        UIRect r=ui_hotbar_rect(i,false);assert(r.x>=0 && r.x+r.w<=SCREEN_W);
        assert(ui_hotbar_hit(r.x+r.w/2,r.y+r.h/2)==(int)i);
        assert(ui_hotbar_hit(r.x+r.w,r.y+r.h)==-1);
    }
    InventoryUI ui={.cursor=4,.source=0,.craft_width=2};g.slots[3]=stack(ITEM_DIAMOND_SWORD,1);g.slots[3].durability=7;
    game_select_hotbar(&g,2);ui_inventory_key(&g,&ui,UI_HOTBAR_POWER);
    assert(ui.cursor==4 && ui.source==-1 && g.selected==2 && g.slots[4].durability==7 && !g.slots[3].count);
    UISlot slots[UI_MAX_SLOTS];ui.chest=1;unsigned n=ui_slots(&ui,slots);assert(n==28);
    for(unsigned i=0;i<n;++i) for(unsigned j=0;j<i;++j) assert(slots[i].id!=slots[j].id);
    puts("PASS: four hotbar rectangles, 16 slots, power-key quickmove preserves durability/cursor/selection and unique UI IDs");
}
static void crowded_run(void)
{
    arena();uint32_t random=17;
    for(unsigned i=0;i<MOB_LIMIT;++i) {
        MobKind kind=i<8?(MobKind)(i%3):i%2?MOB_BAT:MOB_ZOMBIE;
        mob(i,kind,80+(int)(i%8)*12,320-(int)game_mob_height(kind));
    }
    for(unsigned i=0;i<6000;++i) {
        if(i%1000==0) game_set_difficulty(&g,(Difficulty)(i/1000%3));
        random=random*1664525u+1013904223u;
        game_step(&g,(GameInput){.move=(int)((random>>9)%3)-1,.jump=i%29==0,.toggle_sneak=i%211==0,
            .use_held=i%7==0});
        separated();assert(game_validate(&g));
    }
    puts("PASS: 6000 crowded combat ticks across all difficulties preserve terrain clearance and zero player/mob overlap");
}
int main(void) {collisions();diamonds();difficulty();hotbar();crowded_run();return 0;}
