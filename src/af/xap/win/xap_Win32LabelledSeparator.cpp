/* AbiWord
 * Copyright (C) 2002 Gilles Saint-Denis.
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

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_Win32App.h"
#include "xap_Win32LabelledSeparator.h"
#include <commctrl.h>

/*****************************************************************/

#define GWL(hwnd)		(HFONT)GetWindowLongPtrW((hwnd), GWLP_USERDATA)
#define SWL(hwnd, f)	SetWindowLongPtrW((hwnd), GWLP_USERDATA,(LONG_PTR)(f))

/*!
  Spacing between the label text and the line separator part of the 
  control.
 */
#define TEXT_LINE_SPACING	2
/*!
  Line separator height in pixel
 */
#define LINE_HEIGHT			2
/*!
  Identifier of the child line separator window.
 */
#define IDC_LINE_SEPARATOR	1

/*!
  Windows control class name to use in the creation function or template.
 */
static const wchar_t s_LabelledSeparatorWndClassName[] = L"AbiLabelledSeparator";
/*!
  Window procedure function of the control base class.
 */
static WNDPROC s_pfnWndProc;

/*****************************************************************/

/*!
  Function to adapt the separator line length to the remaining
  space available after the displayed text.
  \param hwnd the conrol window handle
  \param text the text to be shown by the control
  \param hFont the font handle used to represent text
 */
static void AdaptSeparatorLength(HWND hwnd, const wchar_t* text, HFONT hFont)
{
	// Provide the right font
	LOGFONTW logFont;
	int status = GetObjectW(hFont, sizeof(LOGFONTW), &logFont);
	UT_return_if_fail(status);
	HFONT hCtrlFont = CreateFontIndirectW(&logFont);
	UT_return_if_fail(hCtrlFont);

	// Evaluate text size
	int length = wcslen(text);
	SIZE textSize;
	HFONT hPrevCtrlFont;
	HDC hdc = GetDC(hwnd);
	hPrevCtrlFont = (HFONT) SelectObject(hdc, hFont);
	GetTextExtentPoint32W(hdc, text, length, &textSize);
	SelectObject(hdc, hPrevCtrlFont);

	// Modifiy separator line length
	RECT controlRect;
	HWND hLine = GetDlgItem(hwnd, IDC_LINE_SEPARATOR);
	GetClientRect(hwnd, &controlRect);
	if(textSize.cx > 0)
	{
		textSize.cx += TEXT_LINE_SPACING;
	}
	UT_sint32 lineWidth = controlRect.right - textSize.cx;
	if(lineWidth < 0)
	{
		lineWidth = 0;
	}
	MoveWindow(hLine, 
		textSize.cx, 
		(controlRect.bottom - controlRect.top) / 2, 
		lineWidth, LINE_HEIGHT, true);
}

/*!
  Window procedure function responsible for implementing the special
  behavior of the control.
  \param hwnd the control window handle
  \param iMsg the message identifier to process
  \param wParam first message parameter
  \param lParam second message parameter
  \result the result of the message processing which depend on the actual 
  message processed
 */
static LRESULT CALLBACK _LabelledSeparatorWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_CREATE:
		{
			CREATESTRUCTW* lpCreate = (CREATESTRUCTW *) lParam;
			UT_DebugOnly<HWND> separator = CreateWindowW(L"STATIC",
								     NULL,
				SS_ETCHEDHORZ | WS_CHILD | WS_VISIBLE,
				0, lpCreate->cy / 2, lpCreate->cx, LINE_HEIGHT,
				hwnd, (HMENU) IDC_LINE_SEPARATOR,
				lpCreate->hInstance, NULL);
			UT_ASSERT_HARMLESS(separator);
			return 0;
		}

	case WM_SETFONT:
		{
			SWL(hwnd, (HFONT) wParam);

			int length = GetWindowTextLengthW(hwnd);
			wchar_t* text = new wchar_t[length + 1];
			length = GetWindowTextW(hwnd, text, length + 1);
			AdaptSeparatorLength(hwnd, text, (HFONT) wParam);
			delete [] text;
			break;
		}

	case WM_SETTEXT:
		{
			AdaptSeparatorLength(hwnd, (const wchar_t*) lParam, (HFONT) GWL(hwnd));
			break;
		}
		
	default:
		break;
	}

	return CallWindowProcW(s_pfnWndProc, hwnd, iMsg, wParam, lParam);   	
}

/*!
  Register the labelled separator control class with Windows.
  This function must be called before the first labelled separator control
  is created.

  A labelled separator control is like a static control with
  a line extending at the end of the text portion. The labelled separator
  control adapt the separator length to the text width. Thus, localized
  label can be immediately followed by the separator line.
  
  A labelled separator control can be created in a resource file by a
  statement like the following:
  \verbatim
    CONTROL    "Actual label",IDC_OF_CONTROL,"AbiLabelledSeparator",0x0,7,17,116,8
  \endverbatim
  The last four numbers are the usual x, y, width and height parameters
  in windows dialog coordinates.
  
  \param app the application object
  \return an indication of the class registration success
 */
bool XAP_Win32LabelledSeparator_RegisterClass(XAP_Win32App * app)
{
	WNDCLASSEXW  wndclass;
	HINSTANCE hinst = app->getInstance();
	UT_DebugOnly<ATOM> a;

	wndclass.cbSize = sizeof(WNDCLASSEXW);
	if(GetClassInfoExW(hinst, s_LabelledSeparatorWndClassName, &wndclass))
	{
		// class information already registered
		return true;
	}

	// get base class information
	if( ! GetClassInfoExW(NULL, L"STATIC", &wndclass))
	{
		return false;
	}

	s_pfnWndProc           = wndclass.lpfnWndProc;
	wndclass.cbSize		   = sizeof(wndclass);
	wndclass.lpfnWndProc   = _LabelledSeparatorWndProc;
	wndclass.hInstance     = hinst;
	wndclass.lpszClassName = s_LabelledSeparatorWndClassName;
	wndclass.lpszMenuName  = NULL;

	a = RegisterClassExW(&wndclass);
	UT_ASSERT(a);

	return true;
}

