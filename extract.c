/* 
 * extract.c
 * Created: Tue Jan 25 09:32:31 2000 by tek@wiw.org
 * Revised: Wed Mar  8 16:05:58 2000 by tek@wiw.org
 * Copyright 2000 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <assert.h>
#include <dlfcn.h>
#include <stdlib.h>

#include "steaghan.h"

u_int8_t *extract(moduleinfo_t prpg, moduleinfo_t wrapper, u_int32_t *seclen);

u_int8_t *extract(moduleinfo_t prpg, moduleinfo_t wrapper, u_int32_t *seclen)
{
    wrapreadfunc_t wrapread;
    permugenfunc_t permugen;
    u_int32_t wraplen, i, k, j;
    u_int8_t *secdata;

    wraplen = (*(wraplenfunc_t)getsym(&wrapper, "wraplen"))(wrapper.handle);
    wrapread = (wrapreadfunc_t)getsym(&wrapper, "wrapread");
    permugen = (permugenfunc_t)getsym(&prpg, "permugen");

    *seclen = 0;
    for(i = 0; i < sizeof(u_int32_t)*8; i++) {
        k = (*permugen)(prpg.handle);
        *seclen |= j = (*wrapread)(wrapper.handle, k)<<i;
    }

    statusbar_init(*seclen);
    
    secdata = (u_int8_t *)calloc(*seclen/8, sizeof(u_int8_t));
    assert(secdata != NULL);
    
    for(i = 0; i < *seclen; i++) {
        k = (*permugen)(prpg.handle);
        j = (*wrapread)(wrapper.handle, k);
        secdata[i/8] |= j <<(i%8);
        statusbar_update(1);
    }

    statusbar_close();
    return secdata;
}

/* EOF extract.c */
