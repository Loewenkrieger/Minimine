#define _POSIX_C_SOURCE 200809L
#define main benchmark_main
#include "main.c"
#undef main
#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gint/clock.h>
static volatile uint32_t timer_value=UINT32_MAX;
volatile uint32_t *prof_tcnt=&timer_value;
static unsigned events,presentations,cancel_at;
static bool sent_cancel,quit;
static bool short_mode;
extern void (*host_present_hook)(void);
int prof_init(void) {return 0;}
void prof_quit(void) {quit=true;}
uint32_t prof_time(prof_t p) {return p.elapsed;}
const clock_frequency_t *clock_freq(void)
{static clock_frequency_t f={116000000,58000000,29000000};return &f;}
void bench_host_begin(void) {}
void bench_host_send(const uint16_t *p,unsigned n,unsigned s) {(void)p;(void)n;(void)s;}
void azrp_r61523_fragment_x1(void *p,int n) {(void)p;(void)n;}
void azrp_r61523_fragment_x2(void *p,int w,int h) {(void)p;(void)w;(void)h;}
int bench_test_write(int fd,const void *p,size_t n) {return (int)write(fd,p,n);}
int bench_test_close(int fd) {return close(fd);}
static void present(void) {++presentations;--timer_value;}
key_event_t getkey(void)
{
    static const int keys[]={KEY_RIGHT,KEY_RIGHT,KEY_EXE,KEY_EXE,KEY_CLEAR};
    static const int quick[]={KEY_EXE,KEY_EXE,KEY_CLEAR};
    unsigned length=short_mode?3u:5u;
    assert(events<length);
    if(events==0) host_write_ppm("build/native/bench-menu.ppm");
    if(events==length-2) host_write_ppm("build/native/bench-finished.ppm");
    return (key_event_t){.type=KEYEV_DOWN,.key=short_mode?quick[events++]:keys[events++]};
}
key_event_t pollevent(void)
{
    if(cancel_at && presentations>=cancel_at && !sent_cancel) {
        sent_cancel=true;return (key_event_t){.type=KEYEV_DOWN,.key=KEY_CLEAR};
    }
    return (key_event_t){.type=KEYEV_NONE};
}
static unsigned last_run(void)
{
    DIR *d=opendir("build/native/bench-logs");assert(d);unsigned last=0;
    for(struct dirent *e=readdir(d);e;e=readdir(d)) {
        unsigned n;if(sscanf(e->d_name,"run%4u-meta.txt",&n)==1 && n>last) last=n;
    }
    closedir(d);return last;
}
static void exercise(unsigned abort_after,bool quick)
{
    unsigned before=last_run();events=presentations=0;quit=sent_cancel=false;cancel_at=abort_after;
    short_mode=quick;
    host_present_hook=present;assert(benchmark_main()==0);assert(quit && events==(quick?3u:5u));
    unsigned after=last_run();assert(after==before+1);
    char path[180],text[6000];snprintf(path,sizeof(path),"build/native/bench-logs/run%04u-summary.csv",after);
    FILE *f=fopen(path,"rb");assert(f);size_t n=fread(text,1,sizeof(text)-1,f);text[n]=0;fclose(f);
    if(abort_after) {
        assert(sent_cancel && strstr(text,",aborted,"));
        unsigned frames=0;char *p=strstr(text,",aborted,");assert(p);assert(sscanf(p,",aborted,%u",&frames)==1);
        if(abort_after>10) assert(frames>0 && frames<(quick?COMPARE_SAMPLES:MEASURE));
        else assert(frames==0);
    } else {
        for(unsigned i=0;i<SCENES;++i) assert(strstr(text,scenes[i]));
        unsigned lines=0;for(char *p=text;*p;++p) if(*p=='\n') ++lines;assert(lines==(quick?SCENES*3:SCENES)+1);
        assert(!strstr(text,"image_mismatch") && !strstr(text,"render_error"));
        if(quick) {
            assert(strstr(text,"scanline_light_1x"));
            assert(!strstr(text,"reference_cpu_1x") && !strstr(text,"optimized_cpu_1x"));
            snprintf(path,sizeof(path),"build/native/bench-logs/run%04u-frames.csv",after);
            f=fopen(path,"rb");assert(f);char line[400];assert(fgets(line,sizeof(line),f));unsigned rows=0;
            while(fgets(line,sizeof(line),f)) {
                unsigned repeat,scale,frame;char scene[40],variant[60];
                assert(sscanf(line,"%u,%39[^,],%59[^,],%u,%u",&repeat,scene,variant,&scale,&frame)==5);
                assert(frame<COMPARE_SAMPLES && scale==1 && repeat==1);
                unsigned simulation=(unsigned)strtoul(strrchr(line,',')+1,NULL,10);
                assert(simulation==WARMUP+frame*(MEASURE-1)/(COMPARE_SAMPLES-1));++rows;
            }
            fclose(f);assert(rows==SCENES*3*COMPARE_SAMPLES);
        }
    }
}
int main(void)
{
    exercise(0,false);exercise(2,false);exercise(50,false);exercise(0,true);exercise(50,true);
    puts("PASS: real menu/default short comparison, 15 complete blocks and 900 evenly spaced samples, full runs, early/partial abort, durable status and exit");
    return 0;
}
