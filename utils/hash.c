/* 
 * hash.c
 * Created: Wed Mar  8 14:14:43 2000 by tek@wiw.org
 * Revised: Wed Mar  8 15:05:03 2000 by tek@wiw.org
 * Copyright 2000 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/mman.h>

#include "steaghanmods.h"


int main(int argc, char **argv)
{
    moduleinfo_t hash;
    hashfunc_t hashfunc;
    u_int8_t *mogo, *file;
    int i, j, fd, length, hashlen;

    if(argc < 2) {
        fprintf(stderr, "usage: hash <hash module> [files..]\n");
        exit(EXIT_FAILURE);
    }
    
    loadmod(&hash, argv[1]);
    hashlen=(*(hashlenfunc_t)dlsym(hash.dlhandle, "hashlen"))();
    mogo = (u_int8_t *)malloc(hashlen/8);
    if(mogo == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }

    hashfunc = (hashfunc_t)dlsym(hash.dlhandle, "hash");

    for(i = 2; i < argc; i++) {
        fd = open(argv[i], O_RDONLY);
        if(fd == -1) {
            fprintf(stderr, "%s: %s", argv[i], strerror(errno));
            exit(EXIT_FAILURE);
        }
        
        length = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);

        file = (u_int8_t *)mmap(0, length, PROT_READ, MAP_PRIVATE, fd, 0);
        if((void *)file == (void *)-1) {
            exit(EXIT_FAILURE);
        }
    
        memset(mogo, 0, hashlen/8);
        (*hashfunc)(file, length, mogo);

        for(j = 0; j < hashlen/8; j++) {
            printf("%02x", mogo[j]);
        }
        printf("  %s\n", argv[i]);
    }

    free(mogo);

    exit(EXIT_SUCCESS);
}

/* EOF hash.c */
