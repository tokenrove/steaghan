/* 
 * composite.c
 * Created: Tue Nov 23 06:06:35 1999 by tek@wiw.org
 * Revised: Wed Nov 24 01:14:11 1999 by tek@wiw.org
 * Copyright 1999 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 *
 * References: Tuomas Aura: "Practical Invisibility in Digital
 *                           Communication"
 *
 */

#include <dlfcn.h>
#include <stdlib.h>

#include "steaghanmods.h"

#define COMPOSITE_MODULENAME "composite [unreliable]"
#define COMPOSITE_MODULEDESC "Luby-Rackoff construct for appropriately composite [and large] n"

/* internal functions */
u_int32_t closestdivisor(u_int32_t n);

/* module internal fu */
typedef struct {
    u_int32_t i, n, x, y;
    u_int8_t *key;
    u_int32_t keylen;
    u_int8_t *catspace;
    u_int32_t catlen;
    hashfunc_t hash;
    u_int32_t hashlen;
    u_int32_t *hashbuf;
} permuhandle_t;

moduleinfo_t moduleinfo(void)
{
    moduleinfo_t mi = { COMPOSITE_MODULENAME, COMPOSITE_MODULEDESC, prpgmod, NULL, NULL };
    return mi;
}

void *permuinit(u_int32_t n, u_int8_t *key, u_int32_t keylen,
                moduleinfo_t hash)
{
    permuhandle_t *p;

    p = (permuhandle_t *)malloc(sizeof(permuhandle_t));
    if(p == NULL) return NULL;

    p->i = 0;
    p->n = n;

    p->keylen = keylen;
    p->key = key;

    p->catlen = p->keylen+sizeof(u_int32_t);
    p->catspace = (u_int8_t *)malloc(p->catlen);
    if(p->catspace == NULL) return NULL;

    if(hash.moduletype != hashmod) return NULL;
    p->hash = (hashfunc_t)dlsym(hash.dlhandle, "hash");
    p->hashlen = (*(hashlenfunc_t)dlsym(hash.dlhandle, "hashlen"))();
    p->hashbuf = (u_int32_t *)malloc(p->hashlen/8);
    if(p->hashbuf == NULL) return NULL;

    p->x = closestdivisor(p->n);
    p->y = p->n/p->x;

    return (void *)p;
}

/*
 */
u_int32_t permugen(void *p_)
{
    permuhandle_t *p = p_;
    u_int32_t X, Y;

    X = p->i/p->x;
    Y = p->i%p->x;

    memcpy(p->catspace, p->key, p->keylen);

    memcpy(p->catspace+p->keylen, &X, sizeof(u_int32_t));
    (*p->hash)(p->catspace, p->catlen, (u_int8_t *)p->hashbuf);
    Y = (Y + p->hashbuf[0]) % p->y;

    memcpy(p->catspace+p->keylen, &Y, sizeof(u_int32_t));
    (*p->hash)(p->catspace, p->catlen, (u_int8_t *)p->hashbuf);
    X = (X + p->hashbuf[1]) % p->x;

    memcpy(p->catspace+p->keylen, &X, sizeof(u_int32_t));
    (*p->hash)(p->catspace, p->catlen, (u_int8_t *)p->hashbuf);
    Y = (Y + p->hashbuf[2]) % p->y;

    memcpy(p->catspace+p->keylen, &Y, sizeof(u_int32_t));
    (*p->hash)(p->catspace, p->catlen, (u_int8_t *)p->hashbuf);
    X = (X + p->hashbuf[3]) % p->x;

    p->i++;
    return Y*p->x+X;
}

void permuclose(void *p_)
{
    permuhandle_t *p = p_;

    free(p->catspace);
    free(p->hashbuf);
    free(p);
}


int32_t abs(int32_t a) { return (a > 0) ? a : -a; }

u_int32_t closestdivisor(u_int32_t n)
{
    int32_t i, diff, x;

    diff = n-1;
    x = 1;
    for(i = 2; i < (n/2); i++) {
        if((n%i) == 0 && abs(i-(n/i)) < diff) {
            diff = abs(i-(n/i));
            x = i;
        }
    }
    return x;
}

/* EOF composite.c */
