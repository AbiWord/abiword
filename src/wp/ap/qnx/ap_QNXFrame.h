/* AbiWord
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

#ifndef AP_QNXFRAME_H
#define AP_QNXFRAME_H

class GR_Graphics;

#include "xap_QNXFrame.h"
#include "ie_types.h"
#include <Pt.h>

/*****************************************************************/

class AP_QNXFrame : public XAP_QNXFrame
{
public:
	AP_QNXFrame(XAP_QNXApp * app);
	AP_QNXFrame(AP_QNXFrame * f);
	virtual ~AP_QNXFrame(void);

	virtual UT_Bool				initialize(void);
	virtual	XAP_Frame *			cloneFrame(void);
	virtual UT_Bool				loadDocument(const char * szFilename, int ieft);
	virtual UT_Bool				initFrameData(void);
	virtual void				killFrameData(void);

	virtual void				setXScrollRange(void);
	virtual void				setYScrollRange(void);
	virtual void				translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y);
	virtual void				setZoomPercentage(UT_uint32 iZoom);
	virtual UT_uint32			getZoomPercentage(void);
	virtual void				setStatusMessage(const char * szMsg);

protected:
	virtual PtWidget_t *			_createDocumentWindow(void);
	virtual void *			_createStatusBarWindow(void);
	virtual void				_setWindowIcon(void);
	UT_Bool						_loadDocument(const char * szFilename, IEFileType ieft);
	UT_Bool						_showDocument(UT_uint32 iZoom=100);
	static void					_scrollFuncX(void * pData, UT_sint32 xoff, UT_sint32 xlimit);
	static void					_scrollFuncY(void * pData, UT_sint32 yoff, UT_sint32 ylimit);
	UT_Bool						_replaceDocument(AD_Document * pDoc);
	

	PtWidget_t *				m_dArea;

	void *				m_pVadj;
	void *				m_pHadj;
	PtWidget_t *			m_hScroll;
	PtWidget_t *			m_vScroll;
	void *				m_table;
	PtWidget_t *			m_topRuler;
	PtWidget_t *			m_leftRuler;
};

#endif /* AP_QNXFRAME_H */
