/* 
 * mods.c
 * Created: Tue Jan 25 12:43:43 2000 by tek@wiw.org
 * Revised: Tue Jan 25 12:43:43 2000 (pending)
 * Copyright 2000 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <dlfcn.h>

#include "steaghan.h"

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
    printf("steaghan: Module ``%s'' [%s],", mip->name, mip->description);
    printf(" is %s module.\n",
           ((mip->moduletype == prpgmod)?"a PRPG":
            (mip->moduletype == hashmod)?"a hash":
            (mip->moduletype == wrappermod)?"a wrapper":"an unknown"));
    
    mip->dlhandle = dlhandle;

    return 0;
}

/* EOF mods.c */
