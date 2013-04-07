/* AbiWord
 * Copyright (C) 2001 Sean Young <sean@mess.org>
 * Copyright (C) 2001 Hubert Figuiere
 * Copyright (C) 2001 Dom Lachowicz
 * Copyright (C) 2010-2011 Ingo Brueckl
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef IE_IMPEXP_MSWRITE_H
#define IE_IMPEXP_MSWRITE_H

#include <gsf/gsf.h>

#define CT_IGNORE 0
#define CT_VALUE  1
#define CT_BLOB   2

// the general structure for all structures
typedef struct
{
	int value;
	char *data;
	short size;         // in bytes
	short type;         // a CT_xxx from above
	const char *name;
} wri_struct;

bool read_wri_struct(wri_struct *w, GsfInput *f);
bool read_wri_struct_mem(wri_struct *w, unsigned char *blob);
int wri_struct_value(const wri_struct *w, const char *name);
void free_wri_struct(wri_struct *w);
void DEBUG_WRI_STRUCT(wri_struct *w, int spaces = 1);

// endian neutral helper macros

#define READ_WORD(d)  (*(d) + (*((d) + 1) << 8))

#define READ_DWORD(d) (*(d) + (*((d) + 1) << 8) + \
                              (*((d) + 2) << 16) + \
                              (*((d) + 3) << 24))

#define WRITE_WORD(d, s) \
{ \
	char *p = reinterpret_cast<char *>(&(d)); \
\
	p[0] = (s) & 0xff; \
	p[1] = ((s) & 0xff00) >> 8; \
}

#define WRITE_DWORD(d, s) \
{ \
	char *p = reinterpret_cast<char *>(&(d)); \
\
	p[0] = (s) & 0xff; \
	p[1] = ((s) & 0xff00) >> 8; \
	p[2] = ((s) & 0xff0000) >> 16; \
	p[3] = ((s) & 0xff000000) >> 24; \
}

#endif
