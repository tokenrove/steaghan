/* 
 * lr.c
 * Created: Tue Nov 23 03:17:34 1999 by tek@wiw.org
 * Revised: Tue Nov 23 06:17:06 1999 by tek@wiw.org
 * Copyright 1999 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <stdlib.h>

#include "steaghanmods.h"

#define LR_MODULENAME "lr"
#define LR_MODULEDESC "Luby-Rackoff construct"

/* module internal fu */
typedef struct {
    u_int32_t i, n, nbits;
    u_int8_t *key;
    u_int32_t keylen;
    u_int8_t *catspace;
    u_int32_t catlen;
    hashfunc_t hash;
    u_int32_t hashlen;
    u_int32_t *hashbuf;
} permuhandle_t;

moduleinfo_t lr_moduleinfo(void);
void *lr_permuinit(u_int32_t n, u_int8_t *key, u_int32_t keylen,
                          moduleinfo_t hash);
u_int32_t lr_permugen(void *p_);
void lr_permuclose(void *p_);

modulefunctable_t *lr_modulefunctable(void)
{
    modulefunctable_t *mft;

    mft = (modulefunctable_t *)malloc(sizeof(modulefunctable_t));
    if(mft == NULL) return NULL;
    mft->nfuncs = 4;
    mft->funcs = (modulefunc_t *)malloc(sizeof(modulefunc_t)*mft->nfuncs);
    if(mft->funcs == NULL) return NULL;

    mft->funcs[0].name = "moduleinfo";
    mft->funcs[0].f = (void *)lr_moduleinfo;

    mft->funcs[1].name = "permuinit";
    mft->funcs[1].f = (void *)lr_permuinit;

    mft->funcs[2].name = "permugen";
    mft->funcs[2].f = (void *)lr_permugen;

    mft->funcs[3].name = "permuclose";
    mft->funcs[3].f = (void *)lr_permuclose;

    return mft;
}

moduleinfo_t lr_moduleinfo(void)
{
    moduleinfo_t mi = { LR_MODULENAME, LR_MODULEDESC, prpgmod, NULL };
    return mi;
}

void *lr_permuinit(u_int32_t n, u_int8_t *key, u_int32_t keylen,
                   moduleinfo_t hash)
{
    permuhandle_t *p;
    int foo;

    p = (permuhandle_t *)malloc(sizeof(permuhandle_t));
    if(p == NULL) return NULL;

    if(n <= 0) return NULL;
    
    p->i = 0;
    p->n = n;
    p->nbits = 0;
    for(foo = p->n; foo > 0; foo >>= 1) p->nbits++;

    p->keylen = keylen;
    p->key = key;

    p->catlen = p->keylen+sizeof(u_int32_t);
    p->catspace = (u_int8_t *)malloc(p->catlen);
    if(p->catspace == NULL) return NULL;

    memcpy(p->catspace, p->key, p->keylen);

    if(hash.moduletype != hashmod) return NULL;
    p->hash = (hashfunc_t)getsym(&hash, "hash");
    p->hashlen = (*(hashlenfunc_t)getsym(&hash, "hashlen"))();
    p->hashbuf = (u_int32_t *)malloc(p->hashlen/8);
    if(p->hashbuf == NULL) return NULL;

    return (void *)p;
}

u_int32_t lr_permugen(void *p_)
{
    permuhandle_t *p = p_;
    u_int32_t X, Y;
    u_int32_t ret;

    do {
        X = p->i&((1<<((p->nbits+1)/2))-1);
        Y = (p->i>>((p->nbits+1)/2))&((1<<((p->nbits+1)/2))-1);

        memcpy(p->catspace+p->keylen, &X+sizeof(u_int32_t)-(p->nbits/2+7)/8,
               (p->nbits/2+7)/8);
        (*p->hash)(p->catspace, p->catlen, (u_int8_t *)p->hashbuf);
        Y = Y ^ (p->hashbuf[0]&((p->nbits+1)/2));

        memcpy(p->catspace+p->keylen, &Y+sizeof(u_int32_t)-(p->nbits/2+7)/8,
               (p->nbits/2+7)/8);
        (*p->hash)(p->catspace, p->catlen, (u_int8_t *)p->hashbuf);
        X = X ^ (p->hashbuf[1]&((p->nbits+1)/2));

        memcpy(p->catspace+p->keylen, &X+sizeof(u_int32_t)-(p->nbits/2+7)/8,
               (p->nbits/2+7)/8);
        (*p->hash)(p->catspace, p->catlen, (u_int8_t *)p->hashbuf);
        Y = Y ^ (p->hashbuf[2]&((p->nbits+1)/2));

        memcpy(p->catspace+p->keylen, &Y+sizeof(u_int32_t)-(p->nbits/2+7)/8,
               (p->nbits/2+7)/8);
        (*p->hash)(p->catspace, p->catlen, (u_int8_t *)p->hashbuf);
        X = X ^ (p->hashbuf[3]&((p->nbits+1)/2));

        ret = X|(Y<<((p->nbits+1)/2));
        p->i++;

    } while(ret >= p->n);

    return ret;
}

void lr_permuclose(void *p_)
{
    permuhandle_t *p = p_;

    free(p->catspace);
    free(p->hashbuf);
    free(p);
}

/* EOF lr.c */
