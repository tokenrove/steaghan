/* 
 * steaghan.h
 * Created: Tue Jan 25 13:32:40 2000 by tek@wiw.org
 * Revised: Sat Mar 25 22:10:23 2000 by tek@wiw.org
 * Copyright 2000 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#ifndef STEAGHAN_H
#define STEAGHAN_H

#include "steaghanmods.h"

typedef struct {
    u_int8_t *key, *cipherkey, *cipheriv;
    u_int32_t keylen, cipherkeylen, cipherivlen, cipherblocklen;
    char *key_filename, *wrapper_filename, *secret_filename;
    char *prpg_modname, *hash_modname, *wrapper_modname, *file_modname,
        *cipher_modname;
    moduleinfo_t prpg, hash, wrapper, filemod, cipher;
    file_t *file;
    char mode, hashimmobile;
} steaghanconf_t;

extern void inject(moduleinfo_t prpg, moduleinfo_t wrapper, u_int8_t *secdata,
                   u_int32_t seclen);
extern u_int8_t *extract(moduleinfo_t prpg, moduleinfo_t wrapper,
                         u_int32_t *seclen);

/* system specific helpers (system.c) */
extern void getpassphrase(char **passphrase);
extern void randominit(void);
extern u_int8_t randombyte(void);

/* statusbar.c (really a progress meter...) */
extern void statusbar_init(int total_);
extern void statusbar_update(int increment);
extern void statusbar_close(void);

/* pkcs5pad.c */
extern void pkcs5pad(u_int8_t *in, u_int32_t inlen, u_int8_t *out,
                     u_int32_t *outlen, u_int8_t blocklen);
extern int pkcs5unpad(u_int8_t *in, u_int32_t inlen, u_int8_t *out,
                      u_int32_t *outlen, u_int8_t blocklen);


#endif

/* EOF steaghan.h */
