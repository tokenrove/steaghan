/* 
 * decipher.c
 * Created: Thu Mar  9 05:09:29 2000 by tek@wiw.org
 * Revised: Mon Mar 20 09:47:25 2000 by tek@wiw.org
 * Copyright 2000 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <stdlib.h>

void sharedcipher(int mode, int argc, char **argv);

int main(int argc, char **argv)
{
    sharedcipher(1, argc, argv);
    exit(EXIT_SUCCESS);
}

/* EOF decipher.c */
