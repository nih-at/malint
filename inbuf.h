#ifndef HAD_INBUF_H
#define HAD_INBUF_H

/*
  inbuf.h -- random access to a stretch of a file
  Copyright (C) 2000 Dieter Baron

  This file is part of malint, an MPEG Audio stream validator.
  The author can be contacted at <dillo@giga.or.at>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/



#include <stdio.h>

#define INBUF_MAX_KEEP	16

struct inbuf {
    FILE *f;		/* underlying file */
    unsigned char *b;	/* buffer */
    int bsize;		/* size of buffer */
    int allocsize;	/* allocation size of buffer */
    long first, last;	/* first, last valid byte in buffer (file position) */
    long keep;		/* earliest byte to keep */
    long okeep[INBUF_MAX_KEEP];	/* stack of keep values */
    int nkeep;		/* number of keeps on stack */
    long length;	/* logical length of file */
    int eof;		/* physical EOF reached */
    
};

#define inbuf_getc(pos, inb) (((inb)->first <= (pos) && (pos) < (inb)->last) \
			      ? (inb)->b[(pos)%(inb)->bsize] \
			      : inbuf_fgetc(pos, inb))

/* #define inbuf_keep(pos, inb)  ((inb)->keep = (pos)) */
/* #define inbuf_unkeep(inb)     ((inb)->keep = -1) */

struct inbuf *inbuf_new(FILE *f, long length);
void inbuf_free(struct inbuf *inb);
int inbuf_keep(long pos, struct inbuf *inb);
int inbuf_unkeep(struct inbuf *inb);

int inbuf_fgetc(long pos, struct inbuf *inb);
int inbuf_getlong(unsigned long *lp, long pos, struct inbuf *inb);
int inbuf_copy(unsigned char **b, long pos, long len, struct inbuf *inb);
int inbuf_length(struct inbuf *inb);

#endif /* inbuf.h */
