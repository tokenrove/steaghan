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

moduleinfo_t moduleinfo(void)
{
    moduleinfo_t mi = { ROT13_MODULENAME, ROT13_MODULEDESC, ciphermod, 0 };
    return mi;
}

u_int32_t cipherkeylen(void)
{
    return 0;
}

u_int32_t cipherivlen(void)
{
    return 0;
}

u_int32_t cipherblocklen(void)
{
    return 1;
}

void cipherphrasetokey(char *phrase, u_int8_t *key, moduleinfo_t hash)
{
    return;
}

void *cipherinit(u_int8_t *key, u_int8_t *iv)
{
    cipherhandle_t *p;

    p = (cipherhandle_t *)malloc(sizeof(cipherhandle_t));
    if(p == NULL) return NULL;

    return (void *)p;
}

void encipher(void *p_, u_int8_t *in, u_int8_t *out, u_int32_t len)
{
    rot13(in, out, len);
    return;
}

void decipher(void *p_, u_int8_t *in, u_int8_t *out, u_int32_t len)
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

void cipherclose(void *p_)
{
    cipherhandle_t *p = (cipherhandle_t *)p_;

    free(p);
    return;
}

/* EOF rot13.c */
