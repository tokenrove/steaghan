/* 
 * ppm.c
 * Created: Wed Dec 15 09:45:27 1999 by tek@wiw.org
 * Revised: Wed Dec 15 09:45:27 1999 (pending)
 * Copyright 1999 Julian E. C. Squires (tek@wiw.org)
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
#include "pnm-shared.h"

#define PPM_MODULENAME "ppm"
#define PPM_MODULEDESC "Portable Pixmap Format"

typedef struct {
    file_t *file;
    u_int32_t w, h, dataoffset, maxintense;
    u_int8_t *data, type;
} wraphandle_t;

moduleinfo_t ppm_moduleinfo(void);
void *ppm_wrapinit(file_t *file);
u_int32_t ppm_wraplen(void *p_);
u_int8_t ppm_wrapread(void *p_, u_int32_t pos);
void ppm_wrapwrite(void *p_, u_int32_t pos, u_int8_t value);
void ppm_wrapclose(void *p_);
void ppm_wrapgetimmobile(void *p_, u_int8_t *immobile);
u_int32_t ppm_wrapgetimmobilelen(void *p_);

modulefunctable_t *ppm_modulefunctable(void)
{
    modulefunctable_t *mft;

    mft = (modulefunctable_t *)malloc(sizeof(modulefunctable_t));
    if(mft == NULL) return NULL;
    mft->nfuncs = 8;
    mft->funcs = (modulefunc_t *)malloc(sizeof(modulefunc_t)*mft->nfuncs);
    if(mft->funcs == NULL) return NULL;

    mft->funcs[0].name = "moduleinfo";
    mft->funcs[0].f = (void *)ppm_moduleinfo;

    mft->funcs[1].name = "wrapinit";
    mft->funcs[1].f = (void *)ppm_wrapinit;

    mft->funcs[2].name = "wraplen";
    mft->funcs[2].f = (void *)ppm_wraplen;

    mft->funcs[3].name = "wrapread";
    mft->funcs[3].f = (void *)ppm_wrapread;

    mft->funcs[4].name = "wrapwrite";
    mft->funcs[4].f = (void *)ppm_wrapwrite;

    mft->funcs[5].name = "wrapclose";
    mft->funcs[5].f = (void *)ppm_wrapclose;

    mft->funcs[6].name = "wrapgetimmobile";
    mft->funcs[6].f = (void *)ppm_wrapgetimmobile;

    mft->funcs[7].name = "wrapgetimmobilelen";
    mft->funcs[7].f = (void *)ppm_wrapgetimmobilelen;

    return mft;
}

moduleinfo_t ppm_moduleinfo(void)
{
    moduleinfo_t mi = { PPM_MODULENAME, PPM_MODULEDESC, wrappermod, 0 };
    return mi;
}

/* lines are max 70 characters, so we should be safe with this */
#define BUFLEN 81

void *ppm_wrapinit(file_t *file)
{
    wraphandle_t *p;
    int i;
    char buffer[BUFLEN];

    p = (wraphandle_t *)malloc(sizeof(wraphandle_t));
    if(p == NULL) return NULL;

    p->file = file;

    /* read header */
    gotonexttoken(p->file);

    readtoken(buffer, BUFLEN-1, p->file);
    if(strcmp(buffer, "P6") == 0)
        p->type = 6;
    else if(strcmp(buffer, "P3") == 0)
        p->type = 3;
    else
        return NULL;
    gotonexttoken(p->file);
    
    /* read width and height */
    readtoken(buffer, BUFLEN-1, p->file); p->w = atoi(buffer);
    gotonexttoken(p->file);
    readtoken(buffer, BUFLEN-1, p->file); p->h = atoi(buffer);

    if(p->w <= 0 || p->h <= 0) return NULL;

    gotonexttoken(p->file);

    readtoken(buffer, BUFLEN-1, p->file);
    p->maxintense = atoi(buffer);
    /* FIXME: we shouldn't ignore maximum intensity... */
    if(p->type == 6)
        p->dataoffset = (*p->file->tell)(p->file->handle);
     else {
        p->data = (u_int8_t *)malloc(p->w*p->h*3);
        if(p->data == NULL) return NULL;

        for(i = 0; i < (p->w * p->h)*3; i++) {
            readtoken(buffer, BUFLEN-1, p->file); gotonexttoken(p->file);
            p->data[i] = atoi(buffer);
        }
    }

    return (void *)p;
}

u_int32_t ppm_wraplen(void *p_)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    return p->w*p->h*3;
}

u_int8_t ppm_wrapread(void *p_, u_int32_t pos)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    u_int8_t c;
    
    if(p->type == 6) {
        (*p->file->read)(p->file->handle, pos+p->dataoffset, 1, &c);
        return c&1;
    } else if(p->type == 3) {
        return p->data[pos]&1;
    } else return 0;
}

void ppm_wrapwrite(void *p_, u_int32_t pos, u_int8_t value)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    u_int8_t x;

    if(p->type == 6) {
        (*p->file->read)(p->file->handle, pos+p->dataoffset, 1, &x);
        x &= 0xFE;
        x |= value;
        (*p->file->write)(p->file->handle, pos+p->dataoffset, 1, &x);

    } else if(p->type == 3) {
        p->data[pos] = (p->data[pos]&0xFE)|(value&1);

    } else {
    }
    
    return;
}

void ppm_wrapclose(void *p_)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    int i;
    FILE *fp;
    
    if(p->type == 3) {
        /* FIXME: it'd be nice not to do this */
        fp = fopen(p->file->filename, "w");

        fprintf(fp, "P3\n");
        fprintf(fp, "%d %d\n%d\n", p->w, p->h, p->maxintense);
        for(i = 0; i < p->w*p->h*3; i++) {
            if((i%16) == 0) fprintf(fp, "\n");
            fprintf(fp, "%d ", p->data[i]);
        }

        free(p->data);
        fclose(fp);
    }
    
    free(p);
    
    return;
}

u_int32_t ppm_wrapgetimmobilelen(void *p_)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    if(p->type == 3)
        return p->dataoffset; /* FIXME? */
    else
        return p->w*p->h+p->dataoffset;
}

void ppm_wrapgetimmobile(void *p_, u_int8_t *immobile)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    int i;

    (*p->file->read)(p->file->handle, 0, p->dataoffset, (void *)immobile);
    if(p->type == 6) {
        (*p->file->read)(p->file->handle, p->dataoffset, p->w*p->h,
                         (void *)(immobile+p->dataoffset));
        for(i = p->dataoffset; i < p->w*p->h; i++) {
            immobile[i] &= 0xFE;
        }
    }

    return;
}

/* EOF ppm.c */
