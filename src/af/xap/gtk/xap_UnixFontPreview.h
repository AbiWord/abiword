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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef XAP_UNIXFONTPREVIEW_H
#define XAP_UNIXFONTPREVIEW_H

#include <gtk/gtk.h>
#include "xap_App.h"
#include "xap_FontPreview.h"

class XAP_Frame;
class GR_CairoGraphics;

class XAP_UnixFontPreview : public XAP_FontPreview
{
public:
	XAP_UnixFontPreview(XAP_Frame * pFrame, UT_sint32 left, UT_uint32 top);
	virtual ~XAP_UnixFontPreview(void);

	GR_CairoGraphics * 		m_gc;
protected:
private:
	// parent frame
	XAP_Frame *			m_pFrame;
	GtkWidget * 			m_pPreviewWindow;
	GtkWidget * 			m_pDrawingArea;
	UT_sint32				m_left;
	UT_sint32				m_top;
};

#endif /* XAP_UNIXFONTPREVIEW_H */
