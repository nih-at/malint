/*
  crc -- MPEG crc functions
  Copyright (C) 1998 by Johannes Overmann
  Copyright (C) 2000 Dieter Baron

  The crc implementation was taken from mp3check, adapted for use in
  malint by Dieter Baron.
  The original author can be contacted at <overmann@iname.com>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */



#include "malint.h"
#include "mpeg.h"

#define MPEG_CRCPOLY	0x18005

static int crc_tab[256];

static void crc_update(int *crc, unsigned char *b, int n);



void
crc_init(void)
{
    int i, x, j;
    
    for(i=0; i<256; i++) {
	x = i << 9;
	for(j=0; j<8; j++, x<<=1)
	    if (x & 0x10000)
		x ^= MPEG_CRCPOLY;
	crc_tab[i] = x >> 1;
    }
}



static void
crc_update(int *crc, unsigned char *b, int n)
{
    int i;

    for (i=0; i<n; i++)
	*crc = crc_tab[(*crc>>8)^b[i]] ^ ((*crc<<8)&0xffff);
}



int
crc_frame(unsigned long h, unsigned char *data, int len)
{
    int crc, n;

    switch (MPEG_LAYER(h)) {
    case 1:
	switch (MPEG_MODE(h)) {
	case MPEG_MODE_SINGLE:
	    n = 16;
	    break;
	case MPEG_MODE_JSTEREO:
	    n = (32+MPEG_JSBOUND(h))/2;
	    break;
	case MPEG_MODE_STEREO:
	case MPEG_MODE_DUAL:
	    n = 32;
	    break;
	}
	break;

    case 2:
	return -1;
	
    case 3:
	n = MPEG_SILEN(h);
	break;
    }

    crc = 0xffff;

    crc_update(&crc, data+2, 2);
    crc_update(&crc, data+6, n);
    
    return crc;
}
