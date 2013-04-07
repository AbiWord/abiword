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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef AP_FRAME_H
#define AP_FRAME_H

#include <vector>
#include "ut_types.h"
#include "xap_Frame.h"
#include "ie_types.h"
#include "ap_FrameListener.h"

class AV_View;
class ap_Scrollbar_ViewListener;
class ap_ViewListener;
class FL_DocLayout;

class ABI_EXPORT AP_Frame : public XAP_Frame
{
 public:
  AP_Frame(XAP_FrameImpl *pFrameImpl) : XAP_Frame(pFrameImpl),m_bShowMargin(false),m_bWordSelections(false) {}
    AP_Frame(AP_Frame *pFrame) : XAP_Frame(static_cast<XAP_Frame *>(pFrame)),m_bShowMargin(false){}
	virtual ~AP_Frame();

	virtual bool				initialize(XAP_FrameMode frameMode=XAP_NormalFrame) = 0;
	virtual	XAP_Frame *			buildFrame(XAP_Frame * pFrame);
	virtual UT_Error   			loadDocument(AD_Document* pDoc);
	virtual UT_Error   			loadDocument(const char * szFilename, int ieft);
	virtual UT_Error			loadDocument(const char * szFilename, int ieft, bool createNew);
	virtual UT_Error			loadDocument(GsfInput * input, int ieft);
	virtual UT_Error			importDocument(const char * szFilename, int ieft, bool markClean);
	virtual bool				initFrameData(void);
	virtual void				killFrameData(void);
	UT_uint32                   getNewZoom(XAP_Frame::tZoomType * tZoom);
	virtual void				setZoomPercentage(UT_uint32 iZoom);
	virtual UT_uint32			getZoomPercentage(void);
	virtual void                quickZoom(UT_uint32 iZoom);
	bool                        isShowMargin(void) const
	{ return m_bShowMargin;}
	void                        setShowMargin(bool b)
	{ m_bShowMargin = b;}
	void                        setDoWordSelections(bool b)
	{ m_bWordSelections = b;}
	bool                        getDoWordSelections(void) const
	{ return m_bWordSelections;}

	UT_sint32					registerListener(AP_FrameListener* pListener);
	void						unregisterListener(UT_sint32 iListenerId);

 protected:

	UT_Error _loadDocument(const char * szFilename, IEFileType ieft, bool createNew);
	UT_Error _loadDocument(GsfInput * input, IEFileType ieft);
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

	void _signal(AP_FrameSignal sig);

 private:
	bool    m_bShowMargin;
	bool    m_bWordSelections;
	std::vector<AP_FrameListener*> m_listeners;
};
#endif // AP_FRAME_H
