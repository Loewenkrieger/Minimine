#include "game.h"
#include "inventory_ui.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static Game g,before;
static ItemStack stack(Item item,unsigned n)
{return (ItemStack){item,(uint16_t)n,(uint16_t)game_tool_durability(item)};}
static void arena(void)
{
    game_init(&g,WORLD_SEED);
    memset(g.tiles,0,sizeof(g.tiles));memset(g.slots,0,sizeof(g.slots));
    for(int x=0;x<WORLD_W;++x) {g.tiles[20][x]=TILE_STONE;g.surface[x]=20;}
    g.player=(Player){.x=160*FIXED_ONE,.y=(320-PLAYER_H)*FIXED_ONE,.facing=1,.grounded=true};
    g.spawn_x=g.player.x;g.spawn_y=g.player.y;g.hurt_ticks=0;
    game_follow_camera(&g);assert(game_validate(&g));
}
static void steps(unsigned n,GameInput in) {while(n--) game_step(&g,in);}
static void aim(void)
{
    arena();
    for(int offset=0;offset<32;++offset) for(int sneak=0;sneak<2;++sneak) {
        g.player.sneaking=sneak!=0;int h=game_player_height(&g.player);
        g.player.x=(160+offset)*FIXED_ONE;g.player.y=(320-h)*FIXED_ONE;
        for(int side=-1;side<=1;side+=2) for(int dy=-1;dy<=1;++dy) {
            g.aim_x=side;g.aim_y=dy;g.aim_manual=true;int x,y;game_target(&g,&x,&y);
            assert(x==(side<0?(160+offset)/16-1:(160+offset+PLAYER_W-1)/16+1));
            int expected=dy<0?(320-h)/16-1:dy>0?19:(320-h+(sneak?3:5))/16;
            assert(y==expected);
        }
        g.aim_x=0;g.aim_y=1;int x,y;game_target(&g,&x,&y);assert(y==20);
        g.aim_y=-1;game_target(&g,&x,&y);assert(y==(320-h)/16-1);
    }
    arena();game_step(&g,(GameInput){.move=1,.aim_x=-1,.aim_y=1});
    assert(g.aim_x==-1 && g.aim_manual);
    game_step(&g,(GameInput){0});assert(g.aim_x==-1 && g.aim_manual);
    game_step(&g,(GameInput){.move=1});assert(g.aim_x==1 && !g.aim_manual);
    puts("PASS: adjacent targeting at 32 block offsets, all six side targets, feet/head and Sneak");
}
static void transfers(void)
{
    arena();g.slots[0]=stack(ITEM_STONE,32);g.slots[4]=stack(ITEM_STONE,60);g.selected=2;
    InventoryUI ui={.cursor=4,.source=2,.craft_width=2};
    ui_inventory_key(&g,&ui,UI_HOTBAR_X);
    assert(g.slots[0].count==28 && g.slots[4].count==64 && ui.cursor==4 && g.selected==2 && ui.source==-1);
    ui.cursor=3;ui_inventory_key(&g,&ui,UI_HOTBAR_X);assert(g.slots[3].count==28 && !g.slots[0].count);
    g.slots[0]=stack(ITEM_WOOD_SWORD,1);g.slots[0].durability=7;
    ui_inventory_key(&g,&ui,UI_HOTBAR_X);assert(g.slots[3].durability==7 && g.slots[0].item==ITEM_STONE);
    ui.cursor=UI_RESULT;before=g;ui_inventory_key(&g,&ui,UI_HOTBAR_X);
    assert(!memcmp(g.slots,before.slots,sizeof(g.slots)));
    ui.cursor=CRAFT_FIRST;ui_inventory_key(&g,&ui,UI_HOTBAR_X);assert(g.crafting[0].count==28);
    ui.source=3;ui_inventory_key(&g,&ui,UI_TOGGLE_ARMOR);assert(ui.armor && ui.source==-1 && g.crafting[0].count==28);
    g.slots[1]=stack(ITEM_IRON_HELMET,1);g.slots[1].durability=13;
    ui_inventory_key(&g,&ui,UI_HOTBAR_Y);assert(g.armor[0].durability==13 && !g.slots[1].count);
    g.slots[1]=stack(ITEM_IRON_BOOTS,1);ui_inventory_key(&g,&ui,UI_HOTBAR_Y);
    assert(g.armor[0].item==ITEM_IRON_HELMET && g.slots[1].item==ITEM_IRON_BOOTS);
    ui_inventory_key(&g,&ui,UI_TOGGLE_ARMOR);assert(ui.cursor==CRAFT_FIRST && !ui.armor);
    assert(ui_inventory_key(&g,&ui,UI_CLOSE) && !g.crafting[0].count);
    /* Every displayed slot must be reachable through the four arrows. */
    for(unsigned mode=0;mode<4;++mode) {
        ui=(InventoryUI){.source=-1,.craft_width=mode==1?3:2,.armor=mode==2,.furnace=mode==3?1:0};
        UISlot slots[UI_MAX_SLOTS];unsigned n=ui_slots(&ui,slots);bool seen[32]={0};seen[0]=true;
        for(unsigned pass=0;pass<n;++pass) for(unsigned i=0;i<n;++i) if(seen[slots[i].id])
            for(unsigned d=UI_LEFT;d<=UI_DOWN;++d) {
                ui.cursor=slots[i].id;ui_inventory_key(&g,&ui,(UICommand)d);assert(ui.cursor<32);seen[ui.cursor]=true;
            }
        for(unsigned i=0;i<n;++i) assert(seen[slots[i].id]);
    }
    puts("PASS: hotbar quickmove/merge/swap, invalid outputs, worn armor, ingredients and menu reachability");
}
static void recipe(const char *mask,Item material,Item output)
{
    memset(g.crafting,0,sizeof(g.crafting));
    for(unsigned i=0;mask[i];++i) if(mask[i]=='X') g.crafting[i]=stack(material,2);
    ItemStack out=game_craft_preview(&g,3);assert(out.item==output && out.count==1);
    assert(out.durability==game_tool_durability(output));assert(game_craft(&g,3));
    for(unsigned i=0;mask[i];++i) if(mask[i]=='X') assert(g.crafting[i].count==1);
    assert(game_close_crafting(&g));
}
static void recipes(void)
{
    arena();recipe("XXXX XXXX",ITEM_STONE,ITEM_FURNACE);
    arena();recipe("XXXX X",ITEM_IRON_INGOT,ITEM_IRON_HELMET);
    arena();recipe("X XXXXXXX",ITEM_IRON_INGOT,ITEM_IRON_CHEST);
    arena();recipe("XXXX XX X",ITEM_IRON_INGOT,ITEM_IRON_LEGS);
    arena();recipe("X XX X",ITEM_IRON_INGOT,ITEM_IRON_BOOTS);
    const Item materials[]={ITEM_PLANKS,ITEM_STONE,ITEM_IRON_INGOT};
    for(unsigned i=0;i<3;++i) for(unsigned col=0;col<3;++col) {
        arena();g.crafting[col]=g.crafting[col+3]=stack(materials[i],1);g.crafting[col+6]=stack(ITEM_STICK,1);
        Item sword=(Item)(ITEM_WOOD_SWORD+i);assert(game_craft_preview(&g,3).item==sword);
        assert(game_craft(&g,3) && game_item_total(&g,sword)==1);
    }
    puts("PASS: furnace, all swords and all four shifted iron armor recipes");
}
static void mining(void)
{
    const Item tools[]={ITEM_NONE,ITEM_WOOD_SWORD,ITEM_WOOD_PICKAXE,ITEM_STONE_PICKAXE};
    for(unsigned t=0;t<4;++t) for(unsigned tile=TILE_ORE;tile<=TILE_COAL;tile+=TILE_COAL-TILE_ORE) {
        arena();if(tools[t]) g.slots[0]=stack(tools[t],1);
        int x,y;game_target(&g,&x,&y);g.tiles[y][x]=(uint8_t)tile;
        unsigned ticks=tile==TILE_ORE?(t==3?60:120):(t==3?30:60);
        for(unsigned i=1;i<=ticks;++i) {steps(1,(GameInput){.mine=true});assert((g.tiles[y][x]==TILE_AIR)==(i==ticks));}
        assert(game_item_total(&g,ITEM_RAW_IRON)==(unsigned)(tile==TILE_ORE && t==3));
        assert(game_item_total(&g,ITEM_COAL)==(unsigned)(tile==TILE_COAL && t>=2));
    }
    game_init(&g,WORLD_SEED);game_init_benchmark(&before,WORLD_SEED);
    unsigned coal=0,stone=0;
    for(int y=0;y<WORLD_H;++y) for(int x=0;x<WORLD_W;++x) {
        coal+=g.tiles[y][x]==TILE_COAL;stone+=g.tiles[y][x]==TILE_STONE;
        assert(g.tiles[y][x]==before.tiles[y][x] || (g.tiles[y][x]==TILE_COAL && before.tiles[y][x]==TILE_STONE));
    }
    assert(coal>0 && coal*17<stone+coal && coal*21>stone+coal);
    puts("PASS: deterministic coal distribution, typed pickaxes and correct iron/coal mining tiers/times");
}
static void furnace(void)
{
    arena();g.slots[0]=stack(ITEM_FURNACE,2);assert(game_place(&g));
    Furnace *f=&g.furnaces[0];assert(f->active && game_interaction(&g)==INTERACT_FURNACE);
    g.slots[1]=stack(ITEM_RAW_IRON,3);g.slots[2]=stack(ITEM_COAL,1);
    InventoryUI ui={.cursor=UI_FURNACE,.source=-1,.furnace=1};
    ui_inventory_key(&g,&ui,UI_HOTBAR_Y);ui.cursor=UI_FURNACE+1;ui_inventory_key(&g,&ui,UI_HOTBAR_Z);
    for(unsigned i=0;i<599;++i) game_furnaces_step(&g);
    assert(f->progress==599 && !f->slots[2].count && f->burn_ticks==4201);
    game_furnaces_step(&g);assert(f->slots[2].item==ITEM_IRON_INGOT && f->slots[2].count==1 && f->slots[0].count==2);
    f->slots[2].count=64;f->progress=100;unsigned burn=f->burn_ticks;
    for(unsigned i=0;i<10;++i) game_furnaces_step(&g);
    assert(f->progress==100 && f->burn_ticks==burn-10);
    g.slots[1]=stack(ITEM_RAW_PORK,2);
    assert(game_transfer(&g,(SlotRef){SLOT_INVENTORY,1,0},(SlotRef){SLOT_FURNACE,0,0},false));
    assert(f->progress==0 && f->recipe==ITEM_COOKED_PORK);
    ui.cursor=UI_FURNACE+2;g.slots[2]=stack(ITEM_IRON_INGOT,2);
    ui_inventory_key(&g,&ui,UI_HOTBAR_Z);assert(f->slots[2].count==64 && g.slots[2].count==2);
    assert(game_transfer(&g,(SlotRef){SLOT_FURNACE,2,0},(SlotRef){SLOT_INVENTORY,2,0},false));
    assert(f->slots[2].count==2 && g.slots[2].count==64);
    memset(&f->slots[2],0,sizeof(f->slots[2]));
    for(unsigned i=0;i<600;++i) game_furnaces_step(&g);
    assert(f->slots[2].item==ITEM_COOKED_PORK && f->slots[2].count==1);
    const Item fuels[]={ITEM_COAL,ITEM_LOG,ITEM_PLANKS,ITEM_STICK};
    const unsigned durations[]={4800,900,900,300};
    for(unsigned i=0;i<4;++i) {
        *f=(Furnace){.active=true,.x=f->x,.y=f->y};f->slots[1]=stack(fuels[i],1);
        game_furnaces_step(&g);assert(f->slots[1].count==1 && !f->burn_ticks);
        f->slots[0]=stack(ITEM_RAW_CHICKEN,2);game_furnaces_step(&g);
        assert(!f->slots[1].count && f->burn_ticks==durations[i]-1 && f->progress==1);
    }
    /* Hand cannot remove a furnace; a failed transaction retains every content. */
    f->slots[0]=stack(ITEM_RAW_BEEF,2);f->slots[1]=stack(ITEM_LOG,1);f->slots[2]=stack(ITEM_COOKED_BEEF,4);
    g.slots[0]=(ItemStack){0};steps(80,(GameInput){.mine=true});assert(f->active && g.notice==NOTICE_PICKAXE);
    for(unsigned i=0;i<TOTAL_SLOTS;++i) g.slots[i]=stack(ITEM_DIRT,64);
    g.slots[0]=stack(ITEM_STONE_PICKAXE,1);before=g;
    steps(30,(GameInput){.mine=true});assert(f->active && g.notice==NOTICE_FULL);
    assert(!memcmp(f->slots,before.furnaces[0].slots,sizeof(f->slots)));
    memset(g.slots+1,0,(TOTAL_SLOTS-1)*sizeof(ItemStack));steps(30,(GameInput){.mine=true});
    assert(!f->active && game_item_total(&g,ITEM_FURNACE)==1 && game_item_total(&g,ITEM_RAW_BEEF)==2);
    arena();g.slots[0]=stack(ITEM_FURNACE,64);
    for(unsigned i=0;i<FURNACE_LIMIT;++i) {
        g.player.x=(int32_t)(160+i*32)*FIXED_ONE;assert(game_place(&g));
    }
    g.player.x+=32*FIXED_ONE;assert(!game_place(&g) && g.notice==NOTICE_FURNACE_LIMIT && g.slots[0].count==32);
    puts("PASS: smelting, all fuels, blocked output, input changes, quickmoves, safe furnace removal and 32 limit");
}
static void mob(MobKind kind,int x,unsigned health)
{
    unsigned i=0;while(i<MOB_LIMIT && g.mobs[i].active) ++i;assert(i<MOB_LIMIT);
    g.mobs[i]=(Mob){.active=true,.kind=kind,.health=(uint16_t)health,.ai_ticks=240,
        .body={.x=x*FIXED_ONE,.y=(320-(int)game_mob_height(kind))*FIXED_ONE,.facing=1,.grounded=true}};
}
static void combat(void)
{
    arena();mob(MOB_COW,175,1000);mob(MOB_PIG,195,600);
    assert(game_attack(&g,true) && g.mobs[0].health==900 && g.mobs[1].health==600 && g.action_ticks==30);
    assert(game_attack(&g,true) && g.mobs[0].health==900);
    g.action_ticks=0;g.slots[0]=stack(ITEM_IRON_SWORD,1);g.slots[0].durability=1;
    assert(game_attack(&g,true) && g.mobs[0].health==300 && !g.slots[0].count);
    g.action_ticks=0;g.slots[0]=stack(ITEM_STONE_SWORD,1);
    assert(game_attack(&g,true) && !g.mobs[0].active && g.drops[0].stack.item==ITEM_RAW_BEEF);
    arena();mob(MOB_COW,192,1000);g.tiles[19][11]=TILE_STONE;
    g.slots[0]=stack(ITEM_WOOD_SWORD,1);unsigned dur=g.slots[0].durability;
    assert(!game_attack(&g,true));assert(game_attack(&g,false) && g.slots[0].durability==dur);
    /* A reachable mob has priority even with a workbench directly targeted. */
    arena();mob(MOB_COW,175,1000);g.aim_manual=true;
    int x,y;game_target(&g,&x,&y);g.tiles[y][x]=TILE_WORKBENCH;
    assert(game_interaction(&g)==INTERACT_WORKBENCH);
    assert(game_step(&g,(GameInput){.use_pressed=true,.use_held=true})==INTERACT_NONE);
    assert(g.mobs[0].health==900);
    g.mobs[0].health=100;g.action_ticks=0;
    assert(game_step(&g,(GameInput){.use_held=true})==INTERACT_NONE && !g.mobs[0].active);
    assert(game_step(&g,(GameInput){.use_held=true})==INTERACT_NONE);
    assert(game_step(&g,(GameInput){.use_pressed=true,.use_held=true})==INTERACT_WORKBENCH);
    arena();game_target(&g,&x,&y);g.tiles[y][x]=TILE_WORKBENCH;
    assert(game_step(&g,(GameInput){.use_held=true})==INTERACT_NONE);
    assert(game_step(&g,(GameInput){.use_pressed=true,.use_held=true})==INTERACT_WORKBENCH);
    /* Same simulated two seconds at each render rate. */
    const unsigned rates[]={10,15,30,60,120};
    for(unsigned r=0;r<5;++r) {
        arena();mob(MOB_ZOMBIE,175,1200);g.mobs[0].body.vx=0;
        unsigned acc=0;
        for(unsigned f=0;f<rates[r]*2;++f) {
            acc+=60;while(acc>=rates[r]) {
                g.mobs[0].body.x=175*FIXED_ONE;g.mobs[0].body.y=(320-22)*FIXED_ONE;
                game_step(&g,(GameInput){.use_held=true});acc-=rates[r];
            }
        }
        assert(g.mobs[0].health==800 && g.exertion==4);
    }
    puts("PASS: closest single hit, reach/walls, cooldown, last sword hit, loot, interaction edges and frame rates");
}
static void survival(void)
{
    arena();
    for(unsigned i=0;i<ARMOR_SLOTS;++i) {g.armor[i]=stack((Item)(ITEM_IRON_HELMET+i),1);g.armor[i].durability=1;}
    game_damage_player(&g,200,true);assert(g.health==1920 && g.hurt_ticks==45);
    for(unsigned i=0;i<ARMOR_SLOTS;++i) assert(!g.armor[i].count);
    game_damage_player(&g,200,true);assert(g.health==1920);
    g.hurt_ticks=0;game_damage_player(&g,200,true);assert(g.health==1720);
    arena();g.health=1500;g.hunger=20;
    for(unsigned i=0;i<960;++i) game_survival_step(&g);
    assert(g.health==1900 && g.hunger==19 && !g.regen_cost);
    arena();for(unsigned i=0;i<3599;++i) game_survival_step(&g);
    assert(g.hunger==20);
    game_survival_step(&g);
    assert(g.hunger==19);
    for(unsigned i=0;i<20;++i) game_exert(&g);
    assert(g.hunger==18 && !g.exertion);
    arena();g.hunger=0;g.hurt_ticks=300;
    for(unsigned i=0;i<240;++i) game_survival_step(&g);
    assert(g.health==1900);
    const Item foods[]={ITEM_RAW_PORK,ITEM_COOKED_PORK,ITEM_RAW_BEEF,ITEM_COOKED_BEEF,ITEM_RAW_CHICKEN,ITEM_COOKED_CHICKEN};
    const unsigned points[]={3,8,3,8,2,6};
    for(unsigned i=0;i<6;++i) {
        arena();g.slots[0]=stack(foods[i],2);assert(!game_eat(&g) && g.slots[0].count==2);
        g.hunger=1;assert(game_eat(&g) && g.hunger==1+points[i] && g.slots[0].count==1);
        assert(!game_eat(&g));g.action_ticks=0;g.hunger=19;assert(game_eat(&g) && g.hunger==20);
    }
    arena();g.slots[0]=stack(ITEM_IRON_SWORD,1);g.slots[0].durability=5;
    g.armor[0]=stack(ITEM_IRON_HELMET,1);g.armor[0].durability=9;
    g.player.x=500*FIXED_ONE;g.tiles[18][10]=TILE_STONE;before=g;
    game_damage_player(&g,HEALTH_MAX,false);
    assert(g.respawns==1 && g.health==HEALTH_MAX && g.hunger==20 && g.hurt_ticks==300);
    assert(g.slots[0].durability==5 && g.armor[0].durability==9 && game_validate(&g));
    assert(g.player.x<500*FIXED_ONE && g.player.grounded && !g.player.sneaking);
    puts("PASS: armor percentage/last-use protection, hurt immunity, hunger, exertion, food, regeneration and respawn");
}
static void spawn_loot_stress(void)
{
    arena();assert(!game_spawn_mob(&g,MOB_ZOMBIE,180*FIXED_ONE,298*FIXED_ONE));
    for(unsigned i=0;i<8;++i) {
        int x=400+(int)i*20;
        assert(game_spawn_mob(&g,(MobKind)(i%3),x*FIXED_ONE,(320-(int)game_mob_height((MobKind)(i%3)))*FIXED_ONE));
        assert(game_spawn_mob(&g,MOB_ZOMBIE,(x+200)*FIXED_ONE,298*FIXED_ONE));
    }
    assert(!game_spawn_mob(&g,MOB_PIG,900*FIXED_ONE,308*FIXED_ONE));
    assert(!game_spawn_mob(&g,MOB_ZOMBIE,900*FIXED_ONE,298*FIXED_ONE));
    assert(game_validate(&g));
    g.mobs[0].body.x=1000*FIXED_ONE;game_survival_step(&g);
    assert(!g.mobs[0].active);
    arena();for(int x=0;x<WORLD_W;++x) g.tiles[30][x]=TILE_STONE;
    assert(!game_spawn_mob(&g,MOB_COW,400*FIXED_ONE,(480-14)*FIXED_ONE));
    assert(game_spawn_mob(&g,MOB_ZOMBIE,400*FIXED_ONE,(480-22)*FIXED_ONE));
    arena();mob(MOB_ZOMBIE,134,1200);g.tiles[19][9]=TILE_STONE;
    game_survival_step(&g);
    assert(g.mobs[0].body.vy<0);
    arena();mob(MOB_PIG,175,600);assert(game_attack(&g,true));int32_t px=g.mobs[0].body.x;
    game_survival_step(&g);
    assert(g.mobs[0].body.x>px && g.mobs[0].flee_ticks==119);
    arena();for(unsigned i=0;i<TOTAL_SLOTS;++i) g.slots[i]=stack(ITEM_DIRT,64);
    game_drop(&g,stack(ITEM_RAW_PORK,2),g.player.x,g.player.y);
    game_drop(&g,stack(ITEM_RAW_PORK,3),g.player.x,g.player.y);assert(g.drops[0].stack.count==5);
    game_survival_step(&g);
    assert(g.drops[0].active && g.drops[0].stack.count==5);
    g.slots[3]=(ItemStack){0};game_survival_step(&g);
    assert(!g.drops[0].active && game_item_total(&g,ITEM_RAW_PORK)==5);
    game_drop(&g,stack(ITEM_RAW_BEEF,1),400*FIXED_ONE,300*FIXED_ONE);g.drops[0].ttl=1;
    game_survival_step(&g);
    assert(!g.drops[0].active);
    for(unsigned i=0;i<DROP_LIMIT;++i) game_drop(&g,stack(ITEM_WOOD_SWORD,1),(int32_t)(i*32)*FIXED_ONE,100*FIXED_ONE);
    g.drops[17].ttl=1;game_drop(&g,stack(ITEM_IRON_SWORD,1),100*FIXED_ONE,100*FIXED_ONE);
    assert(g.drops[17].stack.item==ITEM_IRON_SWORD);
    arena();
    for(unsigned i=0;i<16;++i) mob(i<8?(MobKind)(i%3):MOB_ZOMBIE,200+(int)i*20,game_mob_health(i<8?(MobKind)(i%3):MOB_ZOMBIE));
    for(unsigned i=0;i<FURNACE_LIMIT;++i) {
        Furnace *f=&g.furnaces[i];*f=(Furnace){.active=true,.x=50+(int)i,.y=19};
        g.tiles[f->y][f->x]=TILE_FURNACE;f->slots[0]=stack(ITEM_RAW_IRON,64);f->slots[1]=stack(ITEM_COAL,8);
    }
    for(unsigned i=0;i<18000;++i) {
        game_step(&g,(GameInput){.jump=i%151==0,.use_held=i%3==0,.toggle_sneak=i%211==0});
        if(i%60==0) assert(game_validate(&g));
    }
    assert(game_validate(&g));
    for(unsigned i=0;i<FURNACE_LIMIT;++i) assert(g.furnaces[i].slots[2].count==30);
    puts("PASS: mob caps/safe zone/caves, zombie jump, fleeing, loot retention/expiry/replacement and 18,000-tick stress");
}
int main(void)
{
    aim();transfers();recipes();mining();furnace();combat();survival();spawn_loot_stress();
    puts("All survival checks passed.");return 0;
}
