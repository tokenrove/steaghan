/* 
 * phrasetokey.h
 * Created: Wed Mar 29 20:59:14 2000 by tek@wiw.org
 * Revised: Wed Mar 29 20:59:14 2000 (pending)
 * Copyright 2000 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#ifndef PHRASETOKEY_H
#define PHRASETOKEY_H

extern void standardphrasetokey(char *phrase, u_int8_t *key, u_int32_t keylen,
                                moduleinfo_t hash);

#endif

/* EOF phrasetokey.h */
