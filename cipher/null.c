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

moduleinfo_t moduleinfo(void)
{
    moduleinfo_t mi = { NULL_MODULENAME, NULL_MODULEDESC, ciphermod, 0 };
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
    memmove(out, in, len);
    return;
}

void decipher(void *p_, u_int8_t *in, u_int8_t *out, u_int32_t len)
{
    memmove(out, in, len);
    return;
}

void cipherclose(void *p_)
{
    cipherhandle_t *p = (cipherhandle_t *)p_;

    free(p);
    return;
}

/* EOF null.c */
