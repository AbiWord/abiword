/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 *			 (c) 2002 Jordi Mas i Hernàndez jmas@softcatala.org 
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

#include <windows.h>
#include <stdio.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_Language.h"
#include "ut_Win32OS.h"
#include "ap_Win32StatusBar.h"
#include "xap_Win32App.h"
#include "ap_Win32Frame.h"
#include "ap_Win32App.h"

LRESULT APIENTRY StatusbarWndProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { 

	AP_Win32StatusBar *pBar = reinterpret_cast<AP_Win32StatusBar *>(GetWindowLong(hwnd, GWL_USERDATA));
	UT_return_val_if_fail (pBar, 0);

    if ((uMsg == WM_SIZE || uMsg == SB_SETPARTS) && hwnd) {
	
		// default processing
		LRESULT lresult = CallWindowProc(pBar->getOrgWndProc(), hwnd, uMsg, wParam, lParam);
		
		// resize the 2nd pane so the panels fill-up the whole statusbar width
		// when the statusbar has been resized.
		
		// stopper when nothing has changed
		RECT rect; GetWindowRect(hwnd, &rect);
		
		if (pBar->getPrevWidth() != (rect.right - rect.left)) {
			
			pBar->setPrevWidth(rect.right - rect.left);
			
			// retrieve the number of parts
			int i, width, offset;
			int nParts = SendMessage(hwnd, SB_GETPARTS, 0, 0);

			int *pArOffsets = new int[nParts];    
			int *pArWidths = new int[nParts];    

			int *o = pArOffsets;
			int *w = pArWidths;

			// retrieve the current offsets
			SendMessage(hwnd, SB_GETPARTS, nParts, (LPARAM)pArOffsets);

			// determine the individual panel widths and how much space we 
			// have available for the 2nd panel.
			for (i = 0, width = 0; i < nParts; i++) {
				
				if (i == 0) {
					*w = *o;
					width += *w;
					}
				else if (i > 1 && i < nParts - 1) {
					*w = *o - *(o - 1);
					width += *w;
					}
					
				if (i == nParts - 1) {
					*w = -1;
					*o = -1;
					}
				o++;
				w++;
				}
			
			// adjust and set width.
			width = (rect.right - rect.left) - width;
			
			if (width < 32) 
				width = 32;

			w = pArWidths; w++; *w = width;

			// compute new offsets
			o = pArOffsets;
			w = pArWidths;
			
			for (i = 0, offset = 0; i < nParts; i++) {
				offset += *w;
				*o = (i != nParts - 1) ? offset : -1;
				o++;
				w++;
				}
				
			// update
			SendMessage(hwnd, SB_SETPARTS, nParts, (LPARAM)pArOffsets);

			delete [] pArWidths;
			delete [] pArOffsets;
			}
			
		return lresult;
		}
 
    return CallWindowProc(pBar->getOrgWndProc(), hwnd, uMsg, wParam, lParam); 
	} 

/*
	Callback object used to setup the status bar text
*/
class ABI_EXPORT ap_usb_TextListener : public AP_StatusBarFieldListener
{
public:
	ap_usb_TextListener(AP_StatusBarField *pStatusBarField, HWND hWnd, UINT nID,
						const AP_Win32StatusBar * pSB) :
		AP_StatusBarFieldListener(pStatusBarField),
		m_hWnd(hWnd),
		m_nID(nID),
		m_pSB(pSB)
	{UT_ASSERT_HARMLESS( m_pSB );}
	
	virtual void notify(); 

private:
	HWND	m_hWnd;
	UINT	m_nID;
	const AP_Win32StatusBar * m_pSB;
};

void ap_usb_TextListener::notify()
{

	UT_return_if_fail (m_hWnd && m_pSB);	
	AP_StatusBarField_TextInfo * textInfo = ((AP_StatusBarField_TextInfo *)m_pStatusBarField);
	UT_String 	s =	AP_Win32App::s_fromUTF8ToWinLocale(textInfo->getBuf().utf8_str());	
	SendMessage(m_hWnd, SB_SETTEXT, m_nID | m_pSB->getDir(), (LPARAM)  s.c_str());
	
}


/*****************************************************************/

AP_Win32StatusBar::AP_Win32StatusBar(XAP_Frame * pFrame)
	: AP_StatusBar(pFrame),
	  m_hwndStatusBar(NULL),
	  m_pOrgStatusbarWndProc(NULL),
	  m_iPrevWidth(-1),
	  m_iDIR(0)
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	UT_return_if_fail( pSS );
	
	UT_Language l;
	if(l.getDirFromCode(pSS->getLanguageName()) == UTLANG_RTL)
		m_iDIR = SBT_RTLREADING;

}

AP_Win32StatusBar::~AP_Win32StatusBar(void)
{
	
}

void AP_Win32StatusBar::setView(AV_View * pView)
{	
	AP_StatusBar::setView(pView);
}

/*

*/
HWND AP_Win32StatusBar::createWindow(HWND hwndFrame,
									 UT_uint32 left, UT_uint32 top,
									 UT_uint32 width)
{
	XAP_Win32App * app = static_cast<XAP_Win32App *>(XAP_App::getApp());
	UINT nID = 0;
    int* pArWidths = new int[getFields()->getItemCount()+1];    
    int* pCurWidth = pArWidths;
    int	nWitdh = 0;
	
	/*
		The window procedure for the status bar automatically sets the 
		initial size and position of the window, ignoring the values 
		specified in the CreateWindowEx function.	
	
	*/
	m_hwndStatusBar = UT_CreateWindowEx(0, STATUSCLASSNAME, NULL,
										WS_CHILD | WS_VISIBLE | SBS_SIZEGRIP,
										0, 0, 0, 0,
										hwndFrame, NULL, app->getInstance(), NULL);
	UT_return_val_if_fail (m_hwndStatusBar,0);	

	// route messages through our handler first (to size the status panels).
	m_pOrgStatusbarWndProc = reinterpret_cast<WNDPROC>(SetWindowLong(
		m_hwndStatusBar, GWL_WNDPROC, reinterpret_cast<LONG>(StatusbarWndProc))
		);
	
	// attach a pointer to the s:tatusbar window to <this> so we can get the 
	// original wndproc and previous window-width
	SetWindowLong(m_hwndStatusBar, GWL_USERDATA, reinterpret_cast<LONG>(this));

	for (UT_sint32 k = 0; k < getFields()->getItemCount(); k++) 
	{
 		AP_StatusBarField * pf = (AP_StatusBarField *)m_vecFields.getNthItem(k);
		UT_ASSERT_HARMLESS(pf); // we should NOT have null elements
		
		AP_StatusBarField_TextInfo *pf_TextInfo = static_cast<AP_StatusBarField_TextInfo*>(pf);

		// Create a Text element	
		if (pf_TextInfo) 
		{	
					
			pf->setListener((AP_StatusBarFieldListener *)(new ap_usb_TextListener(pf_TextInfo, m_hwndStatusBar,
																				  nID, this)));
			
			// size and place
			nWitdh+= (strlen(pf_TextInfo->getRepresentativeString())*10);			
			*pCurWidth = nWitdh;											
		}
		else 
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN); // there are no other kinds of elements
		}
				
		pCurWidth++;
		nID++;		
	}
	
	// Blank gadgets that gets the rest of the window
	*pCurWidth = -1;
		
	
	// Set the numer of elements in the statusbar and their size
    SendMessage(m_hwndStatusBar, SB_SETPARTS, getFields()->getItemCount()+1, (LPARAM)pArWidths);
	    
    delete [] pArWidths;

	return m_hwndStatusBar;
}


void AP_Win32StatusBar::show()
{
	ShowWindow(m_hwndStatusBar, SW_SHOW);

	// Defer the screen repaint if we're leaving fullscreen mode
	if (!static_cast<AP_FrameData *>(m_pFrame->getFrameData())->m_bIsFullScreen)
		m_pFrame->queue_resize();
}

void AP_Win32StatusBar::hide()
{
	ShowWindow(m_hwndStatusBar, SW_HIDE);

	// Defer the screen repaint if we're entering fullscreen mode
	if (!static_cast<AP_FrameData *>(m_pFrame->getFrameData())->m_bIsFullScreen)
		m_pFrame->queue_resize();
}

