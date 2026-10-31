#include "game.h"

typedef struct {
    const char *name,*first,*second;
    Tile tile; ItemKind kind;
    uint16_t durability,fuel; uint8_t armor_slot,armor,food,damage;
    Item smelt;
} ItemDef;
#define BLOCK(n,a,b,t) { .name=n,.first=a,.second=b,.tile=t,.kind=KIND_BLOCK }
#define TOOL(n,a,b,k,h,d) { .name=n,.first=a,.second=b,.kind=k,.durability=h,.damage=d }
#define FOOD(n,a,b,f,s) { .name=n,.first=a,.second=b,.kind=KIND_FOOD,.food=f,.smelt=s }
#define ARMOR(n,a,b,s,p,h) { .name=n,.first=a,.second=b,.kind=KIND_ARMOR,.armor_slot=s,.armor=p,.durability=h }
static const ItemDef defs[ITEM_COUNT]={
    [ITEM_NONE]={.name="Leer",.first="Leer",.second=""},
    [ITEM_DIRT]=BLOCK("Erde","Erde","",TILE_DIRT),
    [ITEM_STONE]=BLOCK("Stein","Stein","",TILE_STONE),
    [ITEM_PLANKS]={.name="Holzbretter",.first="Holz-",.second="bretter",
        .tile=TILE_WOOD,.kind=KIND_BLOCK,.fuel=900},
    [ITEM_LOG]={.name="Baumstamm",.first="Baumstamm",.second="",
        .tile=TILE_LOG,.kind=KIND_BLOCK,.fuel=900},
    [ITEM_STICK]={.name="Stock",.first="Stock",.second="",.fuel=300},
    [ITEM_WORKBENCH]=BLOCK("Werkbank","Werkbank","",TILE_WORKBENCH),
    [ITEM_WOOD_PICKAXE]=TOOL("Holzspitzhacke","Holz-","spitzhacke",KIND_PICKAXE,64,0),
    [ITEM_STONE_PICKAXE]=TOOL("Steinspitzhacke","Stein-","spitzhacke",KIND_PICKAXE,128,0),
    [ITEM_COAL]={.name="Kohle",.first="Kohle",.second="",.fuel=4800},
    [ITEM_RAW_IRON]={.name="Rohes Eisen",.first="Rohes",.second="Eisen",.smelt=ITEM_IRON_INGOT},
    [ITEM_IRON_INGOT]={.name="Eisenbarren",.first="Eisen-",.second="barren"},
    [ITEM_FURNACE]=BLOCK("Ofen","Ofen","",TILE_FURNACE),
    [ITEM_WOOD_SWORD]=TOOL("Holzschwert","Holz-","schwert",KIND_SWORD,64,4),
    [ITEM_STONE_SWORD]=TOOL("Steinschwert","Stein-","schwert",KIND_SWORD,128,5),
    [ITEM_IRON_SWORD]=TOOL("Eisenschwert","Eisen-","schwert",KIND_SWORD,256,6),
    [ITEM_IRON_HELMET]=ARMOR("Eisenhelm","Eisenhelm","",0,2,128),
    [ITEM_IRON_CHEST]=ARMOR("Eisenbrustpanzer","Eisen-","panzer",1,6,256),
    [ITEM_IRON_LEGS]=ARMOR("Eisenhose","Eisenhose","",2,5,192),
    [ITEM_IRON_BOOTS]=ARMOR("Eisenstiefel","Eisen-","stiefel",3,2,128),
    [ITEM_RAW_PORK]=FOOD("Rohes Schweinefleisch","Schwein","roh",3,ITEM_COOKED_PORK),
    [ITEM_COOKED_PORK]=FOOD("Gebratenes Schweinefleisch","Schwein","gebraten",8,ITEM_NONE),
    [ITEM_RAW_BEEF]=FOOD("Rohes Rindfleisch","Rind","roh",3,ITEM_COOKED_BEEF),
    [ITEM_COOKED_BEEF]=FOOD("Gebratenes Rindfleisch","Rind","gebraten",8,ITEM_NONE),
    [ITEM_RAW_CHICKEN]=FOOD("Rohes Huehnerfleisch","Huhn","roh",2,ITEM_COOKED_CHICKEN),
    [ITEM_COOKED_CHICKEN]=FOOD("Gebratenes Huehnerfleisch","Huhn","gebraten",6,ITEM_NONE)
};
static const ItemDef *def(Item item)
{ return &defs[(unsigned)item<ITEM_COUNT?item:ITEM_NONE]; }
const char *game_item_name(Item item) {return def(item)->name;}
const char *game_item_label(Item item,unsigned line)
{return line?def(item)->second:def(item)->first;}
Tile game_item_tile(Item item) {return def(item)->tile;}
ItemKind game_item_kind(Item item) {return def(item)->kind;}
unsigned game_tool_durability(Item item) {return def(item)->durability;}
unsigned game_stack_limit(Item item)
{return item<=ITEM_NONE || item>=ITEM_COUNT?0:def(item)->durability?1:64;}
unsigned game_armor_slot(Item item)
{return def(item)->kind==KIND_ARMOR?def(item)->armor_slot:ARMOR_SLOTS;}
unsigned game_armor_points(Item item) {return def(item)->armor;}
unsigned game_food_points(Item item) {return def(item)->food;}
unsigned game_sword_damage(Item item) {return def(item)->damage;}
unsigned game_fuel_ticks(Item item) {return def(item)->fuel;}
Item game_smelt_result(Item item) {return def(item)->smelt;}
bool game_stack_valid(const ItemStack *s)
{
    if(!s->count) return s->item==ITEM_NONE && s->durability==0;
    unsigned max=game_tool_durability(s->item);
    return s->count<=game_stack_limit(s->item) &&
        (max?s->durability>0 && s->durability<=max:s->durability==0);
}
const char *game_notice_text(Notice n)
{
    static const char *const text[]={"","Inventar voll: Platz schaffen","Kein Platz",
        "Gegenstand zerbrochen","Spitzhacke benoetigt","Zu viele Oefen",
        "Passt nicht in diesen Slot","Zurueck am Startpunkt"};
    return (unsigned)n<sizeof(text)/sizeof(text[0])?text[n]:"";
}
