#define _POSIX_C_SOURCE 200809L
#include "bench.h"
#include <gint/display.h>
#include <gint/clock.h>
#include <gint/prof.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
static uint16_t lcd[SCREEN_W*SCREEN_H],reference[SCREEN_W*VIEW_H];
static unsigned cursor;
static Game g,copy;
static int fail_after=-1,short_write,close_failure;
static unsigned bytes_written;
uint32_t bench_now(void)
{struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return UINT32_MAX-(uint32_t)((uint64_t)t.tv_sec*1000000+(unsigned long)t.tv_nsec/1000);}
uint32_t bench_us(uint32_t start,uint32_t end) {return start-end;}
uint32_t prof_time(prof_t p) {return p.elapsed;}
void bench_host_begin(void) {cursor=0;}
void bench_host_send(const uint16_t *p,unsigned n,unsigned scale)
{
    assert(cursor+n*scale<=SCREEN_W*SCREEN_H);
    for(unsigned i=0;i<n;++i) for(unsigned x=0;x<scale;++x) lcd[cursor++]=p[i];
}
void azrp_r61523_fragment_x1(void *p,int n) {bench_host_send(p,(unsigned)n,1);}
void azrp_r61523_fragment_x2(void *p,int width,int height)
{
    uint16_t *src=p;
    for(int y=0;y<height;++y) {bench_host_send(src,(unsigned)width,2);bench_host_send(src,(unsigned)width,2);src+=width;}
}
static void present(void) {bench_host_begin();bench_host_send(gint_vram,SCREEN_W*SCREEN_H,1);}
extern void (*host_present_hook)(void);
void host_check_guards(void);
const clock_frequency_t *clock_freq(void)
{static clock_frequency_t f={116000000,58000000,29000000};return &f;}
int bench_test_write(int fd,const void *p,size_t n)
{
    if(fail_after>=0 && bytes_written>=(unsigned)fail_after) {errno=ENOSPC;return -1;}
    if(short_write && n>7) n=7;
    int k=(int)write(fd,p,n);if(k>0) bytes_written+=(unsigned)k;return k;
}
int bench_test_close(int fd)
{int rc=close(fd);if(close_failure) {errno=EIO;return -1;}return rc;}
static void test_pixels(void)
{
    assert(graphics_init());
    host_present_hook=present;
    for(unsigned scene=0;scene<SCENES;++scene) {
        scene_reset(&g,scene);copy=g;
        if(scene>=3) assert(game_validate(&g));
        if(scene==4) {unsigned n=0;for(unsigned i=0;i<MOB_LIMIT;++i) n+=g.mobs[i].active;assert(n==16);}
        for(unsigned frame=0;frame<180;++frame) {
            scene_step(&g,scene,frame);scene_step(&copy,scene,frame);
            assert(scene_hash(&g)==scene_hash(&copy));
            if(scene>=3) assert(game_validate(&g));
            if(frame!=0 && frame!=89 && frame!=179) continue;
            Measurement m={0};dclear(1234);
            assert(graphics_frame(&g,scene,V_REFERENCE,&m));assert(cursor==SCREEN_W*SCREEN_H);
            memcpy(reference,lcd+VIEW_TOP*SCREEN_W,sizeof(reference));
            for(unsigned v=0;v<VARIANTS;++v) {
                ImageCheck c=graphics_check(&g,scene,(Variant)v);
                if(!c.ok) fprintf(stderr,"image mismatch scene=%u frame=%u variant=%s expected=%08x actual=%08x commands=%u\n",scene,frame,variants[v].id,c.expected,c.actual,c.commands);
                assert(c.ok);
                dclear(1234);memset(&m,0,sizeof(m));
                for(unsigned y=0;y<SCREEN_H;++y) if(y<VIEW_TOP || y>=VIEW_TOP+VIEW_H)
                    for(unsigned x=0;x<SCREEN_W;++x) gint_vram[y*SCREEN_W+x]=(uint16_t)((y*377+x*19)&65535);
                assert(graphics_frame(&g,scene,(Variant)v,&m));assert(cursor==SCREEN_W*SCREEN_H);
                unsigned scale=variants[v].scale;
                for(unsigned y=0;y<VIEW_H;++y) for(unsigned x=0;x<SCREEN_W;++x) {
                    uint16_t expected=reference[(y/scale*scale)*SCREEN_W+x/scale*scale];
                    if(lcd[(VIEW_TOP+y)*SCREEN_W+x]!=expected) {
                        fprintf(stderr,"transport mismatch scene=%u frame=%u variant=%s pixel=%u,%u\n",scene,frame,variants[v].id,x,y);abort();
                    }
                }
                for(unsigned y=0;y<SCREEN_H;++y) if(y<VIEW_TOP || y>=VIEW_TOP+VIEW_H)
                    for(unsigned x=0;x<SCREEN_W;++x)
                        assert(lcd[y*SCREEN_W+x]==(uint16_t)(((y/scale*scale)*377+(x/scale*scale)*19)&65535));
                host_check_guards();
            }
        }
    }
    puts("PASS: 16 pipelines, 5 scenes, 3 snapshots, old/new native and 2x pixel equality, full LCD transfer, deterministic state, framebuffer guards");
}
static void read_file(unsigned run,const char *kind,char *out,size_t size)
{
    char path[160];snprintf(path,sizeof(path),"build/native/bench-logs/run%04u-%s",run,kind);
    FILE *f=fopen(path,"rb");assert(f);size_t n=fread(out,1,size-1,f);out[n]=0;assert(!ferror(f));fclose(f);
}
static void test_logs(void)
{
    BenchLog first,second;short_write=1;
    assert(log_open(&first,3,-1));assert(log_status(&first,"block_started",1,0,V_HALF));
    Measurement m[MEASURE];memset(m,0,sizeof(m));
    for(unsigned i=0;i<MEASURE;++i) {m[i].total=10000+i*100;m[i].logic=10;m[i].lcd=2000;m[i].draw=m[i].total-2010;m[i].state_hash=i;}
    ImageCheck c={123,123,900,true};assert(log_block(&first,1,0,V_HALF,m,MEASURE,c,"ok"));
    assert(log_status(&first,"run_complete",0,SCENES,-1));log_close(&first);
    char text[40000];read_file(first.run,"frames.csv",text,sizeof(text));unsigned lines=0;
    for(char *p=text;*p;++p) if(*p=='\n') ++lines;
    assert(lines==151);
    read_file(first.run,"summary.csv",text,sizeof(text));assert(strstr(text,",150,5730,17450,24200,24900,10,15440,2000,"));
    read_file(first.run,"meta.txt",text,sizeof(text));assert(strstr(text,"game_version=" SANDBOX_VERSION));assert(strstr(text,"peripheral_hz=29000000"));
    assert(log_open(&second,1,V_REFERENCE));assert(second.run==first.run+1);
    assert(log_block(&second,1,0,V_REFERENCE,m,2,c,"aborted"));assert(log_status(&second,"run_aborted",0,SCENES,-1));
    read_file(first.run,"summary.csv",text,sizeof(text));assert(strstr(text,",150,5730,"));
    fail_after=(int)bytes_written;assert(!log_status(&second,"out_of_space",0,SCENES,-1));assert(second.failed);fail_after=-1;
    BenchLog third;assert(log_open(&third,1,-1));close_failure=1;
    assert(!log_status(&third,"close_error",0,SCENES,-1));assert(third.failed);close_failure=0;
    puts("PASS: CSV headers/raw frames/percentiles, short writes, unique runs, partial abort, disk-full and close failures");
}
int main(void) {test_pixels();test_logs();return 0;}
