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

static int raw_bytes;	/* send the bytes as they are, without parity */

static uint8_t
par_or_raw(uint8_t c)
{
	return raw_bytes ? c : par(c);
}

static void
feed(vbi_decoder *vbi, int frame, int f1a, int f1b, int f2a, int f2b)
{
	vbi_sliced s[2];
	unsigned n = 0;

	if (f1a >= 0) {
		s[n].id = VBI_SLICED_CAPTION_525_F1;
		s[n].line = 21;
		s[n].data[0] = par_or_raw(f1a);
		s[n].data[1] = par_or_raw(f1b);
		n++;
	}
	if (f2a >= 0) {
		s[n].id = VBI_SLICED_CAPTION_525_F2;
		s[n].line = 284;
		s[n].data[0] = par_or_raw(f2a);
		s[n].data[1] = par_or_raw(f2b);
		n++;
	}
	vbi_decode(vbi, s, n, frame * 1001.0 / 30000.0);
}

static int
row_has_quiet(vbi_decoder *vbi, int pgno, int row, const char *text)
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
	vbi_unref_page(&pg);
	return ok;
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

/*
 *  CTA-608-E C.21 (p.76-77, REGULATORY): after 45 consecutive frames
 *  without valid data a decoder erases its memory and disables the
 *  display; each field is evaluated on its own (D.7). Data with wrong
 *  parity is not valid data.
 *  (Conformance suite v1.2 720p59, tests 66-68 S03-S05.)
 *
 *  libzvbi sees a loss of data through the time stamps: vbi_decode() is
 *  called for every frame, with no line for a field that has no waveform.
 */

/* A pop-on caption "Hi" on row 5 of CC1 (field 1) and "Yo" of CC3
   (field 2), displayed. */
static int
load_captions(vbi_decoder *vbi, int f)
{
	feed(vbi, f++, 0x14, 0x20, 0x15, 0x20);	/* RCL */
	feed(vbi, f++, 0x14, 0x20, 0x15, 0x50);	/* RCL, CC3 PAC row 5 */
	feed(vbi, f++, 0x15, 0x50, 'Y', 'o');	/* CC1 PAC, CC3 text */
	feed(vbi, f++, 0x15, 0x50, 0x15, 0x2F);	/* CC3 EOC */
	feed(vbi, f++, 'H', 'i', 0, 0);
	feed(vbi, f++, 0x14, 0x2F, 0, 0);		/* CC1 EOC */
	feed(vbi, f++, 0x14, 0x2F, 0, 0);
	return f;
}

static int
test_loss_of_waveform(void)
{
	vbi_decoder *vbi = vbi_decoder_new();
	int f, i, ok = 1;

	f = load_captions(vbi, 0);
	ok &= row_has(vbi, 1, 4, "Hi");
	ok &= row_has(vbi, 3, 4, "Yo");

	/* Nulls on both fields: still valid, the captions stay. */
	for (i = 0; i < 100; ++i)
		feed(vbi, f++, 0, 0, 0, 0);
	ok &= row_has(vbi, 1, 4, "Hi");
	ok &= row_has(vbi, 3, 4, "Yo");

	/* S04: no waveform on field 2 only. */
	for (i = 0; i < 40; ++i)
		feed(vbi, f++, 0, 0, -1, 0);
	ok &= row_has(vbi, 3, 4, "Yo");
	for (i = 0; i < 20; ++i)
		feed(vbi, f++, 0, 0, -1, 0);
	ok &= row_has(vbi, 1, 4, "Hi");
	ok &= !row_has_quiet(vbi, 3, 4, "Yo");

	/* S03: no waveform on either field. */
	for (i = 0; i < 40; ++i)
		feed(vbi, f++, -1, 0, -1, 0);
	ok &= row_has(vbi, 1, 4, "Hi");
	for (i = 0; i < 20; ++i)
		feed(vbi, f++, -1, 0, -1, 0);
	ok &= !row_has_quiet(vbi, 1, 4, "Hi");

	/* Data resumes: a new caption displays normally and the old one
	   does not come back. */
	feed(vbi, f++, 0x14, 0x20, 0, 0);
	feed(vbi, f++, 0x14, 0x20, 0, 0);
	feed(vbi, f++, 0x15, 0x50, 0, 0);
	feed(vbi, f++, 0x15, 0x50, 0, 0);
	feed(vbi, f++, 'N', 'o', 0, 0);
	feed(vbi, f++, 0x14, 0x2F, 0, 0);
	feed(vbi, f++, 0x14, 0x2F, 0, 0);
	ok &= row_has(vbi, 1, 4, "No");
	ok &= !row_has_quiet(vbi, 1, 4, "Hi");

	vbi_decoder_delete(vbi);
	return ok;
}

static int
test_invalid_parity(void)
{
	vbi_decoder *vbi = vbi_decoder_new();
	int f, i, ok = 1;

	f = load_captions(vbi, 0);
	ok &= row_has(vbi, 1, 4, "Hi");

	/* S05: zero filled data, even parity, for 60 frames. */
	raw_bytes = 1;
	for (i = 0; i < 40; ++i)
		feed(vbi, f++, 0, 0, 0, 0);
	ok &= row_has(vbi, 1, 4, "Hi");
	for (i = 0; i < 20; ++i)
		feed(vbi, f++, 0, 0, 0, 0);
	raw_bytes = 0;
	ok &= !row_has_quiet(vbi, 1, 4, "Hi");
	ok &= !row_has_quiet(vbi, 3, 4, "Yo");

	/* Nothing was drawn for the invalid data either. */
	ok &= !row_has_quiet(vbi, 1, 14, "\x7f");

	feed(vbi, f++, 0x14, 0x20, 0, 0);
	feed(vbi, f++, 0x14, 0x20, 0, 0);
	feed(vbi, f++, 0x15, 0x50, 0, 0);
	feed(vbi, f++, 0x15, 0x50, 0, 0);
	feed(vbi, f++, 'N', 'o', 0, 0);
	feed(vbi, f++, 0x14, 0x2F, 0, 0);
	feed(vbi, f++, 0x14, 0x2F, 0, 0);
	ok &= row_has(vbi, 1, 4, "No");

	vbi_decoder_delete(vbi);
	return ok;
}

/*
 *  The null pairs that fill the gaps between data are valid data on field 2
 *  as well as on field 1: after two of them the characters of a paint-on
 *  or roll-up caption that has been received are displayed (the word is
 *  complete).
 *  (Conformance suite v1.2 720p59, test 54 R11 on CC3: "B".)
 */
static int
test_field_2_null_pairs(void)
{
	vbi_decoder *vbi = vbi_decoder_new();
	int f = 0, ok = 1;

	feed(vbi, f++, -1, 0, 0x15, 0x29);	/* CC3 RDC */
	feed(vbi, f++, -1, 0, 0x14, 0x70);	/* PAC row 15 */
	feed(vbi, f++, -1, 0, 'A', 0);
	feed(vbi, f++, -1, 0, 0, 0);
	feed(vbi, f++, -1, 0, 0, 0);

	ok &= row_has(vbi, 3, 14, "A");
	vbi_decoder_delete(vbi);
	return ok;
}

int
main(void)
{
	int ok = 1;

	ok &= test_concurrent_channels();
	ok &= test_loss_of_waveform();
	ok &= test_invalid_parity();
	ok &= test_field_2_null_pairs();
	return ok ? 0 : 1;
}
