#include "game.h"
#include "inventory_ui.h"
#include "storage.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static Game g,snapshot,loaded;
static ItemStack stack(Item item,unsigned count)
{return (ItemStack){item,(uint16_t)count,(uint16_t)game_tool_durability(item)};}
static SlotRef inv(unsigned i) {return (SlotRef){SLOT_INVENTORY,i,0};}
static void arena(void)
{
    game_init_classic(&g,WORLD_SEED);memset(g.tiles,0,sizeof(g.tiles));memset(g.slots,0,sizeof(g.slots));
    for(int x=0;x<WORLD_W;++x) {g.tiles[20][x]=TILE_STONE;g.surface[x]=20;}
    g.player=(Player){.x=160*FIXED_ONE,.y=298*FIXED_ONE,.facing=1,.grounded=true};
    g.spawn_x=g.player.x;g.spawn_y=g.player.y;g.hurt_ticks=0;game_follow_camera(&g);
    assert(game_validate(&g));
}
static void steps(unsigned n,GameInput in) {while(n--) game_step(&g,in);}
static void generation(void)
{
    const uint32_t seeds[]={WORLD_SEED,0,55,UINT32_C(0xdeadbeef)};
    for(unsigned seed=0;seed<4;++seed) {
        game_init(&g,seeds[seed]);game_init(&snapshot,seeds[seed]);
        assert(!memcmp(&g,&snapshot,sizeof(g)) && game_validate(&g));
        assert(g.ruins && !g.boss.gate_open && !g.boss.active);
        for(Item item=ITEM_RELIC_A;item<=ITEM_RELIC_C;item=(Item)(item+1)) assert(game_item_total(&g,item)==1);
        unsigned basalt=0,crystal=0,chests=0;
        for(int y=0;y<WORLD_H;++y) for(int x=0;x<WORLD_W;++x) {
            basalt+=g.tiles[y][x]==TILE_BASALT;crystal+=g.tiles[y][x]==TILE_CRYSTAL;
        }
        for(unsigned i=0;i<CHEST_LIMIT;++i) chests+=g.chests[i].active;
        assert(basalt>1000 && crystal>50 && chests==7);
        for(unsigned i=0;i<CHEST_LIMIT;++i) if(g.chests[i].active) {
            Chest *c=&g.chests[i];
            assert(game_rect_clear(&g,(c->x*16-PLAYER_W-1)*FIXED_ONE,
                ((c->y+1)*16-PLAYER_H)*FIXED_ONE,PLAYER_W,PLAYER_H));
        }
    }
    puts("PASS: deterministic ruins, seven reachable chest interiors, unique relics, basalt/crystals and sealed arena");
}
static void recipes(void)
{
    arena();g.crafting[0]=stack(ITEM_COAL,1);g.crafting[2]=stack(ITEM_STICK,1);
    assert(game_craft_preview(&g,2).item==ITEM_TORCH && game_craft(&g,2));
    assert(game_item_total(&g,ITEM_TORCH)==4);
    const char *patterns[]={"XXXX XXXX","XX XX XX ","SSSXXXXXX","XXX S  S "};
    const Item outputs[]={ITEM_CHEST,ITEM_DOOR,ITEM_BED,ITEM_IRON_PICKAXE};
    for(unsigned r=0;r<4;++r) {
        memset(g.crafting,0,sizeof(g.crafting));
        for(unsigned i=0;i<9;++i) if(patterns[r][i]!=' ')
            g.crafting[i]=stack(patterns[r][i]=='S'?ITEM_STICK:r==3?ITEM_IRON_INGOT:ITEM_PLANKS,1);
        assert(game_craft_preview(&g,3).item==outputs[r] && game_craft(&g,3));
    }
    memset(g.crafting,0,sizeof(g.crafting));
    g.crafting[0]=g.crafting[2]=g.crafting[3]=g.crafting[5]=stack(ITEM_IRON_INGOT,1);
    g.crafting[6]=g.crafting[8]=stack(ITEM_CRYSTAL,1);
    assert(game_craft(&g,3) && game_item_total(&g,ITEM_SPEED_BOOTS)==1);
    g.crafting[1]=g.crafting[3]=g.crafting[5]=g.crafting[7]=stack(ITEM_CRYSTAL,1);
    g.crafting[4]=stack(ITEM_IRON_INGOT,1);assert(!game_craft_preview(&g,3).count);
    g.boss.defeated=true;assert(game_craft(&g,3) && game_item_total(&g,ITEM_JUMP_AMULET)==1);
    puts("PASS: torches, chest, door, bed, iron pick, speed boots and boss-gated jump amulet recipes");
}
static void base(void)
{
    arena();g.slots[0]=stack(ITEM_DOOR,3);assert(game_place(&g));
    assert(g.tiles[19][11]==TILE_DOOR_BOTTOM && g.tiles[18][11]==TILE_DOOR_TOP);
    assert(!game_rect_clear(&g,177*FIXED_ONE,298*FIXED_ONE,10,22));
    assert(game_adventure_interact(&g,11,18) && g.tiles[19][11]==TILE_OPEN_BOTTOM);
    g.player.x=178*FIXED_ONE;game_adventure_interact(&g,11,19);
    assert(g.tiles[19][11]==TILE_OPEN_BOTTOM && g.notice==NOTICE_NO_SPACE);
    g.player.x=160*FIXED_ONE;game_adventure_interact(&g,11,19);assert(g.tiles[19][11]==TILE_DOOR_BOTTOM);
    assert(game_adventure_break(&g,11,18) && g.slots[0].count==3);
    assert(g.tiles[18][11]==TILE_AIR && g.tiles[19][11]==TILE_AIR && game_validate(&g));
    g.slots[0]=stack(ITEM_BED,1);assert(game_place(&g));
    assert(g.tiles[19][11]==TILE_BED_LEFT && g.tiles[19][12]==TILE_BED_RIGHT);
    game_adventure_interact(&g,12,19);assert(g.bed_spawn);
    game_damage_player(&g,2000,false);
    assert(g.player.x==179*FIXED_ONE && g.player.y==(304-22)*FIXED_ONE && g.health==2000);
    g.player.x=500*FIXED_ONE;g.tiles[18][11]=TILE_STONE;
    game_damage_player(&g,2000,false);assert(g.player.x==163*FIXED_ONE);
    assert(game_adventure_break(&g,12,19) && !g.bed_spawn && game_item_total(&g,ITEM_BED)==1);
    assert(game_validate(&g));
    arena();g.slots[0]=stack(ITEM_CHEST,1);assert(game_place(&g));
    for(unsigned i=0;i<CHEST_SLOTS;++i) g.chests[0].slots[i]=stack(ITEM_PLANKS,64);
    for(unsigned i=0;i<TOTAL_SLOTS;++i) g.slots[i]=stack(ITEM_DIRT,64);
    g.slots[0]=stack(ITEM_STONE_PICKAXE,1);snapshot=g;
    assert(!game_adventure_break(&g,11,19) && g.chests[0].active && g.notice==NOTICE_FULL);
    assert(!memcmp(g.slots,snapshot.slots,sizeof(g.slots)) && !memcmp(g.chests,snapshot.chests,sizeof(g.chests)));
    memset(g.slots+1,0,(TOTAL_SLOTS-1)*sizeof(ItemStack));
    assert(game_adventure_break(&g,11,19) && !g.chests[0].active);
    assert(game_item_total(&g,ITEM_CHEST)==1 && game_item_total(&g,ITEM_PLANKS)==768 && game_validate(&g));
    arena();g.slots[0]=stack(ITEM_TORCH,64);
    for(unsigned i=0;i<TORCH_LIMIT;++i) {g.player.x=(int32_t)(160+i*32)*FIXED_ONE;assert(game_place(&g));}
    g.slots[0]=stack(ITEM_TORCH,1);g.player.x=3000*FIXED_ONE;
    assert(!game_place(&g) && g.slots[0].count==1 && game_validate(&g));
    puts("PASS: paired doors/beds, overlap-safe closing, bed respawn/fallback, atomic chest recovery and torch cap");
}
static void tools_accessories(void)
{
    arena();g.slots[0]=stack(ITEM_IRON_PICKAXE,1);int x,y;game_target(&g,&x,&y);
    const Tile tiles[]={TILE_STONE,TILE_COAL,TILE_ORE,TILE_BASALT,TILE_CRYSTAL};
    const unsigned times[]={15,15,30,75,60};
    for(unsigned i=0;i<5;++i) {
        g.tiles[y][x]=(uint8_t)tiles[i];
        for(unsigned tick=1;tick<=times[i];++tick) {
            steps(1,(GameInput){.mine=true});assert((g.tiles[y][x]==TILE_AIR)==(tick==times[i]));
        }
    }
    g.slots[0]=stack(ITEM_STONE_PICKAXE,1);g.tiles[y][x]=TILE_BASALT;
    steps(120,(GameInput){.mine=true});assert(g.tiles[y][x]==TILE_BASALT && g.slots[0].durability==128);
    arena();g.slots[0]=stack(ITEM_MINER_CHARM,1);
    assert(game_transfer(&g,inv(0),(SlotRef){SLOT_ACCESSORY,0,0},false));
    g.slots[1]=stack(ITEM_MINER_CHARM,1);
    assert(!game_transfer(&g,inv(1),(SlotRef){SLOT_ACCESSORY,1,0},false));
    g.slots[0]=stack(ITEM_WOOD_PICKAXE,1);game_target(&g,&x,&y);
    for(unsigned i=0;i<5;++i) {g.tiles[y][x]=TILE_LEAVES;steps(9,(GameInput){.mine=true});}
    assert(g.slots[0].durability==60 && g.mined_blocks==5);
    g.accessories[1]=stack(ITEM_SPEED_BOOTS,1);steps(30,(GameInput){.move=1});assert(g.player.vx==552);
    assert(game_toggle_sneak(&g));steps(30,(GameInput){.move=1});assert(g.player.vx==276);
    arena();g.accessories[0]=stack(ITEM_JUMP_AMULET,1);
    steps(3,(GameInput){.jump=true});assert(g.double_jump_used);
    int32_t vy=g.player.vy;steps(1,(GameInput){.jump=true});assert(g.player.vy==vy+64);
    steps(150,(GameInput){0});assert(g.player.grounded && !g.double_jump_used);
    puts("PASS: iron speeds, basalt gate, accessory slots/no duplicates, one-in-five wear, speed/Sneak and one air jump");
}
static void lighting_enemies(void)
{
    arena();assert(game_adventure_place(&g,TILE_TORCH,25,19));
    assert(game_artificial_light(&g,25,19)==2 && game_artificial_light(&g,28,19)==2);
    assert(game_artificial_light(&g,29,19)==1 && !game_artificial_light(&g,32,19));
    assert(!game_spawn_mob(&g,MOB_ZOMBIE,400*FIXED_ONE,298*FIXED_ONE));
    assert(game_spawn_mob(&g,MOB_ZOMBIE,600*FIXED_ONE,298*FIXED_ONE));
    arena();g.tiles[24][37]=TILE_STONE;
    assert(game_spawn_mob(&g,MOB_BAT,600*FIXED_ONE,400*FIXED_ONE));
    for(unsigned i=0;i<60;++i) game_survival_step(&g);
    assert(g.mobs[0].state==1 && g.mobs[0].state_ticks==24);
    for(unsigned i=0;i<24;++i) game_survival_step(&g);
    assert(g.mobs[0].state==2 && g.mobs[0].body.vx<0);
    assert(game_validate(&g));
    arena();g.player.x=384*FIXED_ONE;assert(game_spawn_mob(&g,MOB_ZOMBIE,395*FIXED_ONE,298*FIXED_ONE));
    steps(1,(GameInput){0});assert(g.mobs[0].state==1 && g.health==2000);
    steps(17,(GameInput){0});assert(g.health==2000);
    steps(1,(GameInput){0});assert(g.health==1800 && g.mobs[0].attack_ticks==60);
    g.aim_x=1;g.action_ticks=0;assert(game_attack(&g,true) && g.mobs[0].hurt_ticks==12);
    int32_t old_x=g.mobs[0].body.x;steps(1,(GameInput){0});assert(g.mobs[0].body.x>old_x);
    puts("PASS: two torch radii, hostile spawn suppression, bat perch/warning/dive and zombie windup/knockback");
}
static Mob *boss(void)
{for(unsigned i=0;i<MOB_LIMIT;++i) if(g.mobs[i].active && g.mobs[i].kind==MOB_WARDEN) return &g.mobs[i];return NULL;}
static void roundtrip(void)
{
    SaveStore store={.root="build/native/adventure-save"};SaveInfo info;
    assert(game_validate(&g));snapshot=g;
    assert(save_slot_write(&store,1,&g,&loaded)==SAVE_OK);
    assert(save_slot_load(&store,1,&loaded,&info)==SAVE_OK && game_validate(&loaded));
    assert(!memcmp(g.chests,loaded.chests,sizeof(g.chests)) && !memcmp(g.torches,loaded.torches,sizeof(g.torches)));
    assert(!memcmp(&g.boss,&loaded.boss,sizeof(g.boss)) && !memcmp(g.accessories,loaded.accessories,sizeof(g.accessories)));
    assert(g.relics_found==loaded.relics_found && g.bed_spawn==loaded.bed_spawn && g.mined_blocks==loaded.mined_blocks);
}
static void quest_boss(void)
{
    game_init(&g,WORLD_SEED);
    assert(game_adventure_interact(&g,ARENA_LEFT,84) && !g.boss.gate_open);
    for(unsigned kind=ITEM_RELIC_A;kind<=ITEM_RELIC_C;++kind) {
        bool found=false;
        for(unsigned c=0;c<CHEST_LIMIT;++c) for(unsigned i=0;i<CHEST_SLOTS;++i)
            if(g.chests[c].active && g.chests[c].slots[i].item==(Item)kind) {
                SlotRef destination=kind==ITEM_RELIC_A?(SlotRef){SLOT_CRAFT,0,0}:inv(3+kind-ITEM_RELIC_A);
                assert(game_transfer(&g,(SlotRef){SLOT_CHEST,i,c},destination,false));found=true;
            }
        assert(found);
    }
    assert(game_relic_count(&g)==3 && game_close_crafting(&g));
    assert(game_relic_count(&g)==3 && game_adventure_interact(&g,ARENA_LEFT,84) && g.boss.gate_open);
    for(int y=GATE_TOP;y<ARENA_FLOOR;++y) assert(g.tiles[y][ARENA_LEFT]==TILE_AIR);
    for(unsigned kind=ITEM_RELIC_A;kind<=ITEM_RELIC_C;++kind) assert(!game_item_total(&g,(Item)kind));
    g.player=(Player){.x=233*16*FIXED_ONE,.y=(ARENA_FLOOR*16-PLAYER_H)*FIXED_ONE,.facing=1,.grounded=true};
    game_follow_camera(&g);game_boss_step(&g);
    assert(g.boss.active && boss() && boss()->health==10000);
    roundtrip();
    while(g.boss.phase==0) game_boss_step(&g);
    assert(g.boss.phase==1);
    while(g.boss.phase==1) game_boss_step(&g);
    assert(g.boss.phase==2);
    while(g.boss.phase==2) game_boss_step(&g);
    assert(g.boss.phase==3);
    g.player.y=(ARENA_FLOOR*16-PLAYER_H-48)*FIXED_ONE;unsigned health=g.health;
    while(g.boss.phase==3) game_boss_step(&g);
    assert(g.boss.shock_ticks && g.health==health);
    g.player.x=g.boss.shock_x+24*FIXED_ONE;g.player.y=(ARENA_FLOOR*16-PLAYER_H)*FIXED_ONE;g.hurt_ticks=0;
    for(unsigned i=0;i<10;++i) game_boss_step(&g);
    assert(g.health==health-300);
    game_damage_player(&g,HEALTH_MAX,false);assert(!g.boss.active && !boss() && g.boss.gate_open);
    g.player.x=233*16*FIXED_ONE;g.player.y=(ARENA_FLOOR*16-PLAYER_H)*FIXED_ONE;game_boss_step(&g);
    assert(boss() && boss()->health==10000);
    int chest=game_chest_at(&g,ARENA_RIGHT-3,ARENA_FLOOR-1);assert(chest>=0);
    for(unsigned i=0;i<CHEST_SLOTS;++i) g.chests[chest].slots[i]=stack(ITEM_DIRT,64);
    g.slots[0]=stack(ITEM_IRON_SWORD,1);g.selected=0;g.aim_x=1;g.aim_y=0;
    unsigned hits=0;
    while(boss()) {
        Mob *m=boss();g.player.x=m->body.x-(PLAYER_W+1)*FIXED_ONE;g.player.y=m->body.y+2*FIXED_ONE;
        g.action_ticks=0;assert(game_attack(&g,true));assert(++hits<=17);
    }
    assert(hits==17 && g.boss.defeated && !g.boss.active && g.boss.reward_pending && !game_item_total(&g,ITEM_JUMP_AMULET));
    roundtrip();g.chests[chest].slots[0]=(ItemStack){0};game_boss_step(&g);
    assert(!g.boss.reward_pending && game_item_total(&g,ITEM_JUMP_AMULET)==1);
    assert(game_transfer(&g,(SlotRef){SLOT_CHEST,0,(unsigned)chest},(SlotRef){SLOT_ACCESSORY,0,0},false));
    assert(game_has_accessory(&g,ITEM_JUMP_AMULET));roundtrip();
    puts("PASS: all relics, permanent seal unlock, boss attacks/dodging, death reset, victory/reward and active/finished save round trips");
}
static void maximum_state(void)
{
    arena();
    for(unsigned i=0;i<CHEST_LIMIT;++i) {
        int x=40+(int)i;assert(game_adventure_place(&g,TILE_CHEST,x,19));
        for(unsigned j=0;j<CHEST_SLOTS;++j) g.chests[i].slots[j]=stack(ITEM_COAL,64);
    }
    assert(!game_adventure_place(&g,TILE_CHEST,60,19));
    for(unsigned i=0;i<TORCH_LIMIT;++i) assert(game_adventure_place(&g,TILE_TORCH,40+(int)i,18));
    for(unsigned i=0;i<FURNACE_LIMIT;++i) {
        int x=150+(int)i;g.tiles[19][x]=TILE_FURNACE;
        g.furnaces[i]=(Furnace){.active=true,.x=x,.y=19,.slots={{ITEM_RAW_IRON,64,0},{ITEM_COAL,8,0},{0}}};
    }
    for(unsigned i=0;i<8;++i) {
        assert(game_spawn_mob(&g,MOB_PIG,(int32_t)(300+i*18)*FIXED_ONE,308*FIXED_ONE));
        assert(game_spawn_mob(&g,MOB_ZOMBIE,(int32_t)(450+i*12)*FIXED_ONE,298*FIXED_ONE));
    }
    for(unsigned i=0;i<DROP_LIMIT;++i)
        game_drop(&g,stack(ITEM_WOOD_SWORD,1),(int32_t)(100+i*20)*FIXED_ONE,240*FIXED_ONE);
    roundtrip();
    for(unsigned i=0;i<600;++i) {game_step(&g,(GameInput){0});if(i%30==0) assert(game_validate(&g));}
    roundtrip();puts("PASS: combined 16 chests, 64 torches, 32 furnaces, 16 mobs and 64 drops saved and simulated");
}
static void long_run(void)
{
    game_init(&g,WORLD_SEED);uint32_t random=123456;
    for(unsigned i=0;i<18000;++i) {
        random=random*1664525u+1013904223u;
        game_step(&g,(GameInput){.move=(int)((random>>12)%3)-1,.jump=i%137==0,
            .aim_x=(int)((random>>8)%3)-1,.aim_y=(int)((random>>10)%3)-1,
            .mine=i%3==0,.use_held=i%5==0});
        if(i%30==0) assert(game_validate(&g));
    }
    assert(game_validate(&g));puts("PASS: 18,000 adventure-world ticks with repeated complete state validation");
}
int main(void)
{generation();recipes();base();tools_accessories();lighting_enemies();quest_boss();maximum_state();long_run();puts("All adventure checks passed.");return 0;}
