#ifndef HAD_VBR_H
#define HAD_VBR_H

/*
  vbr.h -- Xing VBR tag handling
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
