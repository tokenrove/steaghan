/* 
 * phrasetokey.c
 * Created: Tue Mar 28 10:22:12 2000 by tek@wiw.org
 * Revised: Tue Mar 28 10:22:12 2000 (pending)
 * Copyright 2000 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "steaghanmods.h"

void standardphrasetokey(char *phrase, u_int8_t *key, u_int32_t keylen,
                         moduleinfo_t hash);

void standardphrasetokey(char *phrase, u_int8_t *key, u_int32_t keylen,
                         moduleinfo_t hash)
{
    hashfunc_t hashfunc = (hashfunc_t)getsym(&hash, "hash");
    u_int32_t hashlen = (*(hashlenfunc_t)getsym(&hash, "hashlen"))(),
        iterlen = 0;
    u_int8_t *buffer;

    buffer = (u_int8_t *)malloc(hashlen/8);

    (*hashfunc)((u_int8_t *)phrase, strlen(phrase), buffer);
    if(hashlen/8 <= keylen) {
        memmove(key, buffer, hashlen/8);
        iterlen += hashlen/8;
        while(iterlen < keylen) {
            (*hashfunc)(buffer, hashlen/8, buffer);
            memmove(key+iterlen, buffer, hashlen/8);
            iterlen += hashlen/8;
        }
        (*hashfunc)(buffer, hashlen/8, buffer);
        memmove(key+iterlen-hashlen/8, buffer, iterlen%keylen);

    } else {
        memmove(key, buffer, keylen);
    }
    free(buffer);
    return;
}

/* EOF phrasetokey.c */
