#ifndef HAD_MPEG_H
#define HAD_MPEG_H

/*
  mpeg.h -- MPEG tables and field access macros

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



extern int _mp3_samp_tab[2][4];		/* in build_frame_tab.c */
extern int _mp3_bit_tab[2][16][3];	/* in build_frame_tab.c */
extern int _mp3_jsb_tab[3][4];
extern int _mp3_nsamp_tab[3];


/* extracting fields from header */

#define MPEG_VERSION(h)	(2-(((h)&0x00080000)>>19))
#define MPEG_LAYER(h)	(4-(((h)&0x00060000)>>17))
#define MPEG_CRC(h)	(!((h)&0x00010000))
#define MPEG_BITRATE_R(h)  (((h)&0x0000f000)>>12)
#define MPEG_BITRATE(h)	(_mp3_bit_tab[2-MPEG_VERSION(h)]\
				     [MPEG_BITRATE_R(h)][MPEG_LAYER(h)-1])
#define MPEG_SAMPFREQ_R(h) (((h)&0x00000c00)>>10)
#define MPEG_SAMPFREQ(h) (_mp3_samp_tab[2-MPEG_VERSION(h)][MPEG_SAMPFREQ_R(h)])
#define MPEG_PADDING(h)	(((h)&0x00000200)>>9)
#define MPEG_PRIV(h)	(((h)&0x00000100)>>8)
#define MPEG_MODE(h)	(((h)&0x000000c0)>>6)
#define MPEG_MODEEXT(h)	(((h)&0x00000030)>>4)
#define MPEG_COPY(h)	(!((h)&0x00000008))
#define MPEG_ORIG(h)	(!((h)&0x00000004))
#define MPEG_EMPH(h)	((h)&0x00000003)

/* computed values from header */

#define MPEG_FRLEN(h)	(table[((h)&0x000fffe0)>>9])
#define MPEG_JSBOUND(h)	(MPEG_MODE(h) == MPEG_MODE_JSTEREO \
			 ? (MPEG_MODEEXT(h)<<2)+4 \
			 : MPEG_SBLIMIT)
#define MPEG_SILEN(h)   (MPEG_VERSION(h) == 1 \
			 ? (MPEG_MODE(h) == MPEG_MODE_SINGLE ? 17 : 32) \
			 : (MPEG_MODE(h) == MPEG_MODE_SINGLE ?  9 : 17))
#define MPEG_NSAMP(h)	(_mp3_nsamp_tab[MPEG_LAYER(h)-1])

/* header field values */

#define MPEG_MODE_STEREO	0x0
#define MPEG_MODE_JSTEREO	0x1
#define MPEG_MODE_DUAL		0x2
#define MPEG_MODE_SINGLE	0x3

#define MPEG_EMPH_NONE		0x0
#define MPEG_EMPH_MS5015	0x1
#define MPEG_EMPH_RESERVED	0x2
#define MPEG_EMPH_CCITT		0x3

/* other constants */

#define MPEG_SBLIMIT		32
#define MPEG_SCALE_BLOCK	12

#endif /* mpeg.h */
