#include <errno.h>
#include <stdio.h>
#include <string.h>



int table[2048];

/*
#define GET_LONG(x)	((((unsigned long)((x)[0]))<<24) \
                         | (((unsigned long)((x)[1]))<<16) \
                         | (((unsigned long)((x)[2]))<<8) \
		         (unsigned long)(x)[3])
*/

#define GET_LONG(x)	(((x)[0]<<24)|((x)[1]<<16)|((x)[2]<<8)|(x)[3])

void parse_tag_v2(unsigned char *data, int len);
void parse_v1(char *data);



int
main(int argc, char **argv)
{
    FILE *f;
    int i, ret, j, n;
    long l;
    unsigned long h;
    unsigned char b[8192];
    char *data;

    build_length_table(table);

    ret = 0;
    for (i=1; i<argc; i++) {
	if ((f=fopen(argv[i], "r")) == NULL) {
	    fprintf(stderr, "%s: cannot open file `%s': %s\n",
		    argv[0], argv[i], strerror(errno));
	    ret = 1;
	    continue;
	}

	l = 0;
	while(fread(b, 4, 1, f) > 0) {
	    h = GET_LONG(b);

	    if ((h&0xfff00000) == 0xfff00000) {
		/* valid header */
		data = "last frame";
		j = table[(h&0x000fffe0)>>9];
	    }
	    else {
		if ((h&0xffffff00) == (('T'<<24)|('A'<<16)|('G'<<8))) {
		    printf("%s: ID3v1 tag at %ld\n",
			   argv[i], l);
		    data = "ID3v1 tag";
		    if (fread(b+4, 124, 1, f) != 1) {
			/* XXX: short tag */
			printf("    short tag\n");
			break;
		    }
		    parse_v1(b);
		    j = 4;
		}
		else if ((h&0xffffff00) == (('I'<<24)|('D'<<16)|('3'<<8))) {
		    printf("%s: ID3v2 tag at %ld\n", argv[i], l);
		    if (fread(b+4, 6, 1, f) != 1) {
			/* XXX: short header */
			printf("    short header\n");
			break;
		    }
		    j = GET_LONG(b+6);
		    if (fread(b+10, j, 1, f) != 1) {
			/* XXX: short tag */
			printf("    short tag\n");
			break;
		    }
		    parse_tag_v2(b, j);
		    j = 4;
		}
		else {
		    /* not recognized */
		    printf("%s: illegal header at %ld: 0x%lx\n",
			   argv[i], l, h);
		    /* resync? */
		    break;
		}
	    }
	    if ((n=fread(b, 1, j-4, f)) != j-4) {
		printf("%s: short last frame at %ld: %d of %d bytes\n",
		       argv[i], l, n+4, j);
		break;
	    }
	    l += j;
	}
    }
    
    return ret;
}



static char *__tags[] = {
    "TALB", "Album",
    "TIT2", "Title",
    "TPE1", "Artist",
    "TPOS", "CD",
    "TRCK", "Track",
    "TYER", "Year",
    NULL, NULL
};

void
parse_tag_v2(unsigned char *data, int len)
{
    char *p, *end;
    int i;

    if (data[3] > 3) {
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
	for (i=0; __tags[i]; i+=2)
	    if (strncmp(p, __tags[i], 4) == 0) {
		if (p[10] == 0) 
		    printf("   %s:\t%.*s\n", __tags[i+1], len-1, p+11);
		break;
	    }
	p += len + 10;
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
parse_v1(char *data)
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
