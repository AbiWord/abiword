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

#ifndef AP_UNIXFRAME_H
#define AP_UNIXFRAME_H

class GR_Graphics;

#include "xap_Frame.h"
#include "xap_UnixFrame.h"
#include "ie_types.h"

class XAP_UnixApp;
class AP_UnixFrame;

/*****************************************************************/
class AP_UnixFrameHelper : public XAP_UnixFrameHelper
{
 public:
	AP_UnixFrameHelper(AP_UnixFrame *pUnixFrame, XAP_UnixApp *pUnixApp); 

 protected:
	void _showOrHideStatusbar(void);

	void _showOrHideToolbars(void);
	virtual void _refillToolbarsInFrameData();
	void _bindToolbars(AV_View * pView);

	virtual GtkWidget * _createDocumentWindow();
	virtual GtkWidget * _createStatusBarWindow();

	friend class AP_UnixFrame;
	virtual void _setWindowIcon();
	GtkWidget * m_dArea;
	GtkAdjustment *	m_pVadj;
	GtkAdjustment *	m_pHadj;
	GtkWidget * m_hScroll;
	GtkWidget * m_vScroll;
	GtkWidget * m_topRuler;
	GtkWidget * m_leftRuler;
	GtkWidget * m_table;
	GtkWidget * m_innertable;
	GtkWidget * m_wSunkenBox;
};

/*****************************************************************/

class AP_UnixFrame : public XAP_Frame
{
public:
	AP_UnixFrame(XAP_UnixApp * app);
	AP_UnixFrame(AP_UnixFrame * f);
	virtual ~AP_UnixFrame(void);

	virtual bool				initialize(XAP_FrameMode frameMode=XAP_NormalFrame);
	virtual	XAP_Frame *			cloneFrame(void);
	virtual	XAP_Frame *			buildFrame(XAP_Frame * pFrame);
	virtual UT_Error   			loadDocument(const char * szFilename, int ieft);
	virtual UT_Error                        loadDocument(const char * szFilename, int ieft, bool createNew);
	virtual UT_Error            importDocument(const char * szFilename, int ieft, bool markClean);
	virtual bool				initFrameData(void);
	virtual void				killFrameData(void);

	// WL_REFACTOR: the implementation of these functions (e.g.: the widget stuff) to the frame helper
	virtual void				setXScrollRange(void);
	virtual void				setYScrollRange(void);
	virtual void				translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y);
	virtual void				setZoomPercentage(UT_uint32 iZoom);
	virtual UT_uint32			getZoomPercentage(void);
	virtual void				setStatusMessage(const char * szMsg);

	virtual void				toggleRuler(bool bRulerOn);
	virtual void                            toggleTopRuler(bool bRulerOn);
	virtual void                            toggleLeftRuler(bool bRulerOn);
	virtual void				toggleBar(UT_uint32 iBarNb, bool bBarOn);
	virtual void				toggleStatusBar(bool bStatusBarOn);
	
protected:
	friend class AP_UnixFrameHelper;
	UT_Error   					_loadDocument(const char * szFilename, IEFileType ieft, bool createNew);
	virtual UT_Error            _importDocument(const char * szFilename, int ieft, bool markClean);
	UT_Error   					_showDocument(UT_uint32 iZoom=100);
	static void					_scrollFuncX(void * pData, UT_sint32 xoff, UT_sint32 xlimit);
	static void					_scrollFuncY(void * pData, UT_sint32 yoff, UT_sint32 ylimit);
	UT_Error					_replaceDocument(AD_Document * pDoc);
};

#endif /* AP_UNIXFRAME_H */

