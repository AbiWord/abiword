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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef AP_WIN32FRAME_H
#define AP_WIN32FRAME_H

#include "ie_types.h"
#include "ut_assert.h"
#include "ap_Frame.h"
#include "ap_Win32FrameImpl.h"
#include "xap_Win32App.h"
#include "ap_StatusBar.h"

/*****************************************************************/

class ABI_EXPORT AP_Win32Frame : public AP_Frame
{
 public:
	AP_Win32Frame();
	AP_Win32Frame(AP_Win32Frame * f);
	virtual ~AP_Win32Frame(void);

	virtual bool				initialize(XAP_FrameMode frameMode=XAP_NormalFrame);
	virtual XAP_Frame *			cloneFrame(void);

	virtual void				setStatusMessage(const char * szMsg);
	virtual void				setXScrollRange(void) {  getAPWin32FrameImpl()->_setXScrollRange(static_cast<AP_FrameData*>(m_pData), m_pView);  }
	virtual void				setYScrollRange(void) {  getAPWin32FrameImpl()->_setYScrollRange(static_cast<AP_FrameData*>(m_pData), m_pView);  }

	static bool 				RegisterClass(XAP_Win32App * app) {  return AP_Win32FrameImpl::_RegisterClass(app); }

	virtual void 				toggleRuler(bool bRulerOn)     {  toggleTopRuler(bRulerOn);  toggleLeftRuler(bRulerOn);  }
	virtual void				toggleTopRuler(bool bRulerOn)  {  getAPWin32FrameImpl()->_toggleTopRuler(this, bRulerOn);  }
	virtual void				toggleLeftRuler(bool bRulerOn) {  getAPWin32FrameImpl()->_toggleLeftRuler(this, bRulerOn);  }

	virtual HWND				getTopLevelWindow(void) const {  return getAPWin32FrameImpl()->_getTopLevelWindow();  }

	void 						toggleBar(UT_uint32 iBarNb, bool bBarOn) { getAPWin32FrameImpl()->_toggleBar(iBarNb, bBarOn); }
	void 						toggleStatusBar(bool bStatusBarOn);

	// save from thousands of casts
	AP_Win32FrameImpl *			getAPWin32FrameImpl(void) const { return static_cast<AP_Win32FrameImpl *>(getFrameImpl()); }
	AP_FrameData *				getAPFrameData(void) const { return static_cast<AP_FrameData*>(getFrameData()); }

 protected:
	// helper methods for _showDocument
	virtual bool _createViewGraphics(GR_Graphics *& pG, UT_uint32 iZoom);
	virtual void _setViewFocus(AV_View *pView);
	virtual bool _createScrollBarListeners(AV_View * pView, AV_ScrollObj *& pScrollObj,
				      ap_ViewListener *& pViewListener,
				      ap_Scrollbar_ViewListener *& pScrollbarViewListener,
				      AV_ListenerId &lid,
				      AV_ListenerId &lidScrollbarViewListener);
	virtual void _bindToolbars(AV_View *pView) { getAPWin32FrameImpl()->_bindToolbars(pView); }
	virtual void _replaceView(GR_Graphics * pG, FL_DocLayout *pDocLayout,
			  AV_View *pView, AV_ScrollObj * pScrollObj,
			  ap_ViewListener *pViewListener, AD_Document *pOldDoc,
			  ap_Scrollbar_ViewListener *pScrollbarViewListener,
			  AV_ListenerId lid, AV_ListenerId lidScrollbarViewListener,
			  UT_uint32 iZoom);

	// helper methods for helper methods for _showDocument (meta-helper-methods?) :-)
	virtual UT_sint32 _getDocumentAreaWidth(void) { return getAPWin32FrameImpl()->_getDocumentAreaWidth(); }
	virtual UT_sint32 _getDocumentAreaHeight(void) { return getAPWin32FrameImpl()->_getDocumentAreaHeight(); }

 private:

	static void					_scrollFuncX(void * pData, UT_sint32 xoff, UT_sint32 xlimit);
	static void					_scrollFuncY(void * pData, UT_sint32 yoff, UT_sint32 ylimit);
};

#endif /* AP_WIN32FRAME_H */
