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



short III_get_side_info_1(struct sideinfo *si, int stereo,
			  int ms_stereo, long sfreq, int single);
short III_get_side_info_2(struct sideinfo *si, int stereo,
			  int ms_stereo, long sfreq, int single);

int III_get_scale_factors_1(int *scf, struct gr_info_s *gr_info);
int III_get_scale_factors_2(int *scf, struct gr_info_s *gr_info, int i_stereo);

unsigned int mpg123_getbits(int number_of_bits);
unsigned int mpg123_getbits_fast(int number_of_bits);
unsigned int mpg123_get1bit(void);
