/*
 *  libzvbi - Closed Caption glyph test: what the built-in renderer draws
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *  The decoder test checks the page that vbi_fetch_cc_page() returns. This
 *  one draws characters with vbi_draw_cc_page_region() and compares the
 *  pixels, as the font may not have a glyph for a character, or may draw
 *  the glyph of another one.
 */

#include <stdio.h>
#include <string.h>
#include "src/libzvbi.h"

#define CELL_W 16
#define CELL_H 26

typedef struct {
	vbi_rgba px[CELL_W * CELL_H];
} cell;

static vbi_page pg;

/* Draw one character with default attributes, upright or slanted. */
static void
draw(cell *out, unsigned int unicode, int italic)
{
	vbi_char *c = &pg.text[5 * pg.columns + 5];

	memset(c, 0, sizeof *c);
	c->unicode = unicode;
	c->opacity = VBI_OPAQUE;
	c->foreground = VBI_WHITE;
	c->background = VBI_BLACK;
	c->size = VBI_NORMAL_SIZE;
	c->italic = italic;

	memset(out, 0, sizeof *out);
	vbi_draw_cc_page_region(&pg, VBI_PIXFMT_RGBA32_LE, out->px,
				CELL_W * sizeof(vbi_rgba), 5, 5, 1, 1);
}

static int
same(const cell *a, const cell *b)
{
	return 0 == memcmp(a, b, sizeof *a);
}

/* True if b is a, or a moved by up to one pixel to the left or right. */
static int
alike(const cell *a, const cell *b)
{
	int dx, x, y;

	for (dx = -1; dx <= 1; ++dx) {
		int equal = 1;

		for (y = 0; y < CELL_H && equal; ++y)
			for (x = 0; x < CELL_W; ++x) {
				int sx = x + dx;
				vbi_rgba p = (sx >= 0 && sx < CELL_W) ?
					b->px[y * CELL_W + sx] : a->px[y * CELL_W + x];

				if (p != a->px[y * CELL_W + x]) {
					equal = 0;
					break;
				}
			}

		if (equal)
			return 1;
	}

	return 0;
}

/* The font draws this for characters it does not have, upright and slanted. */
static cell placeholder[2];

static int
check_drawn(unsigned int unicode, int italic)
{
	cell c;

	draw(&c, unicode, italic);

	if (same(&c, &placeholder[!!italic])) {
		fprintf(stderr, "U+%04X%s is drawn as the placeholder\n",
			unicode, italic ? " (italic)" : "");
		return 0;
	}

	return 1;
}

static int
check_differs(unsigned int a, unsigned int b, int italic)
{
	cell ca, cb;

	draw(&ca, a, italic);
	draw(&cb, b, italic);

	if (alike(&ca, &cb)) {
		fprintf(stderr, "U+%04X and U+%04X%s are drawn alike\n",
			a, b, italic ? " (italic)" : "");
		return 0;
	}

	return 1;
}

/*
 *  The EIA-608 basic set has accented letters in place of nine ASCII
 *  characters, and the font has copies of their glyphs at the ASCII
 *  positions. The decoder converts those letters to Unicode and reaches
 *  the glyphs through the list of special characters, so only a real
 *  ASCII character, such as the asterisk of CTA-608-E Table 6 or the
 *  braces of Table 8, ends up at the copy. It must draw itself.
 */
static int
test_ascii_punctuation(void)
{
	static const struct {
		unsigned int ascii, letter;
	} t[] = {
		{ '*', 0x00E1 }, { '\\', 0x00E9 }, { '^', 0x00ED },
		{ '_', 0x00F3 }, { '{', 0x00E7 }, { '|', 0x00F7 },
		{ '}', 0x00D1 }, { '~', 0x00F1 }
	};
	unsigned int i;
	int italic, ok = 1;

	for (italic = 0; italic <= 1; ++italic) {
		for (i = 0; i < sizeof t / sizeof *t; ++i) {
			ok &= check_drawn(t[i].ascii, italic);
			ok &= check_differs(t[i].ascii, t[i].letter, italic);
		}
	}

	return ok;
}

int
main(void)
{
	vbi_decoder *vbi = vbi_decoder_new();
	int ok = 1;

	if (!vbi || !vbi_fetch_cc_page(vbi, &pg, 1, FALSE))
		return 2;

	/* A code point the font cannot have. */
	draw(&placeholder[0], 0x0100, 0);
	draw(&placeholder[1], 0x0100, 1);

	ok &= test_ascii_punctuation();

	vbi_unref_page(&pg);
	vbi_decoder_delete(vbi);

	return ok ? 0 : 1;
}
