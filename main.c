/* 
 * main.c
 * Created: Tue Jan 25 14:02:14 2000 by tek@wiw.org
 * Revised: Tue Jan 25 14:02:14 2000 (pending)
 * Copyright 2000 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include "steaghan.h"

extern void inject(moduleinfo_t prpg, moduleinfo_t wrapper, u_int8_t *secdata,
                   u_int32_t seclen);
extern u_int8_t *extract(moduleinfo_t prpg, moduleinfo_t wrapper,
                         u_int32_t *seclen);
extern int loadmod(moduleinfo_t *mip, char *modpath);

/* bomb callback */
void bomb(void) { exit(1); }

void usage(void) {
    printf("steaghan <prpg> <hash> <wrapper> <key> <wrapper> <secret> <mode>\n");
    printf("  where mode is i or e (``i''nject or ``e''xtract).\n");
    return;
}

int main(int argc, char **argv)
{
    moduleinfo_t prpg, hash, wrapper;
    hashfunc_t hashfunc;
    u_int32_t hashlen, wraplen, seclen, keylen;
    char *wrapper_filename, *secret_filename, mode;
    u_int8_t *secdata, *key;
    FILE *fp;

    if(argc < 8) {
        usage();
        exit(EXIT_FAILURE);
    }

    loadmod(&prpg, argv[1]);
    loadmod(&hash, argv[2]);
    loadmod(&wrapper, argv[3]);
    key = (unsigned char *)argv[4]; /* FIXME */
    keylen = strlen(argv[4]);
    wrapper_filename = argv[5];
    secret_filename = argv[6];
    mode = argv[7][0];

    hashlen = (*(hashlenfunc_t)dlsym(hash.dlhandle, "hashlen"))();
    hashfunc = (hashfunc_t)dlsym(hash.dlhandle, "hash");

    wrapper.handle = (*(wrapinitfunc_t)dlsym(wrapper.dlhandle, "wrapinit"))(wrapper_filename);
    if(wrapper.handle == NULL) {
        fprintf(stderr, "failed to setup the wrapper handle. (Is the file really the right type?)\n");
        exit(EXIT_FAILURE);
    }
    wraplen = (*(wraplenfunc_t)dlsym(wrapper.dlhandle, "wraplen"))(wrapper.handle);
    prpg.handle = (*(permuinitfunc_t)dlsym(prpg.dlhandle, "permuinit"))(wraplen, key, keylen, hashfunc, hashlen);

    if(mode == 'i') {
        printf("injecting...\n");
        
        fp = fopen(secret_filename, "r");
        fseek(fp, 0, SEEK_END);
        seclen = ftell(fp)*8;
        rewind(fp);
        
        secdata = (u_int8_t *)malloc(seclen/8);
        assert(secdata != NULL);
        
        fread(secdata, 1, seclen/8, fp);
        fclose(fp);

        inject(prpg, wrapper, secdata, seclen);
    } else {
        printf("extracting...\n");
        
        secdata = extract(prpg, wrapper, &seclen);

        fp = fopen(secret_filename, "w");
        fwrite(secdata, 1, seclen/8, fp);
        fclose(fp);
    }

    free(secdata);
    
    (*(wrapclosefunc_t)dlsym(wrapper.dlhandle, "wrapclose"))(wrapper.handle); 
    (*(permuclosefunc_t)dlsym(prpg.dlhandle, "permuclose"))(prpg.handle);
    
    return EXIT_SUCCESS;
}

/* EOF main.c */
