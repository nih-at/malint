**_This project is not actively maintained. Further updates are unlikely._**

This is `malint`, an MP3 validator.

It parses an MPEG audio stream and prints information about the
stream and format violations it finds.

Many of the checks (and information outputs) can be enabled or
disabled via command line switches.  Run `malint --help` for a list.

`malint -I` (`--fast-info`) only prints information that can be
provided without reading (and parsing) the whole MPEG stream.

For generic installation instructions, see file (INSTALL)[INSTALL].

If you make a binary distribution, please include a pointer to the
distribution site:
	http://nih.at/malint

Mail suggestions and bug reports to <malint@nih.at>.
