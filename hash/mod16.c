/* 
 * mod16.c
 * Created: Wed Dec 15 09:45:27 1999 by tek@wiw.org
 * Revised: Wed Dec 15 09:45:27 1999 (pending)
 * Copyright 1999 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include "steaghanmods.h"

#define MOD16_MODULENAME "mod16"
#define MOD16_MODULEDESC "modulo-2^16 hash [insecure!]"

moduleinfo_t moduleinfo(void);
u_int8_t *hash(u_int8_t *d, u_int32_t len);
u_int32_t hashlen(void);

moduleinfo_t moduleinfo(void)
{
    moduleinfo_t mi = { MOD16_MODULENAME, MOD16_MODULEDESC, hashmod, 0 };
    return mi;
}

u_int32_t hashlen(void)
{
    return 16;
}

u_int8_t *hash(u_int8_t *d, u_int32_t len)
{
    u_int16_t hsh = 0xFFFF;
    u_int8_t *ret;
    int i;

    for(i = 0; i < len; i++) {
        hsh += d[i];
    }

    ret = (u_int8_t *)malloc(2);
    if(!ret) bomb();

    ret[0] = hsh&0xFF;
    ret[1] = hsh>>8;
    return ret;
}

/* EOF mod16.c */
