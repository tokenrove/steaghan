/* 
 * ripemd160.c
 * Copyright 1999 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 *
 * Implementing this in parallel (as two threads, for example) would be
 * interesting.
 */

#include <assert.h>
#include <stdlib.h>

#include "steaghanmods.h"

#define RIPEMD160_MODULENAME "ripemd160"
#define RIPEMD160_MODULEDESC "RIPEMD-160 hash"
#define RIPEMD160_HASHLEN    160

/* RIPEMD160 constants */
#define RIPEMD160_IVSIZE 5
#define RIPEMD160_NROUNDS 5
#define RIPEMD160_YSIZE 80
#define RIPEMD160_ZSIZE 80
#define RIPEMD160_SSIZE 80
#define RIPEMD160_PADMULTIPLE 512

/* initial chaining values */
static u_int32_t ripemd160_iv[RIPEMD160_IVSIZE] = { 0x67452301, 0xefcdab89,
                                                    0x98badcfe, 0x10325476,
                                                    0xc3d2e1f0 };
/* additive 32-bit constants */
static u_int32_t ripemd160_y[2][RIPEMD160_NROUNDS] = {
    { 0, 0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xa953fd4e }, /* left */
    { 0x50a28be6, 0x5c4dd124, 0x6d703ef3, 0x7a6d76e9, 0 } /* right */
};
/* order for accessing source words */
static u_int32_t ripemd160_z[2][RIPEMD160_ZSIZE] = {
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
      7, 4, 13, 1, 10, 6, 15, 3, 12, 0, 9, 5, 2, 14, 11, 8,
      3, 10, 14, 4, 9, 15, 8, 1, 2, 7, 0, 6, 13, 11, 5, 12,
      1, 9, 11, 10, 0, 8, 12, 4, 13, 3, 7, 15, 14, 5, 6, 2,
      4, 0, 5, 9, 7, 12, 2, 10, 14, 1, 3, 8, 11, 6, 15, 13 },
    { 5, 14, 7, 0, 9, 2, 11, 4, 13, 6, 15, 8, 1, 10, 3, 12,
      6, 11, 3, 7, 0, 13, 5, 10, 14, 15, 8, 12, 4, 9, 1, 2,
      15, 5, 1, 3, 7, 14, 6, 9, 11, 8, 12, 2, 10, 0, 4, 13,
      8, 6, 4, 1, 3, 11, 15, 0, 5, 12, 2, 13, 9, 7, 10, 14,
      12, 15, 10, 4, 1, 5, 8, 7, 6, 2, 13, 14, 0, 3, 9, 11 }
};
/* number of bit positions for left rotates */
static u_int32_t ripemd160_s[2][RIPEMD160_SSIZE] = {
    { 11, 14, 15, 12, 5, 8, 7, 9, 11, 13, 14, 15, 6, 7, 9, 8,
      7, 6, 8, 13, 11, 9, 7, 15, 7, 12, 15, 9, 11, 7, 13, 12,
      11, 13, 6, 7, 14, 9, 13, 15, 14, 8, 13, 6, 5, 12, 7, 5,
      11, 12, 14, 15, 14, 15, 9, 8, 9, 14, 5, 6, 8, 6, 5, 12,
      9, 15, 5, 11, 6, 8, 13, 12, 5, 12, 13, 14, 11, 8, 5, 6 },
    { 8, 9, 9, 11, 13, 15, 15, 5, 7, 7, 8, 11, 14, 14, 12, 6,
      9, 13, 15, 7, 12, 8, 9, 11, 7, 7, 12, 7, 6, 15, 13, 11,
      9, 7, 15, 11, 8, 6, 6, 14, 12, 13, 5, 14, 13, 13, 7, 5,
      15, 5, 8, 11, 14, 14, 6, 14, 6, 9, 12, 9, 12, 5, 15, 8,
      8, 5, 12, 9, 12, 5, 14, 6, 8, 13, 6, 5, 15, 13, 11, 11 }
};

moduleinfo_t moduleinfo(void);
void hash(u_int8_t *d, u_int32_t len, u_int8_t *oout);
u_int32_t hashlen(void);
void hash_internal(u_int32_t *X, u_int32_t *H);

moduleinfo_t moduleinfo(void)
{
    moduleinfo_t mi = { RIPEMD160_MODULENAME, RIPEMD160_MODULEDESC, hashmod,
                        0 };
    return mi;
}

u_int32_t hashlen(void)
{
    return RIPEMD160_HASHLEN;
}

/* s/32/sizeof(a)*8/ */
#define RIPEMD160_ROLL(a, b) (((a) << (b))|((a) >> (32-(b))))


void hash(u_int8_t *d, u_int32_t len, u_int8_t *out)
{
    u_int32_t X[16], H[RIPEMD160_IVSIZE];
    u_int32_t i, j, k, r;

    for(i = 0; i < RIPEMD160_IVSIZE; i++) H[i] = ripemd160_iv[i];

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
    r = RIPEMD160_PADMULTIPLE-(((len+1)%RIPEMD160_PADMULTIPLE)+64);
    if(r < 0) {
        hash_internal(X, H);
        for(j = 0; j < 16; j++) X[j] = 0;
    }
    j = 14;

    for(i = 0; i < 64; i++) {
        X[j+(i/32)] |= ((len&1)<<(i%32)); len>>=1;
    }

    hash_internal(X, H);

    /*
#ifdef BIG_ENDIAN
    for(i = 0; i < RIPEMD160_IVSIZE; i++)
        for(j = 0; j < 4; j++)
            out[(i*sizeof(u_int32_t))+j] = ((u_int8_t*)&H[i])[3-j];
#else
    memcpy(out, H, RIPEMD160_IVSIZE*sizeof(u_int32_t));
    #endif */

    memcpy(out, H, RIPEMD160_IVSIZE*sizeof(u_int32_t));

    return;
}

void hash_internal(u_int32_t *X, u_int32_t *H)
{
    u_int32_t A[2], B[2], C[2], D[2], E[2];
    u_int32_t j, t, side;
    
    /* left side */
    side = 0;
        
    A[side] = H[0]; B[side] = H[1]; C[side] = H[2]; D[side] = H[3];
    E[side] = H[4];
            
    /* round one */
    for(j = 0; j < 16; j++) {
        t = A[side]+(B[side]^C[side]^D[side])+X[ripemd160_z[side][j]]+
            ripemd160_y[side][0];
        A[side] = E[side]; E[side] = D[side];
        D[side] = RIPEMD160_ROLL(C[side], 10);
        C[side] = B[side];
        B[side] = A[side]+RIPEMD160_ROLL(t, ripemd160_s[side][j]);
    }
    /* round two */
    for(j = 16; j < 32; j++) {
        t = A[side]+((B[side]&C[side])|(~B[side]&D[side]))+
            X[ripemd160_z[side][j]]+ripemd160_y[side][1];
        A[side] = E[side]; E[side] = D[side];
        D[side] = RIPEMD160_ROLL(C[side], 10);
        C[side] = B[side];
        B[side] = A[side]+RIPEMD160_ROLL(t, ripemd160_s[side][j]);
    }
    /* round three */
    for(j = 32; j < 48; j++) {
        t = A[side]+((B[side]|~C[side])^D[side])+X[ripemd160_z[side][j]]+
            ripemd160_y[side][2];
        A[side] = E[side]; E[side] = D[side];
        D[side] = RIPEMD160_ROLL(C[side], 10);
        C[side] = B[side];
        B[side] = A[side]+RIPEMD160_ROLL(t, ripemd160_s[side][j]);
    }
    /* round four */
    for(j = 48; j < 64; j++) {
        t = A[side]+((B[side]&D[side])|(C[side]&~D[side]))+
            X[ripemd160_z[side][j]]+ripemd160_y[side][3];
        A[side] = E[side]; E[side] = D[side];
        D[side] = RIPEMD160_ROLL(C[side], 10);
        C[side] = B[side];
        B[side] = A[side]+RIPEMD160_ROLL(t, ripemd160_s[side][j]);
    }
    /* round five */
    for(j = 64; j < 80; j++) {
        t = A[side]+(B[side]^(C[side]|~D[side]))+
            X[ripemd160_z[side][j]]+ripemd160_y[side][4];
        A[side] = E[side]; E[side] = D[side];
        D[side] = RIPEMD160_ROLL(C[side], 10);
        C[side] = B[side];
        B[side] = A[side]+RIPEMD160_ROLL(t, ripemd160_s[side][j]);
    }
    
    /* right side */
    side = 1;
    
    A[side] = H[0]; B[side] = H[1]; C[side] = H[2]; D[side] = H[3];
    E[side] = H[4];
            
    /* round one */
    for(j = 0; j < 16; j++) {
        t = A[side]+(B[side]^(C[side]|~D[side]))+
            X[ripemd160_z[side][j]]+ripemd160_y[side][0];
        A[side] = E[side]; E[side] = D[side];
        D[side] = RIPEMD160_ROLL(C[side], 10);
        C[side] = B[side];
        B[side] = A[side]+RIPEMD160_ROLL(t, ripemd160_s[side][j]);
    }
    /* round two */
    for(j = 16; j < 32; j++) {
        t = A[side]+((B[side]&D[side])|(C[side]&~D[side]))+
            X[ripemd160_z[side][j]]+ripemd160_y[side][1];
        A[side] = E[side]; E[side] = D[side];
        D[side] = RIPEMD160_ROLL(C[side], 10);
        C[side] = B[side];
        B[side] = A[side]+RIPEMD160_ROLL(t, ripemd160_s[side][j]);
    }
    /* round three */
    for(j = 32; j < 48; j++) {
        t = A[side]+((B[side]|~C[side])^D[side])+X[ripemd160_z[side][j]]+
            ripemd160_y[side][2];
        A[side] = E[side]; E[side] = D[side];
        D[side] = RIPEMD160_ROLL(C[side], 10);
        C[side] = B[side];
        B[side] = A[side]+RIPEMD160_ROLL(t, ripemd160_s[side][j]);
    }
    /* round four */
    for(j = 48; j < 64; j++) {
        t = A[side]+((B[side]&C[side])|(~B[side]&D[side]))+
            X[ripemd160_z[side][j]]+ripemd160_y[side][3];
        A[side] = E[side]; E[side] = D[side];
        D[side] = RIPEMD160_ROLL(C[side], 10);
        C[side] = B[side];
        B[side] = A[side]+RIPEMD160_ROLL(t, ripemd160_s[side][j]);
    }
    /* round five */
    for(j = 64; j < 80; j++) {
        t = A[side]+(B[side]^C[side]^D[side])+X[ripemd160_z[side][j]]+
            ripemd160_y[side][4];
        A[side] = E[side]; E[side] = D[side];
        D[side] = RIPEMD160_ROLL(C[side], 10);
        C[side] = B[side];
        B[side] = A[side]+RIPEMD160_ROLL(t, ripemd160_s[side][j]);
    }
    
    /* update chaining values */
    t = H[0];
    H[0] = H[1]+C[0]+D[1];
    H[1] = H[2]+D[0]+E[1];
    H[2] = H[3]+E[0]+A[1];
    H[3] = H[4]+A[0]+B[1];
    H[4] = t   +B[0]+C[1];

    return;
}

/* EOF ripemd160.c */
