/* AbiWord
 * Copyright (C) 2002 Dom Lachowicz and others
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

#ifndef AP_FRAME_H
#define AP_FRAME_H

#include "ut_types.h"
#if defined(XP_UNIX_TARGET_GTK) || (defined(__APPLE__) && defined(__MACH__)) || defined(WIN32) || defined(__QNXNTO__)
#include "xap_Frame.h"
#include "fv_View.h"
#include "fl_DocLayout.h"

class ABI_EXPORT AP_Frame : public XAP_Frame
{
 public:
	AP_Frame(XAP_FrameImpl *pFrameImpl, XAP_App *pApp) : XAP_Frame(pFrameImpl, pApp) {}
	AP_Frame(AP_Frame *pFrame) : XAP_Frame(static_cast<XAP_Frame *>(pFrame)) {}
	virtual ~AP_Frame();

	virtual bool				initialize(XAP_FrameMode frameMode=XAP_NormalFrame) = 0;
	virtual	XAP_Frame *			buildFrame(XAP_Frame * pFrame);
	virtual UT_Error   			loadDocument(const char * szFilename, int ieft);
	virtual UT_Error                        loadDocument(const char * szFilename, int ieft, bool createNew);
	virtual UT_Error                        importDocument(const char * szFilename, int ieft, bool markClean);
	virtual bool				initFrameData(void);
	virtual void				killFrameData(void);
	UT_uint32                   getNewZoom(XAP_Frame::tZoomType * tZoom);
 protected:

	UT_Error _loadDocument(const char * szFilename, IEFileType ieft, bool createNew);
	virtual UT_Error _importDocument(const char * szFilename, int ieft, bool markClean);
	UT_Error _replaceDocument(AD_Document * pDoc);
	virtual UT_Error _showDocument(UT_uint32 iZoom = 100);

	// helper methods for _showDocument
	virtual bool _createViewGraphics(GR_Graphics *& pG, UT_uint32 iZoom) = 0;
	virtual void _replaceView(GR_Graphics * pG, FL_DocLayout *pDocLayout,
			  AV_View *pView, AV_ScrollObj * pScrollObj,
			  ap_ViewListener *pViewListener, AD_Document *pOldDoc,
			  ap_Scrollbar_ViewListener *pScrollbarViewListener,
			  AV_ListenerId lid, AV_ListenerId lidScrollbarViewListener,
			  UT_uint32 iZoom);
	virtual bool _createScrollBarListeners(AV_View * pView, AV_ScrollObj *& pScrollObj, 
				       ap_ViewListener *& pViewListener, 
				       ap_Scrollbar_ViewListener *& pScrollbarViewListener,
				       AV_ListenerId &lid, 
				       AV_ListenerId &lidScrollbarViewListener) = 0;	
	virtual void _bindToolbars(AV_View *pView) = 0;
	virtual void _setViewFocus(AV_View *pView) = 0;

	// helper methods for helper methods for _showDocument (meta-helper-methods?) :-)
	virtual UT_sint32 _getDocumentAreaWidth() = 0;
	virtual UT_sint32 _getDocumentAreaHeight() = 0;

 private:
	void _resetInsertionPoint();
};
#else 
class AP_Frame
{
 public:

  virtual ~AP_Frame () ;

 private:
};
#endif
#endif // AP_FRAME_H
