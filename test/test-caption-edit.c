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

#define RCL(v)	ctl(v, 0x14, 0x20)
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

int
main(void)
{
	int ok = 1;

	ok &= test_pac_keeps_row();
	ok &= test_tab_offset_keeps_row();
	return ok ? 0 : 1;
}
