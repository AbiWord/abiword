/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

#ifndef AP_COCOAPREVIEW_ANNOTATION_H
#define AP_COCOAPREVIEW_ANNOTATION_H

#include <Cocoa/Cocoa.h>

#include "ap_Preview_Annotation.h"

class XAP_Frame;
class GR_CairoGraphics;
@class XAP_CocoaNSView;

class AP_CocoaPreview_Annotation : public AP_Preview_Annotation
{
public:
  AP_CocoaPreview_Annotation(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaPreview_Annotation(void);

	virtual void			runModeless(XAP_Frame * pFrame);
	virtual void			destroy(void);
	virtual void			activate(void);
	virtual void			notifyActiveFrame(XAP_Frame *pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);
	void					_constructWindow(void);

/*protected:
	void	_bringToTop(void);*/

private:
	// parent frame
	GR_CairoGraphics * 	m_gc;
	NSWindow * 			m_pPreviewWindow;
	XAP_CocoaNSView * 			m_pDrawingArea;
};

#endif /* AP_COCOAPREVIEW_ANNOTATION_H */
