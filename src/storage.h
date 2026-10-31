#ifndef MINIMINE_STORAGE_H
#define MINIMINE_STORAGE_H
#include "game.h"
#include <stddef.h>
typedef struct {
    void *context;
    int (*open)(void *,const char *,bool);
    int (*read)(void *,int,void *,unsigned);
    int (*write)(void *,int,const void *,unsigned);
    int (*close)(void *,int);
    int (*mkdir)(void *,const char *);
} SaveIO;
typedef struct { const char *root; const SaveIO *io; } SaveStore;
typedef enum { SAVE_OK,SAVE_EMPTY,SAVE_INVALID,SAVE_INCOMPATIBLE,SAVE_IO_ERROR } SaveResult;
typedef struct {
    SaveResult status; uint64_t generation; uint32_t ticks;
    uint16_t health; uint8_t hunger; unsigned copy; bool recovered;
} SaveInfo;
SaveResult save_slot_info(const SaveStore *,unsigned,SaveInfo *,Game *scratch);
SaveResult save_slot_write(const SaveStore *,unsigned,const Game *,Game *scratch);
SaveResult save_slot_load(const SaveStore *,unsigned,Game *scratch,SaveInfo *);
const char *save_result_text(SaveResult result);
#endif
