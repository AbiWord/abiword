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

#ifndef XAP_WIN32PREVIEWWIDGET_H
#define XAP_WIN32PREVIEWWIDGET_H

// this defines the base class for a preview widget
// that will be used in various dialogs.  subclass
// this to define the drawing/interaction necessary
// for a specific dialog.

#include "ut_types.h"
#include "xap_Win32App.h"
#include "gr_Win32Graphics.h"
#include "xap_Preview.h"
#include "xap_Win32Dlg_Insert_Symbol.h"

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class XAP_Win32PreviewWidget
{
public:
	XAP_Win32PreviewWidget(XAP_Win32App * pWin32App,
						   HWND hwndParent,
						   UINT style);			// pass CS_DBLCLKS or zero
	virtual ~XAP_Win32PreviewWidget(void);

	inline HWND					getWindow(void)		const { return m_hwndPreview; };
	inline GR_Win32Graphics *	getGraphics(void)	const { return m_pGraphics; };
	void						getWindowSize(UT_uint32 * pWidth, UT_uint32 * pHeight) const;
	inline void					setPreview(XAP_Preview * pPreview) { m_pPreview = pPreview; };
	inline void					setInsertSymbolParent(XAP_Win32Dialog_Insert_Symbol *pParent) { m_pInsertSymbol = pParent; };

	virtual LRESULT				onPaint(HWND hwnd);
	virtual LRESULT				onLeftButtonDown(UT_sint32 x, UT_sint32 y);
	
protected:
	static LRESULT CALLBACK		_wndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

	static ATOM					m_atomPreviewWidgetClass;		// atom for RegisterClass()
	HWND						m_hwndPreview;					// hwnd that we draw into (child of a dlg control)
	XAP_Win32App *				m_pWin32App;
	GR_Win32Graphics *			m_pGraphics;					// GR_Graphics we give to View to draw in our window
	XAP_Preview *				m_pPreview;						// View which will draw formatted stuff in our window
	XAP_Win32Dialog_Insert_Symbol *m_pInsertSymbol;				// Insert symbol dialog parent (if applicable)
	static char					m_bufClassName[100];			// name for RegisterClass()
	static UT_uint32			m_iInstanceCount;				// Number of instance of this window type.
};

#endif /* XAP_WIN32PREVIEWWIDGET_H */
