#ifndef HAD_INBUF_H
#define HAD_INBUF_H

#include <stdio.h>

struct inbuf {
    FILE *f;		/* underlying file */
    unsigned char *b;	/* buffer */
    int bsize;		/* size of buffer */
    int allocsize;	/* allocation size of buffer */
    long first, last;	/* first, last valid byte in buffer (file position) */
    long keep, cur;	/* earliest byte to keep, current byte position */
    long length;	/* logical length of file */
    int eof;		/* physical EOF reached */
};

#define inbuf_getc(pos, inb) (((inb)->first <= (pos) && (pos) < (inb)->last) \
			      ? (inb)->b[(pos)%(inb)->bsize] \
			      : inbuf_fgetc(pos, inb))

#define inbuf_keep(pos, inb)  ((inb)->keep = (pos))
#define inbuf_unkeep(inb)     ((inb)->keep = -1)

struct inbuf *inbuf_new(FILE *f, long length);
void inbuf_free(struct inbuf *inb);
/* int inbuf_keep(long pos, struct inbuf *inb); */
/* int inbuf_unkeep(struct inbuf *inb); */

int inbuf_fgetc(long pos, struct inbuf *inb);
int inbuf_getlong(unsigned long *lp, long pos, struct inbuf *inb);
int inbuf_copy(unsigned char **b, long pos, long len, struct inbuf *inb);
int inbuf_length(struct inbuf *inb);

#endif /* inbuf.h */
