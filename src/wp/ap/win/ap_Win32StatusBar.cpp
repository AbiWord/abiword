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
#include "ap_Win32StatusBar.h"
#include "xap_Win32App.h"
#include "ap_Win32Frame.h"
#include "xap_EncodingManager.h"

/*
	Callback object used to setup the status bar text
*/
class ap_usb_TextListener : public AP_StatusBarFieldListener
{
public:
	ap_usb_TextListener(AP_StatusBarField *pStatusBarField, HWND hWnd, UINT nID) : AP_StatusBarFieldListener(pStatusBarField) {m_nID=nID; m_hWnd=hWnd; }
	virtual void notify(); 

private:
	HWND	m_hWnd;
	UINT	m_nID;
};

void ap_usb_TextListener::notify()
{
	UT_uint32 uRead, uWrite;
	
	UT_ASSERT(m_hWnd);
	AP_StatusBarField_TextInfo * textInfo = ((AP_StatusBarField_TextInfo *)m_pStatusBarField);
	const UT_UCS4Char * buf = textInfo->getBufUCS();

	char *pText = UT_convert ((char*)buf,
							  UT_UCS4_strlen(buf)*sizeof(UT_UCS4Char),
							  ucs4Internal(),
							  XAP_EncodingManager::get_instance()->getNative8BitEncodingName(),
							  &uRead, &uWrite);

#ifdef DEBUG
	if(strlen(pText) > 10)
	   UT_DEBUGMSG(("ap_usb_TextListener::notify: msg: UCS4 values (1st 10): 0x%04x, 0x%04x, 0x%04x, 0x%04x, 0x%04x, 0x%04x, 0x%04x, 0x%04x, 0x%04x, 0x%04x\nSystemCP=%s\n",*buf, *(buf+1),*(buf+2),*(buf+3),*(buf+4),*(buf+5),*(buf+6),*(buf+7),*(buf+8),*(buf+9),pText));
#endif

	SendMessage(m_hWnd, SB_SETTEXT, m_nID, (LPARAM)  pText);
	FREEP(pText);	
}


/*****************************************************************/

AP_Win32StatusBar::AP_Win32StatusBar(XAP_Frame * pFrame)
	: AP_StatusBar(pFrame)
{
	m_hwndStatusBar = NULL;	
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
	XAP_Win32App * app = static_cast<XAP_Win32App *>(m_pFrame->getApp());
	UINT nID = 0;
    int* pArWidths = new int[getFields()->getItemCount()+1];    
    int* pCurWidth = pArWidths;
    int	nWitdh = 0;
	
	/*
		The window procedure for the status bar automatically sets the 
		initial size and position of the window, ignoring the values 
		specified in the CreateWindowEx function.	
	
	*/
	m_hwndStatusBar = CreateWindowEx(0, STATUSCLASSNAME, NULL,
									WS_CHILD | WS_VISIBLE | SBS_SIZEGRIP,
									0, 0, 0, 0,
									hwndFrame, NULL, app->getInstance(), NULL);
	UT_ASSERT(m_hwndStatusBar);	
				
	for (UT_uint32 k=0; k<getFields()->getItemCount(); k++) 
	{
 		AP_StatusBarField * pf = (AP_StatusBarField *)m_vecFields.getNthItem(k);
		UT_ASSERT(pf); // we should NOT have null elements
		
		AP_StatusBarField_TextInfo *pf_TextInfo = static_cast<AP_StatusBarField_TextInfo*>(pf);

		/* Create a Text element	*/
		if (pf_TextInfo) 
		{	
					
			pf->setListener((AP_StatusBarFieldListener *)(new ap_usb_TextListener(pf_TextInfo, m_hwndStatusBar, nID)));					
									
			// size and place
			nWitdh+= (strlen(pf_TextInfo->getRepresentativeString())*10);			
			*pCurWidth = nWitdh;											
		}
		else 
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN); // there are no other kinds of elements
		}
				
		pCurWidth++;
		nID++;		
	}
	
	// Blank gadgets that gets the rest of the window
	*pCurWidth = -1;
		
	
	/*	Set the numer of elements in the statusbar and their size*/	
    SendMessage(m_hwndStatusBar, SB_SETPARTS, getFields()->getItemCount()+1, (LPARAM)pArWidths);
	    
    delete pArWidths;
	return m_hwndStatusBar;
}


void AP_Win32StatusBar::show()
{
	ShowWindow(m_hwndStatusBar, SW_SHOW);
	m_pFrame->queue_resize();
}

void AP_Win32StatusBar::hide()
{
	ShowWindow(m_hwndStatusBar, SW_HIDE);
	m_pFrame->queue_resize();
}

