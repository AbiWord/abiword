/* AbiSource Application Framework
 * Copyright (C) 1998-2002 AbiSource, Inc.
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

#ifndef XAP_UNIXFONTPREVIEW_H
#define XAP_UNIXFONTPREVIEW_H

#include <gtk/gtk.h>
#include "xap_App.h"
#include "xap_FontPreview.h"
#ifndef WITH_PANGO
#include "xap_UnixFontManager.h"
#else
#include "xap_PangoFontManager.h"
#endif

class XAP_UnixFrame;
class GR_UnixGraphics;

class XAP_UnixFontPreview : public XAP_FontPreview
{
public:
	XAP_UnixFontPreview(XAP_Frame * pFrame, UT_sint32 left, UT_uint32 top);
	virtual ~XAP_UnixFontPreview(void);
	
	GR_UnixGraphics * 		m_gc;
protected:
private:
	// parent frame
	XAP_UnixFrame *			m_pUnixFrame;
	GtkWidget * 			m_pPreviewWindow;
	GtkWidget * 			m_pDrawingArea;
	UT_sint32				m_left;
	UT_sint32				m_top;
};

#endif /* XAP_UNIXFONTPREVIEW_H */
