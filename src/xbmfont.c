/*
 *  libzvbi -- Bitmap font generator
 *
 *  Copyright (C) 2026 Devin Heitmueller
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 */

/*
 *  The Closed Caption and Teletext fonts of libzvbi are X bitmap (XBM)
 *  images in which all glyph cells are tiled, per_row cells to a row.
 *  This program converts such an image to a text file with one picture
 *  per glyph, which can be edited and reviewed, and back:
 *
 *    xbmfont -d <cell width> <cell height> < ccfont2.xbm > ccfont2.glyphs
 *    xbmfont < ccfont2.glyphs > ccfont2.xbm
 *
 *  It is a stand-alone program: cc -o xbmfont xbmfont.c
 *
 *  Glyph file format (lines starting with ';' and blank lines outside
 *  the header are ignored):
 *
 *    header                    The comment preceding the XBM definitions,
 *    ...                       copied verbatim.
 *    endheader
 *    font <name> <cell width> <cell height> <cells per row> <rows>
 *    decl <declaration>        Text preceding "<name>_bits[]", for example
 *                              "static char".
 *    glyph <n>                 <cell height> lines of <cell width> pixels,
 *    ................          '#' foreground, '.' background. Glyph n is
 *    ...                       in row n / <cells per row>, column
 *                              n % <cells per row> of the image.
 *
 *  Every glyph of the font must be present. The cell width is limited to
 *  16 pixels, the limit of draw_char() in src/exp-gfx.c.
 *
 *  src/ccfont2.xbm regenerated from src/ccfont2.glyphs (made with -d 16
 *  26 from the file of libzvbi 0.2.45) is byte-identical to the file
 *  shipped with libzvbi 0.2.45 (commit d3a5ee9); the same holds for
 *  src/wstfont2.xbm (-d 12 10).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_CELL_WIDTH 16
#define BYTES_PER_LINE 12

static void
fatal				(const char *		msg)
{
	fprintf(stderr, "xbmfont: %s\n", msg);
	exit(EXIT_FAILURE);
}

static void
fatal_line			(const char *		msg,
				 int			line)
{
	fprintf(stderr, "xbmfont: line %d: %s\n", line, msg);
	exit(EXIT_FAILURE);
}

static void *
xalloc				(size_t			size)
{
	void *p = calloc(1, size ? size : 1);

	if (!p)
		fatal("out of memory");

	return p;
}

/* Read all of stdin into a NUL terminated buffer. */
static char *
read_all			(void)
{
	size_t size = 1 << 16, n = 0, r;
	char *buf = xalloc(size);

	while ((r = fread(buf + n, 1, size - n - 1, stdin)) > 0) {
		n += r;

		if (n + 1 >= size) {
			char *t = xalloc(size * 2);

			memcpy(t, buf, n);
			free(buf);
			buf = t;
			size *= 2;
		}
	}

	buf[n] = 0;

	return buf;
}

/* Pixel (x, y) of the image, bit order of XBM: least significant first. */
#define PIXEL(bits, stride, x, y) \
	(((bits)[(y) * (stride) + ((x) >> 3)] >> ((x) & 7)) & 1)

/*
 *  Image to glyph file.
 */
static void
dump				(int			cw,
				 int			ch)
{
	char *buf = read_all();
	char *p, *header_end, *bits_start;
	char name[64], decl[64];
	int width = 0, height = 0, stride, per_row, rows, g, x, y;
	unsigned char *bits;
	size_t n, expect;
	char *q;

	p = strstr(buf, "#define ");

	if (!p)
		fatal("not an XBM file: no #define");

	header_end = p;

	/* #define <name>_width <n> */
	if (sscanf(p, "#define %63[A-Za-z0-9_] %d", name, &width) != 2)
		fatal("cannot parse the width");

	q = strstr(name, "_width");

	if (!q || q[6])
		fatal("the first #define is not <name>_width");

	*q = 0;

	p = strstr(p, "_height");

	if (!p || sscanf(p, "_height %d", &height) != 1)
		fatal("cannot parse the height");

	p = strstr(p, "_bits[]");

	if (!p)
		fatal("no <name>_bits[]");

	/* Text preceding "<name>_bits[]" on its line. */
	for (q = p; q > buf && q[-1] != '\n'; --q)
		;

	n = (size_t)(p - q) - strlen(name) - 1;
	/* "static char ccfont2_bits[]": p points to "_bits[]" */
	if (n >= sizeof decl)
		fatal("declaration too long");

	memcpy(decl, q, n);
	decl[n] = 0;

	p = strchr(p, '{');

	if (!p)
		fatal("no {");

	bits_start = p + 1;

	if (cw < 1 || cw > MAX_CELL_WIDTH || ch < 1)
		fatal("bad cell size");

	if (width % cw || height % ch || width % 8)
		fatal("the image is not a whole number of cells");

	stride = width / 8;
	expect = (size_t) stride * height;
	bits = xalloc(expect);

	n = 0;
	p = bits_start;

	while (n < expect && (p = strstr(p, "0x")) != NULL) {
		bits[n++] = (unsigned char) strtoul(p + 2, &q, 16);
		p = q;
	}

	if (n != expect)
		fatal("the number of data bytes does not match the image size");

	per_row = width / cw;
	rows = height / ch;

	printf("header\n");
	fwrite(buf, 1, (size_t)(header_end - buf), stdout);
	printf("endheader\n");
	printf("font %s %d %d %d %d\n", name, cw, ch, per_row, rows);
	printf("decl %s\n", decl);

	for (g = 0; g < per_row * rows; ++g) {
		printf("\nglyph %d\n", g);

		for (y = 0; y < ch; ++y) {
			for (x = 0; x < cw; ++x)
				putchar(PIXEL(bits, stride,
					      (g % per_row) * cw + x,
					      (g / per_row) * ch + y)
					? '#' : '.');

			putchar('\n');
		}
	}

	free(bits);
	free(buf);
}

/*
 *  Glyph file to image.
 */
static void
compile				(void)
{
	char *buf = read_all();
	char *line, *next;
	char name[64] = "", decl[64] = "";
	char *header = NULL;
	size_t header_len = 0;
	int cw = 0, ch = 0, per_row = 0, rows = 0;
	int width = 0, height = 0, stride = 0;
	unsigned char *bits = NULL;
	char *seen = NULL;
	int lineno = 0, in_header = 0, glyph = -1, glyph_line = 0;
	int g, i, nbytes;

	for (line = buf; *line; line = next) {
		char *nl = strchr(line, '\n');

		if (nl) {
			*nl = 0;
			next = nl + 1;
		} else {
			next = line + strlen(line);
		}

		++lineno;

		if (in_header) {
			if (0 == strcmp(line, "endheader")) {
				in_header = 0;
			} else {
				size_t l = strlen(line);

				header = realloc(header, header_len + l + 2);

				if (!header)
					fatal("out of memory");

				memcpy(header + header_len, line, l);
				header[header_len + l] = '\n';
				header_len += l + 1;
			}

			continue;
		}

		if (glyph >= 0 && glyph_line < ch) {
			int x;

			if ((int) strlen(line) != cw)
				fatal_line("wrong number of pixels in a glyph line",
					   lineno);

			for (x = 0; x < cw; ++x) {
				int px = (glyph % per_row) * cw + x;
				int py = (glyph / per_row) * ch + glyph_line;

				if (line[x] == '#')
					bits[py * stride + (px >> 3)] |=
						1 << (px & 7);
				else if (line[x] != '.')
					fatal_line("a glyph line has a character other than '#' and '.'",
						   lineno);
			}

			if (++glyph_line == ch)
				glyph = -1;

			continue;
		}

		if (!*line || *line == ';')
			continue;

		if (0 == strcmp(line, "header")) {
			in_header = 1;
		} else if (0 == strncmp(line, "font ", 5)) {
			if (bits)
				fatal_line("second font line", lineno);

			if (5 != sscanf(line, "font %63s %d %d %d %d",
					name, &cw, &ch, &per_row, &rows))
				fatal_line("cannot parse the font line", lineno);

			if (cw < 1 || cw > MAX_CELL_WIDTH || ch < 1
			    || per_row < 1 || rows < 1)
				fatal_line("bad font dimensions (cell width 1 ... 16)",
					   lineno);

			width = cw * per_row;
			height = ch * rows;

			if (width % 8)
				fatal_line("the image width is not a multiple of 8",
					   lineno);

			stride = width / 8;
			bits = xalloc((size_t) stride * height);
			seen = xalloc((size_t) per_row * rows);
		} else if (0 == strncmp(line, "decl ", 5)) {
			if (strlen(line + 5) >= sizeof decl)
				fatal_line("declaration too long", lineno);

			strcpy(decl, line + 5);
		} else if (0 == strncmp(line, "glyph ", 6)) {
			if (!bits)
				fatal_line("glyph before the font line", lineno);

			glyph = atoi(line + 6);

			if (glyph < 0 || glyph >= per_row * rows)
				fatal_line("glyph number out of range", lineno);

			if (seen[glyph])
				fatal_line("glyph defined twice", lineno);

			seen[glyph] = 1;
			glyph_line = 0;
		} else {
			fatal_line("unknown line", lineno);
		}
	}

	if (!bits || !*decl || !header)
		fatal("missing header, font or decl");

	if (glyph >= 0)
		fatal("the last glyph is incomplete");

	for (g = 0; g < per_row * rows; ++g)
		if (!seen[g]) {
			fprintf(stderr, "xbmfont: glyph %d is missing\n", g);
			exit(EXIT_FAILURE);
		}

	fwrite(header, 1, header_len, stdout);
	printf("#define %s_width %d\n", name, width);
	printf("#define %s_height %d\n", name, height);
	printf("%s %s_bits[] = {\n", decl, name);

	nbytes = stride * height;

	for (i = 0; i < nbytes; ++i) {
		if (i % BYTES_PER_LINE == 0)
			printf("   ");

		printf("0x%02x", bits[i]);

		if (i == nbytes - 1)
			printf(" };\n");
		else if (i % BYTES_PER_LINE == BYTES_PER_LINE - 1)
			printf(",\n");
		else
			printf(", ");
	}

	free(header);
	free(seen);
	free(bits);
	free(buf);
}

int
main				(int			argc,
				 char **		argv)
{
	if (argc == 4 && 0 == strcmp(argv[1], "-d")) {
		dump(atoi(argv[2]), atoi(argv[3]));
	} else if (argc == 1) {
		compile();
	} else {
		fprintf(stderr, "Usage: %s -d <cell width> <cell height> < font.xbm > font.glyphs\n"
			"       %s < font.glyphs > font.xbm\n", argv[0], argv[0]);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Local variables:
c-set-style: K&R
c-basic-offset: 8
End:
*/
