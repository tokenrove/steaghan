/* 
 * main.c
 * Created: Tue Jan 25 14:02:14 2000 by tek@wiw.org
 * Revised: Sat Mar 25 22:07:45 2000 by tek@wiw.org
 * Copyright 2000 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "steaghan.h"

void dealwithargs(steaghanconf_t *conf, char **argv, int argc);
void usage(void);
void loadeachmodule(steaghanconf_t *conf);
void getkeyfromfile(steaghanconf_t *conf);
void setupwrapper(steaghanconf_t *conf);
void setupprpg(steaghanconf_t *conf);

int main(int argc, char **argv)
{
    u_int32_t seclen;
    u_int8_t *secdata;
    FILE *fp;
    steaghanconf_t conf;
    
    char default_prpg[] = "classic";
    char default_hash[] = "ripemd160";
    char default_file[] = "mmap";
    char default_cipher[] = "null";

    conf.secret_filename = NULL;
    conf.wrapper_filename = NULL;
    conf.wrapper_modname = NULL;
    conf.prpg_modname = default_prpg;
    conf.hash_modname = default_hash;
    conf.file_modname = default_file;
    conf.cipher_modname = default_cipher;
    conf.mode = '?';

    dealwithargs(&conf, argv, argc);
    loadeachmodule(&conf);
    getkeyfromfile(&conf);
    setupwrapper(&conf);
    setupprpg(&conf);

    if(conf.mode == 'i') {
        fprintf(stderr, "injecting...\n");

        if(conf.secret_filename != NULL && strcmp(conf.secret_filename, "-")) {
            fp = fopen(conf.secret_filename, "r");
            fseek(fp, 0, SEEK_END);
            seclen = ftell(fp)*8;
            rewind(fp);
        
            secdata = (u_int8_t *)malloc(seclen/8);
            assert(secdata != NULL);
        
            fread(secdata, 1, seclen/8, fp);
            fclose(fp);
        } else {
            secdata = NULL;
            seclen = 0;

            while(!feof(stdin)) {
                secdata = (u_int8_t *)realloc(secdata, seclen+4096);
                seclen += fread(secdata+seclen, 1, 4096, stdin);
            }

            seclen *= 8;
        }

        inject(conf.prpg, conf.wrapper, secdata, seclen);

    } else {
        fprintf(stderr, "extracting...\n");

        secdata = extract(conf.prpg, conf.wrapper, &seclen);

        if(conf.secret_filename != NULL && strcmp(conf.secret_filename, "-")) {
            fp = fopen(conf.secret_filename, "w");
            fwrite(secdata, 1, seclen/8, fp);
            fclose(fp);
        } else {
            fwrite(secdata, 1, seclen/8, stdout);
        }
    }

    free(secdata);
    
    (*(wrapclosefunc_t)dlsym(conf.wrapper.dlhandle,
                             "wrapclose"))(conf.wrapper.handle); 
    (*(permuclosefunc_t)dlsym(conf.prpg.dlhandle,
                              "permuclose"))(conf.prpg.handle);
    (*(fileclosefunc_t)dlsym(conf.filemod.dlhandle,
                             "fileclose"))(conf.file);

    exit(EXIT_SUCCESS);
}

void dealwithargs(steaghanconf_t *conf, char **argv, int argc)
{
    int i;
    
    for(i = 1; i < argc; i++) {
        if(argv[i][0] == '-') {
            if(argv[i][1] == 'p') {
                if(i+1 < argc) conf->prpg_modname = argv[++i];
                else usage();
            } else if(argv[i][1] == 'h') {
                if(i+1 < argc) conf->hash_modname = argv[++i];
                else usage();
            } else if(argv[i][1] == 'f') {
                if(i+1 < argc) conf->file_modname = argv[++i];
                else usage();
            } else if(argv[i][1] == 'w') {
                if(i+1 < argc) conf->wrapper_modname = argv[++i];
                else usage();
            } else if(argv[i][1] == 'k') {
                if(i+1 < argc) conf->key_filename = argv[++i];
                else usage();
            } else if(argv[i][1] == 'c') {
                if(i+1 < argc) conf->cipher_modname = argv[++i];
                else usage();
            } else if(argv[i][1] == '-') {
                break;
            }
        } else {
            if(conf->mode == '?' && (argv[i][0] == 'i' || argv[i][0] == 'e'))
                conf->mode = argv[i][0];
            else if(conf->wrapper_filename == NULL)
                conf->wrapper_filename = argv[i];
            else if(conf->secret_filename == NULL)
                conf->secret_filename = argv[i];
            else
                usage();
        }
    }

    /* if any arguments follow, they are the mandatory ones */
    for(; i < argc; i++) {
        if(conf->mode == '?' && (argv[i][0] == 'i' || argv[i][0] == 'e'))
            conf->mode = argv[i][0];
        else if(conf->wrapper_filename == NULL)
            conf->wrapper_filename = argv[i];
        else if(conf->secret_filename == NULL)
            conf->secret_filename = argv[i];
        else
            usage();
    }

    if(conf->wrapper_filename == NULL ||
       conf->wrapper_modname == NULL ||
       conf->key_filename == NULL) {
        usage();
    }
}

void usage(void)
{
    fprintf(stderr, "steaghan [options] [--] <mode> <wrapper file> [secret ");
    fprintf(stderr, "file]\n");
    fprintf(stderr, "  where mode is i or e (``i''nject or ``e''xtract).\n");
    fprintf(stderr, "options: [options marked with * are mandatory]\n");
    
    fprintf(stderr, "  -p <prpg module> [defaults to classic]\n");
    fprintf(stderr, "  -h <hash module> [defaults to ripemd160]\n");
    fprintf(stderr, "  -f <file module> [defaults to mmap]\n");
    fprintf(stderr, "* -w <wrapper module>\n");
    fprintf(stderr, "* -k <key file>\n");
    fprintf(stderr, "  -c <cipher module> [defaults to null]\n");

    fprintf(stderr, "\nPlease see steaghan(1) for more detail.\n");
    exit(EXIT_FAILURE);
}

void loadeachmodule(steaghanconf_t *conf)
{
    if(loadmod(&conf->prpg, conf->prpg_modname)) {
        fprintf(stderr, "failed to load module %s\n", conf->prpg_modname);
        exit(EXIT_FAILURE);
    }
    describemod(&conf->prpg);
    if(loadmod(&conf->hash, conf->hash_modname)) {
        fprintf(stderr, "failed to load module %s\n", conf->hash_modname);
        exit(EXIT_FAILURE);
    }
    describemod(&conf->hash);
    if(loadmod(&conf->wrapper, conf->wrapper_modname)) {
        fprintf(stderr, "failed to load module %s\n", conf->wrapper_modname);
        exit(EXIT_FAILURE);
    }
    describemod(&conf->wrapper);
    if(loadmod(&conf->filemod, conf->file_modname)) {
        fprintf(stderr, "failed to load module %s\n", conf->file_modname);
        exit(EXIT_FAILURE);
    }
    describemod(&conf->filemod);
    if(loadmod(&conf->cipher, conf->cipher_modname)) {
        fprintf(stderr, "failed to load module %s\n", conf->cipher_modname);
        exit(EXIT_FAILURE);
    }
    describemod(&conf->cipher);
    
    return;
}

void getkeyfromfile(steaghanconf_t *conf)
{
    FILE *fp;
    char *passphrase;

    conf->cipherkey = NULL; conf->cipheriv = NULL;
    conf->cipherkeylen = (*(cipherkeylenfunc_t)dlsym(conf->cipher.dlhandle,
                                                    "cipherkeylen"))();
    conf->cipherivlen = (*(cipherivlenfunc_t)dlsym(conf->cipher.dlhandle,
                                                  "cipherivlen"))();
    conf->cipherblocklen = (*(cipherblocklenfunc_t)dlsym(conf->cipher.dlhandle,
                                                         "cipherblocklen"))();

    if(conf->cipherkeylen != 0) {
        getpassphrase(&passphrase);

        conf->cipherkey = (u_int8_t *)malloc(conf->cipherkeylen);
        if(conf->cipherkey == NULL) {
            fprintf(stderr, "Out of memory on cipher key\n");
            exit(EXIT_FAILURE);
        }
        (*(cipherphrasetokeyfunc_t)dlsym(conf->cipher.dlhandle,
                                         "cipherphrasetokey"))(passphrase,
                                                               conf->cipherkey,
                                                               conf->hash);
        memset(passphrase, 0, strlen(passphrase));
        free(passphrase);
    }

    fp = fopen(conf->key_filename, "r");
    if(fp == NULL) {
        fprintf(stderr, "%s: %s\n", conf->key_filename, strerror(errno));
        exit(EXIT_FAILURE);
    }
    fseek(fp, 0, SEEK_END);
    conf->keylen = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if(conf->cipherivlen != 0) {
        conf->keylen -= conf->cipherivlen;
        conf->cipheriv = (u_int8_t *)malloc(conf->cipherivlen);
        if(conf->cipheriv == NULL) {
            fprintf(stderr, "Out of memory on cipher IV\n");
            exit(EXIT_FAILURE);
        }
        fread(conf->cipheriv, 1, conf->cipherivlen, fp);
    }

    conf->cipher.handle =
        (*(cipherinitfunc_t)dlsym(conf->cipher.dlhandle,
                                  "cipherinit"))(conf->cipherkey,
                                                 conf->cipheriv);
    if(conf->cipher.handle == NULL) {
        fprintf(stderr, "Failed to initialize cipher. (maybe your key is ");
        fprintf(stderr, "weak?)\n");
        exit(EXIT_FAILURE);
    }

    conf->key = (u_int8_t *)malloc(conf->keylen);
    if(conf->key == NULL) {
        fprintf(stderr, "Out of memory loading key\n");
        exit(EXIT_FAILURE);
    }
    if(fread(conf->key, 1, conf->keylen, fp) != conf->keylen) {
        fprintf(stderr, "%s: %s\n", conf->key_filename, strerror(errno));
        exit(EXIT_FAILURE);
    }
    fclose(fp);

    (*(decipherfunc_t)dlsym(conf->cipher.dlhandle,
                            "decipher"))(conf->cipher.handle, conf->key,
                                         conf->key, conf->keylen);
    if(conf->cipherblocklen > 1 && pkcs5unpad(conf->key, conf->keylen,
                                              conf->key, &conf->keylen,
                                              conf->cipherblocklen)) {
        fprintf(stderr, "Unpadding failed! (truncated file?)\n");
        exit(EXIT_FAILURE);
    }

    /* FIXME: K = H(Ku o f(W)) here */

    (*(cipherclosefunc_t)dlsym(conf->cipher.dlhandle,
                               "cipherclose"))(conf->cipher.handle);
    free(conf->cipheriv);
    free(conf->cipherkey);
    fprintf(stderr, "Loaded key successfully\n");
    return;
}

void setupwrapper(steaghanconf_t *conf)
{
    conf->file = (*(fileinitfunc_t)dlsym(conf->filemod.dlhandle,
                                        "fileinit"))(conf->wrapper_filename);
    if(conf->file == NULL) {
        fprintf(stderr, "failed to open the specified wrapper file.\n");
        exit(EXIT_FAILURE);
    }

    conf->wrapper.handle = (*(wrapinitfunc_t)dlsym(conf->wrapper.dlhandle,
                                                  "wrapinit"))(conf->file);
    if(conf->wrapper.handle == NULL) {
        fprintf(stderr, "failed to setup the wrapper handle. (Is the file");
        fprintf(stderr, " really the right type?)\n");
        exit(EXIT_FAILURE);
    }
    
    return;
}

void setupprpg(steaghanconf_t *conf)
{
    u_int32_t wraplen;

    wraplen = (*(wraplenfunc_t)dlsym(conf->wrapper.dlhandle,
                                     "wraplen"))(conf->wrapper.handle);

    conf->prpg.handle = (*(permuinitfunc_t)dlsym(conf->prpg.dlhandle,
                                                "permuinit"))(wraplen,
                                                              conf->key,
                                                              conf->keylen,
                                                              conf->hash);
    if(conf->prpg.handle == NULL) {
        fprintf(stderr, "failed to setup the prpg handle. (Do you have ");
        fprintf(stderr, " enough memory, did you supply a valid hash?)\n");
        exit(EXIT_FAILURE);
    }
    
    return;
}

/* EOF main.c */
