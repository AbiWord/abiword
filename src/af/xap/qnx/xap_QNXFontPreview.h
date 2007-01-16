/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

#ifndef XAP_QNXFONTPREVIEW_H
#define XAP_QNXFONTPREVIEW_H

#include "xap_App.h"
#include "xap_FontPreview.h"

class XAP_QNXFrame;
class GR_QNXGraphics;

class XAP_QNXFontPreview : public XAP_FontPreview
{
public:
	XAP_QNXFontPreview(XAP_Frame * pFrame, UT_sint32 left, UT_uint32 top);
	virtual ~XAP_QNXFontPreview(void);
	
	GR_QNXGraphics * 		m_gc;
protected:

private:
	// parent frame
	XAP_QNXFrame *			m_pQNXFrame;
	PtWidget_t * 			m_pPreviewWindow;
	PtWidget_t * 			m_pDrawingArea;
	UT_sint32				m_left;
	UT_sint32				m_top;
};

#endif /* XAP_QNXFONTPREVIEW_H */
