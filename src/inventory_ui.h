#ifndef MINIMINE_INVENTORY_UI_H
#define MINIMINE_INVENTORY_UI_H
#include "game.h"
enum { UI_RESULT=CRAFT_RESULT, UI_ARMOR=UI_RESULT+1, UI_FURNACE=UI_ARMOR+ARMOR_SLOTS,
    UI_MAX_SLOTS=TOTAL_SLOTS+CHEST_SLOTS, UI_ARMOR_BUTTON=UI_FURNACE+3,
    UI_CLOSE_BUTTON=UI_ARMOR_BUTTON+1,UI_ACCESSORY=UI_CLOSE_BUTTON+1,
    UI_CHEST=UI_ACCESSORY+ACCESSORY_SLOTS,UI_ID_COUNT=UI_CHEST+CHEST_SLOTS };
typedef struct {
    unsigned cursor; int source; unsigned craft_width;
    bool armor; unsigned furnace,return_cursor,chest; /* furnace is index+1; zero means crafting. */
    bool touching,dragging,touch_half,source_half;
    int touch_slot,touch_x,touch_y;
} InventoryUI;
typedef struct {
    unsigned id; SlotRef ref; int x,y,w,h; bool result;
} UISlot;
typedef enum {
    UI_LEFT,UI_RIGHT,UI_UP,UI_DOWN,UI_ACCEPT,UI_ONE,UI_CLOSE,UI_TOGGLE_ARMOR,
    UI_HOTBAR_X,UI_HOTBAR_Y,UI_HOTBAR_Z,UI_HOTBAR_POWER,UI_NONE
} UICommand;
unsigned ui_slots(const InventoryUI *ui,UISlot out[UI_MAX_SLOTS]);
bool ui_inventory_key(Game *game,InventoryUI *ui,UICommand command);
typedef enum {UI_TOUCH_DOWN,UI_TOUCH_DRAG,UI_TOUCH_UP} UITouch;
int ui_inventory_hit(const InventoryUI *ui,int x,int y);
void ui_inventory_cancel_touch(InventoryUI *ui);
bool ui_inventory_touch(Game *game,InventoryUI *ui,UITouch event,int x,int y,bool half);
#endif
