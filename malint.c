#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "mpg123.h"

void build_length_table(int *table);
int process_file(FILE *f, char *fname);

extern int _mp3_samp_tab[2][4];
extern int _mp3_bit_tab[2][16][3];

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

#define MPEG_MODE_STEREO	0x0
#define MPEG_MODE_JSTEREO	0x1
#define MPEG_MODE_DUAL		0x2
#define MPEG_MODE_SINGLE	0x3





int table[2048];

char *prg;

/*
#define GET_LONG(x)	((((unsigned long)((x)[0]))<<24) \
                         | (((unsigned long)((x)[1]))<<16) \
                         | (((unsigned long)((x)[2]))<<8) \
		         (unsigned long)(x)[3])
*/

#define GET_LONG(x)	(((x)[0]<<24)|((x)[1]<<16)|((x)[2]<<8)|(x)[3])
#define GET_INT3(x)	(((x)[0]<<16)|((x)[1]<<8)|(x)[2])
#define GET_SHORT(x)	(((x)[0]<<8)|(x)[1])
#define GET_ID3LEN(x)	((((x)[0]&0x7f)<<21)|(((x)[1]&0x7f)<<14) \
			 |(((x)[2]&0x7f)<<7)|((x)[3]&0x7f))

static void crc_init(void);
static void crc_update(int *crc, unsigned char b);
static int crc_frame(unsigned long h, unsigned char *data, int len);
void check_l3bitres(long pos, unsigned long h, unsigned char *b, int blen,
		    int flen, int *bitresp);

void out_start(char *fname);
void out(long pos, char *fmt, ...);

char *mem2asc(char *mem, int len);
void print_header(long pos, unsigned long h);

void parse_tag_v1(long pos, char *data);
void parse_tag_v2(long pos, unsigned char *data, int len);
void parse_tag_v22(unsigned char *data, int len);



int
main(int argc, char **argv)
{
    FILE *f;
    int i, ret;

    prg = argv[0];

    build_length_table(table);
    crc_init();

    ret = 0;
    if (argc == 1)
	process_file(stdin, "stdin");
    else {
	for (i=1; i<argc; i++) {
	    if ((f=fopen(argv[i], "r")) == NULL) {
		fprintf(stderr, "%s: cannot open file `%s': %s\n",
			argv[0], argv[i], strerror(errno));
		ret = 1;
		continue;
	    }
	    
	    process_file(f, argv[i]);
	}
    }
    
    return ret;
}



int
process_file(FILE *f, char *fname)
{
    int j, n, crc_f, crc_c;
    int bitres, frlen, frback;
    long l, len;
    unsigned long h, h_old;
    unsigned char b[8192];

    out_start(fname);

    if (fseek(f, -128, SEEK_END) >= 0) {
	len = ftell(f);
	if (fread(b, 128, 1, f) == 1) {
	    if (strncmp(b, "TAG", 3) == 0) {
		len -= 128;
		parse_tag_v1(len, b);
	    }
	}
	if (fseek(f, 0, SEEK_SET) < 0) {
	    fprintf(stderr, "%s: cannot rewind %s: %s\n",
		    prg, fname, strerror(errno));
	    return -1;
	}
    }
    else
	len = -1;

    bitres = 0;
    l = 0;
    h_old = 0;
    while((len < 0 || l < len-3) && fread(b, 4, 1, f) > 0) {
	h = GET_LONG(b);
	
	if ((h&0xfff00000) == 0xfff00000) {
	    /* valid header */
	    j = table[(h&0x000fffe0)>>9];
	    if (j == 0) {
		out(l, "illegal header 0x%lx (%s)", h, mem2asc(b, 4));
		break;
	    }
	}
	else {
	    if ((h&0xffffff00) == (('T'<<24)|('A'<<16)|('G'<<8))) {
		if (fread(b+4, 124, 1, f) != 1) {
		    out(l, "ID3v1 tag (in middle of song)");
		    printf("    short tag\n");
		    break;
		}
		else
		    parse_tag_v1(l, b);
		l += 128;
		continue;
	    }
	    else if ((h&0xffffff00) == (('I'<<24)|('D'<<16)|('3'<<8))) {
		if (fread(b+4, 6, 1, f) != 1) {
		    out(l, "ID3v2.%c", (h&0xff)+'0');
		    printf("    short header\n");
		    break;
		}
		j = GET_ID3LEN(b+6);
		if (fread(b+10, j, 1, f) != 1) {
		    out(l, "ID3v2.%c", (h&0xff)+'0');
		    printf("    short tag\n");
		    break;
		}
		parse_tag_v2(l, b, j);
		l += j;
		continue;
	    }
	    else {
		/* not recognized */
		out(l, "illegal header 0x%lx (%s)", h, mem2asc(b, 4));
		/* resync? */
		break;
	    }
	}
	if (j>4) {
	    n = fread(b+4, 1, j-4, f) + 4;
	    if (len >= 0 && l+n > len)
		n = len-l;
	    if (n < j) {
		out(l, "short last frame: %d of %d bytes (%d missing)",
		    n, j, j-n);
		break;
	    }
	}
	/* read complete frame */

	if (h_old == 0)
	    print_header(l, h);
	else {
	    /* XXX: check invariants */
	    /* ignores padding, mode ext. */
	    if ((h_old & 0xfffffddf) != (h & 0xfffffddf))
		print_header(l, h);
	        /* out(l, "header change: 0x%lx -> 0x%lx", h_old, h); */
	}
	h_old = h; 

	if (MPEG_CRC(h)) {
	    crc_c = crc_frame(h, b, j);
	    crc_f = GET_SHORT(b+4);

	    if (crc_c != -1 && crc_c != crc_f)
		out(l, "CRC error (calc:%04x != file:%04x)", crc_c, crc_f);
	}

#if 0
	if (MPEG_LAYER(h) == 3) {
	    check_l3bitres(l, h, b, j, j, &bitres);
	}
#endif

	l += j;
    }

    if (ferror(f)) {
	fprintf(stderr, "%s: read error on %s: %s\n",
		prg, fname, strerror(errno));
	return -1;
    }

    return 0;
}



static char *__tags[] = {
    "TALB", "TAL", "Album",
    "TIT2", "TT2", "Title",
    "TPE1", "TP1", "Artist",
    "TPOS", "TPA", "CD",
    "TRCK", "TRK", "Track",
    "TYER", "TYE", "Year",
    NULL, NULL
};

void
parse_tag_v2(long pos, unsigned char *data, int len)
{
    char *p, *end;
    int i;

    out(pos, "ID3v2.%c.%c tag", data[3]+'0', data[4]+'0');

    if (data[5]&0x80) {
	printf("   unsynchronization not supported\n");
	return;
    }
    if (data[3] == 2)
	parse_tag_v22(data, len);
    else if (data[3] != 3) {
	printf("   unsupported version 2.%d.%d\n",
	       data[3], data[4]);
	return;
    }

    p = data + 10;

    if (data[5]&0x40) { /* extended header */
	len -= GET_LONG(data+16);
	p += GET_LONG(data+10) + 4;
    }

    end = data + len + 10;

    while (p < end) {
	if (memcmp(p, "\0\0\0\0", 4) == 0)
	    break;
	len = GET_LONG(p+4);
	if (len < 0)
	    break;
	for (i=0; __tags[i]; i+=3)
	    if (strncmp(p, __tags[i], 4) == 0) {
		if (p[10] == 0) 
		    printf("   %s:\t%.*s\n", __tags[i+2], len-1, p+11);
		break;
	    }
	p += len + 10;
    }
}



void
parse_tag_v22(unsigned char *data, int len)
{
    char *p, *end;
    int i;

    if (data[5] & 0x40) {
	printf("    version 2.2 compression not supported\n");
	return;
    }

    p = data + 10;

    end = data + len + 10;

    while (p < end) {
	if (memcmp(p, "\0\0\0", 3) == 0)
	    break;
	len = GET_INT3(p+3);
	if (len < 0)
	    break;
	for (i=0; __tags[i]; i+=3)
	    if (strncmp(p, __tags[i+1], 3) == 0) {
		if (p[6] == 0) 
		    printf("   %s:\t%.*s\n", __tags[i+2], len-1, p+7);
		break;
	    }
	p += len + 6;
    }
}



static int
field_len(char *data, int len)
{
    int l;

    for (l=0; l<len && data[l]; l++)
	;

    if (l==len) { /* space padding */
	for (; data[l-1] == ' '; --l)
	    ;
    }

    return l;
}



void
parse_tag_v1(long pos, char *data)
{
    static struct {
	char *name;
	int start, len;
    } field[] = {
	{ "Artist", 33, 30 },
	{ "Title",   3, 30 },
	{ "Album",  63, 30 },
	{ "Year",   93,  4 },
	{ NULL,      0,  0 }
    };

    int v11, i, len;

    v11 = data[126] && data[125] == 0;

    out(pos, "ID3v1%s tag", v11 ? ".1" : "");

    for (i=0; field[i].name; i++) {
	len = field_len(data+field[i].start, field[i].len);
	if (len > 0) {
	    printf("   %s:\t%.*s\n",
		   field[i].name, len, data+field[i].start);
	}
    }
    if (v11)
	printf("   Track:\t%d\n", data[126]);
}



static char *out_fname;
static int out_fname_done;

void
out_start(char *fname)
{
    out_fname = fname;
    out_fname_done = 0;
}

void
out(long pos, char *fmt, ...)
{
    va_list argp;

    if (!out_fname_done) {
	printf("%s:\n", out_fname);
	out_fname_done = 1;
    }

    printf(" at %8ld: ", pos);

    va_start(argp, fmt);
    vprintf(fmt, argp);
    va_end(argp);
    putc('\n', stdout);
}



#define MPEG_CRCPOLY	0x18005

static int crc_tab[256];



static void
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
crc_update(int *crc, unsigned char b)
{
    *crc = crc_tab[(*crc>>8)^b] ^ ((*crc<<8)&0xffff);
}



static int
crc_frame(unsigned long h, unsigned char *data, int len)
{
    int i, crc, s;

    if (MPEG_LAYER(h) != 3) {
	/* mp3check only supports layer 3.  get docu somewhere else */
	return -1;
    }

    crc = 0xffff;

    crc_update(&crc, data[2]);
    crc_update(&crc, data[3]);

    if (MPEG_VERSION(h) == 1) {
	if (MPEG_MODE(h) == MPEG_MODE_SINGLE)
	    s = 17;
	else
	    s = 32;
    }
    else {
	if (MPEG_MODE(h) == MPEG_MODE_SINGLE)
	    s = 9;
	else
	    s = 17;
    }

    for(i=0; i < s; i++)
	crc_update(&crc, data[i+6]);
    
    return crc;
}



char *
mem2asc(char *mem, int len)
{
    static char asc[1025];

    int i;

    if (len > 1024)
	len = 1024;

    for (i=0; i<len; i++)
	if (isprint(mem[i]))
	    asc[i] = mem[i];
	else
	    asc[i] = '.';

    asc[len] = '\0';

    return asc;
}



void
print_header(long pos, unsigned long h)
{
    static char *mode[] = {
	"stereo", "j-stereo", "dual-ch", "mono"
    };
    static char *emph[] = {
	"no emphasis", "50/15 micro seconds", "", "CCITT J.17"
    };

    out(pos, "MPEG %d layer %d%s, %dkbps, %dkHz, %s%s%s%s%s%s",
	MPEG_VERSION(h), MPEG_LAYER(h),
	MPEG_CRC(h) ? ", crc" : "",
	MPEG_BITRATE(h), MPEG_SAMPFREQ(h)/1000,
	MPEG_PRIV(h)? ", priv" : "",
	mode[MPEG_MODE(h)],
	MPEG_COPY(h) ? ", copyright" : "",
	MPEG_ORIG(h) ? ", original" : "",
	MPEG_EMPH(h) ? ", " : "",
	MPEG_EMPH(h) ? emph[MPEG_EMPH(h)] : "");
}



void
check_l3bitres(long pos, unsigned long h, unsigned char *b, int blen,
	       int flen, int *bitresp)
{
    struct sideinfo si;

    unsigned char *sip;
    int hlen, back, dlen;

    hlen = (4 + MPEG_CRC(h)*2
	    + (MPEG_VERSION(h)==2 ? (MPEG_MODE(h) == MPEG_MODE_SINGLE ? 9 : 17)
	       : (MPEG_MODE(h) == MPEG_MODE_SINGLE ? 17 : 32)));

    if (get_sideinfo(&si, h, b, blen) < 0) {
	out(pos, "cannot parse sideinfo");
	return;
    }
	
    back = si.main_data_begin;
    dlen = get_frame_data_len(h, &si);
    
    if (back > *bitresp)
	out(pos, "main_data_begin overflows bit reservoir (%d > %d)",
	    back, *bitresp);
    
    if (dlen-back > flen-hlen)
	out(pos, "frame data overflows frame (%d > %d)",
	    dlen-back, flen-hlen);

    if (back != 511 && back != 0 && back < *bitresp
	&& flen-hlen-dlen+back < 511)
	out(pos, "gap in bit stream (%d < %d)", back, *bitresp);

    out(pos, "debug: bitres=%d, back=%d, dlen=%d\n", *bitresp, back, dlen);

    *bitresp = flen-hlen-dlen+back;
    if (*bitresp > 511)
	*bitresp = 511;
}



int 
get_sideinfo(struct sideinfo *si, unsigned long h, unsigned char *b, int blen)
{
    int ms_stereo, stereo, sfreq;

    wordpointer = b + 4 + MPEG_CRC(h)*2;
    bitindex = 0;

    if (MPEG_MODE(h) == MPEG_MODE_JSTEREO)
	ms_stereo = MPEG_MODEEXT(h) & 0x2;
    else
	ms_stereo = 0;
    stereo = (MPEG_MODE(h) == MPEG_MODE_SINGLE ? 1 : 2);

    if (MPEG_VERSION(h) == 2)
	return III_get_side_info_2(si, stereo, ms_stereo, sfreq, 0);
    else 
	return III_get_side_info_1(si, stereo, ms_stereo, sfreq, 0);
}



int
get_frame_data_len(unsigned long h, struct sideinfo *si)
{
    int i, j, granules, n, stereo;

    stereo = (MPEG_MODE(h) == MPEG_MODE_SINGLE ? 1 : 2);
    granules = (MPEG_VERSION(h) == 2 ? 1 : 2);

    n = 0;
    for (i=0; i<granules; i++) {
	for (j=0; j<stereo; j++) {
	    if (MPEG_VERSION(h) == 2)
		n += III_get_scale_factors_2(NULL, &(si->ch[j].gr[i]),
					     0);
	else
	    n += III_get_scale_factors_1(NULL, &(si->ch[j].gr[i]));
	}
    }

    return (n+7)/8;
}
    
