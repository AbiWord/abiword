/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef UTCONTEXTGLYPH_H
#define UTCONTEXTGLYPH_H

#include "ut_misc.h"

struct Letter
{
	UT_UCSChar code;
	UT_UCSChar initial;
	UT_UCSChar medial;
	UT_UCSChar final;
	UT_UCSChar alone;
};


class UT_contextGlyph
{
	public:
		UT_UCSChar getGlyph(const UT_UCSChar * code, const UT_UCSChar * prev, const UT_UCSChar * next) const;
	private:
		static Letter *s_pGlyphTable;
};
#endif
