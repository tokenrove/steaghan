/* 
 * rc5.c
 * Created: Thu Mar  9 02:41:39 2000 by tek@wiw.org
 * Revised: Sat Mar 25 20:39:11 2000 by tek@wiw.org
 * Copyright 2000 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

#include "steaghanmods.h"

#define RC5_MODULENAME "rc5"
#define RC5_MODULEDESC "RC5/16/12/4 block cipher in CBC mode"

#define RC5_KEYLEN  16
#define RC5_NROUNDS 12
#define RC5_WORDLEN 4

#define RC5_MAGIC_PW 0xB7E15163L
#define RC5_MAGIC_QW 0x9E3779B9L

#define RC5_ROLL(a, s)  (((a) << (s)%32)|((a) >> (32-(s)%32)))
#define RC5_ROLLR(a, s) (((a) >> (s)%32)|((a) << (32-(s)%32)))

typedef struct {
    u_int32_t *K;
    u_int8_t last[RC5_WORDLEN*2];
} cipherhandle_t;

/* internal functions */
void encipher_internal(u_int32_t *K, u_int32_t *A, u_int32_t *B);
void decipher_internal(u_int32_t *K, u_int32_t *A, u_int32_t *B);

moduleinfo_t moduleinfo(void)
{
    moduleinfo_t mi = { RC5_MODULENAME, RC5_MODULEDESC, ciphermod, 0 };
    return mi;
}

u_int32_t cipherkeylen(void)
{
    return RC5_KEYLEN;
}

u_int32_t cipherivlen(void)
{
    return RC5_WORDLEN*2;
}

u_int32_t cipherblocklen(void)
{
    return RC5_WORDLEN*2;
}

void cipherphrasetokey(char *phrase, u_int8_t *key, moduleinfo_t hash)
{
    hashfunc_t hashfunc = (hashfunc_t)dlsym(hash.dlhandle, "hash");
    u_int32_t hashlen = (*(hashlenfunc_t)dlsym(hash.dlhandle, "hashlen"))(),
        iterlen = 0;
    u_int8_t *buffer;

    buffer = (u_int8_t *)malloc(hashlen/8);

    (*hashfunc)((u_int8_t *)phrase, strlen(phrase), buffer);
    if(hashlen/8 <= RC5_KEYLEN) {
        memmove(key, buffer, hashlen/8);
        iterlen += hashlen/8;
        while(iterlen < RC5_KEYLEN) {
            (*hashfunc)(buffer, hashlen/8, buffer);
            memmove(key+iterlen, buffer, hashlen/8);
            iterlen += hashlen/8;
        }
        (*hashfunc)(buffer, hashlen/8, buffer);
        memmove(key+iterlen-hashlen/8, buffer, iterlen%RC5_KEYLEN);

    } else {
        memmove(key, buffer, RC5_KEYLEN);
    }
    free(buffer);
    
    return;
}

void *cipherinit(u_int8_t *key, u_int8_t *iv)
{
    cipherhandle_t *p;
    u_int32_t i, j, c, s, t, A, B, L[(RC5_WORDLEN+RC5_KEYLEN-1)/RC5_WORDLEN];

    p = (cipherhandle_t *)malloc(sizeof(cipherhandle_t));
    if(p == NULL) return NULL;

    p->K = (u_int32_t *)malloc((2*RC5_NROUNDS+2)*sizeof(u_int32_t));
    if(p->K == NULL) return NULL;

    memcpy(p->last, iv, RC5_WORDLEN*2);

    c = (RC5_KEYLEN+RC5_WORDLEN-1)/RC5_WORDLEN;
    for(i = 0; i < c; i++) {
        L[i] = 0;
        for(j = 0; j < RC5_WORDLEN; j++) {
            L[i] |= key[i*RC5_WORDLEN+j]<<(8*j);
        }
    }

    p->K[0] = RC5_MAGIC_PW;
    for(i = 1; i < 2*RC5_NROUNDS+2; i++) {
        p->K[i] = p->K[i-1] + RC5_MAGIC_QW;
    }

    i = j = A = B = 0;
    t = (c > (2*RC5_NROUNDS+2)) ? c : (2*RC5_NROUNDS+2);
    for(s = 0; s < 3*t; s++) {
        p->K[i] = RC5_ROLL(p->K[i]+A+B, 3); A = p->K[i];
        i = (i+1)%(2*RC5_NROUNDS+2);
        L[j] = RC5_ROLL(L[j]+A+B, (A+B)); B = L[j];
        j = (j+1)%c;
    }
    
    return (void *)p;
}

void encipher(void *p_, u_int8_t *in, u_int8_t *out, u_int32_t len)
{
    cipherhandle_t *p = (cipherhandle_t *)p_;
#ifndef LITTLE_ENDIAN
    u_int32_t A, B;
#endif
    int i;

    for(i = 0; len >= (2*RC5_WORDLEN); len -= (2*RC5_WORDLEN), i++) {
#ifdef LITTLE_ENDIAN
        ((u_int32_t *)out)[2*i] =
            ((u_int32_t *)in)[2*i]^((u_int32_t *)p->last)[0];
        ((u_int32_t *)out)[2*i+1] =
            ((u_int32_t *)in)[2*i+1]^((u_int32_t *)p->last)[1];
        encipher_internal(p->K, (u_int32_t *)(out+2*i*RC5_WORDLEN),
                          (u_int32_t *)(out+(2*i+1)*RC5_WORDLEN));
#else
        A=in[8*i]^p->last[0];A|=(in[8*i+1]^p->last[1])<<8;
        A|=(in[8*i+2]^p->last[2])<<16;A|=(in[8*i+3]^p->last[3])<<24;
        B=in[8*i+4]^p->last[4];B|=(in[8*i+5]^p->last[5])<<8;
        B|=(in[8*i+6]^p->last[6])<<16;B|=(in[8*i+7]^p->last[7])<<24;
        encipher_internal(p->K, &A, &B);
        out[8*i]=A;out[8*i+1]=A>>8;out[8*i+2]=A>>16;out[8*i+3]=A>>24;
        out[8*i+4]=B;out[8*i+5]=B>>8;out[8*i+6]=B>>16;out[8*i+7]=B>>24;
#endif
        memcpy(p->last, out+(2*i*RC5_WORDLEN), 2*RC5_WORDLEN);
    }
    return;
}

void decipher(void *p_, u_int8_t *in, u_int8_t *out, u_int32_t len)
{
    cipherhandle_t *p = (cipherhandle_t *)p_;
#ifndef LITTLE_ENDIAN
    u_int32_t A, B;
#endif
    int i;
    u_int8_t buf[RC5_WORDLEN*2];

    for(i = 0; len >= (2*RC5_WORDLEN); len -= (2*RC5_WORDLEN), i++) {
#ifdef LITTLE_ENDIAN
        ((u_int32_t *)out)[2*i] = ((u_int32_t *)in)[2*i];
        ((u_int32_t *)out)[2*i+1] = ((u_int32_t *)in)[2*i+1];
        memcpy(buf, p->last, 2*RC5_WORDLEN);
        memcpy(p->last, out+(2*i*RC5_WORDLEN), 2*RC5_WORDLEN);
        decipher_internal(p->K, (u_int32_t *)(out+2*i*RC5_WORDLEN),
                          (u_int32_t *)(out+(2*i+1)*RC5_WORDLEN));
        ((u_int32_t *)out)[2*i] ^= ((u_int32_t *)buf)[0];
        ((u_int32_t *)out)[2*i+1] ^= ((u_int32_t *)buf)[1];
#else
        A=in[8*i];A|=in[8*i+1]<<8;A|=in[8*i+2]<<16;A|=in[8*i+3]<<24;
        B=in[8*i+4];B|=in[8*i+5]<<8;B|=in[8*i+6]<<16;B|=in[8*i+7]<<24;
        decipher_internal(p->K, &A, &B);
        out[8*i]=A^buf[0];out[8*i+1]=(A>>8)^buf[1];
        out[8*i+2]=(A>>16)^buf[2];out[8*i+3]=(A>>24)^buf[3];
        out[8*i+4]=B^buf[4];out[8*i+5]=(B>>8)^buf[5];
        out[8*i+6]=(B>>16)^buf[6];out[8*i+7]=(B>>24)^buf[7];
#endif
    }
    return;
}

void cipherclose(void *p_)
{
    cipherhandle_t *p = (cipherhandle_t *)p_;

    memset(p->K, 0, (2*RC5_NROUNDS+2)*sizeof(u_int32_t));
    free(p->K);
    free(p);
    
    return;
}

void encipher_internal(u_int32_t *K, u_int32_t *A, u_int32_t *B)
{
    int i;
    *A += K[0]; *B += K[1];
    for(i = 1; i <= RC5_NROUNDS; i++) {
        *A = RC5_ROLL(*A^*B, *B)+K[2*i]; *B = RC5_ROLL(*B^*A, *A)+K[2*i+1];
    }
}

void decipher_internal(u_int32_t *K, u_int32_t *A, u_int32_t *B)
{
    int i;
    for(i = RC5_NROUNDS; i >= 1; i--) {
        *B = RC5_ROLLR(*B-K[2*i+1], *A)^*A; *A = RC5_ROLLR(*A-K[2*i], *B)^*B;
    }
    *B -= K[1]; *A -= K[0];
}

/* EOF rc5.c */
