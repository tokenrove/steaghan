/* 
 * ms-shared.c
 * Created: Sun Mar  5 17:44:09 2000 by tek@wiw.org
 * Revised: Sun Mar  5 17:44:09 2000 (pending)
 * Copyright 2000 Julian E. C. Squires (tek@wiw.org)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * $Id$
 * 
 */

#include "steaghanmods.h"

short getleword(file_t *file);
long getledword(file_t *file);

short getleword(file_t *file)
{
    short x; u_int8_t c;
    (*file->read)(file->handle, (*file->tell)(file->handle)+1, 1, &c);
    x = c;
    (*file->read)(file->handle, (*file->tell)(file->handle), 1, &c);
    x |= c << 8;
    return x;
}

long getledword(file_t *file)
{
    return (long)getleword(file)|((long)getleword(file)<<16);
}

/* EOF ms-shared.c */
