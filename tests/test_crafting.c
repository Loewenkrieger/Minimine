#include "game.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static Game g,saved;
static ItemStack stack(Item item,unsigned count)
{
    return (ItemStack){.item=item,.count=(uint16_t)count,
        .durability=(uint16_t)game_tool_durability(item)};
}
static void empty(void)
{
    memset(&g,0,sizeof(g));
    g.player=(Player){.x=160*FIXED_ONE,.y=(320-PLAYER_H)*FIXED_ONE,
        .grounded=true,.facing=1};
    g.aim_x=1;
    for(int x=0;x<WORLD_W;++x) g.tiles[20][x]=TILE_STONE;
}
static void grid_clear(void) {memset(g.crafting,0,sizeof(g.crafting));}
static void fill_inventory(void)
{
    for(unsigned i=0;i<TOTAL_SLOTS;++i) g.slots[i]=stack(ITEM_DIRT,64);
}
static void pick_recipe(Item material,unsigned count)
{
    grid_clear();
    for(unsigned i=0;i<3;++i) g.crafting[i]=stack(material,count);
    g.crafting[4]=g.crafting[7]=stack(ITEM_STICK,count);
}
static unsigned inventory_count(Item item)
{
    unsigned n=0;
    for(unsigned i=0;i<TOTAL_SLOTS;++i) if(g.slots[i].item==item) n+=g.slots[i].count;
    return n;
}
static void assert_limits(void)
{
    for(unsigned i=0;i<CRAFT_RESULT;++i) {
        const ItemStack *s=game_stack_at(&g,i);
        assert(s && s->count<=game_stack_limit(s->item));
        assert((s->count==0)==(s->item==ITEM_NONE));
        unsigned max=game_tool_durability(s->item);
        if(max) assert(s->count==1 && s->durability && s->durability<=max);
        else assert(!s->durability);
    }
}
static void test_transfer(void)
{
    empty();g.slots[2]=stack(ITEM_PLANKS,32);
    for(unsigned i=0;i<4;++i)
        assert(game_inventory_transfer_one(&g,2,CRAFT_FIRST+i));
    assert(g.slots[2].count==28);
    for(unsigned i=0;i<4;++i) assert(g.crafting[i].count==1);
    assert(game_craft_preview(&g,2).item==ITEM_WORKBENCH);
    assert(game_item_total(&g,ITEM_PLANKS)==32);
    assert(game_close_crafting(&g) && g.slots[2].count==32);
    assert(!game_stack_at(&g,CRAFT_RESULT));
    assert(!game_inventory_transfer_one(&g,99,0));
    assert(!game_inventory_transfer_one(&g,2,CRAFT_RESULT));
    assert(!game_inventory_transfer_one(&g,2,2));
    g.slots[0]=stack(ITEM_STONE,64);
    assert(!game_inventory_transfer_one(&g,2,0));
    g.slots[0]=stack(ITEM_PLANKS,64);
    assert(!game_inventory_transfer_one(&g,2,0));
    g.slots[0].count=63;
    assert(game_inventory_transfer_one(&g,2,0));
    assert(g.slots[0].count==64 && g.slots[2].count==31);
    g.slots[0]=stack(ITEM_WOOD_PICKAXE,1);g.slots[0].durability=7;
    g.slots[1]=stack(ITEM_WOOD_PICKAXE,1);g.slots[1].durability=31;
    assert(!game_inventory_transfer_one(&g,0,1));
    assert(game_inventory_transfer(&g,0,1));
    assert(g.slots[0].durability==31 && g.slots[1].durability==7);
    assert(game_inventory_transfer_one(&g,1,CRAFT_FIRST));
    assert(!g.slots[1].count && g.crafting[0].durability==7);
    assert(game_close_crafting(&g));
    assert(inventory_count(ITEM_WOOD_PICKAXE)==2);
    bool worn=false;
    for(unsigned i=0;i<TOTAL_SLOTS;++i)
        if(g.slots[i].item==ITEM_WOOD_PICKAXE && g.slots[i].durability==7) worn=true;
    assert(worn);assert_limits();
    puts("PASS: single-item crafting transfers, 64 cap and preservation of worn tools");
}
static void test_random_transfers(void)
{
    empty();
    for(unsigned i=1;i<=TOTAL_SLOTS;++i) g.slots[i-1]=stack((Item)i,
        game_tool_durability((Item)i)?1:32);
    g.slots[6].durability=13;g.slots[7].durability=79;
    unsigned counts[ITEM_COUNT]={0},health[ITEM_COUNT]={0};
    for(unsigned item=1;item<ITEM_COUNT;++item) {
        counts[item]=game_item_total(&g,(Item)item);
        for(unsigned i=0;i<CRAFT_RESULT;++i) {
            const ItemStack *at=game_stack_at(&g,i);
            if(at->item==(Item)item) health[item]+=at->durability;
        }
    }
    uint32_t random=98273;
    for(unsigned tick=0;tick<5000;++tick) {
        random=random*1664525u+1013904223u;
        unsigned from=random%CRAFT_RESULT,to=(random>>8)%CRAFT_RESULT;
        if(random&1) game_inventory_transfer(&g,from,to);
        else game_inventory_transfer_one(&g,from,to);
        if(tick%47==0) game_close_crafting(&g);
        assert_limits();
        for(unsigned item=1;item<ITEM_COUNT;++item) {
            assert(game_item_total(&g,(Item)item)==counts[item]);
            unsigned actual=0;
            for(unsigned i=0;i<CRAFT_RESULT;++i) {
                const ItemStack *at=game_stack_at(&g,i);
                if(at->item==(Item)item) actual+=at->durability;
            }
            assert(actual==health[item]);
        }
    }
    puts("PASS: 5,000 mixed inventory/crafting transfers preserve every item and tool durability");
}
static void test_recipes(void)
{
    for(unsigned width=2;width<=3;++width) {
        for(unsigned position=0;position<width*width;++position) {
            empty();g.crafting[position]=stack(ITEM_LOG,2);
            ItemStack p=game_craft_preview(&g,width);
            assert(p.item==ITEM_PLANKS && p.count==4);
            assert(game_craft(&g,width) && g.crafting[position].count==1);
            assert(game_craft(&g,width) && !g.crafting[position].count);
            assert(inventory_count(ITEM_PLANKS)==8);
        }
        for(unsigned row=0;row+1<width;++row) for(unsigned col=0;col<width;++col) {
            empty();unsigned at=row*width+col;
            g.crafting[at]=g.crafting[at+width]=stack(ITEM_PLANKS,2);
            ItemStack p=game_craft_preview(&g,width);
            assert(p.item==ITEM_STICK && p.count==4);
            assert(game_craft(&g,width) && inventory_count(ITEM_STICK)==4);
            assert(g.crafting[at].count==1 && g.crafting[at+width].count==1);
        }
        for(unsigned row=0;row+1<width;++row) for(unsigned col=0;col+1<width;++col) {
            empty();unsigned at=row*width+col;
            g.crafting[at]=g.crafting[at+1]=g.crafting[at+width]=g.crafting[at+width+1]=stack(ITEM_PLANKS,1);
            assert(game_craft_preview(&g,width).item==ITEM_WORKBENCH);
            assert(game_craft(&g,width) && inventory_count(ITEM_WORKBENCH)==1);
            for(unsigned i=0;i<CRAFT_SLOTS;++i) assert(!g.crafting[i].count);
        }
    }
    empty();
    g.crafting[0]=g.crafting[3]=stack(ITEM_PLANKS,1); /* Diagonal in 2x2. */
    assert(!game_craft_preview(&g,2).count && !game_craft(&g,2));
    assert(!game_craft_preview(&g,0).count && !game_craft_preview(&g,4).count);
    g.crafting[8]=stack(ITEM_LOG,1); /* Hidden input may not be ignored. */
    assert(!game_craft_preview(&g,2).count);
    for(unsigned material=ITEM_STONE;material<=ITEM_PLANKS;++material) {
        empty();pick_recipe((Item)material,2);
        Item tool=material==ITEM_STONE?ITEM_STONE_PICKAXE:ITEM_WOOD_PICKAXE;
        ItemStack p=game_craft_preview(&g,3);
        assert(p.item==tool && p.count==1 && p.durability==game_tool_durability(tool));
        assert(!game_craft_preview(&g,2).count);
        assert(game_craft(&g,3) && game_craft(&g,3));
        assert(inventory_count(tool)==2);
        for(unsigned i=0;i<CRAFT_SLOTS;++i) assert(!g.crafting[i].count);
        assert_limits();
        pick_recipe((Item)material,1);
        g.crafting[2]=stack(ITEM_DIRT,1);
        assert(!game_craft_preview(&g,3).count);
        pick_recipe((Item)material,1);g.crafting[8]=stack(ITEM_STICK,1);
        assert(!game_craft_preview(&g,3).count);
    }
    puts("PASS: logs, sticks, shifted 2x2 recipes, both pickaxes, exact quantities and invalid patterns");
}
static void test_full_and_return(void)
{
    empty();fill_inventory();g.crafting[0]=stack(ITEM_LOG,2);
    saved=g;
    assert(!game_craft(&g,2) && g.notice==NOTICE_FULL);
    assert(!memcmp(g.slots,saved.slots,sizeof(g.slots)));
    assert(!memcmp(g.crafting,saved.crafting,sizeof(g.crafting)));
    g.slots[3]=stack(ITEM_PLANKS,62);
    assert(!game_craft(&g,2) && g.slots[3].count==62 && g.crafting[0].count==2);
    g.slots[4]=stack(ITEM_PLANKS,62);
    assert(!game_craft(&g,2)); /* Output fits, but leftover log would prevent closing. */
    assert(g.slots[3].count==62 && g.slots[4].count==62 && g.crafting[0].count==2);
    g.crafting[0].count=1;
    assert(game_craft(&g,2) && g.slots[3].count==64 && g.slots[4].count==64);
    assert(!g.crafting[0].count && game_close_crafting(&g));
    empty();fill_inventory();pick_recipe(ITEM_PLANKS,1);
    assert(!game_craft(&g,3) && g.crafting[0].count==1 && g.crafting[4].count==1);
    g.slots[3]=(ItemStack){0};
    assert(game_craft(&g,3) && g.slots[3].item==ITEM_WOOD_PICKAXE);
    empty();fill_inventory();
    g.crafting[0]=stack(ITEM_LOG,1);g.crafting[1]=stack(ITEM_PLANKS,1);
    g.crafting[2]=stack(ITEM_WOOD_PICKAXE,1);g.crafting[2].durability=9;
    g.slots[3]=(ItemStack){0};saved=g;
    assert(!game_close_crafting(&g));
    assert(!memcmp(g.slots,saved.slots,sizeof(g.slots)));
    assert(!memcmp(g.crafting,saved.crafting,sizeof(g.crafting)));
    g.slots[4]=g.slots[5]=(ItemStack){0};
    assert(game_close_crafting(&g));
    assert(g.slots[3].item==ITEM_LOG && g.slots[4].item==ITEM_PLANKS);
    assert(g.slots[5].item==ITEM_WOOD_PICKAXE && g.slots[5].durability==9);
    for(unsigned i=0;i<CRAFT_SLOTS;++i) assert(!g.crafting[i].count);
    assert_limits();
    puts("PASS: atomic result insertion and atomic ingredient return with full inventories");
}
static void mine(Tile tile,unsigned ticks)
{
    int x,y;game_target(&g,&x,&y);assert(game_set_tile(&g,x,y,tile));
    for(unsigned i=0;i<ticks;++i) {
        game_step(&g,(GameInput){.mine=true});
        if(i+1<ticks) assert(game_tile(&g,x,y)==tile);
    }
    assert(game_tile(&g,x,y)==TILE_AIR);
}
static void test_tools(void)
{
    empty();fill_inventory();mine(TILE_STONE,60);
    assert(inventory_count(ITEM_STONE)==0); /* Full bag must not prevent no-drop mining. */
    empty();g.slots[3]=stack(ITEM_WOOD_PICKAXE,1);
    mine(TILE_STONE,60);assert(!inventory_count(ITEM_STONE)); /* Tool in bag is not equipped. */
    for(int which=0;which<2;++which) {
        Item tool=which?ITEM_STONE_PICKAXE:ITEM_WOOD_PICKAXE;
        unsigned duration=which?30:60;
        empty();g.slots[0]=stack(tool,1);
        mine(TILE_STONE,duration);
        assert(inventory_count(ITEM_STONE)==1);
        assert(g.slots[0].durability==game_tool_durability(tool)-1);
        unsigned health=g.slots[0].durability;
        int x,y;game_target(&g,&x,&y);game_set_tile(&g,x,y,TILE_STONE);
        for(unsigned i=0;i<10;++i) game_step(&g,(GameInput){.mine=true});
        game_step(&g,(GameInput){0});
        assert(!g.mining.active && g.slots[0].durability==health);
        game_step(&g,(GameInput){.mine=true});assert(g.mining.elapsed==1);
        game_select_hotbar(&g,1);assert(!g.mining.active);
        game_step(&g,(GameInput){.mine=true});assert(g.mining.elapsed==1 && g.mining.tool==ITEM_NONE);
        empty();fill_inventory();g.slots[0]=stack(tool,1);
        game_target(&g,&x,&y);game_set_tile(&g,x,y,TILE_STONE);
        for(unsigned i=0;i<duration+10;++i) game_step(&g,(GameInput){.mine=true});
        assert(game_tile(&g,x,y)==TILE_STONE && g.notice==NOTICE_FULL);
        assert(g.slots[0].durability==game_tool_durability(tool));
        empty();g.slots[0]=stack(tool,1);g.slots[0].durability=1;
        mine(TILE_STONE,duration);
        assert(inventory_count(ITEM_STONE)==1 && !g.slots[0].count);
        assert(g.notice==NOTICE_TOOL_BROKEN);
        mine(TILE_STONE,60);assert(inventory_count(ITEM_STONE)==1);
        /* Exactly the promised number of successful blocks, not held ticks. */
        empty();g.slots[0]=stack(tool,1);
        unsigned maximum=game_tool_durability(tool);
        for(unsigned i=0;i<maximum;++i) {
            mine(TILE_LEAVES,9);
            if(i+1<maximum) assert(g.slots[0].durability==maximum-i-1);
        }
        assert(!g.slots[0].count && g.slots[0].item==ITEM_NONE);
        empty();g.slots[0]=stack(tool,1);
        game_target(&g,&x,&y);game_set_tile(&g,x,y,TILE_BEDROCK);
        for(int i=0;i<120;++i) game_step(&g,(GameInput){.mine=true});
        assert(g.slots[0].durability==maximum && !g.mining.active);
    }
    puts("PASS: hand stone drops nothing; equipped tools, mining speed, cancellation and exact 64/128 uses");
}
static void test_workbench(void)
{
    empty();g.slots[0]=stack(ITEM_WORKBENCH,2);
    int x,y;game_target(&g,&x,&y);
    assert(game_place(&g) && g.slots[0].count==1 && game_tile(&g,x,y)==TILE_WORKBENCH);
    assert(game_interaction(&g)==INTERACT_WORKBENCH);
    assert(game_step(&g,(GameInput){.use_held=true,.use_pressed=true})==INTERACT_WORKBENCH);
    assert(g.slots[0].count==1);
    assert(game_step(&g,(GameInput){.use_held=true})==INTERACT_NONE);
    mine(TILE_WORKBENCH,36);
    assert(game_item_total(&g,ITEM_WORKBENCH)==2);
    assert(game_interaction(&g)==INTERACT_NONE);
    g.slots[0]=stack(ITEM_LOG,1);
    assert(game_place(&g) && game_tile(&g,x,y)==TILE_LOG);
    assert(game_tile_solid(TILE_LOG) && game_tile_solid(TILE_WORKBENCH));
    mine(TILE_LOG,36);assert(game_item_total(&g,ITEM_LOG)==1);
    puts("PASS: workbench placement, one-press interaction, recovery and solid placeable logs");
}
int main(void)
{
    test_transfer();test_random_transfers();test_recipes();test_full_and_return();test_tools();test_workbench();
    puts("All crafting and tool checks passed.");
    return 0;
}

