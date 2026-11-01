#include "game.h"
#include "inventory_ui.h"
#include "ui_layout.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static Game g,before;
static InventoryUI ui;
static ItemStack stack(Item item,unsigned count)
{return (ItemStack){item,(uint16_t)count,(uint16_t)game_tool_durability(item)};}
static void reset(void)
{
    game_init(&g,WORLD_SEED);memset(g.slots,0,sizeof(g.slots));
    ui=(InventoryUI){.source=-1,.craft_width=2};
}
static UIRect bounds(unsigned id)
{
    if(id==UI_ARMOR_BUTTON) return ui_armor_rect();
    if(id==UI_CLOSE_BUTTON) return ui_close_rect();
    UISlot list[UI_MAX_SLOTS];unsigned n=ui_slots(&ui,list);
    for(unsigned i=0;i<n;++i) if(list[i].id==id) return (UIRect){list[i].x,list[i].y,list[i].w,list[i].h};
    assert(!"invisible slot");return (UIRect){0};
}
static bool event(UITouch type,unsigned id,bool half)
{UIRect r=bounds(id);return ui_inventory_touch(&g,&ui,type,r.x+r.w/2,r.y+r.h/2,half);}
static bool tap(unsigned id,bool half)
{event(UI_TOUCH_DOWN,id,half);return event(UI_TOUCH_UP,id,half);}
static void drag(unsigned from,unsigned to,bool half)
{event(UI_TOUCH_DOWN,from,half);event(UI_TOUCH_DRAG,to,half);event(UI_TOUCH_UP,to,half);}
static unsigned wear(Item item)
{
    unsigned n=0;
    for(unsigned i=0;i<TOTAL_SLOTS;++i) if(g.slots[i].item==item) n+=g.slots[i].durability;
    for(unsigned i=0;i<CRAFT_SLOTS;++i) if(g.crafting[i].item==item) n+=g.crafting[i].durability;
    for(unsigned i=0;i<ARMOR_SLOTS;++i) if(g.armor[i].item==item) n+=g.armor[i].durability;
    return n;
}
static void basic(void)
{
    reset();g.slots[0]=stack(ITEM_DIRT,33);unsigned selected=g.selected;
    event(UI_TOUCH_DOWN,0,false);assert(g.slots[0].count==33 && ui.source==-1);
    event(UI_TOUCH_UP,0,false);assert(ui.source==0);
    tap(4,false);assert(!g.slots[0].count && g.slots[4].count==33 && ui.source==-1 && g.selected==selected);
    event(UI_TOUCH_UP,4,false);assert(g.slots[4].count==33); /* Duplicate release. */
    drag(4,0,true);assert(g.slots[4].count==16 && g.slots[0].count==17);
    tap(0,true);assert(ui.source==0 && ui.source_half);
    tap(3,false);assert(g.slots[0].count==8 && g.slots[3].count==9 && !ui.source_half);
    tap(3,false);tap(3,false);assert(ui.source==-1 && g.slots[3].count==9);
    g.slots[1]=stack(ITEM_STONE,64);drag(0,1,false);
    assert(g.slots[0].item==ITEM_STONE && g.slots[1].item==ITEM_DIRT && g.slots[1].count==8);
    before=g;drag(1,0,true);assert(!memcmp(g.slots,before.slots,sizeof(g.slots)));
    g.slots[1]=stack(ITEM_STONE,60);drag(0,1,true);
    assert(g.slots[0].count==60 && g.slots[1].count==64); /* Only capacity is moved. */
    g.slots[2]=stack(ITEM_PLANKS,1);drag(2,5,true);assert(!g.slots[2].count && g.slots[5].count==1);
    g.slots[2]=stack(ITEM_PLANKS,33);event(UI_TOUCH_DOWN,2,false);event(UI_TOUCH_UP,6,true);
    assert(g.slots[2].count==16 && g.slots[6].count==17); /* No intermediate drag event needed. */
    assert(g.ticks==0 && game_validate(&g));
    puts("PASS: tap/tap, drag/drop, divide-half, odd/one counts, merges, swaps and release-only commits");
}
static void cancellation(void)
{
    reset();g.slots[0]=stack(ITEM_DIRT,33);before=g;
    event(UI_TOUCH_DOWN,0,false);event(UI_TOUCH_DRAG,CRAFT_RESULT,false);
    event(UI_TOUCH_UP,CRAFT_RESULT,false);assert(!memcmp(g.slots,before.slots,sizeof(g.slots)));
    event(UI_TOUCH_DOWN,0,false);ui_inventory_touch(&g,&ui,UI_TOUCH_DRAG,319,527,false);
    ui_inventory_touch(&g,&ui,UI_TOUCH_UP,319,527,false);
    assert(g.slots[0].count==33 && ui.source==-1);
    event(UI_TOUCH_DOWN,0,false);event(UI_TOUCH_DRAG,UI_ARMOR_BUTTON,false);
    event(UI_TOUCH_UP,UI_ARMOR_BUTTON,false);assert(!ui.armor && g.slots[0].count==33);
    event(UI_TOUCH_DOWN,0,true);ui_inventory_key(&g,&ui,UI_TOGGLE_ARMOR);
    event(UI_TOUCH_UP,UI_ARMOR,false);assert(ui.armor && !ui.touching && ui.source==-1);
    tap(UI_ARMOR_BUTTON,false);assert(!ui.armor);
    event(UI_TOUCH_DOWN,0,false);ui_inventory_key(&g,&ui,UI_CLOSE);
    event(UI_TOUCH_UP,3,false);assert(g.slots[0].count==33 && !g.slots[3].count);
    ui_inventory_touch(&g,&ui,UI_TOUCH_DOWN,319,527,false);event(UI_TOUCH_UP,3,false);
    assert(g.slots[0].count==33 && !g.slots[3].count);
    /* Rectangle edges and all out-of-screen coordinates. */
    UIRect r=bounds(0);assert(ui_inventory_hit(&ui,r.x,r.y)==0);
    assert(ui_inventory_hit(&ui,r.x+r.w-1,r.y+r.h-1)==0);
    assert(ui_inventory_hit(&ui,r.x+r.w,r.y)==-1);
    assert(ui_inventory_hit(&ui,-1,0)==-1 && ui_inventory_hit(&ui,320,528)==-1);
    puts("PASS: cancelled/outside drags, invalid results/buttons, keyboard interruption and slot boundaries");
}
static void crafting_equipment_furnace(void)
{
    reset();g.slots[0]=stack(ITEM_LOG,2);drag(0,CRAFT_FIRST,true);
    assert(g.slots[0].count==1 && g.crafting[0].count==1);
    event(UI_TOUCH_DOWN,CRAFT_RESULT,false);assert(!game_item_total(&g,ITEM_PLANKS));
    event(UI_TOUCH_DRAG,3,false);event(UI_TOUCH_UP,3,false);assert(!game_item_total(&g,ITEM_PLANKS));
    tap(CRAFT_RESULT,false);assert(game_item_total(&g,ITEM_PLANKS)==4 && !g.crafting[0].count);
    event(UI_TOUCH_UP,CRAFT_RESULT,false);assert(game_item_total(&g,ITEM_PLANKS)==4);
    g.slots[1]=stack(ITEM_IRON_HELMET,1);g.slots[1].durability=7;
    tap(UI_ARMOR_BUTTON,false);drag(1,UI_ARMOR,true);
    assert(g.armor[0].durability==7 && !g.slots[1].count);
    drag(UI_ARMOR,4,false);assert(g.slots[4].durability==7 && !g.armor[0].count);
    drag(4,UI_ARMOR+1,false);assert(g.slots[4].durability==7 && !g.armor[1].count);
    tap(UI_ARMOR_BUTTON,false);g.crafting[0]=stack(ITEM_LOG,1);
    assert(tap(UI_CLOSE_BUTTON,false) && !g.crafting[0].count);
    reset();g.furnaces[0]=(Furnace){.active=true};ui.furnace=1;
    g.slots[0]=stack(ITEM_RAW_IRON,33);g.slots[1]=stack(ITEM_COAL,17);
    drag(0,UI_FURNACE,true);drag(1,UI_FURNACE+1,false);
    Furnace *f=&g.furnaces[0];assert(f->slots[0].count==17 && f->slots[1].count==17);
    before=g;drag(0,UI_FURNACE+2,false);
    assert(!memcmp(g.slots,before.slots,sizeof(g.slots)) && !f->slots[2].count);
    f->slots[2]=stack(ITEM_IRON_INGOT,33);drag(UI_FURNACE+2,4,true);
    assert(f->slots[2].count==16 && g.slots[4].count==17);
    for(unsigned i=0;i<TOTAL_SLOTS;++i) g.slots[i]=stack(ITEM_DIRT,64);
    g.slots[0]=(ItemStack){0};g.crafting[0]=stack(ITEM_LOG,1);assert(game_can_return_crafting(&g));
    before=g;drag(UI_FURNACE+2,0,true);
    assert(!memcmp(g.slots,before.slots,sizeof(g.slots)) && f->slots[2].count==16 && g.notice==NOTICE_FULL);
    assert(game_can_return_crafting(&g));
    puts("PASS: tap crafting, no dragged preview duplication, worn equipment, furnace halves and reserved ingredient capacity");
}
static void conservation(void)
{
    uint32_t random=9823;
    for(unsigned round=0;round<4;++round) {
        reset();ui.craft_width=3;
        for(unsigned i=0;i<TOTAL_SLOTS;++i) {
            Item item=(Item)(1+(i+round*7)%(ITEM_COUNT-1));
            g.slots[i]=stack(item,game_tool_durability(item)?1:33);
            if(g.slots[i].durability) g.slots[i].durability/=2;
        }
        unsigned counts[ITEM_COUNT]={0},durability[ITEM_COUNT]={0};
        for(unsigned i=1;i<ITEM_COUNT;++i) {counts[i]=game_item_total(&g,(Item)i);durability[i]=wear((Item)i);}
        for(unsigned t=0;t<2500;++t) {
            random=random*1664525u+1013904223u;
            if(t%31==0) ui_inventory_key(&g,&ui,UI_TOGGLE_ARMOR);
            UISlot list[UI_MAX_SLOTS];unsigned n=ui_slots(&ui,list);
            unsigned from=list[random%n].id,to=list[(random>>8)%n].id;
            drag(from,to,(random&1)!=0);
            for(unsigned i=1;i<ITEM_COUNT;++i) {
                assert(game_item_total(&g,(Item)i)==counts[i] && wear((Item)i)==durability[i]);
            }
            assert(game_can_return_crafting(&g));
            for(unsigned i=0;i<TOTAL_SLOTS;++i) assert(game_stack_valid(&g.slots[i]));
        }
    }
    puts("PASS: 10,000 mixed whole/half touch transfers preserve every item and durability");
}
int main(void)
{basic();cancellation();crafting_equipment_furnace();conservation();puts("All touch checks passed.");return 0;}
