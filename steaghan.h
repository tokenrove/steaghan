/* 
 * steaghan.h
 * Created: Tue Jan 25 13:32:40 2000 by tek@wiw.org
 * Revised: Tue Jan 25 13:32:40 2000 (pending)
 * Copyright 2000 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#ifndef STEAGHAN_H
#define STEAGHAN_H

#include "steaghanmods.h"

typedef struct {
    u_int8_t *key;
    u_int32_t keylen;
    char *wrapper_filename;
    char *secret_filename;
    moduleinfo_t prpg, hash, wrapper;
} steagconf_t;

#endif

/* EOF steaghan.h */
