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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
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
#include "ut_Win32LocaleString.h"

LRESULT APIENTRY StatusbarWndProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { 

	AP_Win32StatusBar *pBar = reinterpret_cast<AP_Win32StatusBar *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
	UT_return_val_if_fail (pBar, 0);

    if ((uMsg == WM_SIZE || uMsg == SB_SETPARTS) && hwnd) {
	
		// default processing
		LRESULT lresult = CallWindowProcW(pBar->getOrgWndProc(), hwnd, uMsg, wParam, lParam);
		
		// resize the 2nd pane so the panels fill-up the whole statusbar width
		// when the statusbar has been resized.
		
		// stopper when nothing has changed
		RECT rect; GetWindowRect(hwnd, &rect);
		
		if (pBar->getPrevWidth() != (rect.right - rect.left)) {
			
			pBar->setPrevWidth(rect.right - rect.left);
			
			// retrieve the number of parts
			int i, width, offset;
			int nParts = SendMessageW(hwnd, SB_GETPARTS, 0, 0);

			int *pArOffsets = new int[nParts];    
			int *pArWidths = new int[nParts];    

			int *o = pArOffsets;
			int *w = pArWidths;

			// retrieve the current offsets
			SendMessageW(hwnd, SB_GETPARTS, nParts, (LPARAM)pArOffsets);

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
					
				else if (i == nParts - 1) {
					*w = -1;
					*o = -1;
					}
				o++;
				w++;
				}
			
			// adjust and set width.
			width = (rect.right - rect.left - (rect.bottom - rect.top)) - width;
			
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
			SendMessageW(hwnd, SB_SETPARTS, nParts, (LPARAM)pArOffsets);	
			MoveWindow(pBar->getProgressBar(),*pArOffsets/2,2,*pArOffsets/2,20,true);

			delete [] pArWidths;
			delete [] pArOffsets;
			}
			
		return lresult;
		}
 
    return CallWindowProcW(pBar->getOrgWndProc(), hwnd, uMsg, wParam, lParam); 
	} 

/*
	Callback object used to setup the status bar text
*/
class ABI_EXPORT ap_usb_TextListener : public AP_StatusBarFieldListener
{
public:
	ap_usb_TextListener(AP_StatusBarField *pStatusBarField, HWND hWnd,UINT nID,
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
	UT_Win32LocaleString str;	
	str.fromUTF8 (textInfo->getBuf().c_str());	

	SendMessageW(m_hWnd, SB_SETTEXTW, m_nID | m_pSB->getDir(), (LPARAM)  str.c_str());
	
}
/*****************************************************************/
class ap_usb_ProgressListener : public AP_StatusBarFieldListener
{
public:
	ap_usb_ProgressListener(AP_StatusBarField *pStatusBarField, HWND wProgress) : AP_StatusBarFieldListener(pStatusBarField) 
	{ 
		m_ProgressWND = wProgress; 
		m_isMARQUEEState=false;
	}
	virtual void notify(); 

protected:
	HWND m_ProgressWND;
	bool m_isMARQUEEState;  //true isMARQUEE; false NoMARQUEE
};

void ap_usb_ProgressListener::notify()
{
	UT_return_if_fail (m_ProgressWND);

	AP_StatusBarField_ProgressBar * pProgress = ((AP_StatusBarField_ProgressBar *)m_pStatusBarField);
	if(pProgress->isDefinate())
	{
		if(m_isMARQUEEState)
		{
			DWORD dwStyle;
			dwStyle=GetWindowLong(m_ProgressWND, GWL_STYLE);
			dwStyle&=~PBS_MARQUEE;
			SetWindowLong(m_ProgressWND, GWL_STYLE, dwStyle );
			m_isMARQUEEState=false;
		}
		double fraction = pProgress->getFraction();
		SendMessage(m_ProgressWND,PBM_SETPOS,fraction*100,0);
	}
	else
	{   //here pulse process bar
		//gtk_progress_bar_pulse(GTK_PROGRESS_BAR(m_wProgress));
		if(!m_isMARQUEEState)
		{
			DWORD dwStyle;
			dwStyle=GetWindowLong(m_ProgressWND, GWL_STYLE);
			dwStyle|=PBS_MARQUEE;
			SetWindowLong(m_ProgressWND, GWL_STYLE, dwStyle );
			SendMessage(m_ProgressWND,PBM_SETMARQUEE,1,100);
			m_isMARQUEEState=true;
		}				
	}
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
	m_hwndStatusBar = UT_CreateWindowEx(0, STATUSCLASSNAMEW, NULL,
										WS_CHILD | WS_VISIBLE | SBS_SIZEGRIP,
										0, 0, 0, 0,
										hwndFrame, NULL, app->getInstance(),NULL);
	// add progress bar and hide it 
	//  set the color and range 
	m_hwndProgressBar= UT_CreateWindowEx(0, PROGRESS_CLASSW, NULL,
			WS_CHILD | WS_VISIBLE|PBS_MARQUEE,
			0, 0, 0, 0,
			m_hwndStatusBar, NULL, app->getInstance(),NULL);
	SendMessage(m_hwndProgressBar,PBM_SETRANGE,0,MAKELONG(0,100));
	
	UT_return_val_if_fail (m_hwndStatusBar,0);	

	// route messages through our handler first (to size the status panels).
	m_pOrgStatusbarWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(
		m_hwndStatusBar, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(StatusbarWndProc))
		);
	
	// attach a pointer to the statusbar window to <this> so we can get the 
	// original wndproc and previous window-width
	SetWindowLongPtrW(m_hwndStatusBar, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	for (UT_sint32 k = 0; k < getFields()->getItemCount(); k++) 
	{
 		AP_StatusBarField * pf = (AP_StatusBarField *)m_vecFields.getNthItem(k);
		UT_ASSERT_HARMLESS(pf); // we should NOT have null elements
		if (pf->getFillMethod() == REPRESENTATIVE_STRING || (pf->getFillMethod() == MAX_POSSIBLE))
		{
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
		}
		else if(pf->getFillMethod() == PROGRESS_BAR)
		{
			pf->setListener((AP_StatusBarFieldListener *)(new ap_usb_ProgressListener(pf, m_hwndProgressBar)));
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
    SendMessageW(m_hwndStatusBar, SB_SETPARTS, getFields()->getItemCount()+1, (LPARAM)pArWidths);
	    
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


void   AP_Win32StatusBar::showProgressBar(void)
{ 
   ShowWindow(m_hwndProgressBar,SW_SHOW);
}
void   AP_Win32StatusBar::hideProgressBar(void) 
{
   ShowWindow(m_hwndProgressBar,SW_HIDE);
   SendMessage(m_hwndProgressBar,PBM_SETPOS,0,0); //hide and set to zero
}
