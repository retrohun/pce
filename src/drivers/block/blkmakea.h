/*****************************************************************************
 * makeab                                                                    *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/blkmakea.h                                 *
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


#ifndef PCE_DEVICES_BLOCK_BLKMAKEA_H
#define PCE_DEVICES_BLOCK_BLKMAKEA_H 1


#include <config.h>

#include <drivers/block/block.h>

#include <stdio.h>
#include <stdint.h>


/*!***************************************************************************
 * @short The makeab image file disk structure
 *****************************************************************************/
typedef struct {
	disk_t   dsk;
	FILE     *fp;
} disk_makeab_t;


disk_t *dsk_makeab_open_fp (FILE *fp, int ro);
disk_t *dsk_makeab_open (const char *fname, int ro);

int dsk_makeab_create_fp (FILE *fp,
	uint32_t n, uint32_t c, uint32_t h, uint32_t s
);

int dsk_makeab_create (const char *fname,
	uint32_t n, uint32_t c, uint32_t h, uint32_t s
);

int dsk_makeab_probe_fp (FILE *fp);
int dsk_makeab_probe (const char *fname);


#endif
