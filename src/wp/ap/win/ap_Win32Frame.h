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

#include "xap_Win32Frame.h"
#include "ie_types.h"

/*****************************************************************/

class AP_Win32Frame : public XAP_Win32Frame
{
public:
	AP_Win32Frame(XAP_Win32App * app);
	AP_Win32Frame(AP_Win32Frame * f);
	virtual ~AP_Win32Frame(void);

	virtual UT_Bool				initialize(void);
	virtual	XAP_Frame *			cloneFrame(void);
	virtual UT_Error			loadDocument(const char * szFilename, int ieft);
	virtual UT_Bool				initFrameData(void);
	virtual void				killFrameData(void);

	virtual void				setXScrollRange(void);
	virtual void				setYScrollRange(void);
	virtual void				translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y);

	virtual void				setZoomPercentage(UT_uint32 iZoom);
	virtual UT_uint32			getZoomPercentage(void);
	virtual void				setStatusMessage(const char * szMsg);
	
	static UT_Bool				RegisterClass(XAP_Win32App * app);

	virtual void				toggleRuler(UT_Bool bRulerOn);

protected:
	virtual HWND				_createDocumentWindow(HWND hwndParent,
													  UT_uint32 iLeft, UT_uint32 iTop,
													  UT_uint32 iWidth, UT_uint32 iHeight);
	virtual HWND				_createStatusBarWindow(HWND hwndParent,
													   UT_uint32 iLeft, UT_uint32 iTop,
													   UT_uint32 iWidth);
	
	void						_createRulers(void);
	void						_getRulerSizes(int &yTopRulerHeight, int &xLeftRulerWidth);
	void						_onSize(int nWidth, int nHeight);

	UT_Error					_loadDocument(const char * szFilename, IEFileType ieft);
	UT_Error					_showDocument(UT_uint32 iZoom=100);
	static void					_scrollFuncX(void * pData, UT_sint32 xoff, UT_sint32 xlimit);
	static void					_scrollFuncY(void * pData, UT_sint32 yoff, UT_sint32 ylimit);

	static LRESULT CALLBACK		_ContainerWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK		_LeftRulerWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK		_DocumentWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

	void						_setVerticalScrollInfo(const SCROLLINFO * psi);
	void						_getVerticalScrollInfo(SCROLLINFO * psi);

	UT_Error					_replaceDocument(AD_Document * pDoc);

	HWND						m_hwndContainer;
	HWND						m_hwndTopRuler;
	HWND						m_hwndLeftRuler;
	HWND						m_hwndDocument;	/* the actual document window */

	UT_uint32					m_vScale; /* vertical scroll scaling to get around 16-bit scrollbar problems */

private:

	UT_Bool m_bMouseWheelTrack;
	UT_sint32 m_startMouseWheelY, m_startScrollPosition;

	void _startTracking(UT_sint32 x, UT_sint32 y);
	void _endTracking(UT_sint32 x, UT_sint32 y)
	{
		m_bMouseWheelTrack = UT_FALSE;
		ReleaseCapture();
	}
	void _track(UT_sint32 x, UT_sint32 y);
	UT_Bool _isTracking()
	{
		return m_bMouseWheelTrack;
	}
};

#endif /* AP_WIN32FRAME_H */
