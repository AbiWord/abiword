/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#ifndef XAP_Win32Dialog_Insert_Symbol_H
#define XAP_Win32Dialog_Insert_Symbol_H

#include "xap_Dlg_Insert_Symbol.h"
class XAP_Win32Frame;
class XAP_Win32PreviewWidget;

/*****************************************************************/

static UT_UCSChar m_CurrentSymbol = UCS_SPACE;
static UT_UCSChar m_PreviousSymbol = UCS_SPACE;

static char Symbol_font_selected[32] = "Symbol";


class XAP_Draw_Symbol_sample : public XAP_Preview
{
public:

	XAP_Draw_Symbol_sample(XAP_Draw_Symbol *pSymbolDraw, GR_Graphics * gc) : XAP_Preview(gc)
		{
		m_pSymbolDraw = pSymbolDraw;
		}
	virtual ~XAP_Draw_Symbol_sample(void)
		{
		}
				
	void	draw(void)
		{
		m_pSymbolDraw->drawarea(m_CurrentSymbol, m_PreviousSymbol);
		}

protected:

	XAP_Draw_Symbol *m_pSymbolDraw;
};



class XAP_Win32Dialog_Insert_Symbol: public XAP_Dialog_Insert_Symbol
{
public:
	XAP_Win32Dialog_Insert_Symbol(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Win32Dialog_Insert_Symbol(void);

	virtual void			runModal(XAP_Frame * pFrame);
	virtual void			runModeless(XAP_Frame * pFrame);
	virtual void			notifyActiveFrame(XAP_Frame *pFrame);
	virtual void			notifyCloseFrame(XAP_Frame *pFrame);
	virtual void			destroy(void);
	virtual void			activate(void) {ShowWindow(m_hDlg, SW_SHOW);};

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	static BOOL CALLBACK	s_dlgProc(HWND,UINT,WPARAM,LPARAM);
	static int CALLBACK		fontEnumProcedure(const LOGFONT *pLogFont, const TEXTMETRIC *pTextMetric, DWORD Font_type, LPARAM lParam);
	
protected:
	BOOL					_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL					_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
	int						_enumFont(const LOGFONT *pLogFont, const TEXTMETRIC *pTextMetric, DWORD Font_type);
	void					_setFontFromCombo(UT_sint32 Index);


	XAP_Win32PreviewWidget *	m_pSymbolPreviewWidget;
	XAP_Win32PreviewWidget *	m_pSamplePreviewWidget;

	XAP_Draw_Symbol_sample *	m_DrawSymbolSample;

	HWND m_hDlg;
};

#endif /* XAP_Win32Dialog_Insert_Symbol_H */
