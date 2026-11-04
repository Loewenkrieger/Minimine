#include "storage.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

enum { FILES=6, CAPACITY=65568 };
typedef struct {bool exists;unsigned size;uint8_t data[CAPACITY];} File;
typedef struct {bool active,writing;unsigned file,pos;} Handle;
typedef struct {
    File files[FILES];Handle handles[4];
    unsigned chunk,write_limit,written,opens,closes;
    bool fail_mkdir,fail_open,fail_read,fail_close,corrupt_write;
} Memory;
static Memory mem;
static Game g,scratch,loaded,original;
static int file_index(const char *p)
{
    const char *name=strrchr(p,'/');unsigned slot=0;char copy=0;
    if(!name || sscanf(name+1,"slot%u%c.sav",&slot,&copy)!=2 || slot<1 || slot>3 || (copy!='a' && copy!='b')) return -1;
    return (int)((slot-1)*2+(copy=='b'));
}
static int mopen(void *context,const char *path,bool writing)
{
    Memory *m=context;int index=file_index(path);assert(index>=0);
    if(m->fail_open) {errno=EACCES;return -1;}
    File *f=&m->files[index];if(!writing && !f->exists) {errno=ENOENT;return -1;}
    for(unsigned fd=0;fd<4;++fd) if(!m->handles[fd].active) {
        m->handles[fd]=(Handle){true,writing,(unsigned)index,0};++m->opens;
        if(writing) {f->exists=true;f->size=0;m->written=0;}
        return (int)fd;
    }
    assert(!"descriptor leak");return -1;
}
static Handle *handle(Memory *m,int fd)
{assert(fd>=0 && fd<4 && m->handles[fd].active);return &m->handles[fd];}
static int mread(void *context,int fd,void *buffer,unsigned n)
{
    Memory *m=context;Handle *h=handle(m,fd);assert(!h->writing);
    if(m->fail_read) {errno=EIO;return -1;}
    File *f=&m->files[h->file];unsigned left=f->size-h->pos;
    if(n>left) n=left;
    if(m->chunk && n>m->chunk) n=m->chunk;
    memcpy(buffer,f->data+h->pos,n);h->pos+=n;return (int)n;
}
static int mwrite(void *context,int fd,const void *buffer,unsigned n)
{
    Memory *m=context;Handle *h=handle(m,fd);assert(h->writing);
    if(m->write_limit && m->written>=m->write_limit) {errno=ENOSPC;return -1;}
    if(m->write_limit && n>m->write_limit-m->written) n=m->write_limit-m->written;
    if(m->chunk && n>m->chunk) n=m->chunk;
    File *f=&m->files[h->file];assert(h->pos+n<=CAPACITY);
    memcpy(f->data+h->pos,buffer,n);if(m->corrupt_write && n) f->data[h->pos]^=1;
    h->pos+=n;m->written+=n;f->size=h->pos;return (int)n;
}
static int mclose(void *context,int fd)
{Memory *m=context;Handle *h=handle(m,fd);h->active=false;++m->closes;return m->fail_close?-1:0;}
static int mmkdir(void *context,const char *path)
{(void)path;return ((Memory *)context)->fail_mkdir?-1:0;}
static SaveIO io={&mem,mopen,mread,mwrite,mclose,mmkdir};
static SaveStore store={"/Minimine",&io};
static void reset(void) {memset(&mem,0,sizeof(mem));game_init_classic(&g,WORLD_SEED);}
static void balanced(void) {assert(mem.opens==mem.closes);for(unsigned i=0;i<4;++i) assert(!mem.handles[i].active);}
static ItemStack stack(Item i,unsigned n)
{return (ItemStack){i,(uint16_t)n,(uint16_t)game_tool_durability(i)};}
static void complex_state(void)
{
    game_init_classic(&g,WORLD_SEED);
    memset(g.tiles,0,sizeof(g.tiles));
    for(int x=0;x<WORLD_W;++x) {g.tiles[20][x]=TILE_STONE;g.surface[x]=20;}
    g.player=(Player){.x=160*FIXED_ONE,.y=306*FIXED_ONE,.vx=-240,.vy=0,.facing=-1,.sneaking=true,.grounded=true};
    g.spawn_x=160*FIXED_ONE;g.spawn_y=298*FIXED_ONE;
    g.aim_x=-1;g.aim_y=1;g.aim_manual=true;g.selected=2;
    g.health=1234;g.hunger=13;g.ticks=98765;g.rng=UINT32_C(0xf1234567);
    g.hurt_ticks=41;g.action_ticks=23;g.swing_ticks=9;g.hunger_ticks=2345;g.exertion=17;
    g.regen_ticks=123;g.regen_cost=234;g.starve_ticks=23;g.spawn_ticks=157;g.respawns=7;g.place_cooldown=8;
    g.slots[0]=stack(ITEM_IRON_SWORD,1);g.slots[0].durability=97;
    g.slots[1]=stack(ITEM_STONE_PICKAXE,1);g.slots[1].durability=31;
    g.slots[2]=stack(ITEM_COOKED_BEEF,32);g.slots[4]=stack(ITEM_IRON_INGOT,64);
    g.slots[3]=stack(ITEM_DIAMOND_PICKAXE,1);g.slots[3].durability=511;
    g.slots[15]=stack(ITEM_DIAMOND,50);g.selected=3;game_set_difficulty(&g,DIFFICULTY_HARD);
    g.crafting[8]=stack(ITEM_LOG,3);
    for(unsigned i=0;i<ARMOR_SLOTS;++i) {g.armor[i]=stack((Item)(ITEM_IRON_HELMET+i),1);g.armor[i].durability=(uint16_t)(7+i);}
    for(unsigned i=0;i<MOB_LIMIT;++i) {
        MobKind k=i<8?(MobKind)(i%3):MOB_ZOMBIE;unsigned h=game_mob_height(k);
        g.mobs[i]=(Mob){.active=true,.kind=k,.health=(uint16_t)(game_mob_health(k)-100),
            .body={.x=(int32_t)(400+i*24)*FIXED_ONE,.y=(320-(int)h)*FIXED_ONE,.vx=96,.vy=0,.facing=1,.grounded=true},
            .ai_ticks=111,.flee_ticks=33,.attack_ticks=51};
    }
    for(unsigned i=0;i<DROP_LIMIT;++i)
        g.drops[i]=(Drop){.active=true,.stack=stack(i%2?ITEM_RAW_PORK:ITEM_WOOD_SWORD,i%2?64:1),
            .x=(int32_t)(100+i*20)*FIXED_ONE,.y=250*FIXED_ONE,.vy=1792,.ttl=17000-i};
    for(unsigned i=0;i<FURNACE_LIMIT;++i) {
        Furnace *f=&g.furnaces[i];*f=(Furnace){.active=true,.x=100+(int)i,.y=19,
            .burn_ticks=1234,.burn_total=4800,.progress=599,.recipe=ITEM_IRON_INGOT};
        f->slots[0]=stack(ITEM_RAW_IRON,32);f->slots[1]=stack(ITEM_COAL,5);f->slots[2]=stack(ITEM_IRON_INGOT,12);
        g.tiles[f->y][f->x]=TILE_FURNACE;
    }
    game_follow_camera(&g);assert(game_validate(&g));
}
static void roundtrips(void)
{
    reset();complex_state();original=g;
    for(unsigned slot=1;slot<=3;++slot) {
        mem.chunk=slot==2?7:0;
        assert(save_slot_write(&store,slot,&g,&scratch)==SAVE_OK);
        SaveInfo info;assert(save_slot_load(&store,slot,&loaded,&info)==SAVE_OK);
        assert(info.generation==1 && !info.recovered && info.copy==0);
        assert(loaded.ticks==98765 && loaded.player.vx==-240 && loaded.player.sneaking);
        assert(loaded.armor[1].durability==8 && loaded.health==1234 && loaded.rng==UINT32_C(0xf1234567));
        assert(loaded.selected==3 && loaded.slots[3].durability==511 && loaded.slots[15].count==50 &&
            loaded.difficulty==DIFFICULTY_HARD && loaded.diamonds_generated && !info.upgraded);
        assert(loaded.furnaces[31].progress==599 && loaded.drops[63].ttl==16937 && loaded.mobs[15].attack_ticks==51);
        assert(game_validate(&loaded));
        /* Canonical re-encoding proves every serialized field survives, including unused pool indices. */
        File previous=mem.files[(slot-1)*2];
        assert(save_slot_write(&store,slot,&loaded,&scratch)==SAVE_OK);
        File *second=&mem.files[(slot-1)*2+1];
        assert(previous.size==second->size && !memcmp(previous.data+32,second->data+32,previous.size-32));
        assert(save_slot_info(&store,slot,&info,&scratch)==SAVE_OK && info.generation==2 && info.copy==1);
        balanced();
    }
    assert(!memcmp(&g,&original,sizeof(g)));
    assert(save_slot_write(&store,1,&g,&g)==SAVE_INVALID);
    puts("PASS: three complete save slots, maximum pools, canonical round trips and seven-byte short I/O");
}
static void backups(void)
{
    reset();SaveInfo info;assert(save_slot_info(&store,1,&info,&scratch)==SAVE_EMPTY);
    assert(save_slot_write(&store,1,&g,&scratch)==SAVE_OK);
    File backup=mem.files[0];
    const unsigned stops[]={1,31,32,511,512,4096,25000};
    for(unsigned i=0;i<sizeof(stops)/sizeof(stops[0]);++i) {
        mem.write_limit=stops[i];g.ticks=100+i;
        assert(save_slot_write(&store,1,&g,&scratch)==SAVE_IO_ERROR);
        assert(!memcmp(&backup,&mem.files[0],sizeof(backup)));
        mem.write_limit=0;
        assert(save_slot_load(&store,1,&loaded,&info)==SAVE_OK && loaded.ticks==0 && info.recovered);
        balanced();
    }
    g.ticks=500;assert(save_slot_write(&store,1,&g,&scratch)==SAVE_OK);
    assert(save_slot_load(&store,1,&loaded,&info)==SAVE_OK && info.generation==2 && loaded.ticks==500);
    mem.files[1].data[33]^=0x80;
    assert(save_slot_load(&store,1,&loaded,&info)==SAVE_OK && loaded.ticks==0 && info.recovered);
    g.ticks=600;assert(save_slot_write(&store,1,&g,&scratch)==SAVE_OK);
    assert(save_slot_load(&store,1,&loaded,&info)==SAVE_OK && loaded.ticks==600 && info.generation==2);
    g.ticks=700;assert(save_slot_write(&store,1,&g,&scratch)==SAVE_OK);
    assert(save_slot_load(&store,1,&loaded,&info)==SAVE_OK && loaded.ticks==700 && info.generation==3 && info.copy==0);
    balanced();puts("PASS: rotating A/B generations, interrupted/no-space writes, recovery and previous-copy preservation");
}
static void put32(uint8_t *p,uint32_t n)
{p[0]=(uint8_t)(n>>24);p[1]=(uint8_t)(n>>16);p[2]=(uint8_t)(n>>8);p[3]=(uint8_t)n;}
static uint32_t crc32(const uint8_t *p,unsigned n)
{
    uint32_t crc=UINT32_MAX;
    for(unsigned i=0;i<n;++i) {crc^=p[i];for(unsigned bit=0;bit<8;++bit) crc=(crc>>1)^((crc&1)?UINT32_C(0xedb88320):0);}
    return crc^UINT32_MAX;
}
static void checksum(File *f)
{memset(f->data+24,0,4);put32(f->data+24,crc32(f->data,f->size));}
static void corruption(void)
{
    reset();assert(crc32((const uint8_t *)"123456789",9)==UINT32_C(0xcbf43926));
    assert(save_slot_write(&store,1,&g,&scratch)==SAVE_OK);File valid=mem.files[0];SaveInfo info;
    const unsigned offsets[]={0,8,12,16,20,24,28,32,1000,24886};
    original=g;
    for(unsigned i=0;i<sizeof(offsets)/sizeof(offsets[0]);++i) {
        mem.files[0]=valid;mem.files[0].data[offsets[i]]^=0x80;
        SaveResult r=save_slot_load(&store,1,&scratch,&info);assert(r==SAVE_INVALID || r==SAVE_INCOMPATIBLE);
        assert(!memcmp(&g,&original,sizeof(g)));balanced();
    }
    const unsigned lengths[]={0,8,31,32,512,24000};
    for(unsigned i=0;i<sizeof(lengths)/sizeof(lengths[0]);++i) {
        mem.files[0]=valid;mem.files[0].size=lengths[i];assert(save_slot_load(&store,1,&scratch,&info)==SAVE_INVALID);
    }
    mem.files[0]=valid;++mem.files[0].size;assert(save_slot_load(&store,1,&scratch,&info)==SAVE_INVALID);
    mem.files[0]=valid;mem.files[0].data[32]=255;checksum(&mem.files[0]);
    assert(save_slot_load(&store,1,&scratch,&info)==SAVE_INVALID); /* Valid CRC, invalid world tile. */
    mem.files[0]=valid;mem.files[0].data[24886]=ITEM_COUNT;checksum(&mem.files[0]);
    assert(save_slot_load(&store,1,&scratch,&info)==SAVE_INVALID); /* Invalid item enum. */
    mem.files[0]=valid;mem.files[0].data[24887]=65;checksum(&mem.files[0]);
    assert(save_slot_load(&store,1,&scratch,&info)==SAVE_INVALID); /* Oversized stack. */
    mem.files[0]=valid;mem.files[0].data[valid.size-2]=DIFFICULTY_COUNT;checksum(&mem.files[0]);
    assert(save_slot_load(&store,1,&scratch,&info)==SAVE_INVALID);
    mem.files[0]=valid;mem.files[0].data[valid.size-1]=2;checksum(&mem.files[0]);
    assert(save_slot_load(&store,1,&scratch,&info)==SAVE_INVALID);
    uint32_t random=123;
    for(unsigned i=0;i<300;++i) {
        mem.files[0]=valid;random=random*1664525u+1013904223u;
        unsigned at=32+random%(valid.size-32);mem.files[0].data[at]^=(uint8_t)(1u<<(random%8));
        checksum(&mem.files[0]);
        SaveResult r=save_slot_load(&store,1,&scratch,&info);
        assert(r!=SAVE_OK || game_validate(&scratch));
        assert(!memcmp(&g,&original,sizeof(g)));
    }
    mem.files[0]=valid;mem.files[0].data[9]=99;checksum(&mem.files[0]);
    assert(save_slot_load(&store,1,&scratch,&info)==SAVE_INCOMPATIBLE);
    assert(save_slot_write(&store,1,&g,&scratch)==SAVE_INCOMPATIBLE);
    balanced();puts("PASS: CRC/header/payload corruption, truncation, extra bytes, unknown versions and semantic bounds");
}
static void failures(void)
{
    reset();mem.fail_mkdir=true;assert(save_slot_write(&store,1,&g,&scratch)==SAVE_IO_ERROR);mem.fail_mkdir=false;
    mem.fail_open=true;assert(save_slot_write(&store,1,&g,&scratch)==SAVE_IO_ERROR);mem.fail_open=false;
    mem.corrupt_write=true;assert(save_slot_write(&store,1,&g,&scratch)==SAVE_IO_ERROR);mem.corrupt_write=false;
    mem.files[0].exists=false;mem.fail_close=true;
    assert(save_slot_write(&store,1,&g,&scratch)==SAVE_IO_ERROR);mem.fail_close=false;
    SaveInfo info;mem.fail_read=true;assert(save_slot_load(&store,1,&scratch,&info)!=SAVE_OK);mem.fail_read=false;
    balanced();assert(save_slot_info(&store,0,&info,&scratch)==SAVE_IO_ERROR);
    assert(save_slot_info(&store,4,&info,&scratch)==SAVE_IO_ERROR);
    puts("PASS: mkdir/open/read/write/close/readback failures and descriptor balance");
}
static void posix_files(void)
{
    SaveStore disk={.root="build/native/save-test"};SaveInfo info;
    /* Only the six fixed test files in the named test directory are replaced. */
    for(unsigned slot=1;slot<=3;++slot) for(unsigned copy=0;copy<2;++copy) {
        char path[80];snprintf(path,sizeof(path),"%s/slot%u%c.sav",disk.root,slot,copy?'b':'a');unlink(path);
    }
    game_init_classic(&g,WORLD_SEED);
    for(unsigned slot=1;slot<=3;++slot) {
        g.ticks=slot;assert(save_slot_write(&disk,slot,&g,&scratch)==SAVE_OK);
        assert(save_slot_load(&disk,slot,&loaded,&info)==SAVE_OK && loaded.ticks==slot);
    }
    puts("PASS: actual host mkdir/open/write/close/read for all three slots");
}
static void legacy(void)
{
    reset();File *f=&mem.files[0];
    FILE *fixture=fopen("tests/fixtures/save-v1.bin","rb");assert(fixture);
    f->size=(unsigned)fread(f->data,1,CAPACITY,fixture);assert(fclose(fixture)==0);
    f->exists=true;assert(f->size>32 && f->data[9]==1);File original_file=*f;
    SaveInfo info;assert(save_slot_load(&store,1,&loaded,&info)==SAVE_OK);
    assert(!loaded.ruins && loaded.player.sneaking && loaded.health==1234 && loaded.hunger==13);
    assert(loaded.ticks==98765 && loaded.slots[0].durability==7 && loaded.armor[0].durability==9);
    assert(game_validate(&loaded));g=loaded;
    assert(save_slot_write(&store,1,&g,&scratch)==SAVE_OK && mem.files[1].data[9]==3);
    assert(!memcmp(f,&original_file,sizeof(*f)));
    assert(save_slot_load(&store,1,&loaded,&info)==SAVE_OK && info.generation==2);
    assert(!loaded.ruins && loaded.slots[0].durability==7 && loaded.armor[0].durability==9 && loaded.ticks==98765);
    assert(loaded.difficulty==DIFFICULTY_MEDIUM && loaded.diamonds_generated && !loaded.slots[3].count);
    balanced();puts("PASS: real 0.4.2 format-1 fixture migrates to format 3 without changing its backup");
}
static void legacy_v2(void)
{
    reset();File *f=&mem.files[0];FILE *fixture=fopen("tests/fixtures/save-v2.bin","rb");assert(fixture);
    f->size=(unsigned)fread(f->data,1,CAPACITY,fixture);assert(fclose(fixture)==0);
    f->exists=true;assert(f->data[9]==2);File backup=*f;SaveInfo info;
    assert(save_slot_load(&store,1,&loaded,&info)==SAVE_OK && info.upgraded);
    assert(loaded.ticks==987654 && loaded.health==1234 && loaded.hunger==13 && loaded.selected==2);
    assert(loaded.slots[0].durability==7 && loaded.slots[1].durability==11 && loaded.slots[2].count==27 &&
        !loaded.slots[3].count && loaded.armor[0].durability==9 && loaded.difficulty==DIFFICULTY_MEDIUM);
    for(unsigned i=3;i<15;++i) assert(loaded.slots[i+1].item==(i%2?ITEM_DIRT:ITEM_PLANKS) && loaded.slots[i+1].count==20+i);
    unsigned ores=0;
    for(int y=0;y<WORLD_H;++y) for(int x=0;x<WORLD_W;++x) if(loaded.tiles[y][x]==TILE_DIAMOND_ORE) ++ores;
    assert(ores==96 && loaded.diamonds_generated && !memcmp(f,&backup,sizeof(backup)));
    bool removed=false;
    for(int y=72;y<94 && !removed;++y) for(int x=0;x<WORLD_W && !removed;++x)
        if(loaded.tiles[y][x]==TILE_DIAMOND_ORE) {loaded.tiles[y][x]=TILE_AIR;removed=true;}
    g=loaded;assert(save_slot_write(&store,1,&g,&scratch)==SAVE_OK && mem.files[1].data[9]==3);
    assert(!memcmp(f,&backup,sizeof(backup)));
    assert(save_slot_load(&store,1,&loaded,&info)==SAVE_OK && !info.upgraded);
    ores=0;for(int y=0;y<WORLD_H;++y) for(int x=0;x<WORLD_W;++x) ores+=loaded.tiles[y][x]==TILE_DIAMOND_ORE;
    assert(ores==95 && game_validate(&loaded));
    balanced();puts("PASS: genuine 0.5.3 format-2 fixture preserves all 15 stacks/durability, gains slot 4 and diamonds once, re-saves format 3 with the old backup intact");
}
int main(void)
{roundtrips();backups();corruption();failures();posix_files();legacy();legacy_v2();puts("All save checks passed.");return 0;}
