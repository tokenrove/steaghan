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

moduleinfo_t moduleinfo(void);
void *wrapinit(char *filename);

typedef struct {
    char *filename;
    FILE *handle;
    u_int32_t nsamples, dataoffset, freq;
    u_int16_t bps, channels;
} wraphandle_t;

moduleinfo_t moduleinfo(void)
{
    moduleinfo_t mi = { WAV_MODULENAME, WAV_MODULEDESC, wrappermod, 0 };
    return mi;
}

short getleword(FILE *fp)
{
    return (fgetc(fp)|(fgetc(fp)<<8));
}

long getledword(FILE *fp)
{
    return (fgetc(fp)|(fgetc(fp)<<8)|(fgetc(fp)<<16)|(fgetc(fp)<<24));
}

void *wrapinit(char *filename)
{
    wraphandle_t *p;
    char buffer[5];

    p = (wraphandle_t *)malloc(sizeof(wraphandle_t));
    if(p == NULL) return NULL;

    p->filename = filename;
    p->handle = fopen(filename, "r+");
    if(p->handle == NULL) return NULL;

    fread(buffer, 4, 1, p->handle); buffer[4] = 0;
    if(strcmp(buffer, "RIFF")) return NULL;
    
    fseek(p->handle, 4, SEEK_CUR); /* file size - 8 */
    
    fread(buffer, 4, 1, p->handle); buffer[4] = 0;
    if(strcmp(buffer, "WAVE")) return NULL;

    fread(buffer, 4, 1, p->handle); buffer[4] = 0;
    if(strcmp(buffer, "fmt ")) return NULL; /* FIXME: no support for
                                               non-canonical wave files */

    if(getledword(p->handle) != 0x00000010) return NULL;
    if(getleword(p->handle) != 0x0001) return NULL; /* FIXME: no
                                                       non-PCM support */

    p->channels = getleword(p->handle);
    p->freq = getledword(p->handle);
    getledword(p->handle); /* bytes/second */
    getleword(p->handle); /* block alignment */
    p->bps = getleword(p->handle);

    fread(buffer, 4, 1, p->handle); buffer[4] = 0;
    if(strcmp(buffer, "data")) return NULL; /* see above FIXME */

    p->nsamples = getledword(p->handle)/(p->bps/8);
    p->dataoffset = ftell(p->handle);
    
    return (void *)p;
}

u_int32_t wraplen(void *p_)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    return p->nsamples;
}

u_int8_t wrapread(void *p_, u_int32_t pos)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    if(p->bps == 8)
        fseek(p->handle, pos+p->dataoffset, SEEK_SET);
    else if(p->bps == 16)
        fseek(p->handle, (2*pos)+p->dataoffset, SEEK_SET);
    return fgetc(p->handle)&1;
}

void wrapwrite(void *p_, u_int32_t pos, u_int8_t value)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    u_int8_t x;

    if(p->bps == 8) {
        fseek(p->handle, pos+p->dataoffset, SEEK_SET);
    } else if(p->bps == 16) {
        fseek(p->handle, (2*pos)+p->dataoffset, SEEK_SET);
    }

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

/* EOF wav.c */
