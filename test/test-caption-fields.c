/*
 *  libzvbi - Caption decoder test: field 1 and field 2 channels are independent
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *  A field 2 service (CC3/CC4) transmitting at the same time as CC1 must not
 *  change where field 1 characters go. CTA-608-E 4.1: each of the four
 *  caption channels is an independent service.
 */

#include <stdio.h>
#include <string.h>
#include "src/libzvbi.h"

static uint8_t
par(uint8_t c)
{
	return vbi_par8 (c & 0x7F);
}

static void
feed(vbi_decoder *vbi, int frame, int f1a, int f1b, int f2a, int f2b)
{
	vbi_sliced s[2];
	unsigned n = 0;

	if (f1a >= 0) {
		s[n].id = VBI_SLICED_CAPTION_525_F1;
		s[n].line = 21;
		s[n].data[0] = par(f1a);
		s[n].data[1] = par(f1b);
		n++;
	}
	if (f2a >= 0) {
		s[n].id = VBI_SLICED_CAPTION_525_F2;
		s[n].line = 284;
		s[n].data[0] = par(f2a);
		s[n].data[1] = par(f2b);
		n++;
	}
	vbi_decode(vbi, s, n, frame * 1001.0 / 30000.0);
}

static int
row_has_at(vbi_decoder *vbi, int pgno, int row, const char *text, int line)
{
	vbi_page pg;
	char buf[64];
	int c, ok;

	if (!vbi_fetch_cc_page(vbi, &pg, pgno, FALSE))
		return 0;
	for (c = 0; c < pg.columns && c < 63; ++c) {
		unsigned u = pg.text[row * pg.columns + c].unicode;
		buf[c] = (u >= 0x20 && u < 0x7F) ? u : ' ';
	}
	buf[c] = 0;
	ok = strstr(buf, text) != NULL;
	if (!ok)
		fprintf(stderr, "line %d: pgno %d row %d: \"%s\" does not contain \"%s\"\n",
			line, pgno, row + 1, buf, text);
	vbi_unref_page(&pg);
	return ok;
}

#define row_has(vbi, pgno, row, text) \
	row_has_at(vbi, pgno, row, text, __LINE__)

static int
test_concurrent_channels(void)
{
	vbi_decoder *vbi = vbi_decoder_new();
	int f = 0, ok = 1;

	if (!vbi)
		return 0;

	/* CC1 (field 1, doubled control codes) and CC4 (field 2, not
	   doubled), both pop-on, row 5, loaded simultaneously. */
	feed(vbi, f++, 0x14, 0x20, 0x1C, 0x20);	/* RCL */
	feed(vbi, f++, 0x14, 0x20, 0x1D, 0x50);	/* RCL / PAC CC4 */
	feed(vbi, f++, 0x15, 0x50, 'Z', 'z');	/* PAC CC1 / text CC4 */
	feed(vbi, f++, 0x15, 0x50, 'Q', 'q');
	feed(vbi, f++, 'H', 'i', 0, 0);
	feed(vbi, f++, 'H', 'o', 0x1C, 0x2F);	/* text / EOC CC4 */
	feed(vbi, f++, 0x14, 0x2F, 0, 0);	/* EOC CC1 */
	feed(vbi, f++, 0x14, 0x2F, 0, 0);

	ok &= row_has(vbi, 1, 4, "HiHo");
	ok &= row_has(vbi, 4, 4, "ZzQq");

	vbi_decoder_delete(vbi);
	return ok;
}

int
main(void)
{
	int ok = 1;

	ok &= test_concurrent_channels();
	return ok ? 0 : 1;
}
