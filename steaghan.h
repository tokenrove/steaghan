/* 
 * steaghan.h
 * Created: Tue Jan 25 13:32:40 2000 by tek@wiw.org
 * Revised: Wed Mar  8 15:56:03 2000 by tek@wiw.org
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
    char mode;
} steaghanconf_t;

extern void inject(moduleinfo_t prpg, moduleinfo_t wrapper, u_int8_t *secdata,
                   u_int32_t seclen);
extern u_int8_t *extract(moduleinfo_t prpg, moduleinfo_t wrapper,
                         u_int32_t *seclen);

extern int loadmod(moduleinfo_t *mip, char *modpath);
extern void describemod(moduleinfo_t *mip);

extern void statusbar_init(int total_);
extern void statusbar_update(int increment);
extern void statusbar_close(void);

extern void pkcs5pad(u_int8_t *in, u_int32_t inlen, u_int8_t *out,
                     u_int32_t *outlen, u_int8_t blocklen);
extern int pkcs5unpad(u_int8_t *in, u_int32_t inlen, u_int8_t *out,
                      u_int32_t *outlen, u_int8_t blocklen);


#endif

/* EOF steaghan.h */
