/* 
 * hash.c
 * Created: Wed Mar  8 14:14:43 2000 by tek@wiw.org
 * Revised: Thu Mar  9 06:53:21 2000 by tek@wiw.org
 * Copyright 2000 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/mman.h>

#include "steaghan.h"

void usage(void)
{
    fprintf(stderr, "usage: hash -h <hash module>");
#ifdef HAVE_DLSYM
    fprintf(stderr, " [-d <top dir>] [file]\n");
#else
    fprintf(stderr, " [file]\n");
    fprintf(stderr, "       hash -l\n");
#endif
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    moduleinfo_t hash;
    hashfunc_t hashfunc;
    u_int8_t *mogo, *file;
    int i, j, fd, length, hashlen;
    char *hashname, *filename;

    hashname = filename = NULL;
    
    for(i = 1; i < argc; i++) {
        if(argv[i][0] == '-') {
            if(argv[i][1] == 'h') {
                if(i+1 < argc) hashname = argv[++i];
                else usage();
#ifdef HAVE_DLSYM
            } else if(argv[i][1] == 'd') {
                if(i+1 < argc) topdir = argv[++i];
                else usage();
#else
            } else if(argv[i][1] == 'l') {
                listmods();
                exit(EXIT_SUCCESS);
#endif
            }
        } else if(filename == NULL)
            filename = argv[i];
        else usage();
    }

    if(hashname == NULL)
        usage();

    if(loadmod(&hash, hashname)) {
        fprintf(stderr, "Unable to load module %s\n", hashname);
        exit(EXIT_FAILURE);
    }
    if(hash.moduletype != hashmod) {
        fprintf(stderr, "%s is not a hash module!\n", hashname);
        exit(EXIT_FAILURE);
    }
    hashlen=(*(hashlenfunc_t)getsym(&hash, "hashlen"))();
    mogo = (u_int8_t *)malloc(hashlen/8);
    if(mogo == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }

    hashfunc = (hashfunc_t)getsym(&hash, "hash");

    if(filename == NULL || !strcmp(filename, "-")) {
        file = NULL;
        length = 0;

        while(!feof(stdin)) {
            file = (u_int8_t *)realloc(file, length+4096);
            length += fread(file+length, 1, 4096, stdin);
        }
    } else {
        fd = open(filename, O_RDONLY);
        if(fd == -1) {
            fprintf(stderr, "%s: %s", filename, strerror(errno));
            exit(EXIT_FAILURE);
        }
        
        length = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);
        
        file = (u_int8_t *)mmap(0, length, PROT_READ, MAP_PRIVATE, fd, 0);
        if((void *)file == (void *)-1) {
            fprintf(stderr, "%s: %s", filename, strerror(errno));
            exit(EXIT_FAILURE);
        }
    
        memset(mogo, 0, hashlen/8);
        (*hashfunc)(file, length, mogo);

        for(j = 0; j < hashlen/8; j++) {
            printf("%02x", mogo[j]);
        }
        printf("  %s\n", filename);

        if(filename && strcmp(filename, "-")) {
            munmap(file, length);
        } else {
            free(file);
        }
    }

    free(mogo);

    exit(EXIT_SUCCESS);
}

/* EOF hash.c */
