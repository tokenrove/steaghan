/* 
 * mods.c
 * Created: Tue Jan 25 12:43:43 2000 by tek@wiw.org
 * Revised: Mon Mar 20 10:18:12 2000 by tek@wiw.org
 * Copyright 2000 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "steaghanmods.h"

#ifndef STEGMODS_TOPDIR
#define STEGMODS_TOPDIR "."
#endif

#ifndef STEGMODS_SUFFIX
#define STEGMODS_SUFFIX ".so"
#endif

int loadmod(moduleinfo_t *mip, char *modpath);
void describemod(moduleinfo_t *mip);
void *getsym(moduleinfo_t *mip, char *sym);

int loadmod(moduleinfo_t *mip, char *modpath)
{
    void *dlhandle;
    char *path, *buffer;
    char prefixen[5][10] = { "/hash/", "/prpg/", "/file/", "/wrapper/",
                             "/cipher/" };
    int longest_prefix = 9, nprefixen = 5, i;

    buffer = (char *)malloc(strlen("moduleinfo")+1+strlen(modpath)+1+1);
    if(buffer == NULL) return 1;
    memset(buffer, 0, strlen("moduleinfo")+1+strlen(modpath)+1+1);
    buffer[0] = 0;

    strcat(buffer, modpath); /* FIXME */
    strcat(buffer, "_");
    strcat(buffer, "moduleinfo");
    buffer[strlen(modpath)+1+strlen("moduleinfo")] = 0;

    dlhandle = dlopen(modpath, RTLD_NOW);
    if(dlhandle != NULL) {
        if(dlsym(dlhandle, buffer) != NULL) {
            *mip = (*(moduleinfofunc_t)dlsym(dlhandle, buffer))();
            mip->dlhandle = dlhandle;
            return 0;
        }
    }

    path = (char *)malloc(strlen(modpath)+longest_prefix+
                          strlen(STEGMODS_TOPDIR)+strlen(STEGMODS_SUFFIX)+10);
    strcpy(path, ".");
    strcat(path, modpath);
    strcat(path, STEGMODS_SUFFIX);
    
    dlhandle = dlopen(path, RTLD_NOW);
    if(dlhandle != NULL) {
        if(dlsym(dlhandle, buffer) != NULL) {
            *mip = (*(moduleinfofunc_t)dlsym(dlhandle, buffer))();
            mip->dlhandle = dlhandle;
            return 0;
        }
    }

    for(i = 0; i < nprefixen; i++) {
        strcpy(path, ".");
        strcat(path, prefixen[i]);
        strcat(path, modpath);
        strcat(path, STEGMODS_SUFFIX);
        
        dlhandle = dlopen(path, RTLD_NOW);
        if(dlhandle != NULL) {
            if(dlsym(dlhandle, buffer) != NULL) {
                *mip = (*(moduleinfofunc_t)dlsym(dlhandle, buffer))();
                mip->dlhandle = dlhandle;
                return 0;
            }
        }
    }
    
    strcpy(path, STEGMODS_TOPDIR);
    strcat(path, modpath);
    strcat(path, STEGMODS_SUFFIX);
    
    dlhandle = dlopen(path, RTLD_NOW);
    if(dlhandle != NULL) {
        if(dlsym(dlhandle, buffer) != NULL) {
            *mip = (*(moduleinfofunc_t)dlsym(dlhandle, buffer))();
            mip->dlhandle = dlhandle;
            return 0;
        }
    }

    for(i = 0; i < nprefixen; i++) {
        strcpy(path, STEGMODS_TOPDIR);
        strcat(path, prefixen[i]);
        strcat(path, modpath);
        strcat(path, STEGMODS_SUFFIX);

        dlhandle = dlopen(path, RTLD_NOW);
        if(dlhandle != NULL) {
            if(dlsym(dlhandle, buffer) != NULL) {
                *mip = (*(moduleinfofunc_t)dlsym(dlhandle, buffer))();
                mip->dlhandle = dlhandle;
                return 0;
            }
        }
    }
    
    return 1;
}

void describemod(moduleinfo_t *mip)
{
    fprintf(stderr, "steaghan: Module ``%s'' [%s], is ", mip->name,
            mip->description);
    if(mip->moduletype == prpgmod) fprintf(stderr, "a PRPG");
    else if(mip->moduletype == hashmod) fprintf(stderr, "a hash");
    else if(mip->moduletype == wrappermod) fprintf(stderr, "a wrapper");
    else if(mip->moduletype == filemod) fprintf(stderr, "a file");
    else if(mip->moduletype == ciphermod) fprintf(stderr, "a cipher");
    else fprintf(stderr, "an unknown");
    fprintf(stderr, " module.\n");
    return;
}

void *getsym(moduleinfo_t *mip, char *sym)
{
    char *buffer;
    void *f;

    buffer = (char *)malloc(strlen(sym)+1+strlen(mip->name));
    if(buffer == NULL) return NULL;
    buffer[0] = 0;

    strcat(buffer, mip->name);
    strcat(buffer, "_");
    strcat(buffer, sym);

    f = dlsym(mip->dlhandle, buffer);
    free(buffer);
    return f;
}

/* EOF mods.c */
