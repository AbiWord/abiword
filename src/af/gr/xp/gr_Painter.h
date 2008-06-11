/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz <cinamod@hotmail.com>
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

#ifndef GR_PAINTER_H
#define GR_PAINTER_H

#include "xap_Features.h"
#include "gr_Graphics.h"

class ABI_EXPORT GR_Painter
{
public:

	GR_Painter (GR_Graphics * pGr);
	GR_Painter (GR_Graphics * pGr, bool bCaret);
	~GR_Painter ();

private:

	GR_Painter ();
	GR_Painter (const GR_Painter & rhs);
	GR_Painter& operator=(const GR_Painter & rhs);

	GR_Graphics * m_pGr;
	GR_CaretDisabler * m_pCaretDisabler;
	UT_GenericVector<GR_CaretDisabler *> m_vecDisablers;
};

#endif // GR_PAINTER_H
