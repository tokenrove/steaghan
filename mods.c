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

int loadmod(moduleinfo_t *mip, char *modpath)
{
    void *dlhandle;
    char *path;
    char prefixen[5][10] = { "/hash/", "/prpg/", "/file/", "/wrapper/",
                             "/cipher/" };
    int longest_prefix = 9, nprefixen = 5, i;

    dlhandle = dlopen(modpath, RTLD_NOW);
    if(dlhandle != NULL) {
        *mip = (*(moduleinfofunc_t)dlsym(dlhandle, "moduleinfo"))();
        mip->dlhandle = dlhandle;
        return 0;
    }

    path = (char *)malloc(strlen(modpath)+longest_prefix+
                          strlen(STEGMODS_TOPDIR)+strlen(STEGMODS_SUFFIX)+10);
    strcpy(path, ".");
    strcat(path, modpath);
    strcat(path, STEGMODS_SUFFIX);
    
    dlhandle = dlopen(path, RTLD_NOW);
    if(dlhandle != NULL) {
        *mip = (*(moduleinfofunc_t)dlsym(dlhandle, "moduleinfo"))();
        mip->dlhandle = dlhandle;
        return 0;
    }

    for(i = 0; i < nprefixen; i++) {
        strcpy(path, ".");
        strcat(path, prefixen[i]);
        strcat(path, modpath);
        strcat(path, STEGMODS_SUFFIX);
        
        dlhandle = dlopen(path, RTLD_NOW);
        if(dlhandle != NULL) {
            *mip = (*(moduleinfofunc_t)dlsym(dlhandle, "moduleinfo"))();
            mip->dlhandle = dlhandle;
            return 0;
        }
    }
    
    strcpy(path, STEGMODS_TOPDIR);
    strcat(path, modpath);
    strcat(path, STEGMODS_SUFFIX);
    
    dlhandle = dlopen(path, RTLD_NOW);
    if(dlhandle != NULL) {
        *mip = (*(moduleinfofunc_t)dlsym(dlhandle, "moduleinfo"))();
        mip->dlhandle = dlhandle;
        return 0;
    }

    for(i = 0; i < nprefixen; i++) {
        strcpy(path, STEGMODS_TOPDIR);
        strcat(path, prefixen[i]);
        strcat(path, modpath);
        strcat(path, STEGMODS_SUFFIX);

        dlhandle = dlopen(path, RTLD_NOW);
        if(dlhandle != NULL) {
            *mip = (*(moduleinfofunc_t)dlsym(dlhandle, "moduleinfo"))();
            mip->dlhandle = dlhandle;
            return 0;
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

/* EOF mods.c */
