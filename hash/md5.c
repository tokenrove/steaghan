/* 
 * md5.c
 * Copyright 1999 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <endian.h>
#include <string.h>

#include "steaghanmods.h"

#define MD5_MODULENAME "md5"
#define MD5_MODULEDESC "MD5 hash [not entirely secure]"
#define MD5_HASHLEN    128

/* MD5 constants */
#define MD5_IVSIZE 4
#define MD5_YSIZE 64
#define MD5_ZSIZE 64
#define MD5_SSIZE 64
#define MD5_PADMULTIPLE 512

/* initial chaining values */
static u_int32_t md5_iv[MD5_IVSIZE] = { 0x67452301, 0xefcdab89, 0x98badcfe,
                                        0x10325476 };
/* additive 32-bit constants */
static u_int32_t md5_y[MD5_YSIZE] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf,
    0x4787c62a, 0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af,
    0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e,
    0x49b40821, 0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8, 0x21e1cde6,
    0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8,
    0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122,
    0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039,
    0xe6db99e5, 0x1fa27cf8, 0xc4ac5665, 0xf4292244, 0x432aff97,
    0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d,
    0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};
/* order for accessing source words */
static u_int32_t md5_z[MD5_ZSIZE] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    1, 6, 11, 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12,
    5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2,
    0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9
};
/* number of bit positions for left rotates */
static u_int32_t md5_s[MD5_SSIZE] = {
    7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
    5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};

moduleinfo_t moduleinfo(void);
void hash(u_int8_t *d, u_int32_t len, u_int8_t *oout);
u_int32_t hashlen(void);
void hash_internal(u_int32_t *X, u_int32_t *H);

moduleinfo_t moduleinfo(void)
{
    moduleinfo_t mi = { MD5_MODULENAME, MD5_MODULEDESC, hashmod, 0 };
    return mi;
}

u_int32_t hashlen(void)
{
    return MD5_HASHLEN;
}

void hash(u_int8_t *d, u_int32_t len, u_int8_t *out)
{
    u_int32_t X[16], H[MD5_IVSIZE];
    u_int32_t i, j, k, r;

    for(i = 0; i < MD5_IVSIZE; i++) H[i] = md5_iv[i];

    for(i = 0; i < len/64; i++) {
        for(j = 0; j < 16; j++)
            for(X[j] = 0, k = 0; k < 4; k++)
                X[j] |= d[(16*i+j)*sizeof(u_int32_t)+(3-k)]<<(8*(3-k));

        hash_internal(X, H);
    }

    for(j = 0; j < 16; j++) X[j] = 0;
    for(i = i*64, j = 0; i < len; i++) {
        X[j] |= d[i]<<(8*(i%4));
        if(i%4 == 3) j++;
    }

    len *= 8; /* FIXME --> we need a long long instead */

    X[j] |= (1<<7)<<(8*(i%4));
    r = MD5_PADMULTIPLE-(((len+1)%MD5_PADMULTIPLE)+64);
    if(r < 0) {
        hash_internal(X, H);
        for(j = 0; j < 16; j++) X[j] = 0;
    }
    j = 14;

    for(i = 0; i < 64; i++) {
        X[j+(i/32)] |= ((len&1)<<(i%32)); len>>=1;
    }

    hash_internal(X, H);

#ifdef LITTLE_ENDIAN
    memcpy(out, H, MD5_IVSIZE*sizeof(u_int32_t));
#else
    for(i = 0; i < MD5_IVSIZE; i++) {
        out[4*i+0]=(H[i]>> 0)&0xFF;
        out[4*i+1]=(H[i]>> 8)&0xFF;
        out[4*i+2]=(H[i]>>16)&0xFF;
        out[4*i+3]=(H[i]>>24)&0xFF;
    }
#endif

    return;
}

void hash_internal(u_int32_t *X, u_int32_t *H)
{
    u_int32_t A, B, C, D, j, t;

    A = H[0]; B = H[1]; C = H[2]; D = H[3];
    /* round one */
    for(j = 0; j < 16; j++) {
        t = A+((B&C)|(~B&D))+X[md5_z[j]]+md5_y[j];
        A = D; D = C; C = B; B += (t << md5_s[j])|(t >> (32-md5_s[j]));
    }
    /* round two */
    for(j = 16; j < 32; j++) {
        t = A+((B&D)|(C&~D))+X[md5_z[j]]+md5_y[j];
        A = D; D = C; C = B; B += (t << md5_s[j])|(t >> (32-md5_s[j]));
    }
    /* round three */
    for(j = 32; j < 48; j++) {
        t = A+(B^C^D)+X[md5_z[j]]+md5_y[j];
        A = D; D = C; C = B; B += (t << md5_s[j])|(t >> (32-md5_s[j]));
    }
    /* round four */
    for(j = 48; j < 64; j++) {
        t = A+(C^(B|~D))+X[md5_z[j]]+md5_y[j];
        A = D; D = C; C = B; B += (t << md5_s[j])|(t >> (32-md5_s[j]));
    }
    /* update chaining values */
    H[0] += A; H[1] += B; H[2] += C; H[3] += D;
    
    return;
}

/* EOF md5.c */
