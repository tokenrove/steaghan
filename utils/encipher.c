/* 
 * encipher.c
 * Created: Thu Mar  9 05:09:29 2000 by tek@wiw.org
 * Revised: Thu Mar  9 07:23:13 2000 by tek@wiw.org
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

#include "steaghan.h"


int main(int argc, char **argv)
{
    moduleinfo_t cipher;
    encipherfunc_t encipher;
    u_int8_t *file, *key, *iv;
    int fd, length;
    u_int32_t keylen, ivlen, blocklen;

    if(argc != 3 && argc != 4) {
        fprintf(stderr, "usage: encipher <cipher module> <key> [file]\n");
        exit(EXIT_FAILURE);
    }

    if(loadmod(&cipher, argv[1])) {
        fprintf(stderr, "Unable to load module %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    if(cipher.moduletype != ciphermod) {
        fprintf(stderr, "%s is not a cipher module!\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    keylen=(*(cipherkeylenfunc_t)dlsym(cipher.dlhandle, "cipherkeylen"))();
    key = (u_int8_t *)malloc(keylen);
    if(key == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }
    ivlen=(*(cipherivlenfunc_t)dlsym(cipher.dlhandle, "cipherivlen"))();
    iv = NULL;
    if(ivlen != 0) {
        fprintf(stderr, "IVs not supported\n");
        exit(EXIT_FAILURE);
    }
    blocklen=(*(cipherblocklenfunc_t)dlsym(cipher.dlhandle,
                                           "cipherblocklen"))();

    memcpy(key, argv[2], keylen);

    encipher = (encipherfunc_t)dlsym(cipher.dlhandle, "encipher");

    cipher.handle = (*(cipherinitfunc_t)dlsym(cipher.dlhandle,
                                              "cipherinit"))(key, iv);
    if(cipher.handle == NULL) {
        fprintf(stderr, "Failed to initialize cipher. (maybe your key is ");
        fprintf(stderr, "weak?)\n");
        exit(EXIT_FAILURE);
    }

    if(argc == 4 && strcmp(argv[3], "-")) {
        fd = open(argv[3], O_RDWR);
        if(fd == -1) {
            fprintf(stderr, "%s: %s", argv[3], strerror(errno));
            exit(EXIT_FAILURE);
        }
        
        length = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);

        if((length%blocklen) != 0)
            length += (blocklen-(length%blocklen));

        file = (u_int8_t *)mmap(0, length, PROT_READ|PROT_WRITE, MAP_SHARED,
                                fd, 0);
        if((void *)file == (void *)-1) {
            exit(EXIT_FAILURE);
        }
    } else {
        file = NULL;
        length = 0;

        while(!feof(stdin)) {
            file = (u_int8_t *)realloc(file, length+4096);
            length += fread(file+length, 1, 4096, stdin);
        }
    }

    (*encipher)(cipher.handle, file, file, length);

    if(argc == 4) {
        munmap(file, length);
    } else {
        fwrite(file, 1, length, stdout);
        free(file);
    }

    (*(cipherclosefunc_t)dlsym(cipher.dlhandle,
                               "cipherclose"))(cipher.handle);
    exit(EXIT_SUCCESS);
}

/* EOF encipher.c */
