/* 
 * steaghanmods.h
 * Created: Tue Nov 23 06:08:49 1999 by tek@wiw.org
 * Revised: Mon Apr 17 12:30:40 2000 by tek@wiw.org
 * Copyright 1999 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#ifndef STEAGHANMODS_H
#define STEAGHANMODS_H

#include <sys/types.h>

typedef struct {
    const char *name;
    void *f;
} modulefunc_t;

typedef struct {
    int nfuncs;
    modulefunc_t *funcs;
} modulefunctable_t;

typedef struct {
    const char *name, *description;
    enum { prpgmod, hashmod, wrappermod, filemod, ciphermod } moduletype;
    void *handle;
    void *dlhandle;
} moduleinfo_t;

typedef enum { CUR, SET, END } filewhence_t;

typedef struct {
    char *filename;
    void *handle;
    void (*read)(void *, u_int32_t, u_int32_t, void *);
    void (*write)(void *, u_int32_t, u_int32_t, void *);
    void (*seek)(void *, int32_t, filewhence_t);
    u_int32_t (*tell)(void *);
} file_t;

/* module fu (mods.*.c) */
extern int loadmod(moduleinfo_t *mip, char *modpath);
extern void describemod(moduleinfo_t *mip);
extern void closemod(moduleinfo_t *mip);
extern void *getsym(moduleinfo_t *mip, char *sym);
#ifdef HAVE_DLSYM
extern char *topdir;
#else
void listmods(void);
#endif

/* all modules */
typedef moduleinfo_t (*moduleinfofunc_t)(void);
typedef modulefunctable_t *(*modulefunctablefunc_t)(void);
/* hash modules */
typedef u_int32_t (*hashlenfunc_t)(void);
typedef u_int8_t *(*hashfunc_t)(u_int8_t *, u_int32_t, u_int8_t *);
/* prpg modules */
typedef void *(*permuinitfunc_t)(u_int32_t, u_int8_t *, u_int32_t,
                                 moduleinfo_t);
typedef u_int32_t (*permugenfunc_t)(void *);
typedef void (*permuclosefunc_t)(void *);
/* wrapper modules */
typedef void *(*wrapinitfunc_t)(file_t *);
typedef u_int32_t (*wraplenfunc_t)(void *);
typedef u_int8_t (*wrapreadfunc_t)(void *, u_int32_t);
typedef void (*wrapwritefunc_t)(void *, u_int32_t, u_int8_t);
typedef void (*wrapclosefunc_t)(void *);
typedef u_int32_t (*wrapgetimmobilelenfunc_t)(void *);
typedef void (*wrapgetimmobilefunc_t)(void *, u_int8_t *);
/* file modules */
typedef void *(*fileinitfunc_t)(char *);
typedef void (*fileclosefunc_t)(file_t *);
/* cipher modules */
typedef u_int32_t (*cipherkeylenfunc_t)(void);
typedef u_int32_t (*cipherivlenfunc_t)(void);
typedef u_int32_t (*cipherblocklenfunc_t)(void);
typedef void (*cipherphrasetokeyfunc_t)(char *, u_int8_t *, moduleinfo_t);
typedef void *(*cipherinitfunc_t)(u_int8_t *, u_int8_t *);
typedef void (*encipherfunc_t)(void *, u_int8_t *, u_int8_t *, u_int32_t);
typedef void (*decipherfunc_t)(void *, u_int8_t *, u_int8_t *, u_int32_t);
typedef void (*cipherclosefunc_t)(void *);

#endif /* STEAGHANMODS_H */

/* EOF steaghanmods.h */
