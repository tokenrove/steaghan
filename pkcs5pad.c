/* 
 * pkcs5pad.c
 * Created: Mon Mar 20 10:36:28 2000 by tek@wiw.org
 * Revised: Mon Mar 20 12:08:36 2000 by tek@wiw.org
 * Copyright 2000 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <string.h>

#include "steaghan.h"

void pkcs5pad(u_int8_t *in, u_int32_t inlen, u_int8_t *out, u_int32_t *outlen,
              u_int8_t blocklen);
int pkcs5unpad(u_int8_t *in, u_int32_t inlen, u_int8_t *out,
               u_int32_t *outlen, u_int8_t blocklen);

void pkcs5pad(u_int8_t *in, u_int32_t inlen, u_int8_t *out, u_int32_t *outlen,
              u_int8_t blocklen)
{
    int r;

    if((inlen%blocklen) == 0) {
        r = blocklen;
    } else {
        r = blocklen-(inlen%blocklen);
    }

    *outlen += r;
    memmove(out, in, inlen);
    memset(out+inlen, r, r);

    return;
}

int pkcs5unpad(u_int8_t *in, u_int32_t inlen, u_int8_t *out,
               u_int32_t *outlen, u_int8_t blocklen)
{
    int r;

    r = in[inlen-1];
    if(r > blocklen || r <= 0) return 1;
    *outlen -= r;
    memmove(out, in, *outlen);

    return 0;
}

/* EOF pkcs5pad.c */
