#include <stdlib.h>
#include <string.h>

#include "malint.h"
#include "vbr.h"



struct vbr *
vbr_parse(long pos, unsigned char *b, int len)
{
    static ltab[] = {
	 8, 12, 12, 16, 108, 112, 112, 116,
	12, 16, 16, 20, 112, 116, 116, 120
    };
    
    struct vbr *vbr;
    unsigned long flags;

    if (len < 8 || !IS_XING(GET_LONG(b)))
	return NULL;
    
    flags = GET_LONG(b+4);
    if ((flags & ~0xf) && (output & OUT_VBR_UNSUPP_FLAG))
	out(pos, "vbr tag with unsuppored flags: 0x%08x", flags);
	
    if (len < ltab[flags&0xf]) {
	if (output & OUT_VBR_SHORT)
	    out(pos, "short vbr tag (%d < %d)", len, ltab[flags&0xf]);
	return NULL;
    }

    if ((vbr=(struct vbr *)malloc(sizeof(struct vbr))) == NULL)
	return NULL;

    vbr->flags = flags;
    b += 8;

    if (flags & VBR_FRAMES) {
	vbr->frames = GET_LONG(b);
	b += 4;
    }
    else
	vbr->frames = 0;
    
    if (flags & VBR_BYTES) {
	vbr->bytes = GET_LONG(b);
	b += 4;
    }
    else
	vbr->bytes = 0;

    if (flags & VBR_TOC) {
	memcpy(vbr->toc, b, 100);
	b += 100;
    }
    else
	memset(vbr->toc, 0, 100);

    if (flags & VBR_VBR_SCALE) {
	vbr->vbr_scale = GET_LONG(b);
	b += 4;
    }
    else
	vbr->vbr_scale = 0;
    
    return vbr;
}
