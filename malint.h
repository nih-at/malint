#ifndef HAD_MALINT_H
#define HAD_MALINT_H

/*
  malint.h -- main include file
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



extern char *prg;
extern int output;



/* data access function */

#define GET_LONG(x)	(((x)[0]<<24)|((x)[1]<<16)|((x)[2]<<8)|(x)[3])
#define GET_INT3(x)	(((x)[0]<<16)|((x)[1]<<8)|(x)[2])
#define GET_SHORT(x)	(((x)[0]<<8)|(x)[1])
#define GET_ID3LEN(x)	((((x)[0]&0x7f)<<21)|(((x)[1]&0x7f)<<14) \
			 |(((x)[2]&0x7f)<<7)|((x)[3]&0x7f))

#define LONG_TO_ID3LEN(l)	((((l)&0x7f000000)>>3) \
				 | (((l)&0x7f0000)>>2) \
				 | (((l)&0x7f00)>>1) \
				 | (((l)&0x7f)))

/* check functions */

#define IS_SYNC(h)	(((h)&0xffe00000) == 0xffe00000)
#define IS_MPEG(h)	(IS_SYNC(h) && MPEG_FRLEN(h) \
			 && MPEG_EMPH(h) != MPEG_EMPH_RESERVED)
#define IS_ID3v1(h)	(((h)&0xffffff00) == (('T'<<24)|('A'<<16)|('G'<<8)))
#define IS_ID3v2(h)	(((h)&0xffffff00) == (('I'<<24)|('D'<<16)|('3'<<8)))
#define IS_ID3(h)	(IS_ID3v1(h) || IS_ID3v2(h))
#define IS_VALID(h)	(IS_MPEG(h) || IS_ID3(h))
#define IS_XING(h)	((h) == (('X'<<24)|('i'<<16)|('n'<<8)|('g')))

/* output and check selection constants */

#define OUT_TAG			0x000001
#define OUT_TAG_CONTENTS	0x000002
#define OUT_TAG_SHORT		0x002000
#define OUT_PLAYTIME		0x008000
#define OUT_HEAD_1ST		0x000004
#define OUT_FASTINFO_ONLY	0x004000
#define OUT_HEAD_CHANGE		0x000008
#define OUT_HEAD_ILLEGAL	0x000010
#define OUT_RESYNC_SKIP		0x000020
#define OUT_RESYNC_BAILOUT	0x000040
#define OUT_CRC_ERROR		0x000080
#define OUT_BITR_OVERFLOW	0x000100
#define OUT_BITR_FRAME_OVER	0x000200
#define OUT_BITR_GAP		0x000400
#define OUT_BITR_TAGIN		0x200000
#define OUT_LFRAME_SHORT	0x000800
#define OUT_LFRAME_PADDING	0x001000
#define OUT_FRAME_SHORT		0x080000
#define OUT_FRAME_PADDING	0x100000
#define OUT_VBR_TOC_NONINC	0x010000
#define OUT_VBR_UNSUPP_FLAG	0x020000
#define OUT_VBR_SHORT		0x040000

#define OUT_M_TAG (OUT_TAG|OUT_TAG_SHORT|OUT_TAG_CONTENTS)

#define OUT_M_ERROR (OUT_TAG_SHORT|OUT_HEAD_CHANGE \
		     |OUT_HEAD_ILLEGAL|OUT_RESYNC_SKIP|OUT_RESYNC_BAILOUT \
		     |OUT_CRC_ERROR|OUT_BITR_OVERFLOW|OUT_BITR_FRAME_OVER \
                     |OUT_BITR_TAGIN \
		     |OUT_LFRAME_SHORT|OUT_VBR_TOC_NONINC|OUT_VBR_SHORT \
                     |OUT_FRAME_SHORT|OUT_FRAME_PADDING)

#define OUT_M_CHECK_FRAME (OUT_BITR_OVERFLOW|OUT_BITR_FRAME_OVER|OUT_BITR_GAP \
			   |OUT_LFRAME_SHORT|OUT_LFRAME_PADDING \
			   |OUT_FRAME_SHORT|OUT_FRAME_PADDING)



/* statistics */

enum mam_stats { s_bitr, s_sampr, s_dlen, s_back, s_bitres, s_nmam };

struct stats {
    int min[s_nmam], max[s_nmam], sum[s_nmam], num[s_nmam];
    int fr_good, fr_bad, skip;
};



/* crc */
void crc_init(void);
int crc_frame(unsigned long h, unsigned char *data, int len);

/* id3 */
void parse_tag_v1(long pos, char *data, int in_middle);
void parse_tag_v2(long pos, unsigned char *data, int len);


#endif /* malint.h */
