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


#ifndef XAP_PREVIEW_ZOOM_H
#define XAP_PREVIEW_ZOOM_H

#include "ut_misc.h"
#include "ut_types.h"

#include "xap_Preview.h"

class XAP_Preview_Zoom : public XAP_Preview
{
public:

	XAP_Preview_Zoom(GR_Graphics * gc);
	virtual ~XAP_Preview_Zoom(void);

	// example placements useful in zoom
	typedef enum { pos_TOP, pos_CENTER, pos_BOTTOM } tPos;
	// example fonts useful in zoom (add more later)
	typedef enum { font_NORMAL } tFont;
				
	// data twiddlers
	void	setDrawAtPosition(XAP_Preview_Zoom::tPos pos);
	void	setFont(XAP_Preview_Zoom::tFont f);
	void	setZoomPercent(UT_uint32 percent);

	// set the string you'd like to display
	UT_Bool	setString(const char * string);
	UT_Bool	setString(UT_UCSChar * string);

    // where all the zoom-specific drawing happens
	void				draw(void);
	
protected:

	XAP_Preview_Zoom::tPos	m_pos;
	XAP_Preview_Zoom::tFont m_previewFont;
	UT_uint32				m_zoomPercent;
	
	UT_UCSChar * 			m_string;
};

#endif /* XAP_PREVIEW_ZOOM_H */
