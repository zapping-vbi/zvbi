/*
 *  libzvbi - Caption decoder tests: cursor addressing and editing commands
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *  Each test sends CC1 byte pairs (field 1, control codes doubled as in
 *  CTA-608-E D.2) to vbi_decode() and checks the resulting page.
 */

#include <stdio.h>
#include <string.h>
#include "src/libzvbi.h"

static int frame;

static uint8_t
par(uint8_t c)
{
	return vbi_par8 (c & 0x7F);
}

static void
pair(vbi_decoder *vbi, int a, int b)
{
	vbi_sliced s;

	s.id = VBI_SLICED_CAPTION_525_F1;
	s.line = 21;
	s.data[0] = par(a);
	s.data[1] = par(b);
	vbi_decode(vbi, &s, 1, frame++ * 1001.0 / 30000.0);
}

/* A control code pair, sent twice. */
static void
ctl(vbi_decoder *vbi, int a, int b)
{
	pair(vbi, a, b);
	pair(vbi, a, b);
}

static void
text(vbi_decoder *vbi, const char *s)
{
	for (; *s; s += s[1] ? 2 : 1)
		pair(vbi, s[0], s[1] ? s[1] : 0);
}

/* Preamble address code for CC1, row 1-15, indent 0, 4, ... 28, white. */
static void
pac(vbi_decoder *vbi, int row, int indent)
{
	static const int idx[15] = { 2, 3, 4, 5, 10, 11, 12, 13, 14, 15,
				     0, 6, 7, 8, 9 };
	int i = idx[row - 1];

	ctl(vbi, 0x10 | (i >> 1), 0x50 | ((i & 1) << 5) | (indent / 2));
}

/* PAC with a colour (0 white, 1 green, 2 blue, 3 cyan, 4 red, 5 yellow,
   6 magenta) and underline, no indent. */
static void
pac_color(vbi_decoder *vbi, int row, int color, int underline)
{
	static const int idx[15] = { 2, 3, 4, 5, 10, 11, 12, 13, 14, 15,
				     0, 6, 7, 8, 9 };
	int i = idx[row - 1];

	ctl(vbi, 0x10 | (i >> 1),
	    0x40 | ((i & 1) << 5) | (color << 1) | underline);
}

#define RCL(v)	ctl(v, 0x14, 0x20)
#define RU(v, n) ctl(v, 0x14, 0x23 + (n))
#define CR(v)	ctl(v, 0x14, 0x2D)
#define EDM(v)	ctl(v, 0x14, 0x2C)
#define EOC(v)	ctl(v, 0x14, 0x2F)

/* The character in the given row and column (both 1-based), or ' '. */
static int
cell(vbi_decoder *vbi, int row, int col)
{
	vbi_page pg;
	unsigned u;

	if (!vbi_fetch_cc_page(vbi, &pg, 1, FALSE))
		return -1;
	u = pg.text[(row - 1) * pg.columns + col].unicode;
	vbi_unref_page(&pg);
	return (u >= 0x20 && u < 0x7F) ? (int) u : ' ';
}

static int
expect(vbi_decoder *vbi, const char *what, int row, int col, int c)
{
	int got = cell(vbi, row, col);

	if (got == c)
		return 1;
	fprintf(stderr, "%s: row %d col %d is '%c', expected '%c'\n",
		what, row, col, got, c);
	return 0;
}

/*
 *  CTA-608-E C.7: a PAC only moves the cursor. Characters already on
 *  the row stay.  (Conformance suite v1.2 720p59, test 32 A06, test 3 G02.)
 */
static int
test_pac_keeps_row(void)
{
	vbi_decoder *vbi = vbi_decoder_new();
	int ok = 1;

	RCL(vbi);
	pac(vbi, 5, 0);
	text(vbi, "A");
	pac(vbi, 5, 20);
	text(vbi, "B");
	EOC(vbi);

	ok &= expect(vbi, "PAC", 5, 1, 'A');
	ok &= expect(vbi, "PAC", 5, 21, 'B');
	vbi_decoder_delete(vbi);
	return ok;
}

/*
 *  CTA-608-E C.7: likewise a Tab Offset only moves the cursor.
 *  (Conformance suite v1.2 720p59, test 32 A06, row 6: GREEN followed by
 *  an indent 4 PAC and TO2 must not erase the end of GREEN.)
 */
static int
test_tab_offset_keeps_row(void)
{
	vbi_decoder *vbi = vbi_decoder_new();
	int ok = 1;

	RCL(vbi);
	pac(vbi, 6, 0);
	text(vbi, "ABCDEF");
	pac(vbi, 6, 4);
	ctl(vbi, 0x17, 0x22);
	text(vbi, "X");
	EOC(vbi);

	ok &= expect(vbi, "TO", 6, 5, 'E');
	ok &= expect(vbi, "TO", 6, 6, 'F');
	ok &= expect(vbi, "TO", 6, 7, 'X');
	vbi_decoder_delete(vbi);
	return ok;
}

/*
 *  CTA-608-E B.12 (p.69): Backspace erases the character to the left of
 *  the cursor. In paint-on mode the result is displayed immediately.
 *  (Conformance suite v1.2 720p59, test 58 D04.)
 */
static int
test_backspace_paint_on(void)
{
	vbi_decoder *vbi = vbi_decoder_new();
	int ok = 1;

	ctl(vbi, 0x14, 0x2C);	/* EDM */
	ctl(vbi, 0x14, 0x29);	/* RDC */
	pac(vbi, 14, 0);
	text(vbi, "AB!");
	pair(vbi, 0, 0);
	pair(vbi, 0, 0);
	ok &= expect(vbi, "BS before", 14, 3, '!');
	ctl(vbi, 0x14, 0x21);	/* BS */

	ok &= expect(vbi, "BS", 14, 1, 'A');
	ok &= expect(vbi, "BS", 14, 2, 'B');
	ok &= expect(vbi, "BS", 14, 3, ' ');
	vbi_decoder_delete(vbi);
	return ok;
}

/*
 *  CTA-608-E 4.1: the channel bit of a control code selects the data
 *  channel, and characters that follow belong to it. After a CC2 caption
 *  in field 1, a PAC for CC1 sends the following text to CC1.
 *  (Conformance suite v1.2 720p59, test 63 D09: text typed after a BS
 *  that follows the "Rendering on CC2" label.)
 */
static int
test_control_code_selects_channel(void)
{
	vbi_decoder *vbi = vbi_decoder_new();
	int ok = 1;

	ctl(vbi, 0x14, 0x29);			/* CC1 RDC */
	ctl(vbi, 0x1C, 0x20);			/* CC2 RCL */
	ctl(vbi, 0x1D, 0x50);			/* CC2 PAC row 5 */
	text(vbi, "Two");
	ctl(vbi, 0x1C, 0x2F);			/* CC2 EOC */
	pair(vbi, 0, 0);

	pac(vbi, 5, 0);				/* CC1 */
	text(vbi, "One");
	pair(vbi, 0, 0);
	pair(vbi, 0, 0);

	ok &= expect(vbi, "CC1 after CC2", 5, 1, 'O');
	ok &= expect(vbi, "CC1 after CC2", 5, 3, 'e');
	vbi_decoder_delete(vbi);
	return ok;
}

/*
 *  CTA-608-E D.2 (p.78): only an identical control pair sent immediately
 *  after the first is redundant. A valid null pair in between makes the
 *  second one a new command.
 *  (Conformance suite v1.2 720p59, test 63 D09: "BS, null, BS".)
 */
static int
test_repeated_control_pair(void)
{
	vbi_decoder *vbi = vbi_decoder_new();
	int ok = 1;

	ctl(vbi, 0x14, 0x29);	/* RDC */
	pac(vbi, 10, 0);
	text(vbi, "ABCDEF");
	pair(vbi, 0, 0);
	pair(vbi, 0, 0);
	ctl(vbi, 0x14, 0x21);	/* BS pair: one BS */
	ok &= expect(vbi, "BS pair", 10, 5, 'E');
	ok &= expect(vbi, "BS pair", 10, 6, ' ');

	pair(vbi, 0x14, 0x21);	/* BS, null, BS: two BS */
	pair(vbi, 0, 0);
	pair(vbi, 0x14, 0x21);
	ok &= expect(vbi, "BS null BS", 10, 3, 'C');
	ok &= expect(vbi, "BS null BS", 10, 4, ' ');
	vbi_decoder_delete(vbi);
	return ok;
}

/*
 *  CTA-608-E C.13 (p.75) and B.12: the cursor never passes column 32, and
 *  a BS after a character in column 32 moves to column 31 and erases it.
 *  (Conformance suite v1.2 720p59, test 33 A07, row 10.)
 */
static int
test_backspace_at_right_margin(void)
{
	vbi_decoder *vbi = vbi_decoder_new();
	int ok = 1;

	RCL(vbi);
	pac(vbi, 10, 0);
	text(vbi, "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345");
	ctl(vbi, 0x14, 0x21);	/* BS */
	text(vbi, "x");
	EOC(vbi);

	ok &= expect(vbi, "BS at col 32", 10, 30, '3');
	ok &= expect(vbi, "BS at col 32", 10, 31, 'x');
	/* Column 32 may or may not have been erased. */
	vbi_decoder_delete(vbi);
	return ok;
}

/*
 *  CTA-608-E 6.2 (p.16): a Background Attribute code is preceded by a
 *  standard space and backspaces, so that the space cell takes the new
 *  background and the text follows at the next column.
 *  (Conformance suite v1.2 720p59, test 35 A09a.)
 */
static int
test_background_attribute(void)
{
	vbi_decoder *vbi = vbi_decoder_new();
	vbi_page pg;
	vbi_char *c;
	int ok = 1;

	RCL(vbi);
	pac(vbi, 5, 0);
	text(vbi, " ");
	ctl(vbi, 0x10, 0x28);	/* background red, opaque */
	text(vbi, "Op");
	EOC(vbi);

	ok &= expect(vbi, "BG", 5, 1, ' ');
	ok &= expect(vbi, "BG", 5, 2, 'O');
	ok &= expect(vbi, "BG", 5, 3, 'p');
	if (!vbi_fetch_cc_page(vbi, &pg, 1, FALSE))
		return 0;
	c = &pg.text[4 * pg.columns + 1];
	if (c->background != VBI_RED || c->opacity != VBI_OPAQUE) {
		fprintf(stderr, "BG: row 5 col 1 is not an opaque red space\n");
		ok = 0;
	}
	vbi_unref_page(&pg);
	vbi_decoder_delete(vbi);
	return ok;
}

/*
 *  CTA-608-E F.1.1.4 Table 52 (p.99) and C.8: Flash On is a spacing
 *  attribute. Like a mid-row code it takes one column, a space that
 *  flashes, and the text after it flashes.
 *  (Conformance suite v1.2 720p59, test 38 A10.)
 */
static int
test_flash_on(void)
{
	vbi_decoder *vbi = vbi_decoder_new();
	vbi_page pg;
	vbi_char *c;
	int ok = 1;

	RCL(vbi);
	pac(vbi, 6, 0);
	text(vbi, "ab ");
	ctl(vbi, 0x14, 0x28);	/* FON */
	text(vbi, "cd");
	EOC(vbi);

	if (!vbi_fetch_cc_page(vbi, &pg, 1, FALSE))
		return 0;
	c = &pg.text[5 * pg.columns];
	ok &= (c[1].unicode == 'a' && !c[1].flash && c[3].unicode == ' ');
	if (c[4].unicode != ' ' || !c[4].flash) {
		fprintf(stderr, "FON: column 4 is not a flashing space\n");
		ok = 0;
	}
	if (c[5].unicode != 'c' || !c[5].flash || c[6].unicode != 'd'
	    || !c[6].flash) {
		fprintf(stderr, "FON: text after FON does not flash\n");
		ok = 0;
	}
	vbi_unref_page(&pg);
	vbi_decoder_delete(vbi);
	return ok;
}

/*
 *  CTA-608-E C.7 (p.73, REGULATORY/PREFERRED): a character deposited next
 *  to an existing character takes the attributes of that character; the
 *  attributes of a PAC that put the cursor there are ignored.
 *  (Conformance suite v1.2 720p59, test 32 A06, row 6: green "GREEN"
 *  followed by an indent 4 PAC + TO2 and "TEXT": all green.)
 */
static int
test_attribute_inheritance(void)
{
	vbi_decoder *vbi = vbi_decoder_new();
	vbi_page pg;
	vbi_char *c;
	int ok = 1;

	RCL(vbi);
	pac(vbi, 6, 0);
	ctl(vbi, 0x11, 0x22);	/* mid-row green (a space) */
	text(vbi, "GREEN");
	pac(vbi, 6, 4);		/* white PAC, column 5 */
	ctl(vbi, 0x17, 0x22);	/* TO2: column 7, next to the N */
	text(vbi, "T");
	pac(vbi, 6, 12);	/* white PAC, not next to anything */
	text(vbi, "W");
	EOC(vbi);

	if (!vbi_fetch_cc_page(vbi, &pg, 1, FALSE))
		return 0;
	c = &pg.text[5 * pg.columns];
	if (c[7].unicode != 'T' || c[7].foreground != VBI_GREEN) {
		fprintf(stderr, "C.7: T (col 7) is not green\n");
		ok = 0;
	}
	if (c[13].unicode != 'W' || c[13].foreground != VBI_WHITE) {
		fprintf(stderr, "C.7: W (col 13) is not white\n");
		ok = 0;
	}
	vbi_unref_page(&pg);
	vbi_decoder_delete(vbi);
	return ok;
}

/* The cell in the given row and column (both 1-based), or NULL. */
static int
get_cell(vbi_decoder *vbi, int row, int col, vbi_char *out)
{
	vbi_page pg;

	if (!vbi_fetch_cc_page(vbi, &pg, 1, FALSE))
		return 0;
	*out = pg.text[(row - 1) * pg.columns + col];
	vbi_unref_page(&pg);
	return 1;
}

/*
 *  CTA-608-E C.14 (p.75, NORMATIVE): when no attributes have been
 *  assigned the display is white, not underlined, not italic and not
 *  flashing. A roll-up row started by CR with no PAC must not inherit the
 *  attributes of the previous row.
 *  (Conformance suite v1.2 720p59, test 34 A08.)
 */
static int
test_cr_resets_attributes(void)
{
	vbi_decoder *vbi = vbi_decoder_new();
	vbi_char a, b;
	int ok = 1;

	EDM(vbi);
	RU(vbi, 2);
	CR(vbi);
	pac_color(vbi, 15, 5, 1);	/* yellow, underlined */
	text(vbi, "Ab");
	CR(vbi);
	text(vbi, "cd");
	pair(vbi, 0, 0);
	pair(vbi, 0, 0);

	if (!get_cell(vbi, 14, 1, &a) || !get_cell(vbi, 15, 1, &b))
		return 0;
	ok &= expect(vbi, "CR", 14, 1, 'A');
	ok &= expect(vbi, "CR", 15, 1, 'c');
	if (a.foreground != VBI_YELLOW || !a.underline) {
		fprintf(stderr, "CR: row 14 is not yellow underlined\n");
		ok = 0;
	}
	if (b.foreground != VBI_WHITE || b.underline || b.italic || b.flash) {
		fprintf(stderr, "CR: row 15 does not have the default attributes\n");
		ok = 0;
	}
	vbi_decoder_delete(vbi);
	return ok;
}

/* Three roll-up rows "one", "two", "three" ending on base row 15. */
static void
three_rows(vbi_decoder *vbi)
{
	EDM(vbi);
	RU(vbi, 3);
	CR(vbi);
	pac(vbi, 15, 0);
	text(vbi, "one");
	CR(vbi);
	pac(vbi, 15, 0);
	text(vbi, "two");
	CR(vbi);
	pac(vbi, 15, 0);
	text(vbi, "three");
	pair(vbi, 0, 0);
	pair(vbi, 0, 0);
}

/*
 *  CTA-608-E C.4 (p.72, REGULATORY) and B.5: a PAC for a different base
 *  row moves the displayed roll-up rows to the new base row, they are
 *  not erased. A 3 row roll-up whose base row is moved to row 2 does not
 *  fit above it; C.4 prefers that the depth wins and no text is lost, so
 *  the base row becomes row 3.
 *  (Conformance suite v1.2 720p59, test 50 R07 and test 51 R08.)
 */
static int
test_roll_up_base_row_move(void)
{
	vbi_decoder *vbi = vbi_decoder_new();
	int ok = 1;

	three_rows(vbi);
	pac(vbi, 8, 0);
	ok &= expect(vbi, "R07", 6, 1, 'o');
	ok &= expect(vbi, "R07", 7, 1, 't');
	ok &= expect(vbi, "R07", 7, 2, 'w');
	ok &= expect(vbi, "R07", 8, 1, 't');
	ok &= expect(vbi, "R07", 8, 3, 'r');
	ok &= expect(vbi, "R07 old rows", 13, 1, ' ');
	ok &= expect(vbi, "R07 old rows", 15, 1, ' ');

	text(vbi, "X");
	pair(vbi, 0, 0);
	pair(vbi, 0, 0);
	ok &= expect(vbi, "R07 text", 8, 1, 'X');
	vbi_decoder_delete(vbi);

	vbi = vbi_decoder_new();
	three_rows(vbi);
	pac(vbi, 2, 0);
	ok &= expect(vbi, "R08", 1, 1, 'o');
	ok &= expect(vbi, "R08", 2, 1, 't');
	ok &= expect(vbi, "R08", 3, 1, 't');

	/* Another channel's data must not disturb the rows. */
	ctl(vbi, 0x1C, 0x20);
	pair(vbi, 0, 0);
	ok &= expect(vbi, "R08 after CC2", 3, 1, 't');
	vbi_decoder_delete(vbi);
	return ok;
}

/*
 *  CTA-608-E C.15 (p.75, PREFERRED): the base row of a roll-up caption
 *  defaults to row 15 and the cursor to column 1 when no roll-up caption
 *  is displayed. After the previous caption was moved elsewhere and
 *  erased, text sent with no PAC lands on row 15.
 *  (Conformance suite v1.2 720p59, test 52 R09, which follows test 51.)
 */
static int
test_roll_up_default_base_row(void)
{
	vbi_decoder *vbi = vbi_decoder_new();
	int ok = 1;

	three_rows(vbi);
	pac(vbi, 2, 0);		/* base row moves to 3 */

	EDM(vbi);
	RU(vbi, 3);
	text(vbi, "No PAC");
	pair(vbi, 0, 0);
	pair(vbi, 0, 0);

	ok &= expect(vbi, "R09", 15, 1, 'N');
	ok &= expect(vbi, "R09", 3, 1, ' ');
	vbi_decoder_delete(vbi);
	return ok;
}

/*
 *  Reducing the depth of a displayed roll-up caption (RU3 then RU2) is not
 *  specified by CTA-608-E; keep the base row and the rows that still fit,
 *  as most decoders do.
 *  (Conformance suite v1.2 720p59, test 53 R10.)
 */
static int
test_roll_up_depth_change(void)
{
	vbi_decoder *vbi = vbi_decoder_new();
	int ok = 1;

	three_rows(vbi);
	RU(vbi, 2);
	ok &= expect(vbi, "R10", 13, 1, ' ');
	ok &= expect(vbi, "R10", 14, 1, 't');
	ok &= expect(vbi, "R10", 14, 2, 'w');
	ok &= expect(vbi, "R10", 15, 1, 't');
	ok &= expect(vbi, "R10", 15, 3, 'r');

	CR(vbi);
	pac(vbi, 15, 0);
	text(vbi, "new");
	pair(vbi, 0, 0);
	pair(vbi, 0, 0);
	ok &= expect(vbi, "R10 after CR", 13, 1, ' ');
	ok &= expect(vbi, "R10 after CR", 14, 3, 'r');
	ok &= expect(vbi, "R10 after CR", 15, 1, 'n');

	/* And increasing it keeps the rows. */
	RU(vbi, 4);
	ok &= expect(vbi, "R10 RU4", 14, 3, 'r');
	ok &= expect(vbi, "R10 RU4", 15, 1, 'n');
	vbi_decoder_delete(vbi);
	return ok;
}

/*
 *  CTA-608-E C.10 and C.11 (p.74, REGULATORY): a roll-up command erases
 *  pop-on data in both memories. An EOC while roll-up is selected swaps
 *  the whole roll-up caption into the non-displayed memory (the screen is
 *  blank, the other memory having been erased) and a second EOC brings it
 *  back.
 *  (Conformance suite v1.2 720p59, test 62 D08.)
 */
static int
test_roll_up_eoc_swap(void)
{
	vbi_decoder *vbi = vbi_decoder_new();
	int ok = 1;

	RCL(vbi);
	pac(vbi, 13, 0);
	text(vbi, "A");
	EOC(vbi);			/* A displayed */
	RCL(vbi);
	ctl(vbi, 0x14, 0x2E);		/* ENM */
	pac(vbi, 12, 0);
	text(vbi, "B");			/* B hidden */
	ok &= expect(vbi, "D08", 13, 1, 'A');

	RU(vbi, 2);
	CR(vbi);
	pac(vbi, 15, 0);
	text(vbi, "R");
	pair(vbi, 0, 0);
	pair(vbi, 0, 0);
	ok &= expect(vbi, "D08 roll-up", 13, 1, ' ');
	ok &= expect(vbi, "D08 roll-up", 12, 1, ' ');
	ok &= expect(vbi, "D08 roll-up", 15, 1, 'R');

	EOC(vbi);
	ok &= expect(vbi, "D08 first EOC", 15, 1, ' ');
	ok &= expect(vbi, "D08 first EOC", 12, 1, ' ');
	EOC(vbi);
	ok &= expect(vbi, "D08 second EOC", 15, 1, 'R');
	ok &= expect(vbi, "D08 second EOC", 12, 1, ' ');
	vbi_decoder_delete(vbi);
	return ok;
}

/*
 *  CTA-608-E 6.4.2 (p.18-20, OPTIONAL): extended characters (0x12 and
 *  0x13 codes, Table 5) follow a fallback character from the basic set
 *  and replace it (automatic backspace). When one is the first character
 *  on a row there is nothing to replace and the cursor stays in column 1.
 *  (Conformance suite v1.2 720p59, tests 8-13 C03a-f and 14 C04.)
 */
static int
test_extended_characters(void)
{
	vbi_decoder *vbi = vbi_decoder_new();
	vbi_char c;
	int ok = 1;

	RCL(vbi);
	pac(vbi, 6, 0);
	text(vbi, "ABCA");
	ctl(vbi, 0x12, 0x20);			/* A acute */
	text(vbi, "D");
	pac(vbi, 8, 0);
	ctl(vbi, 0x12, 0x20);			/* first character on a row */
	text(vbi, "B");
	pac(vbi, 10, 0);
	text(vbi, "xC");
	ctl(vbi, 0x13, 0x20);			/* Table 5, 0x13 0x20 */
	EOC(vbi);

	if (!get_cell(vbi, 6, 4, &c))
		return 0;
	if (c.unicode != 0xC1) {
		fprintf(stderr, "C04: row 6 col 4 is U+%04X, expected A acute\n",
			c.unicode);
		ok = 0;
	}
	ok &= expect(vbi, "C04", 6, 3, 'C');
	ok &= expect(vbi, "C04", 6, 5, 'D');
	ok &= expect(vbi, "C04", 6, 6, ' ');
	get_cell(vbi, 8, 1, &c);
	if (c.unicode != 0xC1) {
		fprintf(stderr, "C04: row 8 col 1 is U+%04X\n", c.unicode);
		ok = 0;
	}
	ok &= expect(vbi, "C04", 8, 2, 'B');
	get_cell(vbi, 10, 2, &c);
	if (c.unicode != vbi_caption_unicode(0x1320, FALSE)) {
		fprintf(stderr, "C03: row 10 col 2 is U+%04X\n", c.unicode);
		ok = 0;
	}

	/* Table 6 (p.21): 0x12 0x2A is an em dash. */
	RCL(vbi);
	pac(vbi, 12, 0);
	text(vbi, "a-");
	ctl(vbi, 0x12, 0x2A);
	text(vbi, "b");
	EOC(vbi);
	get_cell(vbi, 12, 2, &c);
	if (c.unicode != 0x2014) {
		fprintf(stderr, "Table 6: row 12 col 2 is U+%04X, expected the em dash U+2014\n",
			c.unicode);
		ok = 0;
	}
	ok &= expect(vbi, "Table 6", 12, 3, 'b');
	vbi_decoder_delete(vbi);
	return ok;
}

int
main(void)
{
	int ok = 1;

	ok &= test_pac_keeps_row();
	ok &= test_tab_offset_keeps_row();
	ok &= test_backspace_paint_on();
	ok &= test_control_code_selects_channel();
	ok &= test_repeated_control_pair();
	ok &= test_backspace_at_right_margin();
	ok &= test_background_attribute();
	ok &= test_flash_on();
	ok &= test_attribute_inheritance();
	ok &= test_cr_resets_attributes();
	ok &= test_roll_up_base_row_move();
	ok &= test_roll_up_default_base_row();
	ok &= test_roll_up_depth_change();
	ok &= test_roll_up_eoc_swap();
	ok &= test_extended_characters();
	return ok ? 0 : 1;
}
