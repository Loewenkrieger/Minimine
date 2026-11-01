#include "inventory_ui.h"
#include "ui_layout.h"
#include <limits.h>

unsigned ui_slots(const InventoryUI *ui,UISlot out[UI_MAX_SLOTS])
{
    unsigned n=0;
    for(unsigned i=0;i<TOTAL_SLOTS;++i) {
        int row=(int)i/3,col=(int)i%3;
        out[n++]=(UISlot){i,{SLOT_INVENTORY,i,0},16+98*col,row?140+(row-1)*34:85,92,30,false};
    }
    if(ui->armor) {
        for(unsigned i=0;i<ARMOR_SLOTS+ACCESSORY_SLOTS;++i)
            out[n++]=(UISlot){i<ARMOR_SLOTS?UI_ARMOR+i:UI_ACCESSORY+i-ARMOR_SLOTS,
                {i<ARMOR_SLOTS?SLOT_ARMOR:SLOT_ACCESSORY,i<ARMOR_SLOTS?i:i-ARMOR_SLOTS,0},
                16+(int)(i%3)*98,312+(int)(i/3)*55,92,40,false};
    } else if(ui->chest) {
        for(unsigned i=0;i<CHEST_SLOTS;++i)
            out[n++]=(UISlot){UI_CHEST+i,{SLOT_CHEST,i,ui->chest-1},
                16+(int)(i%4)*73,309+(int)(i/4)*36,68,32,false};
    } else if(ui->furnace) {
        out[n++]=(UISlot){UI_FURNACE,{SLOT_FURNACE,0,ui->furnace-1},16,311,80,34,false};
        out[n++]=(UISlot){UI_FURNACE+1,{SLOT_FURNACE,1,ui->furnace-1},16,374,80,34,false};
        out[n++]=(UISlot){UI_FURNACE+2,{SLOT_FURNACE,2,ui->furnace-1},218,340,80,40,false};
    } else {
        unsigned width=ui->craft_width==3?3:2;
        for(unsigned i=0;i<width*width;++i)
            out[n++]=(UISlot){CRAFT_FIRST+i,{SLOT_CRAFT,i,0},
                16+(int)(i%width)*49,309+(int)(i/width)*36,45,32,false};
        out[n++]=(UISlot){UI_RESULT,{SLOT_INVALID,0,0},218,width==3?345:327,70,36,true};
    }
    return n;
}
static const UISlot *find(const UISlot *slots,unsigned count,unsigned id)
{for(unsigned i=0;i<count;++i) if(slots[i].id==id) return &slots[i];return 0;}
static int absolute(int n) {return n<0?-n:n;}
void ui_inventory_cancel_touch(InventoryUI *ui)
{ui->touching=ui->dragging=ui->touch_half=false;ui->touch_slot=-1;}
static void clear_source(InventoryUI *ui)
{ui->source=-1;ui->source_half=false;}
bool ui_inventory_key(Game *g,InventoryUI *ui,UICommand command)
{
    if(command!=UI_NONE) ui_inventory_cancel_touch(ui);
    if(command==UI_CLOSE) {
        if(!game_close_crafting(g)) return false;
        clear_source(ui);return true;
    }
    if(command==UI_TOGGLE_ARMOR) {
        clear_source(ui);
        if(!ui->armor) {ui->return_cursor=ui->cursor;ui->cursor=UI_ARMOR;}
        else ui->cursor=ui->return_cursor;
        ui->armor=!ui->armor;return false;
    }
    UISlot slots[UI_MAX_SLOTS];unsigned count=ui_slots(ui,slots);
    const UISlot *target=find(slots,count,ui->cursor);
    if(!target) {ui->cursor=0;target=&slots[0];}
    if(command>=UI_HOTBAR_X && command<=UI_HOTBAR_Z) {
        clear_source(ui);
        if(!target->result)
            game_transfer(g,(SlotRef){SLOT_INVENTORY,(unsigned)(command-UI_HOTBAR_X),0},target->ref,false);
        return false;
    }
    if(command<=UI_DOWN) {
        int cx=target->x+target->w/2,cy=target->y+target->h/2,best=INT_MAX;
        for(unsigned i=0;i<count;++i) {
            int dx=slots[i].x+slots[i].w/2-cx,dy=slots[i].y+slots[i].h/2-cy;
            int primary=command==UI_LEFT?-dx:command==UI_RIGHT?dx:command==UI_UP?-dy:dy;
            int cross=command==UI_LEFT || command==UI_RIGHT?dy:dx;
            int score=primary+4*absolute(cross);
            if(primary>0 && score<best) {best=score;ui->cursor=slots[i].id;}
        }
        return false;
    }
    if(command!=UI_ACCEPT && command!=UI_ONE) return false;
    if(target->result) {
        if(command==UI_ACCEPT) {game_craft(g,ui->craft_width==3?3:2);clear_source(ui);}
        return false;
    }
    if(ui->source<0) {
        const ItemStack *at=game_ref_stack(g,target->ref);
        if(at && at->count) {ui->source=(int)target->id;ui->source_half=false;}
    } else {
        const UISlot *source=find(slots,count,(unsigned)ui->source);
        if(source) {
            game_transfer(g,source->ref,target->ref,command==UI_ONE);
            const ItemStack *at=game_ref_stack(g,source->ref);
            if(command==UI_ACCEPT || !at || !at->count) clear_source(ui);
        } else clear_source(ui);
    }
    return false;
}
int ui_inventory_hit(const InventoryUI *ui,int x,int y)
{
    if(ui_contains(ui_armor_rect(),x,y)) return UI_ARMOR_BUTTON;
    if(ui_contains(ui_close_rect(),x,y)) return UI_CLOSE_BUTTON;
    UISlot slots[UI_MAX_SLOTS];unsigned n=ui_slots(ui,slots);
    for(unsigned i=0;i<n;++i)
        if(ui_contains((UIRect){slots[i].x,slots[i].y,slots[i].w,slots[i].h},x,y))
            return (int)slots[i].id;
    return -1;
}
bool ui_inventory_touch(Game *g,InventoryUI *ui,UITouch event,int x,int y,bool half)
{
    int hit=ui_inventory_hit(ui,x,y);
    if(event==UI_TOUCH_DOWN) {
        ui_inventory_cancel_touch(ui);
        ui->touching=hit>=0;ui->touch_slot=hit;ui->touch_x=x;ui->touch_y=y;
        ui->touch_half=half;
        if(hit>=0 && hit!=UI_ARMOR_BUTTON && hit!=UI_CLOSE_BUTTON) ui->cursor=(unsigned)hit;
        return false;
    }
    if(!ui->touching) return false;
    ui->touch_half|=half;
    if(absolute(x-ui->touch_x)>=6 || absolute(y-ui->touch_y)>=6) ui->dragging=true;
    UISlot slots[UI_MAX_SLOTS];unsigned n=ui_slots(ui,slots);
    const UISlot *start=find(slots,n,(unsigned)ui->touch_slot);
    const UISlot *target=hit>=0?find(slots,n,(unsigned)hit):0;
    if(event==UI_TOUCH_DRAG) {
        if(ui->dragging && start && !start->result) {
            const ItemStack *at=game_ref_stack(g,start->ref);
            if(at && at->count) {ui->source=(int)start->id;ui->source_half=ui->touch_half;}
            if(target) ui->cursor=target->id;
        }
        return false;
    }
    if(ui->dragging) {
        if(start && !start->result && target && !target->result && start!=target) {
            if(ui->touch_half) game_transfer_half(g,start->ref,target->ref);
            else game_transfer(g,start->ref,target->ref,false);
            ui->cursor=target->id;
        }
        clear_source(ui);
    } else if(hit==ui->touch_slot) {
        if(hit==UI_ARMOR_BUTTON) return ui_inventory_key(g,ui,UI_TOGGLE_ARMOR);
        if(hit==UI_CLOSE_BUTTON) return ui_inventory_key(g,ui,UI_CLOSE);
        if(target && target->result) {
            game_craft(g,ui->craft_width==3?3:2);clear_source(ui);
        } else if(target) {
            if(ui->source<0) {
                const ItemStack *at=game_ref_stack(g,target->ref);
                if(at && at->count) {ui->source=hit;ui->source_half=ui->touch_half;}
            } else {
                const UISlot *source=find(slots,n,(unsigned)ui->source);
                if(source && source!=target) {
                    if(ui->source_half || ui->touch_half) game_transfer_half(g,source->ref,target->ref);
                    else game_transfer(g,source->ref,target->ref,false);
                }
                clear_source(ui);
            }
        }
    }
    ui_inventory_cancel_touch(ui);return false;
}
