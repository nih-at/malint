/*
  malint.c -- main file
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



#include <errno.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "malint.h"
#include "mpg123.h"
#include "inbuf.h"
#include "mpeg.h"
#include "vbr.h"



static char version_out[] = "MPEG Audio lint (" PACKAGE ") " VERSION "\n\
Copyright (C) 2000 Dieter Baron\n"
PACKAGE " comes with ABSOLUTELY NO WARRANTY, to the extent permitted by law.\n\
You may redistribute copies of\n"
PACKAGE " under the terms of the GNU General Public License.\n\
For more information about these matters, see the files named COPYING.\n";

static char help_head[] = "MPEG Audio lint (" PACKAGE ") " VERSION
" by Dieter Baron <dillo@giga.or.at>\n\n";

#define OPTIONS	"hVIEcCpPgGdDN:tTfF"

#define OP_NOTC	256
#define OP_TC	257

struct option options[] = {
    { "help",             0, 0, 'h'     },
    { "version",          0, 0, 'V'     },
    { "fast-info",        0, 0, 'I'     },
    { "error",            0, 0, 'E'     },
    { "no-crc",           0, 0, 'C'     },
    { "crc",              0, 0, 'c'     },
    { "padding",          0, 0, 'p'     },
    { "no-padding",       0, 0, 'P'     },
    { "gap",              0, 0, 'g'     },
    { "no-gap",           0, 0, 'G'     },
    { "duration",         0, 0, 'd'     },
    { "no-duration",      0, 0, 'D'     },
    { "resync-count",     1, 0, 'N'     },
    { "tag-contents",     0, 0, OP_TC   },
    { "no-tag-contents",  0, 0, OP_NOTC },
    { "tag",              0, 0, 't'     },
    { "no-tag",           0, 0, 'T',    },
    { "header-change",    0, 0, 'f',    },
    { "no-header-change", 0, 0, 'F',    },
    { NULL,               0, 0, 0       }
};

static char usage[] =
         "usage: %s [-hV] [-N MIN] [-IEcCpPgGdDtTfF] [FILE ...]\n";

static char help_tail[] = "\n\
  -h, --help               display this help message\n\
  -V, --version            display version number\n\
  -I, --fast-info          display only info, do not parse whole file\n\
  -E, --error              display only error diagnostics\n\
  -c, --crc                check CRC\n\
  -C, --no-crc             do not check CRC\n\
  -p, --padding            check for missing padding in last frame\n\
  -P, --no-padding         do not check for missing padding in last frame\n\
  -g, --gap                check for unused bytes in bit reservoir\n\
  -G, --no-gap             do not check for unused bytes in bit reservoir\n\
  -f, --header-change      display changes in frame header fields\n\
  -F, --no-header-change   do not display changes in frame header fields\n\
  -d, --duration           display duration of song\n\
  -D, --no-duration        do not display duration of song\n\
  -t, --tag                display ID3 tags\n\
  -T, --no-tag             do not display ID3 tags\n\
      --tag-contents       display contents of ID3 tags\n\
      --no-tag-contents    do not display contents of ID3 tags\n\
  -N, --resync-count N     require at least N frames to accept resync\n\
\n\
Report bugs to <dillo@giga.or.at>.\n";



char *prg;
int output;		/* bit mask of messages to output */
int min_consec;		/* number of consecutive headers to accept a resync */



int process_file(FILE *f, char *fname);
int resync(long *lp, unsigned long *hp, struct inbuf *ib,
	   int inframe, int maxtry);

int check_l1(long pos, unsigned long h, unsigned char *b, int blen, int flen);
int check_l3(long pos, unsigned long h, unsigned char *b, int blen,
	     int flen, int *bitresp, int taginbitres);
int get_l1_bit_alloc(int *balloc, unsigned long h,
			 unsigned char *b, int blen);
static void warn_short_frame(long l, int dlen, int flen, int blen, int eof);


void out_start(char *fname);
void out(long pos, char *fmt, ...);

char *mem2asc(char *mem, int len);
char *ulong2asc(unsigned long);
void print_header(long pos, unsigned long h, int vbrkbps);



int
main(int argc, char **argv)
{
    FILE *f;
    int i, ret, c;

    prg = argv[0];

    build_length_table();
    crc_init();

    output = 0xfffffff & ~OUT_FASTINFO_ONLY;
    min_consec = 6;

    opterr = 0;
    while ((c=getopt_long(argc, argv, OPTIONS, options, 0)) != EOF) {
	switch (c) {
	case 'I':
	    output = (OUT_FASTINFO_ONLY|OUT_TAG|OUT_TAG_CONTENTS|OUT_HEAD_1ST
		      |OUT_PLAYTIME);
	    break;
	case 'E':
	    output = OUT_M_ERROR;
	    break;
	case 'c':
	    output |= OUT_CRC_ERROR;
	    break;
	case 'C':
	    output &= ~OUT_CRC_ERROR;
	    break;
	case 'p':
	    output |= OUT_LFRAME_PADDING;
	    break;
	case 'P':
	    output &= ~OUT_LFRAME_PADDING;
	    break;
	case 'g':
	    output |= OUT_BITR_GAP;
	    break;
	case 'G':
	    output &= ~OUT_BITR_GAP;
	    break;
	case 'd':
	    output |= OUT_PLAYTIME;
	    break;
	case 'D':
	    output &= ~OUT_PLAYTIME;
	    break;
	case OP_TC:
	    output |= OUT_TAG_CONTENTS;
	    break;
	case OP_NOTC:
	    output &= ~OUT_TAG_CONTENTS;
	    break;
	case 't':
	    output |= OUT_TAG;
	    break;
	case 'T':
	    output &= ~OUT_TAG;
	    break;
	case 'f':
	    output |= OUT_HEAD_CHANGE;
	    break;
	case 'F':
	    output &= ~OUT_HEAD_CHANGE;
	    break;

	case 'N':
	    min_consec = atoi(optarg);
	    break;
	    
	case 'V':
	    fputs(version_out, stdout);
	    exit(0);
	case 'h':
	    fputs(help_head, stdout);
	    printf(usage, prg);
	    fputs(help_tail, stdout);
	    exit(0);
	case '?':
	    fprintf(stderr, usage, prg);
	    exit(1);
	}
    }

    ret = 0;
    if (optind == argc)
	process_file(stdin, "stdin");
    else {
	for (i=optind; i<argc; i++) {
	    if ((f=fopen(argv[i], "r")) == NULL) {
		fprintf(stderr, "%s: cannot open file `%s': %s\n",
			argv[0], argv[i], strerror(errno));
		ret = 1;
		continue;
	    }
	    
	    process_file(f, argv[i]);

	    fclose(f);
	}
    }
    
    return ret;
}



int
process_file(FILE *f, char *fname)
{
    struct vbr *vbr;
    struct inbuf *ib;
    int endtag_found;
    int dlen, flen, n, crc_f, crc_c, i;
    int taginbitres, bitres, frlen, frback, eof;
    long l, lresync, len, nframes, bitr;
    unsigned long h, h_old, h_next, h_change_mask;
    unsigned char b[130], *p;

    out_start(fname);
    endtag_found = 0;

    if (fseek(f, -128, SEEK_END) >= 0) {
	len = ftell(f);
	if (fread(b, 128, 1, f) == 1) {
	    if (strncmp(b, "TAG", 3) == 0) {
		endtag_found = 1;
		if (output & OUT_M_TAG)
		    parse_tag_v1(len, b, 0);
	    }
	    else
		len += 128;
	}
	if (fseek(f, 0, SEEK_SET) < 0) {
	    fprintf(stderr, "%s: cannot rewind %s: %s\n",
		    prg, fname, strerror(errno));
	    return -1;
	}
    }
    else
	len = -1;

    ib = inbuf_new(f, len);

    vbr = NULL;
    bitr = nframes = 0;
    taginbitres = bitres = 0;
    l = 0;
    h_old = 0;
    h_change_mask = 0xfffffdcf;
    eof = 0;

    while (inbuf_getlong(&h, l, ib) >= 0) {
	if (IS_MPEG(h)) {
	    n = flen = MPEG_FRLEN(h);
	    inbuf_keep(l, ib);
	    h_next = 0;
	    if (inbuf_getlong(&h_next, l+flen, ib) < 0)
		eof = 1;
	    if (!IS_VALID(h_next)) {
		lresync = l;
		h_next = h;
		if (resync(&lresync, &h_next, ib, 1, flen) >= 0)
		    n = ((lresync-l) < flen ? (lresync-l) : flen);
		if (IS_ID3v1(h_next)) {
		    if (inbuf_getc(lresync+128, ib) == -1)
			eof = 1;
		}
	    }
	    inbuf_unkeep(ib);
	    n = inbuf_copy(&p, l, n, ib);

	    if (MPEG_LAYER(h) == 3 && IS_XING(GET_LONG(p+4+MPEG_SILEN(h)))) {
		vbr = vbr_parse(l, p+4+MPEG_SILEN(h), n-4-MPEG_SILEN(h));
		h_change_mask = 0xffff0dcf;
		if (vbr && vbr->flags & VBR_TOC) {
		    for (i=1; i<100; i++)
			if (vbr->toc[i] <= vbr->toc[i-1]) {
			    out(l, "vbr toc not strictly increasing");
			    break;
			}
		}
	    }
	    else {
		if (h_old == 0) {
		    if ((output & OUT_HEAD_1ST)
			&& (!vbr || (output & OUT_FASTINFO_ONLY))) {
			if (vbr) {
			    if (vbr->flags & VBR_FRAMES|VBR_BYTES ==
				VBR_FRAMES|VBR_BYTES)
				bitr = (vbr->bytes)
				    /((125*vbr->frames*MPEG_NSAMP(h))
				      /MPEG_SAMPFREQ(h));
			    else
				bitr = 0;
			}
			else
			    bitr = -1;
			print_header(l, h, bitr);
		    }
		    if (output & OUT_FASTINFO_ONLY) {
			h_old = h;
			break;
		    }
		}
		else if (output & OUT_HEAD_CHANGE) {
		    /* XXX: check invariants */
		    /* ignores padding, mode ext. */
		    if ((h_old & h_change_mask) != (h & h_change_mask))
			print_header(l, h, -1);
		    /* out(l, "header change: 0x%08lx -> 0x%08lx", h_old, h);*/
		}
		h_old = h; 
		
		if ((output & OUT_CRC_ERROR) && MPEG_CRC(h)) {
		    crc_c = crc_frame(h, p, n);
		    crc_f = GET_SHORT(p+4);
		    
		    if (crc_c != -1 && crc_c != crc_f)
			out(l, "CRC error (calc:%04x != file:%04x)",
			    crc_c, crc_f);
		}
		
		if (output & (OUT_M_CHECK_FRAME)) {
		    switch (MPEG_LAYER(h)) {
		    case 1:
			dlen = check_l1(l, h, p, n, flen);
			bitres = 0;
			break;
		    case 2:
			dlen = flen;
			bitres = 0;
			break;
		    case 3:
			dlen = check_l3(l, h, p, n, flen,
					&bitres, taginbitres);
			break;
		    }
		    taginbitres = 0;

		    if (dlen > flen && (output & OUT_BITR_FRAME_OVER))
			out(l, "frame data overflows frame (%d > %d)",
			    dlen, flen);
		    
		    if (n != flen)
			warn_short_frame(l, dlen, flen, n, eof);
		}
#if 0
		else if (n < j && (output & OUT_LFRAME_SHORT)) {
		    /* XXX: check for EOF */
		    out(l, "short frame: %d of %d bytes (%d missing)",
			n, j, j-n);
		}
#endif

		if (vbr)
		    bitr += MPEG_BITRATE(h);
		nframes++;
	    }
	    l += n;
	}
	else {
	    if (IS_ID3v1(h)) {
		taginbitres = 1;
		if (inbuf_copy(&p, l, 128, ib) != 128) {
		    if (output & OUT_TAG_SHORT) {
			out(l, "ID3v1 tag");
			printf("    short tag\n");
		    }
		    break;
		}
		else
		    if (output & OUT_M_TAG) {
			parse_tag_v1(l, p,
				     endtag_found
				     || (inbuf_getc(l+128, ib) != -1));
		    }
		l += 128;
		continue;
	    }
	    else if (IS_ID3v2(h)) {
		taginbitres = 1;
		inbuf_keep(l, ib);
		if (inbuf_getc(l+10, ib) < 0) {
		    if (output & OUT_TAG_SHORT) {
			out(l, "ID3v2.%c", (h&0xff)+'0');
			printf("    short header\n");
		    }
		    break;
		}
		inbuf_getlong(&h_next, l+6, ib);
		inbuf_unkeep(ib);
		n = LONG_TO_ID3LEN(h_next) + 10;
		if (inbuf_copy(&p, l, n, ib) != n) {
		    if (output & OUT_TAG_SHORT) {
			out(l, "ID3v2.%c.%c", (h&0xff)+'0', (h_next>>24)+'0');
			printf("    short tag\n");
		    }
		    break;
		}
		if (output & OUT_M_TAG)
		    parse_tag_v2(l, p, n);
		l += n;
		continue;
	    }
	    else {
		/* no sync */
		if (output & OUT_HEAD_ILLEGAL)
		    out(l, "illegal header 0x%08lx (%s)", h, ulong2asc(h));
		lresync = l;
		if (resync(&l, &h, ib, 0, 0) < 0)
		    break;
		bitres += l-lresync;
	    }
	}
    }

    if ((h_old && vbr
	 && (output & OUT_HEAD_1ST) && !(output & OUT_FASTINFO_ONLY))) {
	bitr /= nframes;
	print_header(l, h_old, bitr);
    }

    if (h_old && (output & OUT_PLAYTIME)) {
	if (!(output & OUT_FASTINFO_ONLY)) {
	    /* XXX: only works if sampfreq doesn't change during song */
	    len = (nframes*MPEG_NSAMP(h_old))/MPEG_SAMPFREQ(h_old);
	    out(l, "play time: %02d:%02d:%02d (%ld frames)",
		len/3600, (len/60)%60, len%60, nframes);
	}
	else if (vbr) {
	    if (vbr->flags & VBR_FRAMES) {
		len = (vbr->frames*MPEG_NSAMP(h_old))/MPEG_SAMPFREQ(h_old);
		out(l, "play time: %02d:%02d:%02d (according to vbr tag)",
		    len/3600, (len/60)%60, len%60);
	    }
	}
	else if (len != -1) {
	    len -= l;
	    len /= 125*MPEG_BITRATE(h_old);
	    out(l, "play time: %02d:%02d:%02d (aproximately)",
		len/3600, (len/60)%60, len%60);
	}
    }	    

    if (ferror(f)) {
	fprintf(stderr, "%s: read error on %s: %s\n",
		prg, fname, strerror(errno));
	inbuf_free(ib);
	return -1;
    }

    inbuf_free(ib);
    return 0;
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



char *
mem2asc(char *mem, int len)
{
    static char asc[1025];

    int i;

    if (len > 1024)
	len = 1024;

    for (i=0; i<len; i++)
	/* XXX: NetBSD's isprint returns true for extended control chars */
	if (isprint(mem[i]) && isascii(mem[i]))
	    asc[i] = mem[i];
	else
	    asc[i] = '.';

    asc[len] = '\0';

    return asc;
}



char *
ulong2asc(unsigned long l)
{
    static char asc[5];

    int i, c;

    for (i=0; i<4; i++) {
	c = (l>>(24-i*8)) & 0xff;
	/* XXX: NetBSD's isprint returns true for extended control chars */
	if (isprint(c) && isascii(c))
	    asc[i] = c;
	else
	    asc[i] = '.';
    }

    asc[4] = '\0';

    return asc;
}



void
print_header(long pos, unsigned long h, int vbrkbps)
{
    static char *version[] = {
	"reserved", "1", "2", "reserved", "2.5"
    };
    static char *mode[] = {
	"stereo", "j-stereo", "dual-ch", "mono"
    };
    static char *emph[] = {
	"no emphasis", "50/15 micro seconds", "reserved", "CCITT J.17"
    };

    out(pos, "MPEG %s layer %d%s, %dkbps%s, %dkHz, %s%s%s%s%s%s",
	version[MPEG_VERSION(h)], MPEG_LAYER(h),
	MPEG_CRC(h) ? ", crc" : "",
	(vbrkbps == -1 ? MPEG_BITRATE(h) : vbrkbps),
	(vbrkbps == -1 ? "" : " vbr"),
	MPEG_SAMPFREQ(h)/1000,
	MPEG_PRIV(h)? "priv, " : "",
	mode[MPEG_MODE(h)],
	MPEG_COPY(h) ? ", copyright" : "",
	MPEG_ORIG(h) ? ", original" : "",
	MPEG_EMPH(h) ? ", " : "",
	MPEG_EMPH(h) ? emph[MPEG_EMPH(h)] : "");
}



int
check_l3(long pos, unsigned long h, unsigned char *b, int blen,
	 int flen, int *bitresp, int taginbitres)
{
    struct sideinfo si;

    unsigned char *sip;
    int hlen, back, dlen, this_len, next_bitres, max_back;

    hlen = 4 + MPEG_CRC(h)*2 + MPEG_SILEN(h);
    max_back = MPEG_VERSION(h) == 1 ? 511 : 255;

    if (blen < hlen) {
	/* XXX: bitresp? */
	return flen;
    }

    if (get_sideinfo(&si, h, b, blen) < 0) {
	out(pos, "cannot parse sideinfo");
	/* XXX: bitresp? */
	return flen;
    }
	
    back = si.main_data_begin;
    if (MPEG_VERSION(h) == 1)
	dlen = (si.ch[0].gr[0].part2_3_length+si.ch[0].gr[1].part2_3_length
		+ ((MPEG_MODE(h) == MPEG_MODE_SINGLE) ? 0
		   : (si.ch[1].gr[0].part2_3_length
		      +si.ch[1].gr[1].part2_3_length)));
    else
	dlen = (si.ch[0].gr[0].part2_3_length
		+ ((MPEG_MODE(h) == MPEG_MODE_SINGLE) ? 0
		   : si.ch[1].gr[0].part2_3_length));
    dlen = (dlen+7)/8;
    this_len = (dlen < back) ? hlen : hlen+dlen-back;
    next_bitres = flen-hlen-dlen+back;
    if (next_bitres > max_back)
	next_bitres = max_back;

    if (back && taginbitres && (output & OUT_BITR_TAGIN))
	out(pos, "bit resrvoir spans across ID3 tag");

    if (back > *bitresp && (output & OUT_BITR_OVERFLOW))
	out(pos, "main_data_begin overflows bit reservoir (%d > %d)",
	    back, *bitresp);
    
    if (back != max_back && back != 0 && back < *bitresp
	&& next_bitres < max_back && (output & OUT_BITR_GAP))
	out(pos, "gap in bit stream (%d < %d)", back, *bitresp);

#if 0
    out(pos, "debug: bitres=%d, back=%d, dlen=%d, this_len=%d, flen=%d\n",
	*bitresp, back, dlen, this_len, flen);
#endif

    *bitresp = next_bitres;

    return this_len;
}



int 
get_sideinfo(struct sideinfo *si, unsigned long h, unsigned char *b, int blen)
{
    int ms_stereo, stereo;

    wordpointer = b + 4 + MPEG_CRC(h)*2;
    bitindex = 0;

    if (MPEG_MODE(h) == MPEG_MODE_JSTEREO)
	ms_stereo = MPEG_MODEEXT(h) & 0x2;
    else
	ms_stereo = 0;
    stereo = (MPEG_MODE(h) == MPEG_MODE_SINGLE ? 1 : 2);

    if (MPEG_VERSION(h) == 1)
	return III_get_side_info_1(si, stereo, ms_stereo,
				   MPEG_SAMPFREQ_R(h), 0);
    else 
	return III_get_side_info_2(si, stereo, ms_stereo,
				   MPEG_SAMPFREQ_R(h), 0);
}
    


#define MAX_SKIP  65536

int
resync(long *lp, unsigned long *hp, struct inbuf *ib, int inframe, int maxtry)
{
    unsigned long h, h_next;
    long l, try, l2;
    int c, j, i, valid;

    l = *lp;
    h = *hp;

    if (!maxtry)
	maxtry = MAX_SKIP;

    for (try=1; (c=inbuf_getc(l+try+3, ib))>=0 && try<maxtry; try++) {
	h = ((h<<8)|(c&0xff)) & 0xffffffff;

	if (IS_VALID(h)) {
	    if (IS_SYNC(h)) {
		inbuf_keep(l+try, ib);
		valid = 1;
		l2 = l+try;
		for (i=0; i<min_consec; i++) {
		    l2 += MPEG_FRLEN(h);
		    if ((c=inbuf_getlong(&h_next, l2, ib)) < 0) {
			if (inframe)
			    valid = 0;
			break;
		    }
		    if (!IS_VALID(h_next)) {
			valid = 0;
			break;
		    }
		    else if (!IS_SYNC(h_next))
			break;
		}
		inbuf_unkeep(ib);

		if (!valid)
		    continue;
	    }
	    if (!inframe && (output & OUT_RESYNC_SKIP))
		out(l, "skipping %d bytes", try);
	    *hp = h;
	    *lp = l+try;
	    return 0;	    
	}
    }

    if (c == -1) {
	if (!inframe && (output & OUT_RESYNC_SKIP))
	    out(l, "skipping %d bytes, reaching EOF", try);
    }
    else if (c == -2) {
	if (!inframe && (output & OUT_RESYNC_BAILOUT))
	    out(l, "inbuf overflow after %d bytes, bailing out", try);
    }
    else if (!inframe && (output & OUT_RESYNC_BAILOUT))
	out(l, "no sync found in 64k, bailing out");
    
    return -1;
}



int 
get_l1_bit_alloc(int *balloc, unsigned long h, unsigned char *b, int blen)
{
    wordpointer = b + 4 + MPEG_CRC(h)*2;
    bitindex = 0;

    return I_get_bit_alloc(h, balloc);
}



int
check_l1(long pos, unsigned long h, unsigned char *b, int blen, int flen)
{
    int balloc[2*MPEG_SBLIMIT];
    int nballoc, i, nsf, samlen, sf2, j;

    nballoc = get_l1_bit_alloc(balloc, h, b, blen);

    sf2 = MPEG_JSBOUND(h)*2;
    if (sf2 > nballoc) /* single */
	sf2 = nballoc;
    
    samlen = nsf = 0;
    for (i=0; i<sf2; i++)
	if (balloc[i]) {
	    nsf++;
	    samlen += balloc[i]+1;
	}
    for (i=sf2; i<nballoc; i++)
	if (balloc[i]) {
	    nsf += 2;
	    samlen += balloc[i]+1;
	}

#if 0
    out(pos, "debug: %d*12+%d*6+%d*4=%d/%d",
	samlen, nsf, nballoc, samlen*12+nsf*6+nballoc*4,
	(flen-4-MPEG_CRC(h))*8);
#endif

    return (samlen*MPEG_SCALE_BLOCK+nsf*6+4*nballoc+7)/8 + 4+MPEG_CRC(h)*2;
}



static void
warn_short_frame(long l, int dlen, int flen, int blen, int eof)
{
    if (dlen > blen) {
	if (eof && (output & OUT_LFRAME_SHORT))
	    out(l, "short last frame %d of %d bytes (%d+%d=%d missing)",
		blen, flen, dlen-blen, flen-dlen, flen-blen);
	else if (!eof && (output & OUT_FRAME_SHORT))
	    out(l, "short frame %d of %d bytes (%d+%d=%d missing)",
		blen, flen, dlen-blen, flen-dlen, flen-blen);
    }
    else if (blen == dlen) {
	if (eof && (output & OUT_LFRAME_PADDING))
	    out(l, "padding missing from last frame (%d bytes)", flen-dlen);
	else if (!eof && (output & OUT_FRAME_PADDING))
	    out(l, "padding missing from frame (%d bytes)", flen-dlen);
    }
    else {
	if (eof && (output & OUT_LFRAME_PADDING))
	    out(l, "padding missing from last frame (%d of %d bytes)",
		flen-blen, flen-dlen);
	else if (!eof && (output & OUT_FRAME_PADDING))
	    out(l, "padding missing from frame (%d of %d bytes)",
		flen-blen, flen-dlen);
    }
}
