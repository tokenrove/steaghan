/* 
 * sharedcipher.c
 * Created: Mon Mar 20 09:38:21 2000 by tek@wiw.org
 * Revised: Mon Mar 20 17:18:50 2000 by tek@wiw.org
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

void usage(int mode)
{
    fprintf(stderr, "usage: %s -c <cipher module> [-k <key>] [-h <hash>] [file]\n",
            (mode == 0)?"encipher":"decipher");
    exit(EXIT_FAILURE);
}

void sharedcipher(int mode, int argc, char **argv)
{
    moduleinfo_t cipher, hash;
    encipherfunc_t encipher;
    decipherfunc_t decipher;
    u_int8_t *file, *key, *iv, c = ' ';
    char *filename, *ciphername, *keyname, *hashname;
    int fd, i;
    u_int32_t length, keylen, ivlen, blocklen;
    char default_hash[] = "ripemd160";

    fd = -1;
    ciphername = keyname = filename = NULL;
    hashname = default_hash;

    for(i = 1; i < argc; i++) {
        if(argv[i][0] == '-') {
            if(argv[i][1] == 'c') {
                if(i+1 < argc) ciphername = argv[++i];
                else usage(mode);
            } else if(argv[i][1] == 'k') {
                if(i+1 < argc) keyname = argv[++i];
                else usage(mode);
            } else if(argv[i][1] == 'h') {
                if(i+1 < argc) hashname = argv[++i];
                else usage(mode);
            }
        } else if(filename == NULL) filename = argv[i];
        else usage(mode);
    }

    if(ciphername == NULL)
        usage(mode);
    
    if(loadmod(&cipher, ciphername)) {
        fprintf(stderr, "Unable to load module %s\n", ciphername);
        exit(EXIT_FAILURE);
    }
    if(cipher.moduletype != ciphermod) {
        fprintf(stderr, "%s is not a cipher module!\n", ciphername);
        exit(EXIT_FAILURE);
    }
    if(loadmod(&hash, hashname)) {
        fprintf(stderr, "failed to load module %s\n", hashname);
        exit(EXIT_FAILURE);
    }

    keylen=(*(cipherkeylenfunc_t)dlsym(cipher.dlhandle, "cipherkeylen"))();
    key = NULL;
    if(keylen != 0) {
        key = (u_int8_t *)malloc(keylen);
        if(key == NULL) {
            fprintf(stderr, "Out of memory\n");
            exit(EXIT_FAILURE);
        }

        if(keyname == NULL) {
            fprintf(stderr, "The %s cipher requires a key\n", ciphername);
            exit(EXIT_FAILURE);
        }
        (*(cipherphrasetokeyfunc_t)dlsym(cipher.dlhandle,
                                         "cipherphrasetokey"))(keyname,
                                                               key,
                                                               hash);
    }
    
    ivlen=(*(cipherivlenfunc_t)dlsym(cipher.dlhandle, "cipherivlen"))();
    iv = NULL;
    if(ivlen != 0) {
        fprintf(stderr, "IVs not supported\n");
        exit(EXIT_FAILURE);
    }

    blocklen=(*(cipherblocklenfunc_t)dlsym(cipher.dlhandle,
                                           "cipherblocklen"))();

    encipher = (encipherfunc_t)dlsym(cipher.dlhandle, "encipher");
    decipher = (decipherfunc_t)dlsym(cipher.dlhandle, "decipher");

    cipher.handle = (*(cipherinitfunc_t)dlsym(cipher.dlhandle,
                                              "cipherinit"))(key, iv);
    if(cipher.handle == NULL) {
        fprintf(stderr, "Failed to initialize cipher. (maybe your key is ");
        fprintf(stderr, "weak?)\n");
        exit(EXIT_FAILURE);
    }

    if(filename != NULL && strcmp(filename, "-")) {
        fd = open(filename, O_RDWR);
        if(fd == -1) {
            fprintf(stderr, "%s: %s", filename, strerror(errno));
            exit(EXIT_FAILURE);
        }

        length = lseek(fd, 0, SEEK_END);
        if(mode == 0 && blocklen > 1) {
            for(i = length;
                i < length+(blocklen-(length%blocklen)); i++) {
                write(fd, &c, 1);
            }
            lseek(fd, 0, SEEK_SET);

            file = (u_int8_t *)mmap(0, length+(blocklen-(length%blocklen)),
                                    PROT_READ|PROT_WRITE, MAP_SHARED, fd,
                                    0);
        } else {
            lseek(fd, 0, SEEK_SET);

            file = (u_int8_t *)mmap(0, length, PROT_READ|PROT_WRITE,
                                    MAP_SHARED, fd, 0);
        }
        
        if((void *)file == (void *)-1) {
            fprintf(stderr, "mmap failure: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

    } else {
        file = NULL;
        length = 0;

        while(!feof(stdin)) {
            file = (u_int8_t *)realloc(file, length+4096);
            /* FIXME check for errors */
            length += fread(file+length, 1, 4096, stdin);
        }
        file = (u_int8_t *)realloc(file, length+blocklen);
    }

    if(mode == 0) {
        if(blocklen > 1) pkcs5pad(file, length, file, &length, blocklen);
        (*encipher)(cipher.handle, file, file, length);
    } else {
        (*decipher)(cipher.handle, file, file, length);
        if(blocklen > 1) {
            if(pkcs5unpad(file, length, file, &length, blocklen)) {
                fprintf(stderr, "Unpadding failed! (truncated file?)\n");
                exit(EXIT_FAILURE);
            }

            ftruncate(fd, length);
        }
    }

    if(filename != NULL && strcmp(filename, "-")) {
        munmap(file, length);
        close(fd);

    } else {
        fwrite(file, 1, length, stdout);
        free(file);
    }

    (*(cipherclosefunc_t)dlsym(cipher.dlhandle,
                               "cipherclose"))(cipher.handle);
    return;
}

/* EOF sharedcipher.c */
