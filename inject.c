/* 
 * inject.c
 * Created: Tue Jan 25 09:22:08 2000 by tek@wiw.org
 * Revised: Tue Jan 25 09:22:08 2000 (pending)
 * Copyright 2000 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <dlfcn.h>
#include <stdlib.h>

#include "steaghanmods.h"

void inject(moduleinfo_t prpg, moduleinfo_t wrapper, u_int8_t *secdata,
            u_int32_t seclen);

void inject(moduleinfo_t prpg, moduleinfo_t wrapper, u_int8_t *secdata,
            u_int32_t seclen)
{
    wrapwritefunc_t wrapwrite;
        wrapreadfunc_t wrapread;
    permugenfunc_t permugen;
    u_int32_t wraplen, i, k;

    wraplen = (*(wraplenfunc_t)dlsym(wrapper.dlhandle, "wraplen"))(wrapper.handle);
        wrapread = (wrapreadfunc_t)dlsym(wrapper.dlhandle, "wrapread");
    wrapwrite = (wrapwritefunc_t)dlsym(wrapper.dlhandle, "wrapwrite");
    permugen = (permugenfunc_t)dlsym(prpg.dlhandle, "permugen");

    for(i = 0; i < sizeof(u_int32_t)*8; i++) {
        k = (*permugen)(prpg.handle);
        (*wrapwrite)(wrapper.handle, k, (seclen>>i)&1);
    }
    
    for(i = 0; i < seclen; i++) {
        k = (*permugen)(prpg.handle);
        (*wrapwrite)(wrapper.handle, k, (secdata[i/8]>>(i%8))&1);
    }

    return;
}

/* EOF inject.c */
