/*
  id3.c -- ID3 tag functions
  Copyright (C) 2000-2007 Dieter Baron and Thomas Klausner

  This file is part of malint, an MPEG Audio stream validator.
  The authors can be contacted at <nih@giga.or.at>

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



#include <iconv.h>
#include <langinfo.h>
#include <stdio.h>
#include <string.h>

#include "malint.h"

static char *__tags[] = {
    "TALB", "TAL", "Album",
    "TIT2", "TT2", "Title",
    "TPE1", "TP1", "Artist",
    "TPOS", "TPA", "CD",
    "TRCK", "TRK", "Track",
    "TYER", "TYE", "Year",
    NULL, NULL
};

static char *__encoding[] = {
    "ISO-8859-1",
    "UTF-16",
    "UTF-16BE",
    "UTF-8"
};

#define ID3_ENCODING_ISO_8859_1	0
#define ID3_ENCODING_UTF_16	1
#define ID3_ENCODING_UTF_16_BE	2
#define ID3_ENCODING_UTF_8	3
#define ID3_ENCODING_MAX	3

#define ID3_HEADER_FLAG_FOOTER		0x10
#define ID3_HEADER_FLAG_EXT_HEADER	0x40
#define HEADER_FLAGS_UNSYNC		0x80

static int field_len(char *data, int len, int dlen);
static void parse_tag_v22(unsigned char *, int);
static void parse_tag_v23(unsigned char *, int);
static void parse_tag_v24(unsigned char *, int);
static void process_tag_34(const unsigned char *, int);
static char *unsynchronise(const unsigned char *, int, int *);



long
check_tag_v2(struct inbuf *ib, long l, unsigned long h)
{
    unsigned char *p;
    unsigned long h_next;
    int n, flags;
    
    inbuf_keep(l, ib);
    if (inbuf_getc(l+10, ib) < 0) {
	if (output & OUT_TAG_SHORT) {
	    out(l, "ID3v2.%c", (h&0xff)+'0');
	    printf("    short header\n");
	}
	/* XXX: return value for resync? */
	return 1;
    }
    flags = inbuf_getc(l+5, ib);
    inbuf_getlong(&h_next, l+6, ib);
    inbuf_unkeep(ib);
    n = LONG_TO_ID3LEN(h_next) + 10
	+ (flags & ID3_HEADER_FLAG_FOOTER ? 10 : 0);
    if (inbuf_copy(&p, l, n, ib) != n) {
	if (output & OUT_TAG_SHORT) {
	    out(l, "ID3v2.%c.%c", (h&0xff)+'0', (h_next>>24)+'0');
	    printf("    short tag\n");
	}
	/* XXX: return value for resync? */
	return 10;
    }
    if (output & OUT_M_TAG)
	parse_tag_v2(l, p, n);

    return n;
}



void
parse_tag_v1(long pos, char *data, int n, int in_middle)
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

    if (n == 128)
	v11 = data[126] && data[125] == 0;
    else
	v11 = 0;

    if ((output & OUT_TAG) || (n!=128 && (output & OUT_TAG_SHORT)))
	out(pos, "%sID3v1%s tag%s",
	    n != 128 ? "short " : "",
	    v11 ? ".1" : "",
	    in_middle ? " (in middle of file)" : "");
    
    if (!(output & OUT_TAG_CONTENTS))
	return;

    for (i=0; field[i].name; i++) {
	len = field_len(data+field[i].start, field[i].len, n-field[i].start);
	if (len > 0) {
	    printf("   %s:\t%.*s\n",
		   field[i].name, len, data+field[i].start);
	}
    }
    if (v11)
	printf("   Track:\t%d\n", data[126]);
}



void
parse_tag_v2(long pos, unsigned char *data, int len)
{
    unsigned char *tagdata;
    int taglen;

    if (!(output & OUT_TAG))
	return;

    out(pos, "ID3v2.%c.%c tag", data[3]+'0', data[4]+'0');

    if (!(output & OUT_TAG_CONTENTS))
	return;

    tagdata = data;
    taglen = len;
    if (data[5] & HEADER_FLAGS_UNSYNC)
	if ((tagdata=unsynchronise(data, len, &taglen)) == NULL)
	    return;

    switch (data[3]) {
    case 2:
	parse_tag_v22(tagdata, taglen);
	break;
    case 3:
	parse_tag_v23(tagdata, taglen);
	break;
    case 4:
	parse_tag_v24(tagdata, taglen);
	break;
    default:
	printf("   unsupported version 2.%d.%d\n",
	       data[3], data[4]);
	break;
    }

    if (tagdata != data)
	free(tagdata);

    return;
}



static int
field_len(char *data, int len, int dlen)
{
    int l;

    if (len > dlen)
	len = dlen;

    for (l=0; l<len && data[l]; l++)
	;

    if (l==len) { /* space padding */
	for (; data[l-1] == ' '; --l)
	    ;
    }

    return l;
}



static void
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



static void
parse_tag_v23(unsigned char *data, int len)
{
    unsigned char *p, *end;

    p = data + 10;

    if (data[5] & ID3_HEADER_FLAG_EXT_HEADER) {
	len -= GET_LONG(data+16);
	p += GET_LONG(data+10) + 4;
    }

    /* XXX: these +10 are wrong, me thinks */
    end = data + len + 10;

    while (p < end) {
	if (memcmp(p, "\0\0\0\0", 4) == 0)
	    break;
	len = GET_LONG(p+4);
	if (len < 0)
	    break;
	process_tag_34(p, len+10);
	p += len + 10;
    }
}



static void
parse_tag_v24(unsigned char *data, int len)
{
    unsigned char *p, *end;

    p = data + 10;

    if (data[5] & ID3_HEADER_FLAG_EXT_HEADER)
	p += LONG_TO_ID3LEN(GET_LONG(data+10));
    if (data[5] & ID3_HEADER_FLAG_FOOTER)
	len -= 10;

    end = data + len;

    while (p < end) {
	if (memcmp(p, "\0\0\0\0", 4) == 0)
	    break;
	len = LONG_TO_ID3LEN(GET_LONG(p+4));
	if (len < 0)
	    break;
	process_tag_34(p, len+10);
	p += len + 10;
    }
}



static void
process_tag_34(const unsigned char *tag, int len)
{
    int i;
    iconv_t cd;
    char buf[128], *dst;
    const char *src;
    size_t srclen, dstlen, ret;
    char *codeset;
    
    for (i=0; __tags[i]; i+=3)
	if (strncmp(tag, __tags[i], 4) == 0) {
	    printf("   %s:\t", __tags[i+2]);
	    if (tag[10] > ID3_ENCODING_MAX) {
		printf("[unknown encoding %d]\n", tag[10]);
		return;
	    }

	    codeset = nl_langinfo(CODESET);
	    cd = iconv_open(codeset, __encoding[tag[10]]);
	    if (cd == (iconv_t)-1) {
		printf("[unsupported encoding %s]\n", __encoding[tag[10]]);
		return;
	    }

	    printf("[%s] ", __encoding[tag[10]]);

	    srclen = len-11;
	    src = tag+11;

	    while (srclen > 0) {
		dst = buf;
		dstlen = sizeof(buf);
		if ((ret=iconv(cd, &src, &srclen, &dst, &dstlen))
		    == (size_t)-1) {
		    /* XXX: error */
		    break;
		}

		printf("%.*s", sizeof(buf)-dstlen, buf);
	    }

	    iconv_close(cd);
	    putchar('\n');
	    break;
	}
}

static char *
unsynchronise(const unsigned char *data, int len, int *taglenp)
{
    unsigned char *tagdata, *p;
    int i;

    if ((tagdata=(unsigned char *)malloc(len)) == NULL)
	return NULL;

    p = tagdata;
    for (i=0; i<len; i++) {
	*p++ = data[i];
	if ((i == len-1) || (data[i] != 0xFF) || (data[i+1] != 0x00))
	    continue;

	if (i == len-2) {
	    printf("    incomplete unsynchronisation at end of data\n");
	    continue;
	}
	if ((data[i+2] != 0x00) && ((data[i+2]&0xe0) != 0xe0))
	    printf("    invalid unsynchronisation data 0x%.2x%.2x%.2x\n",
		data[i], data[i+1], data[i+2]);
	/* skip 0x00 */
	i++;
    }

    *taglenp = p - tagdata;
    return tagdata;
}

