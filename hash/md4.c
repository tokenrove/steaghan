/* 
 * md4.c
 * Copyright 1999 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <assert.h>
#include <stdlib.h>

#include "steaghanmods.h"

#define MD4_MODULENAME "md4"
#define MD4_MODULEDESC "MD4 hash [insecure!]"
#define MD4_HASHLEN    128

/* MD4 constants */
#define MD4_IVSIZE 4
#define MD4_NROUNDS 3
#define MD4_ZSIZE 48
#define MD4_SSIZE 48
#define MD4_PADMULTIPLE 512

/* initial chaining values */
static u_int32_t md4_iv[MD4_IVSIZE] = { 0x67452301, 0xefcdab89, 0x98badcfe,
                                        0x10325476 };
/* additive 32-bit constants */
static u_int32_t md4_y[MD4_NROUNDS] = {
    /* zero */ 0,
    /* square root of two */ 0x5a827999,
    /* square root of three */ 0x6ed9eba1
};
/* order for accessing source words */
static u_int32_t md4_z[MD4_ZSIZE] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15,
    0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15
};
/* number of bit positions for left rotates */
static u_int32_t md4_s[MD4_SSIZE] = {
    3, 7, 11, 19, 3, 7, 11, 19, 3, 7, 11, 19, 3, 7, 11, 19,
    3, 5, 9, 13, 3, 5, 9, 13, 3, 5, 9, 13, 3, 5, 9, 13,
    3, 9, 11, 15, 3, 9, 11, 15, 3, 9, 11, 15, 3, 9, 11, 15
};

moduleinfo_t moduleinfo(void);
u_int8_t *hash(u_int8_t *d, u_int32_t len);
u_int32_t hashlen(void);

moduleinfo_t moduleinfo(void)
{
    moduleinfo_t mi = { MD4_MODULENAME, MD4_MODULEDESC, hashmod, 0 };
    return mi;
}

u_int32_t hashlen(void)
{
    return MD4_HASHLEN;
}

u_int8_t *hash(u_int8_t *d_, u_int32_t len)
{
    u_int8_t *d, *ret;
    u_int32_t *x, X[16], H[MD4_IVSIZE], A, B, C, D;
    u_int32_t i, j, k, t, b, blen, r, m;
    
    b = len*8;
    blen = b;
    d = (u_int8_t *)malloc(len+(MD4_PADMULTIPLE/8));
    assert(d != NULL);
    memset(d+len, 0, MD4_PADMULTIPLE/8);
    memcpy(d, d_, len);

    d[blen/8] |= 1<<7; blen++;
    r = MD4_PADMULTIPLE-((blen%MD4_PADMULTIPLE)+64);
    if(r < 0) r += 512;
    blen += r;

    for(i = 0; i < 64; i++, blen++) {
        d[blen/8] |= ((b&1)<<(blen%8)); b>>=1; if(blen%8 == 0) { len++; }
    }

    m = (blen+MD4_PADMULTIPLE-1)/MD4_PADMULTIPLE;
    x = (u_int32_t *)d;

    for(i = 0; i < MD4_IVSIZE; i++) H[i] = md4_iv[i];

    for(i = 0; i < m; i++) {
        for(j = 0; j < 16; j++)
            for(X[j] = 0, k = 0; k < 4; k++)
                X[j] |= d[(16*i+j)*sizeof(u_int32_t)+(3-k)]<<(8*(3-k));

        A = H[0]; B = H[1]; C = H[2]; D = H[3];
        /* round one */
        for(j = 0; j < 16; j++) {
            t = A+((B&C)|(~B&D))+X[md4_z[j]]+md4_y[0];
            A = D; D = C; C = B; B = (t << md4_s[j])|(t >> (32-md4_s[j]));
        }
        /* round two */
        for(j = 16; j < 32; j++) {
            t = A+((B&C)|(B&D)|(C&D))+X[md4_z[j]]+md4_y[1];
            A = D; D = C; C = B; B = (t << md4_s[j])|(t >> (32-md4_s[j]));
        }
        /* round three */
        for(j = 32; j < 48; j++) {
            t = A+(B^C^D)+X[md4_z[j]]+md4_y[2];
            A = D; D = C; C = B; B = (t << md4_s[j])|(t >> (32-md4_s[j]));
        }
        /* update chaining values */
        H[0] += A; H[1] += B; H[2] += C; H[3] += D;
    }

    if(d != d_) free(d);
    ret = (u_int8_t *)malloc(MD4_HASHLEN/8);
    assert(ret != NULL);
    for(i = 0; i < MD4_IVSIZE; i++)
#ifdef BIG_ENDIAN
        for(j = 0; j < 4; j++)
            memcpy(ret+(i*sizeof(u_int32_t))+j, (&H[i])+(3-j), 1);
#else
        memcpy(ret+(i*sizeof(u_int32_t)), &H[i], sizeof(u_int32_t));
#endif
    
    return ret;
}

/* EOF md4.c */
