/* 
 * lr.c
 * Created: Tue Nov 23 03:17:34 1999 by tek@wiw.org
 * Revised: Tue Nov 23 06:17:06 1999 by tek@wiw.org
 * Copyright 1999 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <assert.h>
#include <stdlib.h>

#include "steaghanmods.h"

#define LR_MODULENAME "lr"
#define LR_MODULEDESC "Luby-Rackoff construct"

/* exportable functions */
moduleinfo_t moduleinfo(void);
void *permuinit(u_int32_t n, u_int8_t *key, u_int32_t keylen,
                hashfunc_t hash, u_int32_t hashlen);
u_int32_t permugen(void *p_);
void permuclose(void *p_);

/* module internal fu */
typedef struct {
    u_int32_t i, n, nbits;
    u_int8_t *key;
    u_int32_t keylen;
    u_int8_t *catspace;
    u_int32_t catlen;
    hashfunc_t hash;
    u_int32_t hashlen;
} permuhandle_t;

moduleinfo_t moduleinfo(void)
{
    moduleinfo_t mi = { LR_MODULENAME, LR_MODULEDESC, prpgmod, NULL };
    return mi;
}

void *permuinit(u_int32_t n, u_int8_t *key, u_int32_t keylen,
                hashfunc_t hash, u_int32_t hashlen)
{
    permuhandle_t *p;
    int foo;

    p = (permuhandle_t *)malloc(sizeof(permuhandle_t));
    assert(p != NULL);

    assert(n > 0);
    
    p->i = 0;
    p->n = n;
    p->nbits = 0;
    for(foo = p->n; foo > 0; foo >>= 1) p->nbits++;

    p->keylen = keylen;
    p->key = key;

    p->catlen = p->keylen+sizeof(u_int32_t);
    p->catspace = (u_int8_t *)malloc(p->catlen);
    assert(p->catspace != NULL);

    memcpy(p->catspace, p->key, p->keylen);

    p->hash = hash;
    p->hashlen = hashlen;
    
    return (void *)p;
}

u_int32_t permugen(void *p_)
{
    permuhandle_t *p = p_;
    u_int32_t X, Y;
    u_int32_t ret;

    do {
        X = p->i&((1<<((p->nbits+1)/2))-1);
        Y = (p->i>>((p->nbits+1)/2))&((1<<((p->nbits+1)/2))-1);

        memcpy(p->catspace+p->keylen, &X+sizeof(u_int32_t)-(p->nbits/2+7)/8,
               (p->nbits/2+7)/8);
        Y = Y ^ ((((u_int32_t*)(*p->hash)(p->catspace, p->catlen))[0])&((p->nbits+1)/2));

        memcpy(p->catspace+p->keylen, &Y+sizeof(u_int32_t)-(p->nbits/2+7)/8,
               (p->nbits/2+7)/8);
        X = X ^ (((u_int32_t*)(*p->hash)(p->catspace, p->catlen))[1]&((p->nbits+1)/2));

        memcpy(p->catspace+p->keylen, &X+sizeof(u_int32_t)-(p->nbits/2+7)/8,
               (p->nbits/2+7)/8);
        Y = Y ^ (((u_int32_t*)(*p->hash)(p->catspace, p->catlen))[2]&((p->nbits+1)/2));

        memcpy(p->catspace+p->keylen, &Y+sizeof(u_int32_t)-(p->nbits/2+7)/8,
               (p->nbits/2+7)/8);
        X = X ^ (((u_int32_t*)(*p->hash)(p->catspace, p->catlen))[3]&((p->nbits+1)/2));

        ret = X|(Y<<((p->nbits+1)/2));
        p->i++;

    } while(ret >= p->n);

    return ret;
}

void permuclose(void *p_)
{
    permuhandle_t *p = p_;

    free(p->catspace);
    free(p);
}

/* EOF lr.c */
