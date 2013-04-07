/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2009 Hubert Figuiere
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

#ifndef XAP_FONTPREVIEW_H
#define XAP_FONTPREVIEW_H

#include <string>
#include <map>

#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "xap_Dlg_FontChooser.h"

class GR_Graphics;

// some hardcoded values for the preview window size
#define PREVIEW_WIDTH 400
#define PREVIEW_HEIGHT 75

class ABI_EXPORT XAP_FontPreview
{
public:
	typedef std::map<std::string,std::string> PropMap;

	XAP_FontPreview();
	virtual ~XAP_FontPreview(void);

	void							addOrReplaceVecProp(const std::string & pszProp,
														const std::string & pszVal);
	void							setFontFamily(const gchar * pFontFamily);
	void							setText(const gchar * pFontFamily);
	void							draw(void);
protected:
	void                            _createFontPreviewFromGC(GR_Graphics * gc,
															 UT_uint32 width,
															 UT_uint32 height);
	UT_sint32						m_width;
	UT_sint32						m_height;
private:
	XAP_Preview_FontPreview *       m_pFontPreview;
	UT_UCSChar *                    m_drawString;
	PropMap 						m_mapProps;
};

#endif /* XAP_FONTPREVIEW_H */
