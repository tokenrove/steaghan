/* 
 * mods.c
 * Created: Tue Jan 25 12:43:43 2000 by tek@wiw.org
 * Revised: Mon Mar 20 10:18:12 2000 by tek@wiw.org
 * Copyright 2000 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "steaghanmods.h"

#include "mods.generic.h"

int loadmod(moduleinfo_t *mip, char *modpath);
void closemod(moduleinfo_t *mip);
void describemod(moduleinfo_t *mip);
void *getsym(moduleinfo_t *mip, char *sym);
void listmods(void);

int loadmod(moduleinfo_t *mip, char *modpath)
{
    modulefunctable_t *mft;
    int i, j;

    for(i = 0; modsgeneric_mfts[i] != NULL; i++) {
        mft = (*modsgeneric_mfts[i])();

        for(j = 0; j < mft->nfuncs; j++) {
            if(!strcmp(mft->funcs[j].name, "moduleinfo")) {
                *mip = (*(moduleinfofunc_t)mft->funcs[j].f)();
                break;
            }
        }
        if(j == mft->nfuncs) {
            free(mft);
            continue;
        }

        if(!strcmp(mip->name, modpath)) {
            mip->dlhandle = (void *)mft;
            break;
        }
    }

    if(mip->dlhandle != (void *)mft) return 1;

    return 0;
}

void closemod(moduleinfo_t *mip)
{
    modulefunctable_t *mft = (modulefunctable_t *)mip->dlhandle;
    free(mft->funcs);
    free(mft);
    return;
}

void describemod(moduleinfo_t *mip)
{
    fprintf(stderr, "steaghan: Module ``%s'' [%s], is ", mip->name,
            mip->description);
    if(mip->moduletype == prpgmod) fprintf(stderr, "a PRPG");
    else if(mip->moduletype == hashmod) fprintf(stderr, "a hash");
    else if(mip->moduletype == wrappermod) fprintf(stderr, "a wrapper");
    else if(mip->moduletype == filemod) fprintf(stderr, "a file");
    else if(mip->moduletype == ciphermod) fprintf(stderr, "a cipher");
    else fprintf(stderr, "an unknown");
    fprintf(stderr, " module.\n");
    return;
}

void *getsym(moduleinfo_t *mip, char *sym)
{
    modulefunctable_t *mft = (modulefunctable_t *)mip->dlhandle;
    int i;

    for(i = 0; i < mft->nfuncs; i++) {
        if(!strcmp(mft->funcs[i].name, sym))
            return mft->funcs[i].f;
    }
    return NULL;
}

void listmods(void)
{
    modulefunctable_t *mft;
    moduleinfo_t mip; 
    int i, j;

    for(i = 0; modsgeneric_mfts[i] != NULL; i++) {
        mft = (*modsgeneric_mfts[i])();

        for(j = 0; j < mft->nfuncs; j++) {
            if(!strcmp(mft->funcs[j].name, "moduleinfo")) {
                mip = (*(moduleinfofunc_t)mft->funcs[j].f)();
                break;
            }
        }
        if(j == mft->nfuncs) {
            free(mft);
            continue;
        }

        fprintf(stderr, "%s - %s\n", mip.name, mip.description);
        free(mft);
    }

    return;
}

/* EOF mods.c */
