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



/*
 * read scalefactors
 */

int
III_get_scale_factors_1(int *scf, struct gr_info_s *gr_info)
{
    static unsigned char slen[2][16] =
    {
	{0, 0, 0, 0, 3, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4},
	{0, 1, 2, 3, 0, 1, 2, 3, 1, 2, 3, 1, 2, 3, 2, 3}
    };
  
    int numbits;
    int num0, num1;
    int scfsi;
    
    num0 = slen[0][gr_info->scalefac_compress];
    num1 = slen[1][gr_info->scalefac_compress];

    if (gr_info->block_type == 2)
    {
	numbits = (num0 + num1) * 18;
	
	if (gr_info->mixed_block_flag)
	    numbits -= num0;	/* num0 * 17 + num1 * 18 */
    }
    else
    {
	scfsi = gr_info->scfsi;
	
	if (scfsi < 0)	{
	    /* scfsi < 0 => granule == 0 */
	    numbits = (num0 + num1) * 10 + num0;
	}
	else
	{
	    numbits = 0;
	    
	    if (!(scfsi & 0x8))
		numbits += num0 * 6;
	    
	    if (!(scfsi & 0x4))
		numbits += num0 * 5;
	    
	    if (!(scfsi & 0x2))
		numbits += num1 * 5;
	    
	    if (!(scfsi & 0x1))
		numbits += num1 * 5;
	}
	
    }
    
    return numbits;
}



int
III_get_scale_factors_2(int *scf, struct gr_info_s *gr_info, int i_stereo)
{
    unsigned char *pnt;
    int i, j;
    unsigned int slen;
    int n = 0;
    int numbits = 0;
    
    static unsigned char stab[3][6][4] =
    {
	{
	    {6, 5, 5, 5},
	    {6, 5, 7, 3},
	    {11, 10, 0, 0},
	    {7, 7, 7, 0},
	    {6, 6, 6, 3},
	    {8, 8, 5, 0}},
	{
	    {9, 9, 9, 9},
	    {9, 9, 12, 6},
	    {18, 18, 0, 0},
	    {12, 12, 12, 0},
	    {12, 9, 9, 6},
	    {15, 12, 9, 0}},
	{
	    {6, 9, 9, 9},
	    {6, 9, 12, 6},
	    {15, 18, 0, 0},
	    {6, 15, 12, 0},
	    {6, 12, 9, 6},
	    {6, 18, 9, 0}}};
    
#if 0 /* XXX: needed! copy over [in]_slen2 init */
    if (i_stereo)	/* i_stereo AND second channel
			   -> mpg123_do_layer3() checks this */
	slen = i_slen2[gr_info->scalefac_compress >> 1];
    else
	slen = n_slen2[gr_info->scalefac_compress];
#endif
    
    gr_info->preflag = (slen >> 15) & 0x1;
    
    n = 0;
    if (gr_info->block_type == 2)
    {
	n++;
	if (gr_info->mixed_block_flag)
	    n++;
    }
    
    pnt = stab[n][(slen >> 12) & 0x7];
    
    for (i = 0; i < 4; i++)
    {
	int num = slen & 0x7;
	
	slen >>= 3;
	if (num)
	{
	    for (j = 0; j < (int) (pnt[i]); j++)
		*scf++ = mpg123_getbits_fast(num);
	    numbits += pnt[i] * num;
	}
	else
	{
	    for (j = 0; j < (int) (pnt[i]); j++)
		*scf++ = 0;
	}
    }
    
    n = (n << 1) + 1;
    for (i = 0; i < n; i++)
	*scf++ = 0;
    
    return numbits;
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
