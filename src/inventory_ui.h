#ifndef MINIMINE_INVENTORY_UI_H
#define MINIMINE_INVENTORY_UI_H
#include "game.h"
enum { UI_RESULT=24, UI_ARMOR=25, UI_FURNACE=29, UI_MAX_SLOTS=25 };
typedef struct {
    unsigned cursor; int source; unsigned craft_width;
    bool armor; unsigned furnace,return_cursor; /* furnace is index+1; zero means crafting. */
} InventoryUI;
typedef struct {
    unsigned id; SlotRef ref; int x,y,w,h; bool result;
} UISlot;
typedef enum {
    UI_LEFT,UI_RIGHT,UI_UP,UI_DOWN,UI_ACCEPT,UI_ONE,UI_CLOSE,UI_TOGGLE_ARMOR,
    UI_HOTBAR_X,UI_HOTBAR_Y,UI_HOTBAR_Z,UI_NONE
} UICommand;
unsigned ui_slots(const InventoryUI *ui,UISlot out[UI_MAX_SLOTS]);
bool ui_inventory_key(Game *game,InventoryUI *ui,UICommand command);
#endif
