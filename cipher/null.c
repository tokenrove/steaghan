/* 
 * null.c
 * Created: Mon Mar 20 09:29:31 2000 by tek@wiw.org
 * Revised: Mon Mar 20 09:29:31 2000 (pending)
 * Copyright 2000 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <stdlib.h>
#include <string.h>

#include "steaghanmods.h"

#define NULL_MODULENAME "null"
#define NULL_MODULEDESC "null cipher [insecure!]"

typedef struct {
    char dummy; /* so that we don't return NULL in cipherinit */
} cipherhandle_t;

moduleinfo_t null_moduleinfo(void);
u_int32_t null_cipherkeylen(void);
u_int32_t null_cipherivlen(void);
u_int32_t null_cipherblocklen(void);
void null_cipherphrasetokey(char *phrase, u_int8_t *key, moduleinfo_t hash);
void *null_cipherinit(u_int8_t *key, u_int8_t *iv);
void null_encipher(void *p_, u_int8_t *in, u_int8_t *out, u_int32_t len);
void null_decipher(void *p_, u_int8_t *in, u_int8_t *out, u_int32_t len);
void null_cipherclose(void *p_);

modulefunctable_t *null_modulefunctable(void)
{
    modulefunctable_t *mft;

    mft = (modulefunctable_t *)malloc(sizeof(modulefunctable_t));
    if(mft == NULL) return NULL;
    mft->nfuncs = 9;
    mft->funcs = (modulefunc_t *)malloc(sizeof(modulefunc_t)*mft->nfuncs);
    if(mft->funcs == NULL) return NULL;

    mft->funcs[0].name = "moduleinfo";
    mft->funcs[0].f = (void *)null_moduleinfo;

    mft->funcs[1].name = "cipherkeylen";
    mft->funcs[1].f = (void *)null_cipherkeylen;

    mft->funcs[2].name = "cipherivlen";
    mft->funcs[2].f = (void *)null_cipherivlen;
    
    mft->funcs[3].name = "cipherblocklen";
    mft->funcs[3].f = (void *)null_cipherblocklen;
    
    mft->funcs[4].name = "cipherphrasetokey";
    mft->funcs[4].f = (void *)null_cipherphrasetokey;

    mft->funcs[5].name = "cipherinit";
    mft->funcs[5].f = (void *)null_cipherinit;

    mft->funcs[6].name = "encipher";
    mft->funcs[6].f = (void *)null_encipher;

    mft->funcs[7].name = "decipher";
    mft->funcs[7].f = (void *)null_decipher;

    mft->funcs[8].name = "cipherclose";
    mft->funcs[8].f = (void *)null_cipherclose;

    return mft;
}

moduleinfo_t null_moduleinfo(void)
{
    moduleinfo_t mi = { NULL_MODULENAME, NULL_MODULEDESC, ciphermod, 0 };
    return mi;
}

u_int32_t null_cipherkeylen(void)
{
    return 0;
}

u_int32_t null_cipherivlen(void)
{
    return 0;
}

u_int32_t null_cipherblocklen(void)
{
    return 1;
}

void null_cipherphrasetokey(char *phrase, u_int8_t *key, moduleinfo_t hash)
{
    return;
}

void *null_cipherinit(u_int8_t *key, u_int8_t *iv)
{
    cipherhandle_t *p;

    p = (cipherhandle_t *)malloc(sizeof(cipherhandle_t));
    if(p == NULL) return NULL;

    return (void *)p;
}

void null_encipher(void *p_, u_int8_t *in, u_int8_t *out, u_int32_t len)
{
    memmove(out, in, len);
    return;
}

void null_decipher(void *p_, u_int8_t *in, u_int8_t *out, u_int32_t len)
{
    memmove(out, in, len);
    return;
}

void null_cipherclose(void *p_)
{
    cipherhandle_t *p = (cipherhandle_t *)p_;

    free(p);
    return;
}

/* EOF null.c */
