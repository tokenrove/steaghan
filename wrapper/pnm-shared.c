/* 
 * pnm-shared.c
 * Created: Sun Mar  5 17:37:09 2000 by tek@wiw.org
 * Revised: Sun Mar  5 17:37:09 2000 (pending)
 * Copyright 2000 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include "steaghanmods.h"

int iswhitespace(char c)
{
    if(c == ' ' || c == '\t' || c == '\n') return 1;
    return 0;
}

void readtoeol(file_t *file)
{
    char c;
    do {
        (*file->read)(file->handle, (*file->tell)(file->handle), 1, &c);
    } while(c != '\n');
}

void readtononws(file_t *file)
{
    char c;
    do {
        (*file->read)(file->handle, (*file->tell)(file->handle), 1, &c);
    } while(!iswhitespace(c));
    (*file->seek)(file->handle, -1, CUR);
}

void gotonexttoken(file_t *file)
{
    char c;
    while(1) {
        (*file->read)(file->handle, (*file->tell)(file->handle), 1, &c);
        if(c == '#') { readtoeol(file); continue; }
        if(iswhitespace(c)) { readtononws(file); }
        break;
    }
    (*file->seek)(file->handle, -1, CUR);
}

void readtoken(char *buffer, int buflen, file_t *file)
{
    int i;
    for(i = 0; i < buflen; i++) {
        (*file->read)(file->handle, (*file->tell)(file->handle), 1,
                      &buffer[i]);
        if(iswhitespace(buffer[i])) { buffer[i] = 0; break; }
    }
}

/* EOF pnm-shared.c */
