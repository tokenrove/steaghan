/* 
 * bmp.c
 * Created: Wed Feb 23 18:13:50 2000 by tek@wiw.org
 * Revised: Wed Feb 23 18:13:50 2000 (pending)
 * Copyright 2000 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "steaghanmods.h"

#define BMP_MODULENAME "bmp"
#define BMP_MODULEDESC "Windows bitmap"

typedef struct {
    file_t *file;
    u_int32_t w, h, type, dataoffset, bpp;
} wraphandle_t;

moduleinfo_t moduleinfo(void)
{
    moduleinfo_t mi = { BMP_MODULENAME, BMP_MODULEDESC, wrappermod, 0 };
    return mi;
}

#include "ms-shared.c"

void *wrapinit(file_t *file)
{
    wraphandle_t *p;
    int i, j;

    p = (wraphandle_t *)malloc(sizeof(wraphandle_t));
    if(p == NULL) return NULL;

    p->file = file;

    j = 0;
    j = getleword(p->file);
    if(j == 0x4D42) {
    } else {
        return NULL;
    }

    (*p->file->seek)(p->file->handle, 4, CUR); /* file size */
    (*p->file->seek)(p->file->handle, 2, CUR); /* X hotspot */
    (*p->file->seek)(p->file->handle, 2, CUR); /* Y hotspot */
    p->dataoffset = getledword(p->file)+(*p->file->tell)(p->file->handle);

    i = getledword(p->file);
    if(i == 12) {
        p->w = getledword(p->file);
        p->h = getledword(p->file);
        (*p->file->seek)(p->file->handle, 2, CUR); /* planes */
        p->bpp = getleword(p->file);
    } else if(i == 40) {
        p->w = getledword(p->file);
        p->h = getledword(p->file);
        (*p->file->seek)(p->file->handle, 2, CUR); /* planes */
        p->bpp = getleword(p->file);
    } else if(i == 64) {
        p->w = getledword(p->file);
        p->h = getledword(p->file);
        (*p->file->seek)(p->file->handle, 2, CUR); /* planes */
        p->bpp = getleword(p->file);
    } else {
        return NULL;
    }
    
    return (void *)p;
}

u_int32_t wraplen(void *p_)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    return p->w*p->h*(p->bpp/8);
}

u_int8_t wrapread(void *p_, u_int32_t pos)
{
    wraphandle_t *p = (wraphandle_t *)p_; u_int8_t c;
    (*p->file->read)(p->file->handle, pos+p->dataoffset, 1, &c);
    return c&1;
}

void wrapwrite(void *p_, u_int32_t pos, u_int8_t value)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    u_int8_t x;

    (*p->file->read)(p->file->handle, pos+p->dataoffset, 1, &x);
    x &= 0xFE;
    x |= value;
    (*p->file->write)(p->file->handle, pos+p->dataoffset, 1, &x);

    return;
}

void wrapclose(void *p_)
{
    free(p_);
    return;
}

u_int32_t wrapgetimmobilelen(void *p_)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    return p->w*p->h*(p->bpp/8)+p->dataoffset;    
}

void wrapgetimmobile(void *p_, u_int8_t *immobile)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    int i;

    (*p->file->read)(p->file->handle, 0, p->dataoffset, (void *)immobile);

    (*p->file->read)(p->file->handle, p->dataoffset, p->w*p->h*(p->bpp/8),
                     (void *)immobile+p->dataoffset);
    for(i = p->dataoffset; i < p->w*p->h*(p->bpp/8); i++) {
        immobile[i] &= 0xFE;
    }

    return;
}

/* EOF bmp.c */
