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

#ifndef AP_BEOSFRAME_H
#define AP_BEOSFRAME_H

#include "xap_BeOSFrame.h"
#include "ie_types.h"
#include "ut_assert.h"

/*****************************************************************/

class AP_BeOSFrame : public XAP_BeOSFrame
{
public:
	AP_BeOSFrame(XAP_BeOSApp * app);
	AP_BeOSFrame(AP_BeOSFrame * f);
	virtual ~AP_BeOSFrame(void);

	virtual bool				initialize(void);
	virtual XAP_Frame *			cloneFrame(void);
	virtual XAP_Frame *			buildFrame(XAP_Frame * pF);
	virtual UT_Error 			loadDocument(const char * szFilename, int ieft);
	virtual UT_Error 			loadDocument(const char * szFilename, int ieft, bool createNew);
	virtual UT_Error 			importDocument(const char * szFilename, int ieft, bool markClean = false);
	virtual bool				initFrameData(void);
	virtual void				killFrameData(void);

	virtual void				setXScrollRange(void);
	virtual void				setYScrollRange(void);

	virtual void 				setZoomPercentage(UT_uint32 iZoom);
	virtual UT_uint32 			getZoomPercentage(void);
	virtual void 				setStatusMessage(const char * szMsg);
	
	virtual void				toggleRuler(bool bRulerOn);
	virtual void				toggleTopRuler(bool bRulerOn);
	virtual void				toggleLeftRuler(bool bRulerOn);
	virtual void 				toggleBar(UT_uint32 iBarNb, bool bToolBarOn);
	virtual void 				toggleStatusBar(bool bStatusBarOn);
	virtual void  				translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y);
	virtual void				refillToolbarsInFrameData(void) {UT_ASSERT(0);}

protected:
	//virtual GtkWidget *			_createDocumentWindow(void);
	UT_Error				_loadDocument(const char * szFilename, IEFileType ieft, bool createNew);
	UT_Error				_showDocument(UT_uint32 zoom=100);
	static void			_scrollFuncX(void * pData, UT_sint32 xoff, UT_sint32 xlimit);
	static void			_scrollFuncY(void * pData, UT_sint32 yoff, UT_sint32 ylimit);
	UT_Error				_replaceDocument(AD_Document * pDoc);
	UT_Error				_importDocument(const char * szFilename, int ieft, bool markClean);

/*
	GtkAdjustment *				m_pVadj;
	GtkAdjustment *				m_pHadj;
	GtkWidget *					m_hScroll;
	GtkWidget *					m_vScroll;
	GtkWidget *					m_dArea;
	GtkWidget *					m_table;
	GtkWidget *					m_topRuler;
	GtkWidget *					m_leftRuler;
*/
};

#endif /* AP_BEOSFRAME_H */
