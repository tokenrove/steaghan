/* 
 * mmap.c
 * Created: Sun Mar  5 14:48:27 2000 by tek@nehwon.local
 * Revised: Sun Mar  5 19:50:14 2000 by tek@nehwon.local
 * Copyright 2000 (Unknown) (tek@nehwon.local)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "steaghanmods.h"

#define MMAP_MODULENAME "mmap"
#define MMAP_MODULEDESC "mmap'd file access"

typedef struct {
    int fd;
    u_int32_t pos, length;
    void *handle;
} filehandle_t;

moduleinfo_t moduleinfo(void);
file_t *fileinit(char *filename);
void fileclose(file_t *file);
void fileread(void *p_, u_int32_t pos, u_int32_t len, void *x);
void filewrite(void *p_, u_int32_t pos, u_int32_t len, void *x);
void fileseek(void *p_, int32_t pos, filewhence_t whence);
u_int32_t filetell(void *p_);

moduleinfo_t moduleinfo(void)
{
    moduleinfo_t mi = { MMAP_MODULENAME, MMAP_MODULEDESC, filemod, 0 };
    return mi;
}

file_t *fileinit(char *filename)
{
    file_t *file;
    filehandle_t *p;

    file = (file_t *)malloc(sizeof(file_t));
    if(file == NULL) return NULL;
    
    p = file->handle = (void *)malloc(sizeof(filehandle_t));
    if(file->handle == NULL) return NULL;

    file->filename = filename;
    file->read = fileread;
    file->write = filewrite;
    file->seek = fileseek;
    file->tell = filetell;
    
    p->fd = open(filename, O_RDWR);
    p->pos = 0;
    p->length = lseek(p->fd, 0, SEEK_END);
    if(p->fd == -1) return NULL;
    p->handle = mmap(0, p->length, PROT_READ|PROT_WRITE, MAP_SHARED, p->fd,
                     0);
    if(p->handle == (void *)-1) return NULL;

    return file;
}

void fileclose(file_t *file)
{
    munmap(((filehandle_t *)file->handle)->handle,
           ((filehandle_t *)file->handle)->length);
    close(((filehandle_t *)file->handle)->fd);
    free(file->handle);
    free(file);
}

void fileread(void *p_, u_int32_t pos, u_int32_t len, void *x)
{
    void *p = ((filehandle_t *)p_)->handle;

    /* FIXME: I bet this and below break on fascistically aligned archs */
    memcpy(x, (void *)((u_int8_t *)p+pos), len);
    ((filehandle_t *)p_)->pos = pos+len;
    return;
}

void filewrite(void *p_, u_int32_t pos, u_int32_t len, void *x)
{
    void *p = ((filehandle_t *)p_)->handle;

    memcpy((void *)((u_int8_t *)p+pos), x, len);
    ((filehandle_t *)p_)->pos = pos+len;
    return;
}

void fileseek(void *p_, int32_t pos, filewhence_t whence)
{
    int32_t *p = (int32_t *)&((filehandle_t *)p_)->pos;

    if(whence == CUR)
        *p += pos;
    else if(whence == SET)
        *p = pos;
    else if(whence == END)
        *p = ((filehandle_t *)p_)->length - pos;
    return;
}

u_int32_t filetell(void *p_)
{
    return ((filehandle_t *)p_)->pos;
}

/* EOF mmap.c */
