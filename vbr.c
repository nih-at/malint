/*
  vbr.c -- Xing VBR tag handling
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
