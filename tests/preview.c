#include "game.h"
#include "render.h"
#include <gint/display.h>
#include <stdio.h>

static Game game;

int main(void)
{
    /* No synthetic FPS values: a host screenshot cannot measure the CP400. */
    RenderInfo info = {0};
    game_init(&game, WORLD_SEED);
    render_game(&game, &info);
    host_write_ppm("build/native/play.ppm");
    for(unsigned i=0;i<3;++i)
        game_step(&game,(GameInput){.aim_y=1,.jump=i==0,.use_held=true});
    render_game(&game,&info);host_write_ppm("build/native/pillar-place.ppm");
    for(unsigned i=0;i<80;++i) game_step(&game,(GameInput){.aim_y=1});
    render_game(&game,&info);host_write_ppm("build/native/pillar-landed.ppm");
    game_init(&game,WORLD_SEED);
    game.health=1350;game.hunger=11;
    for(unsigned i=0;i<4;++i) {
        MobKind k=(MobKind)i;unsigned h=game_mob_height(k);
        game.mobs[i]=(Mob){.active=true,.kind=k,.health=(uint16_t)(game_mob_health(k)/2),
            .body={.x=game.player.x+((int)i*40-70)*FIXED_ONE,.y=game.player.y+(PLAYER_H-(int)h)*FIXED_ONE,.facing=1}};
        game.armor[i]=(ItemStack){.item=(Item)(ITEM_IRON_HELMET+i),.count=1,
            .durability=(uint16_t)(game_tool_durability((Item)(ITEM_IRON_HELMET+i))/2)};
    }
    game.slots[0]=(ItemStack){.item=ITEM_IRON_SWORD,.count=1,.durability=125};
    render_game(&game,&info);host_write_ppm("build/native/survival.ppm");
    InventoryUI equipment={.cursor=UI_ARMOR,.source=-1,.armor=true};
    render_inventory(&game,&equipment);host_write_ppm("build/native/armor.ppm");
    game.furnaces[0]=(Furnace){.active=true,.burn_ticks=3700,.burn_total=4800,.progress=350,.recipe=ITEM_IRON_INGOT,
        .slots={{.item=ITEM_RAW_IRON,.count=23},{.item=ITEM_COAL,.count=4},{.item=ITEM_IRON_INGOT,.count=8}}};
    equipment=(InventoryUI){.cursor=UI_FURNACE,.source=-1,.furnace=1};
    render_inventory(&game,&equipment);host_write_ppm("build/native/furnace.ppm");
    render_menu(false,0);host_write_ppm("build/native/start.ppm");
    render_menu(true,1);host_write_ppm("build/native/pause.ppm");
    SaveInfo saves[3]={{.status=SAVE_OK,.generation=3,.ticks=10800,.health=1350,.hunger=11},
        {.status=SAVE_EMPTY},{.status=SAVE_INVALID}};
    render_slots(true,saves,0);host_write_ppm("build/native/saves.ppm");
    render_confirm("Aktuelles Spiel verwerfen?",false);host_write_ppm("build/native/confirm.ppm");
    game_init(&game,WORLD_SEED);
    InventoryUI ui={.cursor=4,.source=0};
    render_inventory(&game,&ui);
    host_write_ppm("build/native/inventory.ppm");
    game.slots[0]=(ItemStack){.item=ITEM_DIRT,.count=33};
    ui.source=-1;
    ui_inventory_touch(&game,&ui,UI_TOUCH_DOWN,50,100,true);
    ui_inventory_touch(&game,&ui,UI_TOUCH_UP,50,100,true);
    render_inventory(&game,&ui);host_write_ppm("build/native/touch-half-selected.ppm");
    ui_inventory_touch(&game,&ui,UI_TOUCH_DOWN,50,100,true);
    ui_inventory_touch(&game,&ui,UI_TOUCH_DRAG,50,155,true);
    render_inventory(&game,&ui);host_write_ppm("build/native/touch-drag.ppm");
    ui_inventory_touch(&game,&ui,UI_TOUCH_UP,50,155,true);
    render_inventory(&game,&ui);host_write_ppm("build/native/touch-half-dropped.ppm");
    game_init(&game,WORLD_SEED);
    for(unsigned i=0;i<4;++i)
        game.crafting[i]=(ItemStack){.item=ITEM_PLANKS,.count=1};
    ui=(InventoryUI){.cursor=CRAFT_RESULT,.source=-1,.craft_width=2};
    render_inventory(&game,&ui);
    host_write_ppm("build/native/crafting-2x2.ppm");
    game_close_crafting(&game);
    for(unsigned i=0;i<3;++i)
        game.crafting[i]=(ItemStack){.item=ITEM_PLANKS,.count=1};
    game.crafting[4]=game.crafting[7]=(ItemStack){.item=ITEM_STICK,.count=1};
    ui.craft_width=3;
    render_inventory(&game,&ui);
    host_write_ppm("build/native/workbench-3x3.ppm");
    game_close_crafting(&game);
    game.slots[0]=(ItemStack){.item=ITEM_WOOD_PICKAXE,.count=1,.durability=32};
    game.slots[1]=(ItemStack){.item=ITEM_STONE_PICKAXE,.count=1,.durability=96};
    game.slots[2]=(ItemStack){.item=ITEM_WORKBENCH,.count=12};
    render_game(&game,&info);
    host_write_ppm("build/native/tool-hotbar.ppm");
    int bx,by;game_target(&game,&bx,&by);
    game_set_tile(&game,bx,by,TILE_WORKBENCH);
    game_set_tile(&game,bx+1,by,TILE_LOG);
    render_game(&game,&info);
    host_write_ppm("build/native/workbench-world.ppm");
    game_init(&game,WORLD_SEED);
    game_toggle_sneak(&game);
    game.aim_x=game.aim_y=1;
    game.mining=(Mining){.active=true,.required=60,.elapsed=30};
    game_target(&game,&game.mining.x,&game.mining.y);
    render_game(&game,&info);
    host_write_ppm("build/native/sneak-mining.ppm");
    game_init(&game,WORLD_SEED);
    Chest *mine=&game.chests[0];
    game.player=(Player){.x=(mine->x*16-PLAYER_W-1)*FIXED_ONE,
        .y=((mine->y+1)*16-PLAYER_H)*FIXED_ONE,.facing=1,.grounded=true};
    game.slots[0]=(ItemStack){.item=ITEM_TORCH,.count=12};game_follow_camera(&game);
    render_game(&game,&info);host_write_ppm("build/native/expedition-mine.ppm");
    ui=(InventoryUI){.cursor=UI_CHEST+2,.source=-1,.chest=1};
    render_inventory(&game,&ui);host_write_ppm("build/native/chest.ppm");
    game.accessories[0]=(ItemStack){.item=ITEM_MINER_CHARM,.count=1};
    game.accessories[1]=(ItemStack){.item=ITEM_SPEED_BOOTS,.count=1};
    ui=(InventoryUI){.cursor=UI_ACCESSORY,.source=-1,.armor=true};
    render_inventory(&game,&ui);host_write_ppm("build/native/accessories.ppm");
    game_init(&game,WORLD_SEED);
    for(int x=9;x<=22;++x) game_set_tile(&game,x,25,TILE_WOOD);
    for(int y=26;y<=29;++y) {game_set_tile(&game,9,y,TILE_WOOD);game_set_tile(&game,22,y,TILE_WOOD);}
    game_set_tile(&game,22,28,TILE_AIR);game_set_tile(&game,22,29,TILE_AIR);
    game_adventure_place(&game,TILE_DOOR_BOTTOM,22,29);
    game_adventure_place(&game,TILE_CHEST,16,29);game_adventure_place(&game,TILE_BED_LEFT,18,29);
    game_adventure_place(&game,TILE_TORCH,11,29);
    game.player.x=(14*16+3)*FIXED_ONE;game_follow_camera(&game);
    render_game(&game,&info);host_write_ppm("build/native/base.ppm");
    game_init(&game,WORLD_SEED);game.relics_found=7;game.boss.gate_open=true;
    for(int y=GATE_TOP;y<ARENA_FLOOR;++y) game_set_tile(&game,ARENA_LEFT,y,TILE_AIR);
    game.player=(Player){.x=233*16*FIXED_ONE,.y=(ARENA_FLOOR*16-PLAYER_H)*FIXED_ONE,.facing=1,.grounded=true};
    game.slots[0]=(ItemStack){.item=ITEM_IRON_SWORD,.count=1,.durability=200};
    game_follow_camera(&game);game_boss_step(&game);game.boss.phase=3;game.boss.timer=32;
    for(unsigned i=0;i<MOB_LIMIT;++i) if(game.mobs[i].active && game.mobs[i].kind==MOB_WARDEN) game.mobs[i].health=6000;
    render_game(&game,&info);host_write_ppm("build/native/warden.ppm");
    render_menu(true,0);render_progress(&game);host_write_ppm("build/native/quest-progress.ppm");
    game_init(&game,WORLD_SEED);
    game.player=(Player){.x=193*16*FIXED_ONE,.y=82*16*FIXED_ONE,.facing=1};
    game.slots[0]=(ItemStack){.item=ITEM_TORCH,.count=8};game_follow_camera(&game);
    render_game(&game,&info);host_write_ppm("build/native/basalt.ppm");
    game_init_benchmark(&game,WORLD_SEED);
    render_help();
    host_write_ppm("build/native/help.ppm");
    for(unsigned phase = 0; phase < BENCH_PHASES; ++phase) {
        game_benchmark_step(&game, phase, 90);
        info.benchmark = true;
        info.phase = phase;
        info.phase_frame = 90;
        render_game(&game, &info);
        char path[80];
        snprintf(path, sizeof(path), "build/native/scene-%u.ppm", phase);
        host_write_ppm(path);
    }
    PerfSummary results[BENCH_PHASES] = {{0}};
    render_results(results);
    host_write_ppm("build/native/results.ppm");
    game_init(&game,WORLD_SEED);
    for(int y=74;y<=83;++y) for(int x=80;x<=108;++x)
        game.tiles[y][x]=(uint8_t)(y>=82?TILE_BASALT:TILE_AIR);
    game.player=(Player){.x=90*16*FIXED_ONE,.y=(82*16-PLAYER_H)*FIXED_ONE,.facing=1,.grounded=true};
    game.slots[0]=(ItemStack){ITEM_DIAMOND_SWORD,1,410};
    game.slots[1]=(ItemStack){ITEM_DIAMOND_PICKAXE,1,430};
    game.slots[2]=(ItemStack){ITEM_TORCH,32,0};game.slots[3]=(ItemStack){ITEM_DIAMOND,47,0};
    game.selected=2;game.tiles[80][94]=TILE_DIAMOND_ORE;game.tiles[81][95]=TILE_DIAMOND_ORE;
    game.tiles[82][92]=TILE_DIAMOND_ORE;game_adventure_place(&game,TILE_TORCH,87,81);
    for(unsigned i=0;i<ARMOR_SLOTS;++i)
        game.armor[i]=(ItemStack){(Item)(ITEM_DIAMOND_HELMET+i),1,(uint16_t)game_tool_durability((Item)(ITEM_DIAMOND_HELMET+i))};
    game_follow_camera(&game);info=(RenderInfo){0};render_invalidate();render_game(&game,&info);
    host_write_ppm("build/native/diamond-play.ppm");
    ui=(InventoryUI){.cursor=3,.source=-1,.craft_width=2};render_inventory(&game,&ui);
    host_write_ppm("build/native/diamond-inventory.ppm");
    ui.armor=true;ui.cursor=UI_ARMOR;render_inventory(&game,&ui);host_write_ppm("build/native/diamond-armor.ppm");
    render_difficulty(DIFFICULTY_HARD,false);host_write_ppm("build/native/difficulty.ppm");
    render_menu(true,4);render_progress(&game);host_write_ppm("build/native/pause-060.ppm");
    puts("Host previews rendered (no device performance measurement).");
    return 0;
}

