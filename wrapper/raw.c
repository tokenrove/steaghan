/* 
 * raw.c
 * Created: Wed Dec 15 09:45:27 1999 by tek@wiw.org
 * Revised: Wed Dec 15 09:45:27 1999 (pending)
 * Copyright 1999 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 *
 * This basically just considers the low-order bit of each byte to
 * be fair game.
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "steaghanmods.h"

#define RAW_MODULENAME "raw"
#define RAW_MODULEDESC "raw 8-bit data [assumed to be noisy]"

#define OURCRCMAGIC 0x04C11DB7

moduleinfo_t moduleinfo(void);
void *wrapinit(char *filename);

typedef struct {
    char *filename;
    FILE *handle;
    u_int32_t nbytes;
    u_int32_t crctable[256];
} wraphandle_t;

moduleinfo_t moduleinfo(void)
{
    moduleinfo_t mi = { RAW_MODULENAME, RAW_MODULEDESC, wrappermod, 0 };
    return mi;
}

void *wrapinit(char *filename)
{
    wraphandle_t *p;
    int i, j;
    u_int32_t c;

    p = (wraphandle_t *)malloc(sizeof(wraphandle_t));
    if(p == NULL) return NULL;

    p->filename = filename;
    p->handle = fopen(filename, "r+");
    if(p->handle == NULL) return NULL;
    fseek(p->handle, 0, SEEK_END);
    p->nbytes = ftell(p->handle);
    fseek(p->handle, 0, SEEK_SET);

    for(i = 0; i < 256; i++) {
        for(c = (i<<24), j = 8; j > 0; j--) {
            c = (c & 0x80000000) ? ((c << 1) ^ OURCRCMAGIC) : (c << 1);
        }
        p->crctable[i] = c;
    }
    
    return (void *)p;
}

u_int32_t wraplen(void *p_)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    return p->nbytes;
}

u_int8_t wrapread(void *p_, u_int32_t pos)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    fseek(p->handle, pos, SEEK_SET);
    return fgetc(p->handle)&1;
}

void wrapwrite(void *p_, u_int32_t pos, u_int8_t value)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    u_int8_t x;

    fseek(p->handle, pos, SEEK_SET);
    x = fgetc(p->handle)&0xFE;
    x |= value;

    fseek(p->handle, -1, SEEK_CUR);
    fputc(x, p->handle);
    return;
}

void wrapclose(void *p_)
{
    wraphandle_t *p = (wraphandle_t *)p_;

    fclose(p->handle);
    free(p);
    
    return;
}

/* EOF raw.c */
