/* 
 * mods.c
 * Created: Tue Jan 25 12:43:43 2000 by tek@wiw.org
 * Revised: Wed Mar  8 15:10:08 2000 by tek@wiw.org
 * Copyright 2000 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <dlfcn.h>
#include <stdio.h>

#include "steaghanmods.h"

int loadmod(moduleinfo_t *mip, char *modpath);
void describemod(moduleinfo_t *mip);

int loadmod(moduleinfo_t *mip, char *modpath)
{
    void *dlhandle;
    char *err;
    
    dlhandle = dlopen(modpath, RTLD_NOW);
    if(dlhandle == NULL) {
        err = dlerror();
        if(err == NULL) err = "Unknown DL error";
/*        warn(err); */
        return 1;
    }
    *mip = (*(moduleinfofunc_t)dlsym(dlhandle, "moduleinfo"))();
    mip->dlhandle = dlhandle;

    return 0;
}

void describemod(moduleinfo_t *mip)
{
    printf("steaghan: Module ``%s'' [%s], is ", mip->name, mip->description);
    if(mip->moduletype == prpgmod) printf("a PRPG");
    else if(mip->moduletype == hashmod) printf("a hash");
    else if(mip->moduletype == wrappermod) printf("a wrapper");
    else if(mip->moduletype == filemod) printf("a file");
    else printf("an unknown");
    printf(" module.\n");
    return;
}

/* EOF mods.c */
