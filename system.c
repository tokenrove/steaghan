/* 
 * system.c
 * Created: Sat Mar 25 21:59:10 2000 by tek@wiw.org
 * Revised: Sat Mar 25 22:13:09 2000 by tek@wiw.org
 * Copyright 2000 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <sys/types.h>

#define PASSPHRASECHUNK 128

void getpassphrase(char **passphrase);
void randominit(void);
u_int8_t randombyte(void);

void getpassphrase(char **passphrase)
{
    int passphraselen;
    
    fprintf(stderr, "Enter passphrase: (should not echo)\n");
    /* FIXME: don't use stty */
    system("stty -echo");

    passphraselen = 0;
    *passphrase = NULL;
    do {
        *passphrase = (char *)realloc(*passphrase,
                                      passphraselen+PASSPHRASECHUNK);
        if(*passphrase == NULL) {
            fprintf(stderr, "Out of memory reading passphrase\n");
            system("stty echo");
            exit(EXIT_FAILURE);
        }

        /* FIXME: potential off-by-one errors here */
        (*passphrase)[passphraselen] = 0;
        passphraselen += PASSPHRASECHUNK;
        fgets(*passphrase+strlen(*passphrase), PASSPHRASECHUNK, stdin);
    } while((*passphrase)[strlen(*passphrase)-1] != '\n');

    (*passphrase)[strlen(*passphrase)-1] = 0;
    system("stty echo");

    return;
}

void randominit(void)
{
    srand(time(NULL)+getpid());
}

u_int8_t randombyte(void)
{
    return (256.0*(rand()/(1.0+RAND_MAX)));
}


/* EOF system.c */
