/* 
 * rot13.c
 * Created: Mon Mar 20 17:24:32 2000 by tek@wiw.org
 * Revised: Mon Mar 20 18:28:25 2000 by tek@wiw.org
 * Copyright 2000 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <stdlib.h>
#include <string.h>

#include "steaghanmods.h"

#define ROT13_MODULENAME "rot13"
#define ROT13_MODULEDESC "rot13 cipher [insecure!]"

typedef struct {
    char dummy; /* so that we don't return NULL in cipherinit */
} cipherhandle_t;

void rot13(u_int8_t *in, u_int8_t *out, u_int32_t len);

moduleinfo_t rot13_moduleinfo(void);
u_int32_t rot13_cipherkeylen(void);
u_int32_t rot13_cipherivlen(void);
u_int32_t rot13_cipherblocklen(void);
void rot13_cipherphrasetokey(char *phrase, u_int8_t *key, moduleinfo_t hash);
void *rot13_cipherinit(u_int8_t *key, u_int8_t *iv);
void rot13_encipher(void *p_, u_int8_t *in, u_int8_t *out, u_int32_t len);
void rot13_decipher(void *p_, u_int8_t *in, u_int8_t *out, u_int32_t len);
void rot13_cipherclose(void *p_);


modulefunctable_t *rot13_modulefunctable(void)
{
    modulefunctable_t *mft;

    mft = (modulefunctable_t *)malloc(sizeof(modulefunctable_t));
    if(mft == NULL) return NULL;
    mft->nfuncs = 9;
    mft->funcs = (modulefunc_t *)malloc(sizeof(modulefunc_t)*mft->nfuncs);
    if(mft->funcs == NULL) return NULL;

    mft->funcs[0].name = "moduleinfo";
    mft->funcs[0].f = (void *)rot13_moduleinfo;

    mft->funcs[1].name = "cipherkeylen";
    mft->funcs[1].f = (void *)rot13_cipherkeylen;

    mft->funcs[2].name = "cipherivlen";
    mft->funcs[2].f = (void *)rot13_cipherivlen;
    
    mft->funcs[3].name = "cipherblocklen";
    mft->funcs[3].f = (void *)rot13_cipherblocklen;
    
    mft->funcs[4].name = "cipherphrasetokey";
    mft->funcs[4].f = (void *)rot13_cipherphrasetokey;

    mft->funcs[5].name = "cipherinit";
    mft->funcs[5].f = (void *)rot13_cipherinit;

    mft->funcs[6].name = "encipher";
    mft->funcs[6].f = (void *)rot13_encipher;

    mft->funcs[7].name = "decipher";
    mft->funcs[7].f = (void *)rot13_decipher;

    mft->funcs[8].name = "cipherclose";
    mft->funcs[8].f = (void *)rot13_cipherclose;

    return mft;
}

moduleinfo_t rot13_moduleinfo(void)
{
    moduleinfo_t mi = { ROT13_MODULENAME, ROT13_MODULEDESC, ciphermod, 0 };
    return mi;
}

u_int32_t rot13_cipherkeylen(void)
{
    return 0;
}

u_int32_t rot13_cipherivlen(void)
{
    return 0;
}

u_int32_t rot13_cipherblocklen(void)
{
    return 1;
}

void rot13_cipherphrasetokey(char *phrase, u_int8_t *key, moduleinfo_t hash)
{
    return;
}

void *rot13_cipherinit(u_int8_t *key, u_int8_t *iv)
{
    cipherhandle_t *p;

    p = (cipherhandle_t *)malloc(sizeof(cipherhandle_t));
    if(p == NULL) return NULL;

    return (void *)p;
}

void rot13_encipher(void *p_, u_int8_t *in, u_int8_t *out, u_int32_t len)
{
    rot13(in, out, len);
    return;
}

void rot13_decipher(void *p_, u_int8_t *in, u_int8_t *out, u_int32_t len)
{
    rot13(in, out, len);
    return;
}

void rot13(u_int8_t *in, u_int8_t *out, u_int32_t len)
{
    for(; len > 0; len--) {
        /* FIXME: Doesn't work with EBCDIC!!@# ;-) */
        if(((in[len-1]|0x20) >= 'a') && ((in[len-1]|0x20) <= 'm'))
            out[len-1] = in[len-1]+13;
        else if((in[len-1]|0x20) >= 'n' && (in[len-1]|0x20) <= 'z')
            out[len-1] = in[len-1]-13;
        else
            out[len-1] = in[len-1];
    }
    return;
}

void rot13_cipherclose(void *p_)
{
    cipherhandle_t *p = (cipherhandle_t *)p_;

    free(p);
    return;
}

/* EOF rot13.c */
