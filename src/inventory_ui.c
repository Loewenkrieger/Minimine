#include "inventory_ui.h"
#include <limits.h>

unsigned ui_slots(const InventoryUI *ui,UISlot out[UI_MAX_SLOTS])
{
    unsigned n=0;
    for(unsigned i=0;i<TOTAL_SLOTS;++i) {
        int row=(int)i/3,col=(int)i%3;
        out[n++]=(UISlot){i,{SLOT_INVENTORY,i,0},16+98*col,row?140+(row-1)*34:85,92,30,false};
    }
    if(ui->armor) {
        for(unsigned i=0;i<ARMOR_SLOTS;++i)
            out[n++]=(UISlot){UI_ARMOR+i,{SLOT_ARMOR,i,0},
                16+(int)(i%2)*145,312+(int)(i/2)*55,135,40,false};
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
bool ui_inventory_key(Game *g,InventoryUI *ui,UICommand command)
{
    if(command==UI_CLOSE) {
        if(!game_close_crafting(g)) return false;
        ui->source=-1;return true;
    }
    if(command==UI_TOGGLE_ARMOR) {
        ui->source=-1;
        if(!ui->armor) {ui->return_cursor=ui->cursor;ui->cursor=UI_ARMOR;}
        else ui->cursor=ui->return_cursor;
        ui->armor=!ui->armor;return false;
    }
    UISlot slots[UI_MAX_SLOTS];unsigned count=ui_slots(ui,slots);
    const UISlot *target=find(slots,count,ui->cursor);
    if(!target) {ui->cursor=0;target=&slots[0];}
    if(command>=UI_HOTBAR_X && command<=UI_HOTBAR_Z) {
        ui->source=-1;
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
        if(command==UI_ACCEPT) {game_craft(g,ui->craft_width==3?3:2);ui->source=-1;}
        return false;
    }
    if(ui->source<0) {
        const ItemStack *at=game_ref_stack(g,target->ref);
        if(at && at->count) ui->source=(int)target->id;
    } else {
        const UISlot *source=find(slots,count,(unsigned)ui->source);
        if(source) {
            game_transfer(g,source->ref,target->ref,command==UI_ONE);
            const ItemStack *at=game_ref_stack(g,source->ref);
            if(command==UI_ACCEPT || !at || !at->count) ui->source=-1;
        } else ui->source=-1;
    }
    return false;
}
