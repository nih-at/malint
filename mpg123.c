/*
  mpg123.c -- mpeg structure parsing routines

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



#include "mpeg.h"
#include "mpg123.h"

int bitindex;
unsigned char *wordpointer;



short
III_get_side_info_1(struct sideinfo *si, int stereo,
		    int ms_stereo, long sfreq, int single)
{
    int ch, gr;
#if 0
    int powdiff = (single == 3) ? 4 : 0;
#endif

    si->main_data_begin = mpg123_getbits(9);
    if (stereo == 1)
	si->private_bits = mpg123_getbits_fast(5);
    else
	si->private_bits = mpg123_getbits_fast(3);

    for (ch = 0; ch < stereo; ch++)
    {
	si->ch[ch].gr[0].scfsi = -1;
	si->ch[ch].gr[1].scfsi = mpg123_getbits_fast(4);
    }

    for (gr = 0; gr < 2; gr++)
    {
	for (ch = 0; ch < stereo; ch++)
	{
	    struct gr_info_s *gr_info = &(si->ch[ch].gr[gr]);
	    
	    gr_info->part2_3_length = mpg123_getbits(12);
	    gr_info->big_values = mpg123_getbits_fast(9);
	    if (gr_info->big_values > 288)
	    {
		return 0;
	    }
#if 0
	    gr_info->pow2gain = gainpow2 + 256 - mpg123_getbits_fast(8) + powdiff;
	    if (ms_stereo)
		gr_info->pow2gain += 2;
#else
	    mpg123_getbits_fast(8);
#endif
	    gr_info->scalefac_compress = mpg123_getbits_fast(4);
/* window-switching flag == 1 for block_Type != 0 .. and block-type == 0 -> win-sw-flag = 0 */
	    if (mpg123_get1bit())
	    {
		int i;
		
		gr_info->block_type = mpg123_getbits_fast(2);
		gr_info->mixed_block_flag = mpg123_get1bit();
		gr_info->table_select[0] = mpg123_getbits_fast(5);
		gr_info->table_select[1] = mpg123_getbits_fast(5);
		/*
		 * table_select[2] not needed, because there is no region2,
		 * but to satisfy some verifications tools we set it either.
		 */
		gr_info->table_select[2] = 0;
		for (i = 0; i < 3; i++)
#if 0
		    gr_info->full_gain[i] = gr_info->pow2gain + (mpg123_getbits_fast(3) << 3);
#else
		mpg123_getbits_fast(3);
#endif
		
		if (gr_info->block_type == 0)
		    return 0;
				/* region_count/start parameters are implicit in this case. */
		gr_info->region1start = 36 >> 1;
		gr_info->region2start = 576 >> 1;
	    }
	    else
	    {
		int i, r0c, r1c;
		
		for (i = 0; i < 3; i++)
		    gr_info->table_select[i] = mpg123_getbits_fast(5);
		r0c = mpg123_getbits_fast(4);
		r1c = mpg123_getbits_fast(3);
#if 0
		gr_info->region1start = bandInfo[sfreq].longIdx[r0c + 1] >> 1;
		if(r0c + r1c + 2 > 22)
		    gr_info->region2start = 576 >> 1;
		else
		    gr_info->region2start = bandInfo[sfreq].longIdx[r0c + 1 + r1c + 1] >> 1;
#endif
		gr_info->block_type = 0;
		gr_info->mixed_block_flag = 0;
	    }
	    gr_info->preflag = mpg123_get1bit();
	    gr_info->scalefac_scale = mpg123_get1bit();
	    gr_info->count1table_select = mpg123_get1bit();
	}
    }
    return 1;
}



/*
 * Side Info for MPEG 2.0 / LSF
 */

short
III_get_side_info_2(struct sideinfo *si, int stereo,
		    int ms_stereo, long sfreq, int single)
{
    int ch;
#if 0
    int powdiff = (single == 3) ? 4 : 0;
#endif
    
    si->main_data_begin = mpg123_getbits(8);
    if (stereo == 1)
	si->private_bits = mpg123_get1bit();
    else
	si->private_bits = mpg123_getbits_fast(2);
    
    for (ch = 0; ch < stereo; ch++)
    {
	register struct gr_info_s *gr_info = &(si->ch[ch].gr[0]);
	
	gr_info->part2_3_length = mpg123_getbits(12);
	gr_info->big_values = mpg123_getbits_fast(9);
	if (gr_info->big_values > 288)
	{
	    return 0;
	}
#if 0
	gr_info->pow2gain = gainpow2 + 256 - mpg123_getbits_fast(8) + powdiff;
	if (ms_stereo)
	    gr_info->pow2gain += 2;
#else
	mpg123_getbits_fast(8);
#endif
	gr_info->scalefac_compress = mpg123_getbits(9);
/* window-switching flag == 1 for block_Type != 0 .. and block-type == 0 -> win-sw-flag = 0 */
	if (mpg123_get1bit())
	{
	    int i;
	    
	    gr_info->block_type = mpg123_getbits_fast(2);
	    gr_info->mixed_block_flag = mpg123_get1bit();
	    gr_info->table_select[0] = mpg123_getbits_fast(5);
	    gr_info->table_select[1] = mpg123_getbits_fast(5);
	    /*
	     * table_select[2] not needed, because there is no region2,
	     * but to satisfy some verifications tools we set it either.
	     */
	    gr_info->table_select[2] = 0;
	    for (i = 0; i < 3; i++)
#if 0
		gr_info->full_gain[i] = gr_info->pow2gain + (mpg123_getbits_fast(3) << 3);
#else
	    mpg123_getbits_fast(3);
#endif
	    
	    if (gr_info->block_type == 0)
		return 0;
	    /* region_count/start parameters are implicit in this case. */
/* check this again! */
	    if (gr_info->block_type == 2)
		gr_info->region1start = 36 >> 1;
	    else if (sfreq == 8)
/* check this for 2.5 and sfreq=8 */
		gr_info->region1start = 108 >> 1;
	    else
		gr_info->region1start = 54 >> 1;
	    gr_info->region2start = 576 >> 1;
	}
	else
	{
	    int i, r0c, r1c;
	    
	    for (i = 0; i < 3; i++)
		gr_info->table_select[i] = mpg123_getbits_fast(5);
	    r0c = mpg123_getbits_fast(4);
	    r1c = mpg123_getbits_fast(3);
#if 0
	    gr_info->region1start = bandInfo[sfreq].longIdx[r0c + 1] >> 1;
	    if(r0c + r1c + 2 > 22)
		gr_info->region2start = 576 >> 1;
	    else
		gr_info->region2start = bandInfo[sfreq].longIdx[r0c + 1 + r1c + 1] >> 1;
#endif
	    gr_info->block_type = 0;
	    gr_info->mixed_block_flag = 0;
	}
	gr_info->scalefac_scale = mpg123_get1bit();
	gr_info->count1table_select = mpg123_get1bit();
    }
    return 1;
}



unsigned int
mpg123_getbits(int number_of_bits)
{
    unsigned long rval;
    
    if (!number_of_bits)
	return 0;
    
    {
	rval = wordpointer[0];
	rval <<= 8;
	rval |= wordpointer[1];
	rval <<= 8;
	rval |= wordpointer[2];
	rval <<= bitindex;
	rval &= 0xffffff;
	
	bitindex += number_of_bits;
	
	rval >>= (24 - number_of_bits);
	
	wordpointer += (bitindex >> 3);
	bitindex &= 7;
    }

    return rval;
}



unsigned int
mpg123_getbits_fast(int number_of_bits)
{
    unsigned long rval;

    {
	rval = wordpointer[0];
	rval <<= 8;
	rval |= wordpointer[1];
	rval <<= bitindex;
	rval &= 0xffff;
	bitindex += number_of_bits;
	
	rval >>= (16 - number_of_bits);
	
	wordpointer += (bitindex >> 3);
	bitindex &= 7;
    }

    return rval;
}



unsigned
int mpg123_get1bit(void)
{
    unsigned char rval;

    rval = *wordpointer << bitindex;

    bitindex++;
    wordpointer += (bitindex >> 3);
    bitindex &= 7;
    
    return rval >> 7;
}



int
I_get_bit_alloc(unsigned long h, int *balloc)
{
    int *ba = balloc;
    int i, jsbound;
    
    if (MPEG_MODE(h) != MPEG_MODE_SINGLE) {
	jsbound = MPEG_JSBOUND(h);
	
	for (i=0; i<jsbound; i++) {
	    *ba++ = mpg123_getbits_fast(4);
	    *ba++ = mpg123_getbits_fast(4);
	}
	for (i=jsbound; i<MPEG_SBLIMIT; i++)
	    *ba++ = mpg123_getbits_fast(4);
    }
    else
	for (i=0; i<MPEG_SBLIMIT; i++)
	    *ba++ = mpg123_getbits_fast(4);

    return ba-balloc;
}
