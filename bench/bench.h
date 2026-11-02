#ifndef MINIMINE_BENCH_H
#define MINIMINE_BENCH_H
#include "game.h"
#include "perf.h"
#include <stddef.h>
#define BENCH_VERSION "1.2.0"
enum { SCENES=5, WARMUP=30, MEASURE=150, COMPARE_SAMPLES=60, COMMAND_LIMIT=4096 };
typedef enum {
    V_REFERENCE,V_HALF,V_LINES,V_XRAM,V_YRAM,V_ILRAM,V_X_IL,
    V_DMA_FULL,V_DMA_32,V_DMA_LINES,V_AZUR,V_AZUR_HALF,
    V_OPTIMIZED,V_REFERENCE_CPU,V_OPTIMIZED_CPU,V_ROW_LIGHT,VARIANTS
} Variant;
typedef struct {const char *id,*name; unsigned scale; bool azur,dma;} VariantInfo;
extern const VariantInfo variants[VARIANTS];
extern const char *const scenes[SCENES];
typedef struct {
    uint32_t total,logic,draw,lcd,sort,shader,commands,state_hash,simulation_frame;
} Measurement;
typedef struct {
    uint32_t expected,actual,commands; bool ok;
} ImageCheck;
typedef struct {int raw,summary,status; unsigned run; bool failed;} BenchLog;
void scene_reset(Game *g,unsigned scene);
void scene_step(Game *g,unsigned scene,unsigned frame);
uint32_t scene_hash(const Game *g);
bool graphics_init(void);
bool graphics_frame(const Game *g,unsigned scene,Variant v,Measurement *m);
ImageCheck graphics_check(const Game *g,unsigned scene,Variant v);
size_t graphics_bytes(void);
size_t graphics_working_ram(Variant v);
size_t graphics_onchip_data(Variant v);
void bench_lcd_begin(void);
uint32_t bench_now(void);
uint32_t bench_us(uint32_t start,uint32_t end);
bool log_open(BenchLog *l,unsigned repeats,int selected);
bool log_status(BenchLog *l,const char *status,unsigned repeat,unsigned scene,int variant);
bool log_block(BenchLog *l,unsigned repeat,unsigned scene,Variant v,
    const Measurement *m,unsigned count,ImageCheck check,const char *status);
void log_close(BenchLog *l);
#endif
