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

typedef struct {
    char *filename;
    file_t *file;
    u_int32_t nbytes;
} wraphandle_t;

moduleinfo_t raw_moduleinfo(void);
void *raw_wrapinit(file_t *file);
u_int32_t raw_wraplen(void *p_);
u_int8_t raw_wrapread(void *p_, u_int32_t pos);
void raw_wrapwrite(void *p_, u_int32_t pos, u_int8_t value);
void raw_wrapclose(void *p_);
void raw_wrapgetimmobile(void *p_, u_int8_t *immobile);
u_int32_t raw_wrapgetimmobilelen(void *p_);

modulefunctable_t *raw_modulefunctable(void)
{
    modulefunctable_t *mft;

    mft = (modulefunctable_t *)malloc(sizeof(modulefunctable_t));
    if(mft == NULL) return NULL;
    mft->nfuncs = 8;
    mft->funcs = (modulefunc_t *)malloc(sizeof(modulefunc_t)*mft->nfuncs);
    if(mft->funcs == NULL) return NULL;

    mft->funcs[0].name = "moduleinfo";
    mft->funcs[0].f = (void *)raw_moduleinfo;

    mft->funcs[1].name = "wrapinit";
    mft->funcs[1].f = (void *)raw_wrapinit;

    mft->funcs[2].name = "wraplen";
    mft->funcs[2].f = (void *)raw_wraplen;

    mft->funcs[3].name = "wrapread";
    mft->funcs[3].f = (void *)raw_wrapread;

    mft->funcs[4].name = "wrapwrite";
    mft->funcs[4].f = (void *)raw_wrapwrite;

    mft->funcs[5].name = "wrapclose";
    mft->funcs[5].f = (void *)raw_wrapclose;

    mft->funcs[6].name = "wrapgetimmobile";
    mft->funcs[6].f = (void *)raw_wrapgetimmobile;

    mft->funcs[7].name = "wrapgetimmobilelen";
    mft->funcs[7].f = (void *)raw_wrapgetimmobilelen;

    return mft;
}

moduleinfo_t raw_moduleinfo(void)
{
    moduleinfo_t mi = { RAW_MODULENAME, RAW_MODULEDESC, wrappermod, 0 };
    return mi;
}

void *raw_wrapinit(file_t *file)
{
    wraphandle_t *p;

    p = (wraphandle_t *)malloc(sizeof(wraphandle_t));
    if(p == NULL) return NULL;

    p->file = file;

    (*p->file->seek)(p->file->handle, 0, END);
    p->nbytes = (*p->file->tell)(p->file->handle);

    return (void *)p;
}

u_int32_t raw_wraplen(void *p_)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    return p->nbytes;
}

u_int8_t raw_wrapread(void *p_, u_int32_t pos)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    u_int8_t c;
    
    (*p->file->read)(p->file->handle, pos, 1, &c);

    return c&1;
}

void raw_wrapwrite(void *p_, u_int32_t pos, u_int8_t value)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    u_int8_t x;
    
    (*p->file->read)(p->file->handle, pos, 1, &x);
    x &= 0xFE;
    x |= value;
    (*p->file->write)(p->file->handle, pos, 1, &x);

    return;
}

void raw_wrapclose(void *p_)
{
    wraphandle_t *p = (wraphandle_t *)p_;

    free(p);
    return;
}

u_int32_t raw_wrapgetimmobilelen(void *p_)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    return p->nbytes;
}

void raw_wrapgetimmobile(void *p_, u_int8_t *immobile)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    int i;

    (*p->file->read)(p->file->handle, 0, p->nbytes, (void *)immobile);
    for(i = 0; i < p->nbytes; i++) {
        immobile[i] &= 0xFE;
    }
    return;
}

/* EOF raw.c */
