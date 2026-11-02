#include "bench.h"
#include <gint/config.h>
#include <gint/display.h>
#include <gint/keyboard.h>
#include <gint/prof.h>
#include <string.h>
HHK_NAME("Minimine Bench")
HHK_DESCRIPTION("Renderer comparison with CSV logs")
HHK_AUTHOR("Minimine project")
HHK_VERSION(BENCH_VERSION)
static Game world;
static Measurement samples[MEASURE];
static bool initialized;
uint32_t bench_now(void) {return *prof_tcnt;}
uint32_t bench_us(uint32_t start,uint32_t end)
{prof_t p=prof_make();p.elapsed=start-end;return prof_time(p);}
static bool cancelled(void)
{
    bool stop=false;
    for(key_event_t e=pollevent();e.type!=KEYEV_NONE;e=pollevent())
        if(e.type==KEYEV_DOWN && e.key==KEY_CLEAR) stop=true;
    return stop;
}
static void block_screen(unsigned run,unsigned repeat,unsigned repeats,unsigned scene,Variant v,unsigned block,unsigned total)
{
    dclear(C_RGB(2,3,5));dprint(8,5,C_WHITE,"TEST %u/%u  %u%%  LOG %u",block,total,(block-1)*100/total,run);
    dprint(8,25,C_WHITE,"%s  %u/%u",variants[v].name,repeat,repeats);
    dtext(8,VIEW_TOP+VIEW_H+8,C_WHITE,scenes[scene]);
    dtext(8,VIEW_TOP+VIEW_H+28,C_WHITE,"CLEAR: Abbrechen + Logs behalten");
    dupdate();
}
static void result(const char *message,unsigned run)
{
    dclear(C_RGB(2,3,5));dtext(10,40,C_WHITE,message);
    if(run) {
        dtext(10,80,C_WHITE,"Logs im Ordner /MinimineBench");
        dprint(10,110,C_WHITE,"Dateien: run%04u-*",run);
        dtext(10,150,C_WHITE,"Bitte alle Dateien dieses Laufs senden.");
    }
    dtext(10,220,C_WHITE,"EXE: Zurueck");dupdate();
    while(getkey().key!=KEY_EXE) {}
}
static void run_bench(unsigned repeats,int selected)
{
    if(!initialized) {initialized=graphics_init();if(!initialized) {result("Grafikstart fehlgeschlagen",0);return;}}
    BenchLog log;
    if(!log_open(&log,repeats,selected)) {result("Log-Dateien konnten nicht angelegt werden",log.run);return;}
    bool abort=false,error=false,variant_error=false;
    static const Variant comparison[]={V_REFERENCE,V_OPTIMIZED,V_ROW_LIGHT};
    unsigned variants_count=selected==-2?3u:selected<0?VARIANTS:1u;
    unsigned wanted=selected==-2?COMPARE_SAMPLES:MEASURE,block=0,total=repeats*SCENES*variants_count;
    log_status(&log,"run_started",0,SCENES,-1);
    for(unsigned rep=0;rep<repeats && !abort && !error;++rep)
        for(unsigned scene=0;scene<SCENES && !abort && !error;++scene)
            for(unsigned order=0;order<variants_count && !abort && !error;++order) {
                /* Rotating order avoids always running the reference first. */
                Variant v=selected==-2?comparison[(order+rep)%3]:
                    (Variant)(selected<0?(order+rep)%VARIANTS:(unsigned)selected);
                if(!log_status(&log,"block_started",rep+1,scene,(int)v)) {error=true;break;}
                ++block;block_screen(log.run,rep+1,repeats,scene,v,block,total);scene_reset(&world,scene);
                ImageCheck check={0};unsigned count=0;
                bool failed=false;
                /* Check three deterministic snapshots before timed playback. */
                for(unsigned frame=0;frame<WARMUP+MEASURE;++frame) {
                    scene_step(&world,scene,frame);
                    if(frame==0 || frame==89 || frame==179) {
                        check=graphics_check(&world,scene,v);
                        if(!check.ok) {failed=true;break;}
                    }
                    if(cancelled()) {abort=true;break;}
                }
                if(!failed && !abort) {
                    block_screen(log.run,rep+1,repeats,scene,v,block,total);scene_reset(&world,scene);
                    for(unsigned frame=0;frame<WARMUP+MEASURE;++frame) {
                        if(cancelled()) {abort=true;break;}
                        Measurement m={0};uint32_t start=bench_now();
                        scene_step(&world,scene,frame);m.logic=bench_us(start,bench_now());
                        /* Keep all 180 simulation steps but render only evenly
                           spaced samples in the short A/B comparison. */
                        if(frame>=WARMUP && frame!=WARMUP+count*(MEASURE-1)/(wanted-1)) continue;
                        if(!graphics_frame(&world,scene,v,&m)) {failed=true;break;}
                        m.total+=m.logic;
                        /* Hashing does not contaminate any measured interval. */
                        m.state_hash=scene_hash(&world);m.simulation_frame=frame;
                        if(frame>=WARMUP) samples[count++]=m;
                    }
                }
                const char *status=abort?"aborted":failed?(check.ok?"render_error":"image_mismatch"):"ok";
                if(failed) variant_error=true;
                if(!log_block(&log,rep+1,scene,v,samples,count,check,status) ||
                    !log_status(&log,status,rep+1,scene,(int)v)) error=true;
                /* A bad variant is logged and the other variants still run. */
            }
    if(!log.failed) log_status(&log,abort?"run_aborted":error?"run_error":variant_error?"run_complete_with_errors":"run_complete",0,SCENES,-1);
    log_close(&log);
    result(log.failed?"Schreibfehler - Lauf gestoppt":abort?"Abgebrochen - Teilmessung gesichert":
        error || variant_error?"Fertig mit Fehlern - Logs gespeichert":"Fertig - Logs gespeichert",log.run);
}
int main(void)
{
    if(prof_init()) {result("Kein Timer fuer Messung frei",0);return 1;}
    unsigned repeats=1;int selected=-2;bool running=true;
    while(running) {
        dclear(C_RGB(2,3,5));dtext(8,16,C_WHITE,"MINIMINE BENCH " BENCH_VERSION);
        dtext(8,48,C_WHITE,"EXE: Messung starten");
        dprint(8,85,C_WHITE,"Modus: %s",selected==-2?"Kurzvergleich Alt / Neu":selected<0?"Alle Varianten (lang)":variants[selected].name);
        dprint(8,122,C_WHITE,"Hoch/Runter: %u Durchlaeufe",repeats);
        dprint(8,160,C_WHITE,"%u Varianten / 5 feste Szenen",selected==-2?3u:selected<0?(unsigned)VARIANTS:1u);
        dprint(8,190,C_WHITE,"30 Aufwaerm- + %u Messbilder",selected==-2?COMPARE_SAMPLES:MEASURE);
        dtext(8,230,C_WHITE,"CSV-Logs: /MinimineBench");
        dtext(8,270,C_WHITE,"Links/Rechts: Modus waehlen");
        dtext(8,300,C_WHITE,"Kurzvergleich ist voreingestellt");
        dtext(8,350,C_WHITE,"CLEAR: Beenden");dupdate();
        key_event_t e=getkey();
        switch(e.key) {
        case KEY_EXE:run_bench(repeats,selected);break;
        case KEY_LEFT:selected=selected<=-2?VARIANTS-1:selected-1;break;
        case KEY_RIGHT:selected=selected>=VARIANTS-1?-2:selected+1;break;
        case KEY_UP:case KEY_DOWN:repeats=repeats==1?3:1;break;
        case KEY_CLEAR:running=false;break;
        }
    }
    prof_quit();return 0;
}
