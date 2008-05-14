/* AbiSource Application Framework
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

#ifndef XAP_FONTPREVIEW_H
#define XAP_FONTPREVIEW_H

#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_string.h"
#include "ut_vector.h"
#include "gr_Graphics.h"
#include "xap_Dlg_FontChooser.h"

// some hardcoded values for the preview window size
#define PREVIEW_WIDTH 400
#define PREVIEW_HEIGHT 75

class ABI_EXPORT XAP_FontPreview
{
public:
	XAP_FontPreview();	
	virtual ~XAP_FontPreview(void);

	void							addOrReplaceVecProp(const gchar * pszProp,
														const gchar * pszVal);
	void							setFontFamily(const gchar * pFontFamily);
	void							setText(const gchar * pFontFamily);
	void							draw(void);
protected:
	void                            _createFontPreviewFromGC(GR_Graphics * gc,
															 UT_uint32 width,
															 UT_uint32 height);
	const gchar *                m_pColorBackground;
	UT_sint32						m_width;
	UT_sint32						m_height;
private:
	XAP_Preview_FontPreview *       m_pFontPreview;
	UT_UCSChar *                    m_drawString;
	UT_Vector						m_vecProps;
};

#endif /* XAP_FONTPREVIEW_H */
