/*
  id3.c -- ID3 tag functions
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

static void parse_tag_v22(unsigned char *data, int len);
static int field_len(char *data, int len, int dlen);



void
parse_tag_v2(long pos, unsigned char *data, int len)
{
    unsigned char *p, *end;
    int i;

    if (!(output & OUT_TAG))
	return;

    out(pos, "ID3v2.%c.%c tag", data[3]+'0', data[4]+'0');

    if (!(output & OUT_TAG_CONTENTS))
	return;

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


