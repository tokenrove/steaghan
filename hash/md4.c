/* 
 * md4.c
 * Copyright 1999 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <stdlib.h>
#include <string.h>

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

moduleinfo_t md4_moduleinfo(void);
void md4_hash(u_int8_t *d, u_int32_t len, u_int8_t *oout);
u_int32_t md4_hashlen(void);
void md4_hash_internal(u_int32_t *X, u_int32_t *H);

modulefunctable_t *md4_modulefunctable(void)
{
    modulefunctable_t *mft;

    mft = (modulefunctable_t *)malloc(sizeof(modulefunctable_t));
    if(mft == NULL) return NULL;
    mft->nfuncs = 3;
    mft->funcs = (modulefunc_t *)malloc(sizeof(modulefunc_t)*mft->nfuncs);
    if(mft->funcs == NULL) return NULL;

    mft->funcs[0].name = "moduleinfo";
    mft->funcs[0].f = (void *)md4_moduleinfo;

    mft->funcs[1].name = "hash";
    mft->funcs[1].f = (void *)md4_hash;

    mft->funcs[2].name = "hashlen";
    mft->funcs[2].f = (void *)md4_hashlen;

    return mft;
}

moduleinfo_t md4_moduleinfo(void)
{
    moduleinfo_t mi = { MD4_MODULENAME, MD4_MODULEDESC, hashmod, 0 };
    return mi;
}

u_int32_t md4_hashlen(void)
{
    return MD4_HASHLEN;
}

void md4_hash(u_int8_t *d, u_int32_t len, u_int8_t *out)
{
    u_int32_t X[16], H[MD4_IVSIZE];
    u_int32_t i, j, k;

    for(i = 0; i < MD4_IVSIZE; i++) H[i] = md4_iv[i];

    for(i = 0; i < len/64; i++) {
        for(j = 0; j < 16; j++)
            for(X[j] = 0, k = 0; k < 4; k++)
                X[j] |= d[(16*i+j)*sizeof(u_int32_t)+(3-k)]<<(8*(3-k));

        md4_hash_internal(X, H);
    }

    memset(X, 0, sizeof(u_int32_t)*16);
    for(i = i*64, j = 0; i < len; i++) {
        X[j] |= d[i]<<(8*(i%4));
        if(i%4 == 3) j++;
    }

    len *= 8; /* FIXME --> we need a long long */
    X[j] |= (1<<7)<<(8*(i%4));
    if((signed)(MD4_PADMULTIPLE-(((len+1)%MD4_PADMULTIPLE)+64)) < 0) {
        md4_hash_internal(X, H);
        memset(X, 0, sizeof(u_int32_t)*16);
    }
    j = 14;

    for(i = 0; i < 64; i++) {
        X[j+(i/32)] |= ((len&1)<<(i%32)); len>>=1;
    }

    md4_hash_internal(X, H);

#ifdef USE_LITTLE_ENDIAN
    memcpy(out, H, MD4_IVSIZE*sizeof(u_int32_t));
#else
    for(i = 0; i < MD4_IVSIZE; i++) {
        out[4*i]=H[i];
        out[4*i+1]=H[i]>>8;
        out[4*i+2]=H[i]>>16;
        out[4*i+3]=H[i]>>24;
    }
#endif

    return;
}

void md4_hash_internal(u_int32_t *X, u_int32_t *H)
{
    u_int32_t A, B, C, D, j, t;
    
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
    return;
}

/* EOF md4.c */
