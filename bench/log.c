#include "bench.h"
#include "build_id.h"
#include <gint/clock.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#ifndef BENCH_LOG_ROOT
#define BENCH_LOG_ROOT "/MinimineBench"
#endif
#define ROOT BENCH_LOG_ROOT
#ifdef BENCH_IO_TEST
int bench_test_write(int,const void *,size_t);
int bench_test_close(int);
#define write bench_test_write
#define close bench_test_close
#endif
#ifndef BENCH_HOST
extern char end_user_ram __asm__("_euram"), size_ilram __asm__("_silram");
#define STATIC_RAM_BYTES ((unsigned)((uintptr_t)&end_user_ram-UINT32_C(0x8c052800)))
#define ILRAM_BYTES ((unsigned)(uintptr_t)&size_ilram)
#else
#define STATIC_RAM_BYTES 0u
#define ILRAM_BYTES 0u
#endif
static bool write_all(int fd,const char *p,size_t n)
{
    while(n) {int k=(int)write(fd,p,n);if(k<=0 || (size_t)k>n) return false;p+=k;n-=(size_t)k;}
    return true;
}
static void path(char out[80],unsigned run,const char *kind)
{snprintf(out,80,ROOT "/run%04u-%s",run,kind);}
static bool append(BenchLog *l,const char *kind,const char *text)
{
    char p[80];path(p,l->run,kind);int fd=open(p,O_WRONLY|O_APPEND,0666);
    if(fd<0) {l->failed=true;return false;}
    bool ok=write_all(fd,text,strlen(text));if(close(fd)!=0) ok=false;
    if(!ok) l->failed=true;
    return ok;
}
static bool create(BenchLog *l,const char *kind,const char *text)
{
    char p[80];path(p,l->run,kind);int fd=open(p,O_WRONLY|O_CREAT|O_EXCL,0666);
    if(fd<0) {l->failed=true;return false;}
    bool ok=write_all(fd,text,strlen(text));if(close(fd)!=0) ok=false;
    if(!ok) l->failed=true;
    return ok;
}
bool log_open(BenchLog *l,unsigned repeats,int selected)
{
    memset(l,0,sizeof(*l));
    if(mkdir(ROOT,0777)!=0 && errno!=EEXIST) return false;
    char p[80];int fd=-1;
    for(l->run=1;l->run<=9999;++l->run) {
        path(p,l->run,"meta.txt");fd=open(p,O_WRONLY|O_CREAT|O_EXCL,0666);
        if(fd>=0) break;
        if(errno!=EEXIST) return false;
    }
    if(fd<0) return false;
    const clock_frequency_t *f=clock_freq();char text[4096];
    int n=snprintf(text,sizeof(text),
        "MinimineBench %s\nsource_sha256=%s\ngame_version=%s\nazur_commit=%s\n"
        "toolchain=%s\nbuild_utc=%s\nrun=%u\nrepetitions=%u\nselected_variant=%d\n"
        "warmup_frames=%u\nmeasured_frames=%u\nscene_seed=%08lx\n"
        "reference_game_version=0.5.0\nshort_comparison=%u\n"
        "short_profile=reference_1x;optimized_1x;scanline_light_1x\n"
        "sampling=short_mode_60_evenly_spaced_frames_30_through_179; otherwise_all_150\n"
        "cpu_hz=%d\nbus_hz=%d\nperipheral_hz=%d\n"
        "game_state_bytes=%u\nmeasurement_buffer_bytes=%u\nrenderer_buffers_including_azur_queue_and_fragment=%u\n"
        "static_ram_bytes=%u\nilram_bytes=%u\n"
        "scene_steps=one_60Hz_step_per_simulation_frame; sampled_rendering_in_short_mode\nclock_changes=none\n"
        "native_resolution=320x528; world=320x400\nupscaling_world_resolution=160x200\n"
        "output=full_screen_for_every_variant\nraw_time_unit=microseconds\n"
        "io=only_outside_measured_frames; files_closed_after_each_block\n"
        "draw_us=render_total_us-lcd_us; includes_commands_sort_and_overhead\n"
        "total_us=logic_us+render_total_us; excludes_input_poll_hashes_and_log_io\n"
        "xram_scope=8_row_buffer_at_e5007020\nyram_scope=8_row_buffer_at_e5017020\n"
        "ilram_scope=identical_row_blit_kernel\n"
        "dma_scope=channel0_normal_RAM_sources_synchronous_completion_500ms_timeout\n"
        "azur_scope=upstream_fragment_pipeline_with_shared_tile_sprite_shader\n"
        "azur_compat=LCD_data_command_and_last_partial_fragment_and_queue_read_guard\n"
        "firmware_and_hollyhock_version=please_supply_if_known\n",
        BENCH_VERSION,BENCH_SOURCE_SHA,SANDBOX_VERSION,BENCH_AZUR_SHA,BENCH_TOOLCHAIN,BENCH_BUILD_UTC,
        l->run,repeats,selected,WARMUP,selected==-2?COMPARE_SAMPLES:MEASURE,(unsigned long)WORLD_SEED,selected==-2?1u:0u,
        f->Iphi_f,f->Bphi_f,f->Pphi_f,(unsigned)sizeof(Game),(unsigned)(sizeof(Measurement)*MEASURE),
        (unsigned)graphics_bytes(),STATIC_RAM_BYTES,ILRAM_BYTES);
    bool ok=n>0 && (size_t)n<sizeof(text) && write_all(fd,text,(size_t)n);
    if(close(fd)!=0) ok=false;
    if(!ok) {l->failed=true;return false;}
    return create(l,"frames.csv","repeat,scene,variant,scale,frame,total_us,logic_us,draw_us,lcd_us,azur_sort_us,azur_shader_us,commands,state_hash,simulation_frame\n") &&
        create(l,"summary.csv","repeat,scene,variant,scale,status,frames,fps_x100,mean_us,p95_us,max_us,logic_us,draw_us,lcd_us,image_expected,image_actual,image_ok,commands,working_ram_buffers,onchip_data_buffers\n") &&
        create(l,"status.csv","status,repeat,scene,variant\n");
}
bool log_status(BenchLog *l,const char *status,unsigned repeat,unsigned scene,int v)
{
    char line[180];snprintf(line,sizeof(line),"%s,%u,%s,%s\n",status,repeat,
        scene<SCENES?scenes[scene]:"all",v>=0 && v<VARIANTS?variants[v].id:"all");
    return append(l,"status.csv",line);
}
bool log_block(BenchLog *l,unsigned repeat,unsigned scene,Variant v,const Measurement *m,
    unsigned count,ImageCheck check,const char *status)
{
    char p[80],line[320];path(p,l->run,"frames.csv");int fd=open(p,O_WRONLY|O_APPEND,0666);
    if(fd<0) {l->failed=true;return false;}
    bool ok=true;uint64_t total=0,logic=0,draw=0,lcd=0;
    uint32_t sorted[MEASURE];
    for(unsigned i=0;i<count;++i) {
        int n=snprintf(line,sizeof(line),"%u,%s,%s,%u,%u,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%08lx,%lu\n",
            repeat,scenes[scene],variants[v].id,variants[v].scale,i,
            (unsigned long)m[i].total,(unsigned long)m[i].logic,(unsigned long)m[i].draw,
            (unsigned long)m[i].lcd,(unsigned long)m[i].sort,(unsigned long)m[i].shader,
            (unsigned long)m[i].commands,(unsigned long)m[i].state_hash,(unsigned long)m[i].simulation_frame);
        if(n<0 || (size_t)n>=sizeof(line) || !write_all(fd,line,(size_t)n)) {ok=false;break;}
        total+=m[i].total;logic+=m[i].logic;draw+=m[i].draw;lcd+=m[i].lcd;
        unsigned j=i;while(j && sorted[j-1]>m[i].total) {sorted[j]=sorted[j-1];--j;}sorted[j]=m[i].total;
    }
    if(close(fd)!=0) ok=false;
    if(!ok) {l->failed=true;return false;}
    unsigned p95=count?(count*95+99)/100-1:0;
    snprintf(line,sizeof(line),"%u,%s,%s,%u,%s,%u,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%08lx,%08lx,%u,%lu,%u,%u\n",
        repeat,scenes[scene],variants[v].id,variants[v].scale,status,count,
        (unsigned long)(total?(uint64_t)count*100000000/total:0),
        (unsigned long)(count?total/count:0),(unsigned long)(count?sorted[p95]:0),
        (unsigned long)(count?sorted[count-1]:0),(unsigned long)(count?logic/count:0),
        (unsigned long)(count?draw/count:0),(unsigned long)(count?lcd/count:0),
        (unsigned long)check.expected,(unsigned long)check.actual,check.ok?1u:0u,(unsigned long)check.commands,
        (unsigned)graphics_working_ram(v),(unsigned)graphics_onchip_data(v));
    return append(l,"summary.csv",line);
}
void log_close(BenchLog *l) {(void)l;/* Every operation closes its file immediately. */}
