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
    puts("Host previews rendered (no device performance measurement).");
    return 0;
}

