SUBDIRS=regress

EXTRA_DIST=README.md

LDADD=	@LIBOBJS@

bin_PROGRAMS=malint

malint_SOURCES=malint.c build_frame_tab.c mpeg.c mpg123.c inbuf.c vbr.c \
	crc.c id3.c

noinst_HEADERS=getopt.h mpeg.h mpg123.h inbuf.h vbr.h malint.h

SUFFIXES=.man .mdoc .html

.mdoc.man:
	mandoc -T man $< | sed '/TH/s,"NetBSD [^"]*","NiH",' > $@.$$$$ && diff -I NiH $@.$$$$ $@ || mv $@.$$$$ $@; \
	rm -f $@.$$$$

.mdoc.html:
	mandoc -Thtml -Oman="%N.html",style=../nih-man.css $< > $@.$$$$ && mv $@.$$$$ $@
