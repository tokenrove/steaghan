/* 
 * pnm-shared.h
 * Created: Wed Mar 29 20:43:13 2000 by tek@wiw.org
 * Revised: Wed Mar 29 20:43:13 2000 (pending)
 * Copyright 2000 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#ifndef PNM_SHARED_H
#define PNM_SHARED_H

extern int iswhitespace(char c);
extern void readtoeol(file_t *file);
extern void readtononws(file_t *file);
extern void gotonexttoken(file_t *file);
extern void readtoken(char *buffer, int buflen, file_t *file);

#endif

/* EOF pnm-shared.h */
