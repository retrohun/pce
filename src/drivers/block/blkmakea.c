/*****************************************************************************
 * makeab                                                                    *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/blkmakea.c                                 *
 * Created:     2018-05-20 by NASZVADI, Peter                                *
 * Copyright:   (C) 2018 by NASZVADI, Peter, based on Hampa's work           *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  General *
 * Public License for more details.                                          *
 *****************************************************************************/


#include "blkmakea.h"

#include <stdlib.h>
#include <string.h>

#define DSK_MAKEAB_MAGIC 0x41424f4f54000000UL

/*
 * MAKEAB image file format, used by amiga pc emulators
 *
 * 0    5    Magic (ABOOT)
 * 5    3    zero triplets (0, 0, 0)
 * 8    1    heads >> 8
 * 9    1    heads & 0xFF
 * 10   1    sectors >> 8
 * 11   1    sectors & 0xFF
 * 12   1    cylinders >> 8
 * 13   1    cylinders & 0xFF
 * 14   498  padded with 0xF6
 *
 * total 512 bytes, a sector inserted as a header, the image is basically raw
 */


static
int dsk_makeab_read (disk_t *dsk, void *buf, uint32_t i, uint32_t n)
{
	disk_makeab_t *img;
	uint64_t   ofs, cnt;

	img = dsk->ext;

	if ((i + n) > img->dsk.blocks) {
		return (1);
	}

	ofs = 512 + 512 * (uint64_t) i;
	cnt = 512 * (uint64_t) n;

	if (dsk_read (img->fp, buf, ofs, cnt)) {
		return (1);
	}

	return (0);
}

static
int dsk_makeab_write (disk_t *dsk, const void *buf, uint32_t i, uint32_t n)
{
	disk_makeab_t *img;
	uint64_t   ofs, cnt;

	img = dsk->ext;

	if (dsk->readonly) {
		return (1);
	}

	if ((i + n) > dsk->blocks) {
		return (1);
	}

	ofs = 512 + 512 * (uint64_t) i;
	cnt = 512 * (uint64_t) n;

	if (dsk_write (img->fp, buf, ofs, cnt)) {
		return (1);
	}

	fflush (img->fp);

	return (0);
}

static
void dsk_makeab_del (disk_t *dsk)
{
	disk_makeab_t *img;

	img = dsk->ext;

	if (img->fp != NULL) {
		fclose (img->fp);
	}

	free (img);
}

disk_t *dsk_makeab_open_fp (FILE *fp, int ro)
{
	disk_makeab_t    *img;
	uint32_t      c, h, s, n;
	unsigned char buf[1024];

	if (fread (buf, 1, 32, fp) != 32) {
		return (NULL);
	}

	if (dsk_get_uint64_be (buf, 0) != DSK_MAKEAB_MAGIC) {
		return (NULL);
	}

	h = dsk_get_uint16_be (buf, 8);
	s = dsk_get_uint16_be (buf, 10);
	c = dsk_get_uint16_be (buf, 12);
	n = h * s * c;

	img = malloc (sizeof (disk_makeab_t));
	if (img == NULL) {
		return (NULL);
	}

	dsk_init (&img->dsk, img, n, c, h, s);

	dsk_set_type (&img->dsk, PCE_DISK_MAKEAB);

	dsk_set_readonly (&img->dsk, ro);

	img->fp = fp;

	img->dsk.del = dsk_makeab_del;
	img->dsk.read = dsk_makeab_read;
	img->dsk.write = dsk_makeab_write;

	return (&img->dsk);
}

disk_t *dsk_makeab_open (const char *fname, int ro)
{
	disk_t *dsk;
	FILE   *fp;

	if (ro) {
		fp = fopen (fname, "rb");
	}
	else {
		fp = fopen (fname, "r+b");

		if (fp == NULL) {
			fp = fopen (fname, "rb");
			ro = 1;
		}
	}

	if (fp == NULL) {
		return (NULL);
	}

	dsk = dsk_makeab_open_fp (fp, ro);

	if (dsk == NULL) {
		fclose (fp);
		return (NULL);
	}

	dsk_set_fname (dsk, fname);

	return (dsk);
}

int dsk_makeab_create_fp (FILE *fp, uint32_t n, uint32_t c, uint32_t h, uint32_t s)
{
	int i;
	unsigned char buf[1024];

	if (dsk_adjust_chs (&n, &c, &h, &s)) {
		return (1);
	}

	dsk_set_uint64_be (buf, 0, DSK_MAKEAB_MAGIC);
	dsk_set_uint16_be (buf, 8, h);
	dsk_set_uint16_be (buf, 10, s);
	dsk_set_uint16_be (buf, 12, c);

	for(i = 14; i < 512; i += 2){
		dsk_set_uint16_be (buf, i, 0xF6F6);
	}

	if (dsk_write (fp, buf, 0, 512)) {
		return (1);
	}

	if (dsk_set_filesize (fp, 512 + 512 * (uint64_t) n)) {
		buf[0] = 0;
		if (dsk_write (fp, buf, 512 * (uint64_t) n + 511, 1)) {
			return (1);
		}
	}

	return (0);
}

int dsk_makeab_create (const char *fname, uint32_t n, uint32_t c, uint32_t h, uint32_t s)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "wb");
	if (fp == NULL) {
		return (1);
	}

	r = dsk_makeab_create_fp (fp, n, c, h, s);

	fclose (fp);

	return (r);
}

int dsk_makeab_probe_fp (FILE *fp)
{
	unsigned char buf[8];

	if (dsk_read (fp, buf, 0, 8)) {
		return (0);
	}

	if (dsk_get_uint64_be (buf, 0) != DSK_MAKEAB_MAGIC) {
		return (0);
	}

	return (1);
}

int dsk_makeab_probe (const char *fname)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "rb");

	if (fp == NULL) {
		return (0);
	}

	r = dsk_makeab_probe_fp (fp);

	fclose (fp);

	return (r);
}
