/* 
 * seek.c
 * Created: Sun Mar  5 16:15:41 2000 by tek@wiw.org
 * Revised: Sun Mar  5 16:15:41 2000 (pending)
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

#define SEEK_MODULENAME "seek"
#define SEEK_MODULEDESC "standard file access with excessive seeks"

typedef struct {
    FILE *handle;
} filehandle_t;

moduleinfo_t seek_moduleinfo(void);
file_t *seek_fileinit(char *filename);
void seek_fileclose(file_t *file);
void seek_fileread(void *p_, u_int32_t pos, u_int32_t len, void *x);
void seek_filewrite(void *p_, u_int32_t pos, u_int32_t len, void *x);
void seek_fileseek(void *p_, int32_t pos, filewhence_t whence);
u_int32_t seek_filetell(void *p_);

modulefunctable_t *seek_modulefunctable(void)
{
    modulefunctable_t *mft;

    mft = (modulefunctable_t *)malloc(sizeof(modulefunctable_t));
    if(mft == NULL) return NULL;
    mft->nfuncs = 3;
    mft->funcs = (modulefunc_t *)malloc(sizeof(modulefunc_t)*mft->nfuncs);
    if(mft->funcs == NULL) return NULL;

    mft->funcs[0].name = "moduleinfo";
    mft->funcs[0].f = (void *)seek_moduleinfo;

    mft->funcs[1].name = "fileinit";
    mft->funcs[1].f = (void *)seek_fileinit;

    mft->funcs[2].name = "fileclose";
    mft->funcs[2].f = (void *)seek_fileclose;

    return mft;
}

moduleinfo_t seek_moduleinfo(void)
{
    moduleinfo_t mi = { SEEK_MODULENAME, SEEK_MODULEDESC, filemod, 0 };
    return mi;
}

file_t *seek_fileinit(char *filename)
{
    file_t *file;

    file = (file_t *)malloc(sizeof(file_t));
    if(file == NULL) return NULL;
    
    file->handle = (void *)malloc(sizeof(filehandle_t));
    if(file->handle == NULL) return NULL;

    file->filename = filename;
    file->read = seek_fileread;
    file->write = seek_filewrite;
    file->seek = seek_fileseek;
    file->tell = seek_filetell;
    
    ((filehandle_t *)file->handle)->handle = fopen(filename, "r+");
    if(((filehandle_t *)file->handle)->handle == NULL) return NULL;

    return file;
}

void seek_fileclose(file_t *file)
{
    fclose(((filehandle_t *)file->handle)->handle);
    free(file->handle);
    free(file);
}

void seek_fileread(void *p_, u_int32_t pos, u_int32_t len, void *x)
{
    FILE *p = ((filehandle_t *)p_)->handle;

    fseek(p, pos, SEEK_SET);
    fread(x, len, 1, p);
    return;
}

void seek_filewrite(void *p_, u_int32_t pos, u_int32_t len, void *x)
{
    FILE *p = ((filehandle_t *)p_)->handle;

    fseek(p, pos, SEEK_SET);
    fwrite(x, len, 1, p);
    return;
}

void seek_fileseek(void *p_, int32_t pos, filewhence_t whence)
{
    FILE *p = ((filehandle_t *)p_)->handle;

    if(whence == CUR)
        fseek(p, pos, SEEK_CUR);
    else if(whence == SET)
        fseek(p, pos, SEEK_SET);
    else if(whence == END)
        fseek(p, pos, SEEK_END);
    return;
}

u_int32_t seek_filetell(void *p_)
{
    FILE *p = ((filehandle_t *)p_)->handle;

    return ftell(p);

}

/* EOF seek.c */
