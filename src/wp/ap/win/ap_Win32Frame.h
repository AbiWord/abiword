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
	AP_Win32Frame(XAP_Win32App * app);
	AP_Win32Frame(AP_Win32Frame * f);
	virtual ~AP_Win32Frame(void);

	virtual bool				initialize(XAP_FrameMode frameMode=XAP_NormalFrame);
	virtual XAP_Frame *			cloneFrame(void);

	virtual void				setXScrollRange(void) {  static_cast<AP_Win32FrameImpl*>(getFrameImpl())->_setXScrollRange(static_cast<AP_FrameData*>(m_pData), m_pView);  }
	virtual void				setYScrollRange(void) {  static_cast<AP_Win32FrameImpl*>(getFrameImpl())->_setYScrollRange(static_cast<AP_FrameData*>(m_pData), m_pView);  }

	virtual void				setStatusMessage(const char * szMsg) {  static_cast<AP_FrameData*>(m_pData)->m_pStatusBar->setStatusMessage(szMsg);  }

	static bool 				RegisterClass(XAP_Win32App * app) {  return AP_Win32FrameImpl::_RegisterClass(app); }

	virtual void				toggleTopRuler(bool bRulerOn);
	virtual void				toggleLeftRuler(bool bRulerOn);

	virtual HWND				getTopLevelWindow(void) const {  return static_cast<AP_Win32FrameImpl*>(getFrameImpl())->_getTopLevelWindow();  }

 protected:
	// helper methods for _showDocument
	virtual bool _createViewGraphics(GR_Graphics *& pG, UT_uint32 iZoom);
	virtual bool _createScrollBarListeners(AV_View * pView, AV_ScrollObj *& pScrollObj, 
				       ap_ViewListener *& pViewListener, 
				       ap_Scrollbar_ViewListener *& pScrollbarViewListener,
				       AV_ListenerId &lid, 
				       AV_ListenerId &lidScrollbarViewListener);	
	virtual void _bindToolbars(AV_View *pView);
	virtual void _setViewFocus(AV_View *pView);

	// helper methods for helper methods for _showDocument (meta-helper-methods?) :-)
	virtual UT_sint32 _getDocumentAreaWidth();
	virtual UT_sint32 _getDocumentAreaHeight();

 private:
};

#endif /* AP_WIN32FRAME_H */
