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
#include "ms-shared.h"

#define WAV_MODULENAME "wav"
#define WAV_MODULEDESC "Microsoft WAVE audio"

typedef struct {
    file_t *file;
    u_int32_t nsamples, dataoffset, freq;
    u_int16_t bps, channels;
} wraphandle_t;

moduleinfo_t wav_moduleinfo(void);
void *wav_wrapinit(file_t *file);
u_int32_t wav_wraplen(void *p_);
u_int8_t wav_wrapread(void *p_, u_int32_t pos);
void wav_wrapwrite(void *p_, u_int32_t pos, u_int8_t value);
void wav_wrapclose(void *p_);
void wav_wrapgetimmobile(void *p_, u_int8_t *immobile);
u_int32_t wav_wrapgetimmobilelen(void *p_);

modulefunctable_t *wav_modulefunctable(void)
{
    modulefunctable_t *mft;

    mft = (modulefunctable_t *)malloc(sizeof(modulefunctable_t));
    if(mft == NULL) return NULL;
    mft->nfuncs = 8;
    mft->funcs = (modulefunc_t *)malloc(sizeof(modulefunc_t)*mft->nfuncs);
    if(mft->funcs == NULL) return NULL;

    mft->funcs[0].name = "moduleinfo";
    mft->funcs[0].f = (void *)wav_moduleinfo;

    mft->funcs[1].name = "wrapinit";
    mft->funcs[1].f = (void *)wav_wrapinit;

    mft->funcs[2].name = "wraplen";
    mft->funcs[2].f = (void *)wav_wraplen;

    mft->funcs[3].name = "wrapread";
    mft->funcs[3].f = (void *)wav_wrapread;

    mft->funcs[4].name = "wrapwrite";
    mft->funcs[4].f = (void *)wav_wrapwrite;

    mft->funcs[5].name = "wrapclose";
    mft->funcs[5].f = (void *)wav_wrapclose;

    mft->funcs[6].name = "wrapgetimmobile";
    mft->funcs[6].f = (void *)wav_wrapgetimmobile;

    mft->funcs[7].name = "wrapgetimmobilelen";
    mft->funcs[7].f = (void *)wav_wrapgetimmobilelen;

    return mft;
}

moduleinfo_t wav_moduleinfo(void)
{
    moduleinfo_t mi = { WAV_MODULENAME, WAV_MODULEDESC, wrappermod, 0 };
    return mi;
}

void *wav_wrapinit(file_t *file)
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

u_int32_t wav_wraplen(void *p_)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    return p->nsamples;
}

u_int8_t wav_wrapread(void *p_, u_int32_t pos)
{
    wraphandle_t *p = (wraphandle_t *)p_; u_int8_t c;
    if(p->bps == 8)
        (*p->file->read)(p->file->handle, pos+p->dataoffset, 1, &c);
    else if(p->bps == 16)
        (*p->file->read)(p->file->handle, (2*pos)+p->dataoffset, 1, &c);
    return c&1;
}

void wav_wrapwrite(void *p_, u_int32_t pos, u_int8_t value)
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

void wav_wrapclose(void *p_)
{
    wraphandle_t *p = (wraphandle_t *)p_;

    free(p);
    
    return;
}


u_int32_t wav_wrapgetimmobilelen(void *p_)
{ 
    wraphandle_t *p = (wraphandle_t *)p_;
    return p->nsamples+p->dataoffset;
} 

void wav_wrapgetimmobile(void *p_, u_int8_t *immobile)
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
