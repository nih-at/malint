#ifndef HAD_MPG123_H
#define HAD_MPG123_H

/*
  mpg123.h -- mpeg structure definitions and parsing function prototypes

  Copyright (C) 1995,1996,1997 by Michael Hipp.
  Changes Copyright (C) 2000 Dieter Baron

  The routines in this file were taken from xmms's mpg123 lib, adapted
  for use in malint by Dieter Baron.
  The original author can be contacted at <hippm@informatik.uni-tuebingen.de>

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



struct gr_info_s
{
	int scfsi;
	unsigned part2_3_length;
	unsigned big_values;
	unsigned scalefac_compress;
	unsigned block_type;
	unsigned mixed_block_flag;
	unsigned table_select[3];
	unsigned subblock_gain[3];
	unsigned maxband[3];
	unsigned maxbandl;
	unsigned maxb;
	unsigned region1start;
	unsigned region2start;
	unsigned preflag;
	unsigned scalefac_scale;
	unsigned count1table_select;
#if 0
	real *full_gain[3];
	real *pow2gain;
#endif
};

struct sideinfo
{
	unsigned main_data_begin;
	unsigned private_bits;
	struct
	{
		struct gr_info_s gr[2];
	}
	ch[2];
};



extern int bitindex;
extern unsigned char *wordpointer;



int I_get_bit_alloc(unsigned long h, int *balloc);

short III_get_side_info_1(struct sideinfo *si, int stereo,
			  int ms_stereo, long sfreq, int single);
short III_get_side_info_2(struct sideinfo *si, int stereo,
			  int ms_stereo, long sfreq, int single);

int III_get_scale_factors_1(int *scf, struct gr_info_s *gr_info);
int III_get_scale_factors_2(int *scf, struct gr_info_s *gr_info, int i_stereo);

unsigned int mpg123_getbits(int number_of_bits);
unsigned int mpg123_getbits_fast(int number_of_bits);
unsigned int mpg123_get1bit(void);

#endif /* mpg123.h */
