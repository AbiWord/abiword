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

#ifndef XAP_DIALOG_ZOOM_H
#define XAP_DIALOG_ZOOM_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xap_Preview_Zoom.h"

#include "xav_View.h"

class XAP_Frame;

class XAP_Dialog_Zoom : public XAP_Dialog_NonPersistent
{
public:
	XAP_Dialog_Zoom(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Dialog_Zoom(void);

	virtual void					runModal(XAP_Frame * pFrame) = 0;

	typedef enum { a_OK, a_CANCEL } tAnswer;
	typedef enum { z_200, z_100, z_75, z_PAGEWIDTH, z_WHOLEPAGE, z_PERCENT } zoomType;	

	XAP_Dialog_Zoom::tAnswer		getAnswer(void) const;

	// supply this on run
	void							setZoomPercent(UT_uint32 zoom);

	// read these back
	XAP_Dialog_Zoom::zoomType		getZoomType(void);
	UT_uint32						getZoomPercent(void);
	
protected:

	// handle the XP-job of telling m_zoomPreview what to draw
	void							_updatePreviewZoomPercent(UT_uint32 percent);

	// handle the XP-job of attaching something to our m_zoomPreview
	void							_createPreviewFromGC(GR_Graphics * gc,
														 UT_uint32 width,
														 UT_uint32 height);

	XAP_Preview_Zoom * 				m_zoomPreview;
	
	XAP_Dialog_Zoom::zoomType		m_zoomType;
	UT_uint32						m_zoomPercent;

	XAP_Dialog_Zoom::tAnswer		m_answer;
};

#endif /* XAP_DIALOG_ZOOM_H */
