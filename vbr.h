#ifndef HAD_VBR_H
#define HAD_VBR_H

struct vbr {
    unsigned long flags;
    unsigned long frames, bytes;
    unsigned char toc[100];
    unsigned long vbr_scale;
};

#define VBR_FRAMES	0x01
#define VBR_BYTES	0x02
#define VBR_TOC		0x04
#define VBR_VBR_SCALE	0x08

struct vbr *vbr_parse(long pos, unsigned char *b, int len);

#endif /* vbr.h */
