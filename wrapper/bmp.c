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

moduleinfo_t moduleinfo(void);
void *wrapinit(char *filename);

typedef struct {
    char *filename;
    FILE *handle;
    u_int32_t w, h, type, dataoffset, bpp;
} wraphandle_t;

moduleinfo_t moduleinfo(void)
{
    moduleinfo_t mi = { BMP_MODULENAME, BMP_MODULEDESC, wrappermod, 0 };
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
    int i, j;

    p = (wraphandle_t *)malloc(sizeof(wraphandle_t));
    if(p == NULL) return NULL;

    p->filename = filename;
    p->handle = fopen(filename, "r+");
    if(p->handle == NULL) return NULL;

    j = 0;
    j = getleword(p->handle);
    if(j == 0x4D42) {
    } else {
        return NULL;
    }

    fseek(p->handle, 4, SEEK_CUR); /* file size */
    fseek(p->handle, 2, SEEK_CUR); /* X hotspot */
    fseek(p->handle, 2, SEEK_CUR); /* Y hotspot */
    p->dataoffset = getledword(p->handle)+ftell(p->handle);

    i = getledword(p->handle);
    if(i == 12) {
        p->w = getledword(p->handle);
        p->h = getledword(p->handle);
        fseek(p->handle, 2, SEEK_CUR); /* planes */
        p->bpp = getleword(p->handle);
    } else if(i == 40) {
        p->w = getledword(p->handle);
        p->h = getledword(p->handle);
        fseek(p->handle, 2, SEEK_CUR); /* planes */
        p->bpp = getleword(p->handle);
    } else if(i == 64) {
        p->w = getledword(p->handle);
        p->h = getledword(p->handle);
        fseek(p->handle, 2, SEEK_CUR); /* planes */
        p->bpp = getleword(p->handle);
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
    wraphandle_t *p = (wraphandle_t *)p_;
    fseek(p->handle, pos+p->dataoffset, SEEK_SET);
    return fgetc(p->handle)&1;
}

void wrapwrite(void *p_, u_int32_t pos, u_int8_t value)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    u_int8_t x;

    fseek(p->handle, pos+p->dataoffset, SEEK_SET);
    x = fgetc(p->handle)&0xFE;
    x |= value&1;

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

/* EOF bmp.c */
