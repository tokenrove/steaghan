/* 
 * steaghanmods.h
 * Created: Tue Nov 23 06:08:49 1999 by tek@wiw.org
 * Revised: Wed Nov 24 02:05:47 1999 by tek@wiw.org
 * Copyright 1999 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#ifndef STEAGHANMODS_H
#define STEAGHANMODS_H

#include <sys/types.h>

typedef struct {
    const char *name, *description;
    enum { prpgmod, hashmod, wrappermod } moduletype;
    void *dlhandle;
    void *handle;
} moduleinfo_t;

/* all modules */
typedef moduleinfo_t (*moduleinfofunc_t)(void);
/* hash modules */
typedef u_int32_t (*hashlenfunc_t)(void);
typedef u_int8_t *(*hashfunc_t)(u_int8_t *, u_int32_t, u_int8_t *);
/* prpg modules */
typedef void *(*permuinitfunc_t)(u_int32_t, u_int8_t *, u_int32_t,
                                 hashfunc_t, u_int32_t);
typedef u_int32_t (*permugenfunc_t)(void *);
typedef void (*permuclosefunc_t)(void *);
/* wrapper modules */
typedef void *(*wrapinitfunc_t)(char *filename);
typedef u_int32_t (*wraplenfunc_t)(void *p_);
typedef u_int8_t (*wrapreadfunc_t)(void *p_, u_int32_t pos);
typedef void (*wrapwritefunc_t)(void *p_, u_int32_t pos, u_int8_t value);
typedef void (*wrapclosefunc_t)(void *p_);

#endif /* STEAGHANMODS_H */

/* EOF steaghanmods.h */
