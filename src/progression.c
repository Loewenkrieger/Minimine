#include "game.h"

static const DifficultyRules rules[DIFFICULTY_COUNT]={
    {75,5400,30,480,4},{100,3600,20,300,8},{125,2700,15,180,8}
};
const DifficultyRules *game_difficulty_rules(Difficulty difficulty)
{return &rules[(unsigned)difficulty<DIFFICULTY_COUNT?difficulty:DIFFICULTY_MEDIUM];}
const char *game_difficulty_name(Difficulty difficulty)
{
    static const char *const names[]={"Leicht","Mittel","Schwer"};
    return names[(unsigned)difficulty<DIFFICULTY_COUNT?difficulty:DIFFICULTY_MEDIUM];
}
void game_set_difficulty(Game *g,Difficulty difficulty)
{
    if((unsigned)difficulty>=DIFFICULTY_COUNT || difficulty==g->difficulty) return;
    const DifficultyRules *old=game_difficulty_rules(g->difficulty),*next=game_difficulty_rules(difficulty);
    g->hunger_ticks=g->hunger_ticks*next->hunger_ticks/old->hunger_ticks;
    g->exertion=g->exertion*next->exertion/old->exertion;
    g->spawn_ticks=g->spawn_ticks*next->spawn_ticks/old->spawn_ticks;
    g->difficulty=difficulty;++g->revision;
}
static uint32_t random_next(uint32_t *state)
{*state^=*state<<13;*state^=*state>>17;*state^=*state<<5;return *state;}
static bool suitable(const Game *g,int x,int y)
{
    return x>=0 && x<WORLD_W && y>=72 && y<WORLD_H-2 &&
        game_tile(g,x,y)==TILE_BASALT && !game_protected_tile(g,x,y);
}
unsigned game_populate_diamonds(Game *g)
{
    if(g->diamonds_generated) return 0;
    /* A separate stream leaves the simulation RNG and historical routes intact. */
    uint32_t rng=g->seed^UINT32_C(0xd1a06096);if(!rng) rng=1;
    const unsigned area=(WORLD_H-2-72)*WORLD_W;
    unsigned start=random_next(&rng)%area,placed=0;
    /* 313 is coprime to 5632: visit every candidate once, without an array. */
    for(unsigned i=0;i<area && placed<96;++i) {
        unsigned at=(start+i*313u)%area;
        int x=(int)(at%WORLD_W),y=72+(int)(at/WORLD_W);
        unsigned length=1+random_next(&rng)%3;
        for(unsigned n=0;n<length && placed<96 && suitable(g,x,y);++n) {
            g->tiles[y][x]=TILE_DIAMOND_ORE;++placed;
            switch(random_next(&rng)%4) {
            case 0:++x;break;case 1:--x;break;case 2:++y;break;default:--y;break;
            }
        }
    }
    g->diamonds_generated=true;++g->revision;return placed;
}
