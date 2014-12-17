/* AbiWord
 * Copyright (C) 2001 Sean Young <sean@mess.org>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gsf/gsf-input.h>

#include "ie_impexp_MSWrite.h"
#include "ut_debugmsg.h"
#include "ut_types.h"

bool read_wri_struct (wri_struct *w, GsfInput *f)
{
	int i, size;
	unsigned char *blob;
	bool result;

	// first we need to calculate the size
	i = size = 0;

	while (w[i].name) {
		size += w[i++].size;
	}

	if (size == 0) {
		return false;
	}
	// got the size, read the blob
	blob = static_cast<unsigned char *>(malloc(size));

	if (!blob)
	{
		UT_WARNINGMSG(("read_wri_struct: Out of memory!\n"));
		return false;
	}

	if (!gsf_input_read(f, size, blob))
	{
		UT_WARNINGMSG(("read_wri_struct: File not big enough!\n"));
		return false;
	}

	result = read_wri_struct_mem(w, blob);
	free(blob);

	return result;
}

bool read_wri_struct_mem (wri_struct *w, unsigned char *blob)
{
	int n;

	for (int i = 0; w[i].name; i++)
	{
		switch (w[i].type)
		{
			case CT_VALUE:
				n = w[i].size;
				w[i].value = 0;

				while (n--) w[i].value = (w[i].value * 256) + blob[n];

				break;

			case CT_BLOB:
				w[i].data = static_cast<char *>(malloc(w[i].size));

				if (!w[i].data)
				{
					UT_WARNINGMSG(("read_wri_struct_mem: Out of memory!\n"));
					return false;
				}

				memcpy(w[i].data, blob, w[i].size);
				break;

			case CT_IGNORE:
				break;
		}

		blob += w[i].size;
	}

	return true;
}

int wri_struct_value (const wri_struct *w, const char *name)
{
	for (int i = 0; w[i].name; i++)
		if (strcmp(w[i].name, name) == 0) return w[i].value;

	/* This should never happen! */
	UT_WARNINGMSG(("wri_struct_value: Internal error, '%s' not found!\n", name));
	exit(1);
	return 0;
}

void free_wri_struct (wri_struct *w)
{
	for (int i = 0; w[i].name; i++)
	{
		w[i].value = 0;

		if (w[i].data)
		{
			free(w[i].data);
			w[i].data = NULL;
		}
	}
}

void DEBUG_WRI_STRUCT (wri_struct *w, int spaces)
{
#ifdef DEBUG
	char sp[16], format[48], x[8];

	sprintf(sp, "%%-%d.%ds", spaces, spaces);

	for (int i = 0; w[i].name; i++)
	{
		switch (w[i].type)
		{
			case CT_VALUE:
				sprintf(x, "%%0%dX", w[i].size << 1);
				sprintf(format, "%s%%-13.13s= 0x%s (%%d)\n", sp, x);
				UT_DEBUGMSG((format, " ", w[i].name, w[i].value, w[i].value));
				break;

			case CT_BLOB:
				sprintf(format, "%s%%-13.13s: tblob (%%d)\n", sp);
				UT_DEBUGMSG((format, " ", w[i].name, w[i].size));
				break;

			case CT_IGNORE:
				sprintf(format, "%s%%-13.13s  ignored\n", sp);
				UT_DEBUGMSG((format, " ", w[i].name));
				break;
		}
	}
#else
	UT_UNUSED(w);
	UT_UNUSED(spaces);
#endif
}
