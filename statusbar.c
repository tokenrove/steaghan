/* 
 * statusbar.c
 * Created: Wed Mar  8 15:51:31 2000 by tek@wiw.org
 * Revised: Wed Mar  8 16:03:21 2000 by tek@wiw.org
 * Copyright 2000 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include <stdio.h>

#define STATUSBAR_WIDTH 78
#define STATUSBAR_INDENT 1

static int total, current, delta, lastdelta;

void statusbar_init(int total_);
void statusbar_update(int increment);
void statusbar_close(void);

void statusbar_init(int total_)
{
    int i;
    
    total = total_;
    current = 0;
    delta = total/STATUSBAR_WIDTH;
    lastdelta = 0;
    for(i = 0; i < STATUSBAR_INDENT; i++) printf(" ");
    return;
}

void statusbar_update(int increment)
{
    current += increment;
    if(current-lastdelta > delta) {
        printf("*"); fflush(stdout);
        lastdelta = current;
    }
    return;
}

void statusbar_close(void)
{
    printf("\n");
    total = 0;
    current = 0;
    return;
}

/* EOF statusbar.c */
