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

#define PPM_MODULENAME "ppm"
#define PPM_MODULEDESC "Portable Pixmap Format"

moduleinfo_t moduleinfo(void);
void *wrapinit(char *filename);

typedef struct {
    char *filename;
    FILE *handle;
    u_int32_t w, h, dataoffset, maxintense;
    u_int8_t *data, type;
} wraphandle_t;

moduleinfo_t moduleinfo(void)
{
    moduleinfo_t mi = { PPM_MODULENAME, PPM_MODULEDESC, wrappermod, 0 };
    return mi;
}

int iswhitespace(char c)
{
    if(c == ' ' || c == '\t' || c == '\n') return 1;
    return 0;
}

void readtoeol(FILE *fp)
{
    char c;
    do {
        c = fgetc(fp);
    } while(c != '\n');
}

void readtononws(FILE *fp)
{
    char c;
    do {
        c = fgetc(fp);
    } while(!iswhitespace(c));
    ungetc(c, fp);
}

void gotonexttoken(FILE *fp)
{
    char c;
    while(1) {
        c = fgetc(fp);
        if(c == '#') { readtoeol(fp); continue; }
        if(iswhitespace(c)) { readtononws(fp); }
        break;
    }
    ungetc(c, fp);
}

void readtoken(char *buffer, int buflen, FILE *fp)
{
    int i;
    for(i = 0; i < buflen; i++) {
        buffer[i] = fgetc(fp);
        if(iswhitespace(buffer[i])) { buffer[i] = 0; break; }
    }
}

/* lines are max 70 characters, so we should be safe with this */
#define BUFLEN 81

void *wrapinit(char *filename)
{
    wraphandle_t *p;
    int i;
    char buffer[BUFLEN];

    
    p = (wraphandle_t *)malloc(sizeof(wraphandle_t));
    if(p == NULL) return NULL;

    p->filename = filename;
    p->handle = fopen(filename, "r+");
    if(p->handle == NULL) return NULL;

    /* read header */
    gotonexttoken(p->handle);

    readtoken(buffer, BUFLEN-1, p->handle);
    if(strcmp(buffer, "P6") == 0)
        p->type = 6;
    else if(strcmp(buffer, "P3") == 0)
        p->type = 3;
    else
        return NULL;
    gotonexttoken(p->handle);
    
    /* read width and height */
    readtoken(buffer, BUFLEN-1, p->handle); p->w = atoi(buffer);
    gotonexttoken(p->handle);
    readtoken(buffer, BUFLEN-1, p->handle); p->h = atoi(buffer);

    if(p->w <= 0 || p->h <= 0) return NULL;

    gotonexttoken(p->handle);

    readtoken(buffer, BUFLEN-1, p->handle);
    p->maxintense = atoi(buffer);
    /* FIXME: we shouldn't ignore maximum intensity... */
    if(p->type == 6)
        p->dataoffset = ftell(p->handle);
    else {
        p->data = (char *)malloc(p->w*p->h*3);
        if(p->data == NULL) return NULL;

        for(i = 0; i < (p->w * p->h)*3; i++) {
            readtoken(buffer, BUFLEN-1, p->handle); gotonexttoken(p->handle);
            p->data[i] = atoi(buffer);
        }

        fclose(p->handle);
    }

    if(ferror(p->handle)) return NULL;

    return (void *)p;
}

u_int32_t wraplen(void *p_)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    return p->w*p->h*3;
}

u_int8_t wrapread(void *p_, u_int32_t pos)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    if(p->type == 6) {
        fseek(p->handle, pos+p->dataoffset, SEEK_SET);
        return fgetc(p->handle)&1;
    } else if(p->type == 3) {
        return p->data[pos]&1;
    } else return 0;
}

void wrapwrite(void *p_, u_int32_t pos, u_int8_t value)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    u_int8_t x;

    if(p->type == 6) {
        fseek(p->handle, pos+p->dataoffset, SEEK_SET);
        x = fgetc(p->handle)&0xFE;
        x |= value&1;

        fseek(p->handle, -1, SEEK_CUR);
        fputc(x, p->handle);

    } else if(p->type == 3) {
        p->data[pos] = (p->data[pos]&0xFE)|(value&1);

    } else {
    }
    
    return;
}

void wrapclose(void *p_)
{
    wraphandle_t *p = (wraphandle_t *)p_;
    int i;
    
    if(p->type == 3) {
        p->handle = fopen(p->filename, "w");

        fprintf(p->handle, "P3\n");
        fprintf(p->handle, "%d %d\n%d\n", p->w, p->h, p->maxintense);
        for(i = 0; i < p->w*p->h; i++) {
            if((i%16) == 0) printf("\n");
            fprintf(p->handle, "%d ", p->data[i]);
        }

        free(p->data);
    }
    
    fclose(p->handle);
    free(p);
    
    return;
}

/* EOF ppm.c */
