/* AbiWord
 * Copyright (c) 2005 Martin Sevior <msevior@physics.unimelb.edu.au>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef FV_UNIXVISUALINLINEIMAGE_H
#define FV_UNIXVISUALINLINEIMAGE_H

#include "pt_Types.h"
#include "fl_FrameLayout.h"
#include "ut_string_class.h"
#include "fv_InlineImage.h"

class FV_View;

class ABI_EXPORT FV_UnixVisualInlineImage : public FV_VisualInlineImage
{

public:

	FV_UnixVisualInlineImage (FV_View * pView);
	virtual ~FV_UnixVisualInlineImage();
	virtual void mouseDrag(UT_sint32 x, UT_sint32 y);
private:
	bool    m_bDragOut;
};

#endif /* FV_UNIXVISUALINLINEIMAGE_H */
