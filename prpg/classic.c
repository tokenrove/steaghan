/* 
 * classic.c
 * Copyright 1999 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>

#include "steaghanmods.h"

#define CLASSIC_MODULENAME "classic"
#define CLASSIC_MODULEDESC "Classical shuffle method"

/* module internal fu */
typedef struct {
    u_int32_t i, n, nbits;
    u_int32_t *shuffle;
    hashfunc_t hash;
    u_int32_t hashlen;
} permuhandle_t;

moduleinfo_t moduleinfo(void)
{
    moduleinfo_t mi = { CLASSIC_MODULENAME, CLASSIC_MODULEDESC, prpgmod,
                        NULL };
    return mi;
}

/*
 */
void *permuinit(u_int32_t n, u_int8_t *key, u_int32_t keylen,
                moduleinfo_t hash)
{
    permuhandle_t *p;
    int i, j;
    u_int32_t *h, hcount, hpermute, catlen;
    u_int8_t *catspace;

    p = (permuhandle_t *)malloc(sizeof(permuhandle_t));
    if(p == NULL) return NULL;

    if(n <= 0) return NULL;
    
    p->i = 0;
    p->n = n;
    p->shuffle = (u_int32_t *)malloc(p->n*sizeof(u_int32_t));
    if(p->shuffle == NULL) return NULL;

    for(i = 0; i < n; i++) {
        p->shuffle[i] = i;
    }

    if(hash.moduletype != hashmod) return NULL;
    p->hash = (hashfunc_t)dlsym(hash.dlhandle, "hash");
    p->hashlen = (*(hashlenfunc_t)dlsym(hash.dlhandle, "hashlen"))();

    hpermute = 0;
    catlen = keylen+sizeof(u_int32_t);
    catspace = (u_int8_t *)malloc(catlen);
    if(catspace == NULL) return NULL;
    memcpy(catspace, key, keylen);
    memcpy(catspace+keylen, &hpermute, sizeof(u_int32_t));

    h = (u_int32_t *)malloc(p->hashlen/8);
    if(h == NULL) return NULL;
    
    (*p->hash)(catspace, catlen, (u_int8_t *)h);
    hcount = p->hashlen/32;

    /* FIXME: this shouldn't assume h is at least 32 bits */
    for(i = 0; i < p->n-1 && n > 0; i++) {
        j = h[0]%n;
        if(j != n-1) {
            p->shuffle[--n] ^= p->shuffle[j] ^= p->shuffle[n] ^= p->shuffle[j];
        }

        if(hcount-1 > 0) {
            for(j = 0; j < hcount; j++) {
                h[j] = h[j+1];
            }
            hpermute++;
            hcount--;
        } else {
            memcpy(catspace+keylen, &hpermute, sizeof(u_int32_t));
            (*p->hash)(catspace, catlen, (u_int8_t *)h);
            hcount = p->hashlen/32;
        }
    }

    free(h);
    free(catspace);

    return (void *)p;
}

u_int32_t permugen(void *p_)
{
    permuhandle_t *p = p_;
    
    u_int32_t ret = p->shuffle[p->i];
    p->i++;
    return ret;
}

void permuclose(void *p_)
{
    permuhandle_t *p = p_;

    if(p->shuffle) free(p->shuffle);
    free(p);
}

/* EOF classic.c */
