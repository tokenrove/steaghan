/* 
 * sharedcipher.c
 * Created: Mon Mar 20 09:38:21 2000 by tek@wiw.org
 * Revised: Sat Mar 25 21:56:58 2000 by tek@wiw.org
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

void usage(int mode)
{
    fprintf(stderr, "usage: %s -c <cipher module> [-k <key>] [-h <hash>]",
            (mode == 0)?"encipher":"decipher");
#ifdef HAVE_DLSYM
    fprintf(stderr, " [-d <top dir>] [file]\n");
#else
    fprintf(stderr, " [file]\n");
    fprintf(stderr, "       %s -l\n", (mode == 0)?"encipher":"decipher");
#endif
    exit(EXIT_FAILURE);
}

void sharedcipher(int mode, int argc, char **argv)
{
    moduleinfo_t cipher, hash;
    encipherfunc_t encipher;
    decipherfunc_t decipher;
    u_int8_t *file, *key, *iv, c = ' ';
    hashfunc_t hashfunc;
    u_int32_t hashlen, iterlen = 0;
    u_int8_t *buffer;
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
#ifdef HAVE_DLSYM
            } else if(argv[i][1] == 'd') {
                if(i+1 < argc) topdir = argv[++i];
                else usage(mode);
#else
            } else if(argv[i][1] == 'l') {
                listmods();
                exit(EXIT_SUCCESS);
#endif
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

    hashfunc = (hashfunc_t)getsym(&hash, "hash");
    hashlen = (*(hashlenfunc_t)getsym(&hash, "hashlen"))();

    keylen=(*(cipherkeylenfunc_t)getsym(&cipher, "cipherkeylen"))();
    key = NULL;
    if(keylen != 0) {
        key = (u_int8_t *)malloc(keylen);
        if(key == NULL) {
            fprintf(stderr, "Out of memory\n");
            exit(EXIT_FAILURE);
        }

        if(keyname == NULL) {
            if(filename == NULL || !strcmp(filename, "-")) {
                fprintf(stderr, "The %s cipher requires a key\n", ciphername);
                exit(EXIT_FAILURE);
            }

            getpassphrase(&keyname);
        }
        (*(cipherphrasetokeyfunc_t)getsym(&cipher,
                                          "cipherphrasetokey"))(keyname,
                                                                key,
                                                                hash);
    }
    
    ivlen=(*(cipherivlenfunc_t)getsym(&cipher, "cipherivlen"))();
    iv = NULL;
    if(ivlen != 0) {
        iv = (u_int8_t *)malloc(ivlen);
        if(iv == NULL) {
            fprintf(stderr, "Out of memory on IV\n");
            exit(EXIT_FAILURE);
        }

        if(mode == 0) {
            buffer = (u_int8_t *)malloc(hashlen/8);

            randominit();
            for(i = 0; i < hashlen/8; i++)
                buffer[i] = randombyte();
            (*hashfunc)((u_int8_t *)buffer, hashlen/8, buffer);
            if(hashlen/8 < ivlen) {
                memmove(iv, buffer, hashlen/8);
                iterlen += hashlen/8;
                while(iterlen < ivlen) {
                    (*hashfunc)(buffer, hashlen/8, buffer);
                    memmove(iv+iterlen, buffer, hashlen/8);
                    iterlen += hashlen/8;
                }
                (*hashfunc)(buffer, hashlen/8, buffer);
                memmove(iv+iterlen-hashlen/8, buffer, iterlen%ivlen);

            } else {
                memmove(iv, buffer, ivlen);
            }
            free(buffer);  
        }
    }

    blocklen=(*(cipherblocklenfunc_t)getsym(&cipher, "cipherblocklen"))();

    encipher = (encipherfunc_t)getsym(&cipher, "encipher");
    decipher = (decipherfunc_t)getsym(&cipher, "decipher");

    if(filename != NULL && strcmp(filename, "-")) {
        if(ivlen == 0) {
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
            fd = open(filename, O_RDWR);
            if(fd == -1) {
                fprintf(stderr, "%s: %s", filename, strerror(errno));
                exit(EXIT_FAILURE);
            }
            
            length = lseek(fd, 0, SEEK_END);
            lseek(fd, 0, SEEK_SET);
            
            if(mode == 0) {
                if(blocklen > 1)
                    file = (u_int8_t *)malloc(length+(blocklen-
                                                      (length%blocklen)));
                else
                    file = (u_int8_t *)malloc(length);
                if(file == NULL) {
                    fprintf(stderr, "Out of memory\n");
                    exit(EXIT_FAILURE);
                }

            } else {
                length -= ivlen;
                file = (u_int8_t *)malloc(length);
                if(file == NULL) {
                    fprintf(stderr, "Out of memory\n");
                    exit(EXIT_FAILURE);
                }
                read(fd, iv, ivlen);
            }

            read(fd, file, length);
            lseek(fd, 0, SEEK_SET);

            if(mode == 0) {
                write(fd, iv, ivlen);
            }
        }

    } else {
        fd = -1;
        file = NULL;
        length = 0;

        while(!feof(stdin)) {
            file = (u_int8_t *)realloc(file, length+4096);
            /* FIXME check for errors */
            length += fread(file+length, 1, 4096, stdin);
        }
        file = (u_int8_t *)realloc(file, length+blocklen);
    }

    cipher.handle = (*(cipherinitfunc_t)getsym(&cipher,
                                               "cipherinit"))(key, iv);
    if(cipher.handle == NULL) {
        fprintf(stderr, "Failed to initialize cipher. (maybe your key is ");
        fprintf(stderr, "weak?)\n");
        exit(EXIT_FAILURE);
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

            if(fd != -1)
                ftruncate(fd, length);
        }
    }

    if(fd != -1) {
        if(ivlen == 0) {
            munmap(file, length);
            close(fd);

        } else {
            if(mode == 0) {
                lseek(fd, ivlen, SEEK_SET);
            } else {
                lseek(fd, 0, SEEK_SET);
            }

            write(fd, file, length);
            free(file);
        }

    } else {
        fwrite(file, 1, length, stdout);
        free(file);
    }

    (*(cipherclosefunc_t)getsym(&cipher,
                                "cipherclose"))(cipher.handle);
    return;
}

/* EOF sharedcipher.c */
