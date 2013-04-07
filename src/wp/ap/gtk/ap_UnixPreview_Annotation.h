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

#ifndef AP_UNIXPREVIEW_ANNOTATION_H
#define AP_UNIXPREVIEW_ANNOTATION_H

#include <gtk/gtk.h>
#include "xap_App.h"
#include "ap_Preview_Annotation.h"

class XAP_Frame;
class GR_CairoGraphics;

class AP_UnixPreview_Annotation : public AP_Preview_Annotation
{
public:
  AP_UnixPreview_Annotation(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixPreview_Annotation(void);

	virtual void			runModeless(XAP_Frame * pFrame);
	virtual void			destroy(void);
	virtual void			activate(void);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	void					_constructWindow(void);

private:
	// parent frame
	GR_CairoGraphics * 	m_gc;
	GtkWidget * 			m_pPreviewWindow;
	GtkWidget * 			m_pDrawingArea;
};

#endif /* AP_UNIXPREVIEW_ANNOTATION_H */
