/* 
 * wav.c
 * Created: Sat Feb 26 12:16:49 2000 by tek@wiw.org
 * Revised: Sat Feb 26 12:16:49 2000 (pending)
 * Copyright 2000 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "steaghanmods.h"

#define WAV_MODULENAME "wav"
#define WAV_MODULEDESC "Microsoft WAVE audio"

typedef struct {
    file_t *file;
    u_int32_t nsamples, dataoffset, freq;
    u_int16_t bps, channels;
} wraphandle_t;

moduleinfo_t moduleinfo(void)
{
    moduleinfo_t mi = { WAV_MODULENAME, WAV_MODULEDESC, wrappermod, 0 };
    return mi;
}

#include "ms-shared.c"

void *wrapinit(file_t *file)
{
    wraphandle_t *p;
    char buffer[5];

    p = (wraphandle_t *)malloc(sizeof(wraphandle_t));
    if(p == NULL) return NULL;

    p->file = file;

    (*p->file->read)(p->file->handle, 0, 4, buffer); buffer[4] = 0;
    if(strcmp(buffer, "RIFF")) return NULL;
    
    (*p->file->read)(p->file->handle, 8, 4, buffer); buffer[4] = 0;
    if(strcmp(buffer, "WAVE")) return NULL;

    (*p->file->read)(p->file->handle, 12, 4, buffer); buffer[4] = 0;
    if(strcmp(buffer, "fmt ")) return NULL; /* FIXME: no support for
                                               non-canonical wave files */

    if(getledword(p->file) != 0x00000010) return NULL;
    if(getleword(p->file) != 0x0001) return NULL; /* FIXME: no
                                                       non-PCM support */

    p->channels = getleword(p->file);
    p->freq = getledword(p->file);
    getledword(p->file); /* bytes/second */
    getleword(p->file); /* block alignment */
    p->bps = getleword(p->file);

    (*p->file->read)(p->file->handle, (*p->file->tell)(p->file->handle),
                     4, buffer); buffer[4] = 0;
    if(strcmp(buffer, "data")) return NULL; /* see above FIXME */

    p->nsamples = (getledword(p->file)/(p->bps/8))-1;
    p->dataoffset = (*p->file->tell)(p->file->handle);

    return (void *)p;
}

u_int32_t wraplen(void *p_)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    return p->nsamples;
}

u_int8_t wrapread(void *p_, u_int32_t pos)
{
    wraphandle_t *p = (wraphandle_t *)p_; u_int8_t c;
    if(p->bps == 8)
        (*p->file->read)(p->file->handle, pos+p->dataoffset, 1, &c);
    else if(p->bps == 16)
        (*p->file->read)(p->file->handle, (2*pos)+p->dataoffset, 1, &c);
    return c&1;
}

void wrapwrite(void *p_, u_int32_t pos, u_int8_t value)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    u_int8_t x;

    if(p->bps == 8) {
        (*p->file->read)(p->file->handle, pos+p->dataoffset, 1, &x);
    } else if(p->bps == 16) {
        (*p->file->read)(p->file->handle, (2*pos)+p->dataoffset, 1, &x);
    }

    x &= 0xFE;
    x |= value;

    if(p->bps == 8) {
        (*p->file->write)(p->file->handle, pos+p->dataoffset, 1, &x);
    } else if(p->bps == 16) {
        (*p->file->write)(p->file->handle, (2*pos)+p->dataoffset, 1, &x);
    }

    return;
}

void wrapclose(void *p_)
{
    wraphandle_t *p = (wraphandle_t *)p_;

    free(p);
    
    return;
}


u_int32_t wrapgetimmobilelen(void *p_)
{ 
    wraphandle_t *p = (wraphandle_t *)p_;
    return p->nsamples+p->dataoffset;
} 

void wrapgetimmobile(void *p_, u_int8_t *immobile)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    int i;

    (*p->file->read)(p->file->handle, 0, p->dataoffset, (void *)immobile);

    (*p->file->read)(p->file->handle, p->dataoffset, p->nsamples*(p->bps/8),
                     (void *)immobile+p->dataoffset);
    for(i = p->dataoffset; i < p->nsamples; i++) {
        immobile[i*(p->bps/8)] &= 0xFE;
    }

    return;
}

/* EOF wav.c */
