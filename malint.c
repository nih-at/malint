#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void build_length_table(int *table);
int process_file(FILE *f, char *fname);



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
#define GET_ID3LEN(x)	((((x)[0]&0x7f)<<21)|(((x)[1]&0x7f)<<14) \
			 |(((x)[2]&0x7f)<<7)|((x)[3]&0x7f))

void out_start(char *fname);
void out(long pos, char *fmt, ...);

void parse_tag_v2(unsigned char *data, int len);
void parse_tag_v22(unsigned char *data, int len);
void parse_tag_v1(char *data);



int
main(int argc, char **argv)
{
    FILE *f;
    int i, ret;

    prg = argv[0];

    build_length_table(table);

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
    int j, n;
    long l, len;
    unsigned long h;
    unsigned char b[8192];

    out_start(fname);

    if (fseek(f, -128, SEEK_END) >= 0) {
	len = ftell(f);
	if (fread(b, 128, 1, f) == 1) {
	    if (strncmp(b, "TAG", 3) == 0) {
		len -= 128;
		out(len, "ID3v1");
		parse_tag_v1(b);
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

    l = 0;
    while((len < 0 || l < len) && fread(b, 4, 1, f) > 0) {
	h = GET_LONG(b);
	
	if ((h&0xfff00000) == 0xfff00000) {
	    /* valid header */
	    j = table[(h&0x000fffe0)>>9];
	    if (j == 0) {
		out(l, "illegal header 0x%lx", h);
		break;
	    }
	}
	else {
	    if ((h&0xffffff00) == (('T'<<24)|('A'<<16)|('G'<<8))) {
		out(l, "ID3v1 tag (in middle of song)");
		if (fread(b+4, 124, 1, f) != 1) {
		    printf("    short tag\n");
		    break;
		}
		parse_tag_v1(b);
		l += 128;
		continue;
	    }
	    else if ((h&0xffffff00) == (('I'<<24)|('D'<<16)|('3'<<8))) {
		out(l, "ID3v2.%c", h&0xff);
		if (fread(b+4, 6, 1, f) != 1) {
		    printf("    short header\n");
		    break;
		}
		j = GET_ID3LEN(b+6);
		if (fread(b+10, j, 1, f) != 1) {
		    printf("    short tag\n");
		    break;
		}
		parse_tag_v2(b, j);
		l += j;
		continue;
	    }
	    else {
		/* not recognized */
		out(l, "illegal header 0x%lx", h);
		/* resync? */
		break;
	    }
	}
	if (j>4) {
	    if ((n=fread(b, 1, j-4, f)) != j-4) {
		out(l, "short last frame: %d of %d bytes", n+4, j);
		break;
	    }
	}
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
parse_tag_v2(unsigned char *data, int len)
{
    char *p, *end;
    int i;

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



static void
print_field(char *data, int len)
{
    int l;

    for (l=0; l<len && data[l]; l++)
	;

    if (l==len) { /* space padding */
	for (; data[l-1] == ' '; --l)
	    ;
    }

    printf("%.*s\n", l, data);
}

void
parse_tag_v1(char *data)
{
    
    printf("   Artist:\t");
    print_field(data+33, 30);
    printf("   Title:\t");
    print_field(data+3, 30);
    printf("   Album:\t");
    print_field(data+63, 30);
    if (data[126] && data[125] == 0) { /* v1.1 */
	printf("   Track:\t%d\n", data[126]);
    }
    printf("   Year:\t");
    print_field(data+93, 4);
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

    printf(" at %ld: ", pos);

    va_start(argp, fmt);
    vprintf(fmt, argp);
    va_end(argp);
    putc('\n', stdout);
}
