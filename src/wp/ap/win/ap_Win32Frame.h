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

	virtual bool				initialize(void);
	virtual	XAP_Frame *			cloneFrame(void);
	virtual UT_Error			loadDocument(const char * szFilename, int ieft);
	virtual UT_Error			loadDocument(const char * szFilename, int ieft, bool createNew);
	virtual UT_Error            importDocument(const char * szFilename, int ieft, bool markClean);
	virtual bool				initFrameData(void);
	virtual void				killFrameData(void);

	virtual void				setXScrollRange(void);
	virtual void				setYScrollRange(void);
	virtual void				translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y);

	virtual void				setZoomPercentage(UT_uint32 iZoom);
	virtual UT_uint32			getZoomPercentage(void);
	virtual void				setStatusMessage(const char * szMsg);

	static bool				RegisterClass(XAP_Win32App * app);

	virtual void				toggleRuler(bool bRulerOn);

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
	virtual UT_Error            _importDocument(const char * szFilename, int ieft, bool markClean);
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
	virtual void				toggleBar(UT_uint32 iBarNb, bool bBarOn );
	virtual void				toggleStatusBar(bool bStatusBarOn);
	virtual bool				getBarVisibility(UT_uint32 iBarNb) { return true; }

	void						_startTracking(UT_sint32 x, UT_sint32 y);
	void						_endTracking(UT_sint32 x, UT_sint32 y);
	void						_track(UT_sint32 x, UT_sint32 y);
	bool						_isTracking() const
								{
									return m_bMouseWheelTrack;
								}

	void						_showOrHideToolbars(void);
	void						_showOrHideStatusbar(void);

	bool						m_bMouseWheelTrack;
	bool						m_bMouseActivateReceived;
	HWND						m_hWndHScroll;
	HWND						m_hWndVScroll;
	HWND						m_hWndGripperHack;
	UT_sint32					m_startMouseWheelY;
	UT_sint32					m_startScrollPosition;
};

#endif /* AP_WIN32FRAME_H */
