/* 
 * main.c
 * Created: Tue Jan 25 14:02:14 2000 by tek@wiw.org
 * Revised: Wed Mar  8 15:10:41 2000 by tek@wiw.org
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

/* bomb callback */
void bomb(void) { exit(1); }

void usage(void) {
    printf("steaghan <prpg-mod> <hash-mod> <wrapper-mod> <file-mod> <key> <wrapper> <secret> <mode>\n");
    printf("  where mode is i or e (``i''nject or ``e''xtract).\n");
    return;
}

int main(int argc, char **argv)
{
    moduleinfo_t prpg, hash, wrapper, filemod;
    hashfunc_t hashfunc;
    u_int32_t hashlen, wraplen, seclen, keylen;
    char *wrapper_filename, *secret_filename, mode;
    file_t *file;
    u_int8_t *secdata, *key;
    FILE *fp;

    if(argc < 9) {
        usage();
        exit(EXIT_FAILURE);
    }

    loadmod(&prpg, argv[1]);
    describemod(&prpg);
    loadmod(&hash, argv[2]);
    describemod(&hash);
    loadmod(&wrapper, argv[3]);
    describemod(&wrapper);
    loadmod(&filemod, argv[4]);
    describemod(&filemod);
    key = (unsigned char *)argv[5]; /* FIXME */
    keylen = strlen(argv[5]);
    wrapper_filename = argv[6];
    secret_filename = argv[7];
    mode = argv[8][0];

    file = (*(fileinitfunc_t)dlsym(filemod.dlhandle,
                                   "fileinit"))(wrapper_filename);
    if(file == NULL) {
        fprintf(stderr, "failed to open the specified wrapper file.\n");
        exit(EXIT_FAILURE);
    }

    hashlen = (*(hashlenfunc_t)dlsym(hash.dlhandle, "hashlen"))();
    hashfunc = (hashfunc_t)dlsym(hash.dlhandle, "hash");

    wrapper.handle = (*(wrapinitfunc_t)dlsym(wrapper.dlhandle,
                                             "wrapinit"))(file);
    if(wrapper.handle == NULL) {
        fprintf(stderr, "failed to setup the wrapper handle. (Is the file");
        fprintf(stderr, " really the right type?)\n");
        exit(EXIT_FAILURE);
    }
    wraplen = (*(wraplenfunc_t)dlsym(wrapper.dlhandle,
                                     "wraplen"))(wrapper.handle);
    prpg.handle = (*(permuinitfunc_t)dlsym(prpg.dlhandle,
                                           "permuinit"))(wraplen, key,
                                                         keylen, hashfunc,
                                                         hashlen);

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
    (*(fileclosefunc_t)dlsym(filemod.dlhandle, "fileclose"))(file);

    return EXIT_SUCCESS;
}

/* EOF main.c */
