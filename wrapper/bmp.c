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
#include "ms-shared.h"

#define BMP_MODULENAME "bmp"
#define BMP_MODULEDESC "Windows bitmap"

typedef struct {
    file_t *file;
    u_int32_t w, h, type, dataoffset, bpp;
} wraphandle_t;

moduleinfo_t bmp_moduleinfo(void);
void *bmp_wrapinit(file_t *file);
u_int32_t bmp_wraplen(void *p_);
u_int8_t bmp_wrapread(void *p_, u_int32_t pos);
void bmp_wrapwrite(void *p_, u_int32_t pos, u_int8_t value);
void bmp_wrapclose(void *p_);
void bmp_wrapgetimmobile(void *p_, u_int8_t *immobile);
u_int32_t bmp_wrapgetimmobilelen(void *p_);

modulefunctable_t *bmp_modulefunctable(void)
{
    modulefunctable_t *mft;

    mft = (modulefunctable_t *)malloc(sizeof(modulefunctable_t));
    if(mft == NULL) return NULL;
    mft->nfuncs = 8;
    mft->funcs = (modulefunc_t *)malloc(sizeof(modulefunc_t)*mft->nfuncs);
    if(mft->funcs == NULL) return NULL;

    mft->funcs[0].name = "moduleinfo";
    mft->funcs[0].f = (void *)bmp_moduleinfo;

    mft->funcs[1].name = "wrapinit";
    mft->funcs[1].f = (void *)bmp_wrapinit;

    mft->funcs[2].name = "wraplen";
    mft->funcs[2].f = (void *)bmp_wraplen;

    mft->funcs[3].name = "wrapread";
    mft->funcs[3].f = (void *)bmp_wrapread;

    mft->funcs[4].name = "wrapwrite";
    mft->funcs[4].f = (void *)bmp_wrapwrite;

    mft->funcs[5].name = "wrapclose";
    mft->funcs[5].f = (void *)bmp_wrapclose;

    mft->funcs[6].name = "wrapgetimmobile";
    mft->funcs[6].f = (void *)bmp_wrapgetimmobile;

    mft->funcs[7].name = "wrapgetimmobilelen";
    mft->funcs[7].f = (void *)bmp_wrapgetimmobilelen;

    return mft;
}

moduleinfo_t bmp_moduleinfo(void)
{
    moduleinfo_t mi = { BMP_MODULENAME, BMP_MODULEDESC, wrappermod, 0 };
    return mi;
}

void *bmp_wrapinit(file_t *file)
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

u_int32_t bmp_wraplen(void *p_)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    return p->w*p->h*(p->bpp/8);
}

u_int8_t bmp_wrapread(void *p_, u_int32_t pos)
{
    wraphandle_t *p = (wraphandle_t *)p_; u_int8_t c;
    (*p->file->read)(p->file->handle, pos+p->dataoffset, 1, &c);
    return c&1;
}

void bmp_wrapwrite(void *p_, u_int32_t pos, u_int8_t value)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    u_int8_t x;

    (*p->file->read)(p->file->handle, pos+p->dataoffset, 1, &x);
    x &= 0xFE;
    x |= value;
    (*p->file->write)(p->file->handle, pos+p->dataoffset, 1, &x);

    return;
}

void bmp_wrapclose(void *p_)
{
    free(p_);
    return;
}

u_int32_t bmp_wrapgetimmobilelen(void *p_)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    return p->w*p->h*(p->bpp/8)+p->dataoffset;    
}

void bmp_wrapgetimmobile(void *p_, u_int8_t *immobile)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    int i;

    (*p->file->read)(p->file->handle, 0, p->dataoffset, (void *)immobile);

    (*p->file->read)(p->file->handle, p->dataoffset, p->w*p->h*(p->bpp/8),
                     (void *)(immobile+p->dataoffset));
    for(i = p->dataoffset; i < p->w*p->h*(p->bpp/8); i++) {
        immobile[i] &= 0xFE;
    }

    return;
}

/* EOF bmp.c */
