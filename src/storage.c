#include "storage.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

enum { HEADER_SIZE=32, FORMAT_VERSION=2, MAX_PAYLOAD=65536 };
static int posix_open(void *c,const char *p,bool w)
{(void)c;return open(p,w?O_WRONLY|O_CREAT|O_TRUNC:O_RDONLY,0666);}
static int posix_read(void *c,int fd,void *p,unsigned n)
{(void)c;return (int)read(fd,p,n);}
static int posix_write(void *c,int fd,const void *p,unsigned n)
{(void)c;return (int)write(fd,p,n);}
static int posix_close(void *c,int fd) {(void)c;return close(fd);}
static int posix_mkdir(void *c,const char *p)
{(void)c;return mkdir(p,0777)==0 || errno==EEXIST?0:-1;}
static const SaveIO posix_io={NULL,posix_open,posix_read,posix_write,posix_close,posix_mkdir};
static const SaveIO *ops(const SaveStore *s) {return s->io?s->io:&posix_io;}
static uint32_t crc_byte(uint32_t crc,uint8_t b)
{
    crc^=b;for(unsigned i=0;i<8;++i) crc=(crc>>1)^((crc&1)?UINT32_C(0xedb88320):0);
    return crc;
}
static uint32_t header_crc(const uint8_t h[HEADER_SIZE])
{
    uint32_t crc=UINT32_MAX;
    for(unsigned i=0;i<HEADER_SIZE;++i) crc=crc_byte(crc,i>=24 && i<28?0:h[i]);
    return crc;
}
static uint32_t get32(const uint8_t *p)
{return (uint32_t)p[0]<<24|(uint32_t)p[1]<<16|(uint32_t)p[2]<<8|p[3];}
static void put32(uint8_t *p,uint32_t v)
{p[0]=(uint8_t)(v>>24);p[1]=(uint8_t)(v>>16);p[2]=(uint8_t)(v>>8);p[3]=(uint8_t)v;}
typedef struct {
    const SaveStore *store;int fd;bool reading,measure,failed;
    uint32_t crc,total,limit;unsigned used,pos,version;uint8_t buffer[512];
} Codec;
static void flush(Codec *c)
{
    if(c->reading || c->measure || c->failed) return;
    const SaveIO *io=ops(c->store);unsigned at=0;
    while(at<c->used) {
        int n=io->write(io->context,c->fd,c->buffer+at,c->used-at);
        if(n<=0 || (unsigned)n>c->used-at) {c->failed=true;return;}
        at+=(unsigned)n;
    }
    c->used=0;
}
static uint8_t byte(Codec *c,uint8_t value)
{
    if(c->failed) return 0;
    if(c->reading) {
        if(c->total>=c->limit) {c->failed=true;return 0;}
        if(c->pos==c->used) {
            unsigned remaining=c->limit-c->total;
            unsigned want=remaining<sizeof(c->buffer)?remaining:(unsigned)sizeof(c->buffer);
            const SaveIO *io=ops(c->store);
            int n=io->read(io->context,c->fd,c->buffer,want);
            if(n<=0 || (unsigned)n>want) {c->failed=true;return 0;}
            c->used=(unsigned)n;c->pos=0;
        }
        value=c->buffer[c->pos++];
    } else if(!c->measure) {
        c->buffer[c->used++]=value;
        if(c->used==sizeof(c->buffer)) flush(c);
    }
    c->crc=crc_byte(c->crc,value);++c->total;return value;
}
static uint32_t u32(Codec *c,uint32_t v)
{
    uint32_t out=0;
    for(int shift=24;shift>=0;shift-=8) out=(out<<8)|byte(c,(uint8_t)(v>>(unsigned)shift));
    return out;
}
static int32_t i32(Codec *c,int32_t v)
{
    uint32_t n=u32(c,(uint32_t)v);
    return n<=INT32_MAX?(int32_t)n:-1-(int32_t)(UINT32_MAX-n);
}
static uint16_t u16(Codec *c,uint16_t v)
{
    uint8_t high=byte(c,(uint8_t)(v>>8));
    uint8_t low=byte(c,(uint8_t)v);
    return (uint16_t)((uint16_t)high<<8|low);
}
static bool boolean(Codec *c,bool v)
{
    uint8_t n=byte(c,(uint8_t)v);if(n>1) c->failed=true;return n!=0;
}
#define U(x) do {uint32_t n=u32(c,(uint32_t)(x));if(c->reading) (x)=n;} while(0)
#define I(x) do {int32_t n=i32(c,(int32_t)(x));if(c->reading) (x)=n;} while(0)
#define B(x) do {bool n=boolean(c,(x));if(c->reading) (x)=n;} while(0)
static void stack(Codec *c,ItemStack *s)
{
    uint8_t item=byte(c,(uint8_t)s->item),count=byte(c,(uint8_t)s->count);
    uint16_t durability=u16(c,s->durability);
    if(c->reading) *s=(ItemStack){(Item)item,count,durability};
}
static void body(Codec *c,Player *p)
{I(p->x);I(p->y);I(p->vx);I(p->vy);I(p->facing);B(p->grounded);B(p->sneaking);}
static void visit(Codec *c,Game *g)
{
    for(unsigned y=0;y<WORLD_H;++y) for(unsigned x=0;x<WORLD_W;++x) {
        uint8_t n=byte(c,g->tiles[y][x]);if(c->reading) g->tiles[y][x]=n;
    }
    for(unsigned x=0;x<WORLD_W;++x) {
        uint8_t n=byte(c,g->surface[x]);if(c->reading) g->surface[x]=n;
    }
    body(c,&g->player);
    for(unsigned i=0;i<TOTAL_SLOTS;++i) stack(c,&g->slots[i]);
    for(unsigned i=0;i<CRAFT_SLOTS;++i) stack(c,&g->crafting[i]);
    for(unsigned i=0;i<ARMOR_SLOTS;++i) stack(c,&g->armor[i]);
    U(g->selected);I(g->aim_x);I(g->aim_y);B(g->aim_manual);
    I(g->camera_x);I(g->camera_y);U(g->ticks);U(g->seed);U(g->rng);
    U(g->place_cooldown);
    uint16_t hp=u16(c,g->health);uint8_t hunger=byte(c,g->hunger);
    if(c->reading) {g->health=hp;g->hunger=hunger;}
    U(g->hurt_ticks);U(g->action_ticks);U(g->swing_ticks);
    U(g->hunger_ticks);U(g->exertion);U(g->regen_ticks);U(g->regen_cost);U(g->starve_ticks);
    U(g->spawn_ticks);U(g->respawns);I(g->spawn_x);I(g->spawn_y);
    for(unsigned i=0;i<MOB_LIMIT;++i) {
        Mob *m=&g->mobs[i];B(m->active);if(!m->active) continue;
        uint8_t k=byte(c,(uint8_t)m->kind);if(c->reading) m->kind=(MobKind)k;
        body(c,&m->body);hp=u16(c,m->health);if(c->reading) m->health=hp;
        U(m->ai_ticks);U(m->flee_ticks);U(m->attack_ticks);
    }
    for(unsigned i=0;i<DROP_LIMIT;++i) {
        Drop *d=&g->drops[i];B(d->active);if(!d->active) continue;
        stack(c,&d->stack);I(d->x);I(d->y);I(d->vy);U(d->ttl);
    }
    for(unsigned i=0;i<FURNACE_LIMIT;++i) {
        Furnace *f=&g->furnaces[i];B(f->active);if(!f->active) continue;
        I(f->x);I(f->y);for(unsigned j=0;j<3;++j) stack(c,&f->slots[j]);
        U(f->burn_ticks);U(f->burn_total);U(f->progress);
        uint8_t r=byte(c,(uint8_t)f->recipe);if(c->reading) f->recipe=(Item)r;
    }
    if(c->version<2) return;
    for(unsigned i=0;i<MOB_LIMIT;++i) if(g->mobs[i].active) {
        U(g->mobs[i].state);U(g->mobs[i].state_ticks);U(g->mobs[i].hurt_ticks);
    }
    for(unsigned i=0;i<ACCESSORY_SLOTS;++i) stack(c,&g->accessories[i]);
    for(unsigned i=0;i<CHEST_LIMIT;++i) {
        Chest *ch=&g->chests[i];B(ch->active);if(!ch->active) continue;
        I(ch->x);I(ch->y);for(unsigned j=0;j<CHEST_SLOTS;++j) stack(c,&ch->slots[j]);
    }
    for(unsigned i=0;i<TORCH_LIMIT;++i) {
        Torch *t=&g->torches[i];B(t->active);if(t->active) {I(t->x);I(t->y);}
    }
    B(g->ruins);B(g->bed_spawn);B(g->double_jump_used);I(g->bed_x);I(g->bed_y);
    U(g->relics_found);U(g->mined_blocks);
    Boss *b=&g->boss;B(b->gate_open);B(b->active);B(b->defeated);B(b->reward_pending);
    U(b->phase);U(b->timer);U(b->pattern);U(b->shock_ticks);I(b->direction);I(b->shock_x);

}
static bool path_for(const SaveStore *s,unsigned slot,unsigned copy,char path[128])
{
    if(!s->root || slot<1 || slot>3 || copy>1) return false;
    int n=snprintf(path,128,"%s/slot%u%c.sav",s->root,slot,copy?'b':'a');
    return n>0 && n<128;
}
static bool read_exact(const SaveStore *s,int fd,uint8_t *buf,unsigned n)
{
    const SaveIO *io=ops(s);unsigned at=0;
    while(at<n) {
        int got=io->read(io->context,fd,buf+at,n-at);
        if(got<=0 || (unsigned)got>n-at) return false;
        at+=(unsigned)got;
    }
    return true;
}
static SaveResult candidate(const SaveStore *s,unsigned slot,unsigned copy,Game *g,SaveInfo *info)
{
    char path[128];if(!path_for(s,slot,copy,path)) return SAVE_IO_ERROR;
    const SaveIO *io=ops(s);errno=0;int fd=io->open(io->context,path,false);
    if(fd<0) return errno==ENOENT?SAVE_EMPTY:SAVE_IO_ERROR;
    uint8_t h[HEADER_SIZE];SaveResult result=SAVE_INVALID;
    if(!read_exact(s,fd,h,sizeof(h))) goto done;
    if(memcmp(h,"MINIMINE",8) || h[10]!=0 || h[11]!=HEADER_SIZE || get32(h+28)!=0) goto done;
    if(h[8]!=0 || (h[9]!=1 && h[9]!=FORMAT_VERSION)) {result=SAVE_INCOMPATIBLE;goto done;}
    uint32_t length=get32(h+12);if(!length || length>MAX_PAYLOAD) goto done;
    memset(g,0,sizeof(*g));g->survival=true;
    Codec c={.store=s,.fd=fd,.reading=true,.version=h[9],.limit=length,.crc=header_crc(h)};
    visit(&c,g);
    if(c.failed || c.total!=length || (c.crc^UINT32_MAX)!=get32(h+24)) goto done;
    uint8_t extra;int trailing=io->read(io->context,fd,&extra,1);
    if(trailing!=0 || !game_validate(g)) goto done;
    *info=(SaveInfo){.status=SAVE_OK,.generation=(uint64_t)get32(h+16)<<32|get32(h+20),
        .ticks=g->ticks,.health=g->health,.hunger=g->hunger,.copy=copy};
    if(!info->generation) goto done;
    result=SAVE_OK;
done:
    if(io->close(io->context,fd)<0) result=SAVE_IO_ERROR;
    return result;
}
SaveResult save_slot_info(const SaveStore *s,unsigned slot,SaveInfo *info,Game *scratch)
{
    SaveInfo a={0},b={0};
    SaveResult ra=candidate(s,slot,0,scratch,&a),rb=candidate(s,slot,1,scratch,&b);
    if(ra==SAVE_OK || rb==SAVE_OK) {
        *info=ra==SAVE_OK && (rb!=SAVE_OK || a.generation>=b.generation)?a:b;
        info->recovered=(ra!=SAVE_OK && ra!=SAVE_EMPTY)||(rb!=SAVE_OK && rb!=SAVE_EMPTY);
        return SAVE_OK;
    }
    SaveResult r=ra==SAVE_EMPTY && rb==SAVE_EMPTY?SAVE_EMPTY:
        ra==SAVE_INCOMPATIBLE || rb==SAVE_INCOMPATIBLE?SAVE_INCOMPATIBLE:
        ra==SAVE_IO_ERROR || rb==SAVE_IO_ERROR?SAVE_IO_ERROR:SAVE_INVALID;
    *info=(SaveInfo){.status=r};return r;
}
SaveResult save_slot_load(const SaveStore *s,unsigned slot,Game *scratch,SaveInfo *info)
{
    SaveInfo chosen;SaveResult r=save_slot_info(s,slot,&chosen,scratch);
    if(r!=SAVE_OK) return r;
    r=candidate(s,slot,chosen.copy,scratch,info);
    if(r==SAVE_OK) {info->recovered=chosen.recovered;game_cancel_mining(scratch);}
    return r;
}
SaveResult save_slot_write(const SaveStore *s,unsigned slot,const Game *g,Game *scratch)
{
    if(!game_validate(g) || g==scratch) return SAVE_INVALID;
    SaveInfo previous;SaveResult prior=save_slot_info(s,slot,&previous,scratch);
    if(prior==SAVE_IO_ERROR || prior==SAVE_INCOMPATIBLE) return prior;
    uint64_t generation=prior==SAVE_OK?previous.generation+1:1;
    if(!generation) return SAVE_IO_ERROR;
    unsigned copy=prior==SAVE_OK?1-previous.copy:0;
    char path[128];if(!path_for(s,slot,copy,path)) return SAVE_IO_ERROR;
    const SaveIO *io=ops(s);
    if(io->mkdir(io->context,s->root)<0) return SAVE_IO_ERROR;
    Codec c={.store=s,.measure=true,.version=FORMAT_VERSION};
    /* Encoding never writes through g when reading=false. */
    visit(&c,(Game *)g);uint32_t length=c.total;
    if(length>MAX_PAYLOAD) return SAVE_INVALID;
    uint8_t header[HEADER_SIZE]={0};memcpy(header,"MINIMINE",8);
    header[9]=FORMAT_VERSION;header[11]=HEADER_SIZE;put32(header+12,length);
    put32(header+16,(uint32_t)(generation>>32));put32(header+20,(uint32_t)generation);
    c=(Codec){.store=s,.measure=true,.version=FORMAT_VERSION,.crc=header_crc(header)};
    visit(&c,(Game *)g);put32(header+24,c.crc^UINT32_MAX);
    int fd=io->open(io->context,path,true);if(fd<0) return SAVE_IO_ERROR;
    c=(Codec){.store=s,.fd=fd,.version=FORMAT_VERSION};
    for(unsigned i=0;i<HEADER_SIZE;++i) byte(&c,header[i]);
    visit(&c,(Game *)g);flush(&c);
    bool failed=c.failed;if(io->close(io->context,fd)<0) failed=true;
    if(failed) return SAVE_IO_ERROR;
    SaveInfo checked;
    if(candidate(s,slot,copy,scratch,&checked)!=SAVE_OK || checked.generation!=generation)
        return SAVE_IO_ERROR;
    return SAVE_OK;
}
const char *save_result_text(SaveResult r)
{
    static const char *const text[]={"Gespeichert","Leer","Spielstand beschaedigt",
        "Spielstand-Version unbekannt","Dateizugriff fehlgeschlagen"};
    return (unsigned)r<sizeof(text)/sizeof(text[0])?text[r]:"Fehler";
}
