#include "game.h"
#include <string.h>

static bool full(Game *g)
{g->notice=NOTICE_FULL;g->notice_ticks=SIM_HZ;return false;}
static void consume(ItemStack *s)
{if(!--s->count) *s=(ItemStack){0};}
bool game_insert_stack(ItemStack slots[TOTAL_SLOTS],ItemStack in)
{
    if(!in.count || !game_stack_valid(&in)) return false;
    unsigned limit=game_stack_limit(in.item),capacity=0;
    for(unsigned i=0;i<TOTAL_SLOTS;++i) {
        if(!slots[i].count) capacity+=limit;
        else if(limit>1 && slots[i].item==in.item && slots[i].count<limit)
            capacity+=limit-slots[i].count;
    }
    if(capacity<in.count) return false;
    unsigned left=in.count;
    if(limit>1) for(unsigned i=0;i<TOTAL_SLOTS && left;++i) {
        if(slots[i].item!=in.item || slots[i].count>=limit) continue;
        unsigned take=limit-slots[i].count;if(take>left) take=left;
        slots[i].count=(uint16_t)(slots[i].count+take);left-=take;
    }
    for(unsigned pass=0;pass<2 && left;++pass) {
        unsigned begin=pass?0:HOTBAR_SLOTS,end=pass?HOTBAR_SLOTS:TOTAL_SLOTS;
        for(unsigned i=begin;i<end && left;++i) if(!slots[i].count) {
            unsigned take=left<limit?left:limit;
            slots[i]=in;slots[i].count=(uint16_t)take;left-=take;
        }
    }
    return true;
}
bool game_add_stack(Game *g,ItemStack in)
{
    if(!game_insert_stack(g->slots,in)) return false;
    ++g->revision;return true;
}
bool game_add_item(Game *g,Item item)
{
    return game_add_stack(g,(ItemStack){.item=item,.count=1,
        .durability=(uint16_t)game_tool_durability(item)});
}
static ItemStack *ref_ptr(Game *g,SlotRef r)
{
    switch(r.kind) {
    case SLOT_INVENTORY:return r.index<TOTAL_SLOTS?&g->slots[r.index]:NULL;
    case SLOT_CRAFT:return r.index<CRAFT_SLOTS?&g->crafting[r.index]:NULL;
    case SLOT_ARMOR:return r.index<ARMOR_SLOTS?&g->armor[r.index]:NULL;
    case SLOT_FURNACE:
        return r.container<FURNACE_LIMIT && g->furnaces[r.container].active && r.index<3?
            &g->furnaces[r.container].slots[r.index]:NULL;
    default:return NULL;
    }
}
const ItemStack *game_ref_stack(const Game *g,SlotRef r)
{
    switch(r.kind) {
    case SLOT_INVENTORY:return r.index<TOTAL_SLOTS?&g->slots[r.index]:NULL;
    case SLOT_CRAFT:return r.index<CRAFT_SLOTS?&g->crafting[r.index]:NULL;
    case SLOT_ARMOR:return r.index<ARMOR_SLOTS?&g->armor[r.index]:NULL;
    case SLOT_FURNACE:
        return r.container<FURNACE_LIMIT && g->furnaces[r.container].active && r.index<3?
            &g->furnaces[r.container].slots[r.index]:NULL;
    default:return NULL;
    }
}
const ItemStack *game_stack_at(const Game *g,unsigned slot)
{
    if(slot<TOTAL_SLOTS) return &g->slots[slot];
    return slot<CRAFT_RESULT?&g->crafting[slot-CRAFT_FIRST]:NULL;
}
static bool accepts(SlotRef r,ItemStack old,ItemStack in)
{
    if(!game_stack_valid(&in)) return false;
    if(!in.count) return true;
    if(r.kind==SLOT_ARMOR) return game_armor_slot(in.item)==r.index;
    if(r.kind==SLOT_FURNACE) {
        if(r.index==0) return game_smelt_result(in.item)!=ITEM_NONE;
        if(r.index==1) return game_fuel_ticks(in.item)!=0;
        return old.item==in.item && old.durability==in.durability && in.count<=old.count;
    }
    return true;
}
bool game_can_return_crafting(const Game *g)
{
    ItemStack returned[TOTAL_SLOTS];memcpy(returned,g->slots,sizeof(returned));
    for(unsigned i=0;i<CRAFT_SLOTS;++i)
        if(g->crafting[i].count && !game_insert_stack(returned,g->crafting[i])) return false;
    return true;
}
bool game_transfer(Game *g,SlotRef from,SlotRef to,bool one)
{
    ItemStack *a=ref_ptr(g,from),*b=ref_ptr(g,to);
    if(!a || !b || a==b || !a->count || !game_stack_valid(a)) return false;
    ItemStack old_a=*a,old_b=*b;
    if(one) {
        if(b->count) {
            if(a->item!=b->item || game_stack_limit(a->item)==1 ||
               b->count>=game_stack_limit(b->item)) return false;
            ++b->count;
        } else {*b=*a;b->count=1;}
        consume(a);
    } else if(b->count && a->item==b->item && game_stack_limit(a->item)>1) {
        unsigned cap=game_stack_limit(b->item);
        if(b->count>=cap) return false;
        unsigned count=cap-b->count;if(count>a->count) count=a->count;
        b->count=(uint16_t)(b->count+count);a->count=(uint16_t)(a->count-count);
        if(!a->count) *a=(ItemStack){0};
    } else {*b=old_a;*a=old_b;}
    bool fits=accepts(from,old_a,*a) && accepts(to,old_b,*b);
    bool returns=fits && game_can_return_crafting(g);
    if(!fits || !returns) {
        *a=old_a;*b=old_b;
        g->notice=fits?NOTICE_FULL:NOTICE_WRONG_SLOT;g->notice_ticks=SIM_HZ;return false;
    }
    SlotRef refs[2]={from,to};ItemStack olds[2]={old_a,old_b};
    for(unsigned i=0;i<2;++i) if(refs[i].kind==SLOT_FURNACE && refs[i].index==0) {
        Furnace *f=&g->furnaces[refs[i].container];
        if(olds[i].item!=f->slots[0].item) {f->progress=0;f->recipe=game_smelt_result(f->slots[0].item);}
    }
    ++g->revision;
    if(g->notice==NOTICE_FULL || g->notice==NOTICE_WRONG_SLOT)
        {g->notice=NOTICE_NONE;g->notice_ticks=0;}
    return true;
}
static SlotRef legacy_ref(unsigned n)
{
    return n<TOTAL_SLOTS?(SlotRef){SLOT_INVENTORY,n,0}:
        n<CRAFT_RESULT?(SlotRef){SLOT_CRAFT,n-CRAFT_FIRST,0}:(SlotRef){SLOT_INVALID,0,0};
}
bool game_inventory_transfer(Game *g,unsigned a,unsigned b)
{return game_transfer(g,legacy_ref(a),legacy_ref(b),false);}
bool game_inventory_transfer_one(Game *g,unsigned a,unsigned b)
{return game_transfer(g,legacy_ref(a),legacy_ref(b),true);}
void game_select_hotbar(Game *g,unsigned slot)
{
    if(slot<HOTBAR_SLOTS && slot!=g->selected)
        {g->selected=slot;game_cancel_mining(g);++g->revision;}
}
unsigned game_item_total(const Game *g,Item item)
{
    unsigned count=0;
    for(unsigned i=0;i<TOTAL_SLOTS;++i) if(g->slots[i].item==item) count+=g->slots[i].count;
    for(unsigned i=0;i<CRAFT_SLOTS;++i) if(g->crafting[i].item==item) count+=g->crafting[i].count;
    for(unsigned i=0;i<ARMOR_SLOTS;++i) if(g->armor[i].item==item) count+=g->armor[i].count;
    for(unsigned i=0;i<FURNACE_LIMIT;++i) if(g->furnaces[i].active)
        for(unsigned s=0;s<3;++s) if(g->furnaces[i].slots[s].item==item) count+=g->furnaces[i].slots[s].count;
    return count;
}
static ItemStack result(Item item,unsigned count)
{
    return (ItemStack){.item=item,.count=(uint16_t)count,
        .durability=(uint16_t)game_tool_durability(item)};
}
static bool pattern(const Game *g,unsigned width,unsigned first,
    unsigned w,unsigned h,const char *mask,Item material)
{
    for(unsigned y=0;y<h;++y) for(unsigned x=0;x<w;++x) {
        const ItemStack *s=&g->crafting[first+y*width+x];
        if(mask[y*w+x]=='X'?(s->count==0 || s->item!=material):s->count!=0) return false;
    }
    return true;
}
ItemStack game_craft_preview(const Game *g,unsigned width)
{
    ItemStack none={0};if(width!=2 && width!=3) return none;
    unsigned left=width,top=width,right=0,bottom=0,count=0;
    for(unsigned i=0;i<CRAFT_SLOTS;++i) if(g->crafting[i].count) {
        if(i>=width*width || !game_stack_valid(&g->crafting[i])) return none;
        unsigned x=i%width,y=i/width;
        if(x<left) left=x;
        if(x>right) right=x;
        if(y<top) top=y;
        if(y>bottom) bottom=y;
        ++count;
    }
    if(!count) return none;
    unsigned w=right-left+1,h=bottom-top+1,first=top*width+left;
    Item material=g->crafting[first].item;
    if(count==1 && material==ITEM_LOG) return result(ITEM_PLANKS,4);
    if(w==1 && h==2 && count==2 && pattern(g,width,first,1,2,"XX",ITEM_PLANKS))
        return result(ITEM_STICK,4);
    if(w==2 && h==2 && count==4 && pattern(g,width,first,2,2,"XXXX",ITEM_PLANKS))
        return result(ITEM_WORKBENCH,1);
    if(width!=3) return none;
    if(w==1 && h==3 && count==3 && g->crafting[first+width].item==material &&
       g->crafting[first+2*width].item==ITEM_STICK) {
        Item sword=material==ITEM_PLANKS?ITEM_WOOD_SWORD:
            material==ITEM_STONE?ITEM_STONE_SWORD:material==ITEM_IRON_INGOT?ITEM_IRON_SWORD:ITEM_NONE;
        if(sword) return result(sword,1);
    }
    if(w==3 && h==3 && count==5 &&
       (material==ITEM_PLANKS || material==ITEM_STONE) &&
       g->crafting[1].item==material && g->crafting[2].item==material &&
       g->crafting[4].item==ITEM_STICK && g->crafting[7].item==ITEM_STICK)
        return result(material==ITEM_PLANKS?ITEM_WOOD_PICKAXE:ITEM_STONE_PICKAXE,1);
    if(w==3 && h==3 && count==8 && pattern(g,width,first,3,3,"XXXX XXXX",ITEM_STONE))
        return result(ITEM_FURNACE,1);
    if(w==3 && h==2 && count==5 && pattern(g,width,first,3,2,"XXXX X",ITEM_IRON_INGOT))
        return result(ITEM_IRON_HELMET,1);
    if(w==3 && h==3 && count==8 && pattern(g,width,first,3,3,"X XXXXXXX",ITEM_IRON_INGOT))
        return result(ITEM_IRON_CHEST,1);
    if(w==3 && h==3 && count==7 && pattern(g,width,first,3,3,"XXXX XX X",ITEM_IRON_INGOT))
        return result(ITEM_IRON_LEGS,1);
    if(w==3 && h==2 && count==4 && pattern(g,width,first,3,2,"X XX X",ITEM_IRON_INGOT))
        return result(ITEM_IRON_BOOTS,1);
    return none;
}
bool game_craft(Game *g,unsigned width)
{
    ItemStack output=game_craft_preview(g,width);if(!output.count) return false;
    ItemStack produced[TOTAL_SLOTS],returned[TOTAL_SLOTS];
    memcpy(produced,g->slots,sizeof(produced));
    if(!game_insert_stack(produced,output)) return full(g);
    memcpy(returned,produced,sizeof(returned));
    for(unsigned i=0;i<width*width;++i) if(g->crafting[i].count>1) {
        ItemStack remaining=g->crafting[i];--remaining.count;
        if(!game_insert_stack(returned,remaining)) return full(g);
    }
    memcpy(g->slots,produced,sizeof(produced));
    for(unsigned i=0;i<width*width;++i) if(g->crafting[i].count) consume(&g->crafting[i]);
    g->notice=NOTICE_NONE;g->notice_ticks=0;++g->revision;return true;
}
bool game_close_crafting(Game *g)
{
    ItemStack returned[TOTAL_SLOTS];memcpy(returned,g->slots,sizeof(returned));
    for(unsigned i=0;i<CRAFT_SLOTS;++i)
        if(g->crafting[i].count && !game_insert_stack(returned,g->crafting[i])) return full(g);
    memcpy(g->slots,returned,sizeof(returned));
    bool changed=false;
    for(unsigned i=0;i<CRAFT_SLOTS;++i) changed|=g->crafting[i].count!=0;
    memset(g->crafting,0,sizeof(g->crafting));
    if(changed) ++g->revision;
    if(g->notice==NOTICE_FULL) {g->notice=NOTICE_NONE;g->notice_ticks=0;}
    return true;
}
int game_furnace_at(const Game *g,int x,int y)
{
    for(int i=0;i<FURNACE_LIMIT;++i)
        if(g->furnaces[i].active && g->furnaces[i].x==x && g->furnaces[i].y==y) return i;
    return -1;
}
void game_furnaces_step(Game *g)
{
    for(unsigned i=0;i<FURNACE_LIMIT;++i) {
        Furnace *f=&g->furnaces[i];if(!f->active) continue;
        Item out=game_smelt_result(f->slots[0].item);
        if(f->recipe!=out) {f->progress=0;f->recipe=out;}
        bool ready=f->slots[0].count && out &&
            (!f->slots[2].count || (f->slots[2].item==out && f->slots[2].count<64));
        if(!f->burn_ticks && ready && f->slots[1].count) {
            f->burn_ticks=game_fuel_ticks(f->slots[1].item);
            f->burn_total=f->burn_ticks;
            if(f->burn_ticks) consume(&f->slots[1]);
        }
        if(f->burn_ticks && ready && ++f->progress==600) {
            consume(&f->slots[0]);
            if(!f->slots[2].count) f->slots[2]=result(out,1);else ++f->slots[2].count;
            f->progress=0;
        }
        if(f->burn_ticks) --f->burn_ticks;
        if(!f->slots[0].count) {f->progress=0;f->recipe=ITEM_NONE;}
    }
}
