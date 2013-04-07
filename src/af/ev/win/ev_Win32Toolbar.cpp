/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
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

#define WIN32_LEAN_AND_MEAN

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <windows.h>
#include <commctrl.h>   // includes the common control header

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_vector.h"
#include "ut_Win32OS.h"
#include "ev_Win32Toolbar.h"
#include "xap_Win32App.h"
#include "xap_Frame.h"
#include "xap_Win32FrameImpl.h"
#include "ev_Toolbar_Actions.h"
#include "ev_Toolbar_Layouts.h"
#include "ev_Toolbar_Labels.h"
#include "ev_Toolbar_Control.h"
#include "ev_EditEventMapper.h"
#include "xap_Win32Toolbar_Icons.h"
#include "xap_EncodingManager.h"
#include "ev_Win32Toolbar_ViewListener.h"
#include "xav_View.h"
#include "ut_xml.h"
#include "xap_Prefs.h"
// The following are for the windows font colour dialog hack
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Background.h"
#include "fv_View.h"
#include "pt_PieceTable.h"
#include "ap_Win32Toolbar_FontCombo.h"
#include "ap_Win32App.h"
#include "ut_Win32LocaleString.h"


#ifndef TBSTYLE_EX_DRAWDDARROWS
#define TBSTYLE_EX_DRAWDDARROWS 0x00000001
#endif

#ifndef TB_SETEXTENDEDSTYLE
#define TB_SETEXTENDEDSTYLE     (WM_USER + 84)  // For TBSTYLE_EX_*
#endif

#ifndef TB_GETEXTENDEDSTYLE
#define TB_GETEXTENDEDSTYLE     (WM_USER + 85)  // For TBSTYLE_EX_*
#endif

#ifndef TBSTYLE_AUTOSIZE
#define TBSTYLE_AUTOSIZE  0x10
#endif

/*****************************************************************/

//
// Janitor class to cleanup bitmap handles at program termination.
// This one should really be in the anonymous namespace in this translation
// unit, but since we don't use namespaces...
//
class ABI_EXPORT foo_Bitmap_container
{
public:	// d'tor needs to be public due to buggy MSVC compilers
	~foo_Bitmap_container();

	static foo_Bitmap_container& instance();	// singleton
	void addBitmap(HBITMAP hBm)
	{
		UT_ASSERT(hBm);
		m_bitmaps_vector.addItem(reinterpret_cast<void*>(hBm));
	}

private:
	foo_Bitmap_container() : m_bitmaps_vector(50) {}
	foo_Bitmap_container(const foo_Bitmap_container&);	// no impl
	void operator=(const foo_Bitmap_container&);		// no impl
	UT_Vector m_bitmaps_vector;
};

foo_Bitmap_container& foo_Bitmap_container::instance()
{
	static foo_Bitmap_container container;
	return container;
}

foo_Bitmap_container::~foo_Bitmap_container()
{
	const UT_uint32 nItems = m_bitmaps_vector.getItemCount();
	for (UT_uint32 iItem = nItems; iItem; --iItem)
	{
		void* p = (void*)m_bitmaps_vector.getNthItem(iItem - 1);
		// This is really, *REALLY* bad! The same entity that created this
		// bitmap should also be responsible to destroy it. Time permitting,
		// refactor all this code to make it correct.
		::DeleteObject((HGDIOBJ)p);
	}
}


/*****************************************************************/


EV_Win32Toolbar::EV_Win32Toolbar(XAP_Win32App * pWin32App, XAP_Frame * pFrame,
								 const char * szToolbarLayoutName,
								 const char * szToolbarLabelSetName)
:	EV_Toolbar(pWin32App->getEditMethodContainer(),
				szToolbarLayoutName,
				szToolbarLabelSetName),
	m_pWin32App(pWin32App),
	m_pWin32Frame(pFrame),
	m_pViewListener(NULL),
	m_lid(0),				// view listener id
	m_hwnd(0),
	m_pFontCtrl(NULL)
{
}

EV_Win32Toolbar::~EV_Win32Toolbar(void)
{
	_releaseListener();

	if (m_pFontCtrl)
		delete m_pFontCtrl;

	for (UT_sint32 c=0; c < m_vecOrgStylesNames.getItemCount(); c++)	
		delete m_vecOrgStylesNames.getNthItem(c);

}

bool EV_Win32Toolbar::toolbarEvent(XAP_Toolbar_Id id,
									  UT_UCSChar * pData,
									  UT_uint32 dataLength)
{
	// user selected something from this toolbar.
	// invoke the appropriate function.
	// return true iff handled.

	// we use this for the color dialog hack	
	

	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pWin32App->getToolbarActionSet();
	UT_return_val_if_fail(pToolbarActionSet,false);

	const EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
	if (!pAction)
		return false;

	AV_View * pView = m_pWin32Frame->getCurrentView();

#if 0
	// make sure we ignore presses on "down" group buttons
	if (pAction->getItemType() == EV_TBIT_GroupButton)
	{
		const char * szState = 0;
		EV_Toolbar_ItemState tis = pAction->getToolbarItemState(pView,&szState);

		if (EV_TIS_ShouldBeToggled(tis))
		{
			// if this assert fires, you got a click while the button is down
			// if your widget set won't let you prevent this, handle it here
			UT_ASSERT_HARMLESS(UT_TODO);
			
			// can safely ignore this event
			return true;
		}
	}
#endif 

	// TODO this windows font color dialog is a hack
	// TODO true implementation requires a custom widget on the toolbar
	if (pAction->getItemType() == EV_TBIT_ColorFore || pAction->getItemType() == EV_TBIT_ColorBack)
	{
		UT_UCS4String ucs4color;
		UT_uint32 dataLength;
		XAP_Win32FrameImpl* fimpl = static_cast<XAP_Win32FrameImpl*>(m_pWin32Frame->getFrameImpl());
		//int id = ItemIdFromWmCommand(cmd);

		UT_ASSERT(id== AP_TOOLBAR_ID_COLOR_BACK || id== AP_TOOLBAR_ID_COLOR_FORE);
		
		const EV_Toolbar_ActionSet * pToolbarActionSet = m_pWin32App->getToolbarActionSet();
		UT_return_val_if_fail(pToolbarActionSet,false);

		const EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);

		if (!pAction) 
            return true;

		AV_View * pView = m_pWin32Frame->getCurrentView();
		
		const char * szMethodName = pAction->getMethodName();
		if (!szMethodName) 
            return true;
		
		const EV_EditMethodContainer * pEMC = m_pWin32App->getEditMethodContainer();
		EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);

		if (id==AP_TOOLBAR_ID_COLOR_BACK)
		{
			ucs4color = fimpl->m_sColorBack.ucs4_str();
			dataLength =  fimpl->m_sColorBack.size();
		}
		else
		{
			ucs4color = fimpl->m_sColorFore.ucs4_str();
			dataLength =  fimpl->m_sColorFore.size();
		}	

		invokeToolbarMethod(pView,pEM, ucs4color.ucs4_str(),dataLength);
	}

	const char * szMethodName = pAction->getMethodName();
	if (!szMethodName)
		return false;
	
	const EV_EditMethodContainer * pEMC = m_pWin32App->getEditMethodContainer();
	UT_return_val_if_fail(pEMC,false);

	EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
	UT_ASSERT(pEM);						// make sure it's bound to something

	invokeToolbarMethod(pView,pEM,pData,dataLength);
	return true;
}

/*****************************************************************/

// HACK: forward declarations for subclassed controls
#ifdef STRICT   
#define WHICHPROC	WNDPROC
#else   
#define WHICHPROC	FARPROC
#endif

WHICHPROC s_lpfnDefCombo; 
WHICHPROC s_lpfnDefToolBar;


#define COMBO_BUF_LEN 256



LRESULT CALLBACK EV_Win32Toolbar::_ToolBarWndProc (HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	if (uMessage == WM_CTLCOLORSTATIC) {
		if(SetBkColor((HDC) wParam, GetSysColor(COLOR_WINDOW)) == CLR_INVALID)
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		}
		if(SetTextColor((HDC) wParam, GetSysColor(COLOR_WINDOWTEXT)) == CLR_INVALID)
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		}
		return (LRESULT) GetSysColorBrush (COLOR_WINDOW);
	}
	
	return (CallWindowProcW(s_lpfnDefToolBar, hWnd, uMessage, wParam, lParam));
}

LRESULT CALLBACK EV_Win32Toolbar::_ComboWndProc( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
		/* Draws the font preview in the font selection combobox*/
		case WM_DRAWITEM:
		{						
			EV_Win32Toolbar * t = (EV_Win32Toolbar *) GetWindowLongPtrW(hWnd, GWLP_USERDATA);			
			UINT u = GetDlgCtrlID(hWnd);
			XAP_Toolbar_Id id = t->ItemIdFromWmCommand(u);
						
			if (id!=AP_TOOLBAR_ID_FMT_FONT)	/* Only owner draw the font selection*/
			{
				UT_ASSERT_HARMLESS(0);
				break;
			}
								
			DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)lParam;
			if((UT_sint32)dis->itemData < 0)
			{
				return TRUE;
			}
			
			XAP_Toolbar_ControlFactory * pFactory = t->m_pWin32App->getControlFactory();			

			if (!t->m_pFontCtrl)
			{	
				EV_Toolbar_Control * pControl = pFactory->getControl(t, AP_TOOLBAR_ID_FMT_FONT);                                                                         						
				t->m_pFontCtrl = static_cast<AP_Win32Toolbar_FontCombo *>(pControl);
				t->m_pFontCtrl->populate();
			}

			HFONT hFont, hUIFont;
			LOGFONTW logfont;
			SIZE size;
			HFONT hfontSave;
            UT_Win32LocaleString str;
			const UT_GenericVector<const char*> * v = t->m_pFontCtrl->getContents();
			str.fromUTF8 (v->getNthItem(dis->itemData));

			if(dis->itemState & ODS_COMBOBOXEDIT)
			{
				HFONT hUIFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
				hfontSave = (HFONT) SelectObject (dis->hDC, hUIFont);
				ExtTextOutW(dis->hDC, dis->rcItem.left, dis->rcItem.top, ETO_OPAQUE | ETO_CLIPPED, 0, str.c_str(), str.size(), 0);
				SelectObject (dis->hDC, hfontSave);				
			}
			else
			{	
				memset (&logfont, 0, sizeof (logfont));

				/* Create current font */
				logfont.lfHeight = 18;
				logfont.lfWidth = 0;
				logfont.lfEscapement = 0;
				logfont.lfOrientation = 0;
				logfont.lfWeight = FW_BOLD;
				logfont.lfItalic = FALSE;
				logfont.lfUnderline = FALSE;
				logfont.lfStrikeOut = FALSE;
				logfont.lfCharSet = DEFAULT_CHARSET;
				logfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
				logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
				logfont.lfQuality = DEFAULT_QUALITY;
				logfont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
				lstrcpynW(logfont.lfFaceName, str.c_str(), LF_FACESIZE);
				hFont = CreateFontIndirectW (&logfont);			

				if(dis->itemState & ODS_SELECTED) /* HighLight the selected text*/
				{
					SetTextColor(dis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
					SetBkColor(dis->hDC, GetSysColor(COLOR_HIGHLIGHT));
					FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
				}
				else
				{
					SetTextColor(dis->hDC, GetSysColor(COLOR_WINDOWTEXT));
					SetBkColor(dis->hDC, GetSysColor(COLOR_WINDOW));
					FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_WINDOW) );				
				}

				hUIFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);

				/*Fontname in regular font*/
				hfontSave = (HFONT) SelectObject (dis->hDC, hUIFont);
                ExtTextOutW(dis->hDC, dis->rcItem.left, dis->rcItem.top, ETO_OPAQUE | ETO_CLIPPED, 0, str.c_str(), str.size(), 0);
				
				/*Font example after the name*/
				const wchar_t* szSample = L"AbCdEfGhIj";
                GetTextExtentPoint32W(dis->hDC, str.c_str(), str.size(), &size);
				SelectObject (dis->hDC, hFont);
				ExtTextOutW(dis->hDC, dis->rcItem.left+size.cx+5, dis->rcItem.top, ETO_OPAQUE | ETO_CLIPPED, 0, szSample, wcslen(szSample), 0);
				SelectObject (dis->hDC, hfontSave);									
				DeleteObject(hFont); 
			}

			return TRUE;
		}
		
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		{
			// relay mouse messages from the combo box to get tool tips to work
			// TODO: similar relay needed from edit control, too??
			// ALT:  tell tooltip control to subclass for these events
			MSG msg;
			HWND hWndTT;
			msg.lParam = lParam;
			msg.wParam = wParam;
			msg.message = uMessage;
			msg.hwnd = hWnd;

			HWND hwndToolbar = GetParent(hWnd);

			hWndTT = (HWND)SendMessageW(hwndToolbar, TB_GETTOOLTIPS, 0,0);
			SendMessageW(hWndTT, TTM_RELAYEVENT, 0, (LPARAM)(LPMSG)&msg);
			break;
		}

		case WM_COMMAND:
		{
			switch (HIWORD(wParam))
			{								
				case CBN_SELCHANGE:
				{
					UT_sint32 iSelected = SendMessageW(hWnd, CB_GETCURSEL, 0, 0);
									
					if(iSelected != -1)
					{							
						static UT_UCSChar ucs_buf[COMBO_BUF_LEN];
                        UT_Win32LocaleString str;
						UT_uint32 bufLength = SendMessageW(hWnd, CB_GETLBTEXTLEN, iSelected, (LPARAM)0) + 1;
						wchar_t* buf = (wchar_t*)g_try_malloc(bufLength * sizeof (wchar_t));
						UT_uint32 dataLength = SendMessageW(hWnd, CB_GETLBTEXT, iSelected, (LPARAM)buf);
                        str.fromLocale (buf);

                        UT_UCS4_strcpy_utf8_char(ucs_buf, str.utf8_str().utf8_str());
						UT_UCSChar * pData = (UT_UCSChar *) ucs_buf;	// HACK: should be void *

						EV_Win32Toolbar * t = (EV_Win32Toolbar *) GetWindowLongPtrW(hWnd, GWLP_USERDATA);
						UT_ASSERT(t);

						if(dataLength)
						{	
							UINT u = GetDlgCtrlID(hWnd);
							XAP_Toolbar_Id id = t->ItemIdFromWmCommand(u);

							// If is a STYLE_NAME we should pass the internal one, not the localised				
							if (id==AP_TOOLBAR_ID_FMT_STYLE)
							{									
								UT_sint32 iSelected;					
								int nData;
								
								iSelected = SendMessageW(hWnd, CB_GETCURSEL, 0, 0);										

								// Find the proper non-localised text                                                                             					
								XAP_Toolbar_ControlFactory * pFactory = t->m_pWin32App->getControlFactory();
								EV_Toolbar_Control * pControl = pFactory->getControl(t, AP_TOOLBAR_ID_FMT_STYLE);                                                                         
								AP_Win32Toolbar_StyleCombo * pStyleC = static_cast<AP_Win32Toolbar_StyleCombo *>(pControl);
								pStyleC->repopulate();                                                                                                

								nData  = SendMessageW(hWnd, CB_GETITEMDATA, iSelected, 0);								

								UT_UTF8String *utf = t->m_vecOrgStylesNames.getNthItem(nData);
								UT_ASSERT((utf && (utf->length() < (COMBO_BUF_LEN-1))));
								UT_UCS4_strcpy_utf8_char(ucs_buf, utf->utf8_str());	
								dataLength = UT_UCS4_strlen (ucs_buf);

								DELETEP(pControl);
							}
							
							t->toolbarEvent(id, pData, dataLength);
						}

						SetFocus(static_cast<XAP_Win32FrameImpl*>(t->m_pWin32Frame->getFrameImpl())->getTopLevelWindow());
					}	
					else
					{
						PostMessageW(hWnd, WM_KEYDOWN, VK_ESCAPE, 0);
					}
						
					break;
				}					
				
							
				break;
				
			} // swich
		} // case command
			
		case WM_KEYDOWN:
		{
			EV_Win32Toolbar * t = (EV_Win32Toolbar *) GetWindowLongPtrW(hWnd, GWLP_USERDATA);
			UT_ASSERT(t);

			switch (wParam)
			{
				case VK_ESCAPE:
				{
					// restore combo state
					UINT u = GetDlgCtrlID(hWnd);
					XAP_Toolbar_Id id = t->ItemIdFromWmCommand(u);

					t->_refreshID(id);
				}
				// fall through

				case VK_TAB:
				case VK_RETURN:
				{
					SetFocus(static_cast<XAP_Win32FrameImpl*>(t->m_pWin32Frame->getFrameImpl())->getTopLevelWindow());
					return 0;
				}
			}
		}
	}
	return (CallWindowProcW(s_lpfnDefCombo, hWnd, uMessage, wParam, lParam));
}


/*****************************************************************/

bool EV_Win32Toolbar::synthesize(void)
{
	// create a toolbar from the info provided.

	////////////////////////////////////////////////////////////////
	// get toolbar button appearance from the preferences
	////////////////////////////////////////////////////////////////

	bool bIcons = true;
	bool bText = false;
	const gchar * szValue = NULL;
	m_pWin32App->getPrefsValue(XAP_PREF_KEY_ToolbarAppearance,&szValue);
	UT_return_val_if_fail((szValue) && (*szValue),false);

	if (g_ascii_strcasecmp(szValue,"icon") == 0)
	{
		bIcons = true;
		bText = false;
	}
	else if (g_ascii_strcasecmp(szValue,"text") == 0)
	{
		bIcons = false;
		bText = true;
	}
	else if (g_ascii_strcasecmp(szValue,"both") == 0)
	{
		bIcons = true;
		bText = true;
	}

	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pWin32App->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pToolbarLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	XAP_Toolbar_ControlFactory * pFactory = m_pWin32App->getControlFactory();
	UT_ASSERT(pFactory);

	HWND hwndParent = static_cast<XAP_Win32FrameImpl*>(m_pWin32Frame->getFrameImpl())->getToolbarWindow();
	if(hwndParent == NULL)
		return false;

	// NOTE: this toolbar will get placed later, by frame or rebar

	m_hwnd = UT_CreateWindowEx(0,                              
							   TOOLBARCLASSNAMEW, // window class name
							   NULL,			// window caption
							   WS_CHILD | WS_VISIBLE 
							   | WS_CLIPCHILDREN | WS_CLIPSIBLINGS 
							   | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT
							   | ( bText && !bIcons ? TBSTYLE_LIST : 0 )
							   | TBSTYLE_CUSTOMERASE
							   | CCS_NOPARENTALIGN | CCS_NODIVIDER
							   | CCS_NORESIZE
							   ,						// window style
							   0,						// initial x position
							   0,						// initial y position
							   0,						// initial x size
							   0,						// initial y size
							   hwndParent,				// parent window handle
							   NULL,					// window menu handle
							   m_pWin32App->getInstance(),		// program instance handle
							   NULL);  // creation parameters
							   					

	UT_ASSERT(m_hwnd);

	// override the window procedure 		
	s_lpfnDefToolBar = (WHICHPROC)GetWindowLongPtrW(m_hwnd, GWLP_WNDPROC);
	SetWindowLongPtrW(m_hwnd, GWLP_WNDPROC, (LONG_PTR)_ToolBarWndProc);
	SetWindowLongPtrW(m_hwnd, GWLP_USERDATA, (LONG_PTR)this);


	SendMessageW(m_hwnd, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);  

	// the Windows Common Control Toolbar requires that we set
	// a bitmap size in the toolbar window **before** we actually
	// add any of them.  at this point in the code, we haven't
	// loaded any of the bitmaps yet and thus don't know the maximum
	// size.  we could go thru the layout twice and compute the
	// maxium before calling this, but this seems overkill since
	// we know at compile time what all of the bitmaps are....
	//
	// Now bitmaps are cut down to requested size if too large - HB
	
	const WORD MY_MAXIMUM_BITMAP_X = 24;
	const WORD MY_MAXIMUM_BITMAP_Y = 24;

	if( bIcons )
		SendMessageW(m_hwnd, TB_SETBITMAPSIZE, 0,
					(LPARAM) MAKELONG(MY_MAXIMUM_BITMAP_X,MY_MAXIMUM_BITMAP_Y));
	else
		SendMessageW(m_hwnd, TB_SETBITMAPSIZE, 0, (LPARAM) MAKELONG(0,0));

	DWORD dwColor = GetSysColor(COLOR_BTNFACE);
	UT_RGBColor backgroundColor(GetRValue(dwColor),GetGValue(dwColor),GetBValue(dwColor));

	// TODO: is there any advantage to building up all the TBBUTTONs at once
	//		 and then adding them en masse, instead of one at a time? 
	UINT last_id=0;
	bool bControls = false;

	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Toolbar_LayoutItem * pLayoutItem = m_pToolbarLayout->getLayoutItem(k);
		UT_continue_if_fail(pLayoutItem);

		XAP_Toolbar_Id id = pLayoutItem->getToolbarId();
		EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
		UT_ASSERT(pAction);
		EV_Toolbar_Label * pLabel = m_pToolbarLabelSet->getLabel(id);
		UT_ASSERT(pLabel);

		UINT u = WmCommandFromItemId(id);

		TBBUTTON tbb;
		memset(&tbb, 0, sizeof(tbb));
		
		switch (pLayoutItem->getToolbarLayoutFlags())
		{
		case EV_TLF_Normal:
			{
				tbb.idCommand = u;
				tbb.dwData = 0;
				
				last_id = u;

				bool bButton = false;

				switch (pAction->getItemType())
				{
					//TODO: For now these are buttons which bring up a dialog, as some point,
           		 	//make them be able to bring up some sort of additional selector.
				case EV_TBIT_ColorFore:
				case EV_TBIT_ColorBack:
					UT_DEBUGMSG(("TODO: Hey Windows needs some tender love and care and a colour selector! \n"));
					// UT_ASSERT(UT_NOT_IMPLEMENTED);
					UT_DEBUGMSG(("TODO: Handle the colour selector case \n"));
					/* Fall through and make a push button */	

				case EV_TBIT_PushButton:
					bButton = true;
					tbb.fsState = TBSTATE_ENABLED; 
					tbb.fsStyle = TBSTYLE_BUTTON;     
					break;

				case EV_TBIT_ToggleButton:
					bButton = true;
					tbb.fsState = TBSTATE_ENABLED; 
					tbb.fsStyle = TBSTYLE_CHECK;     
					break;

				case EV_TBIT_GroupButton:
					bButton = true;
					tbb.fsState = TBSTATE_ENABLED; 
					tbb.fsStyle = TBSTYLE_CHECKGROUP;     
					break;

				case EV_TBIT_ComboBox:
					{
						EV_Toolbar_Control * pControl = pFactory->getControl(this, id);
						UT_ASSERT_HARMLESS(pControl);

						int iWidth = 100;

						if (pControl)
						{
							iWidth = pControl->getPixelWidth();
						}
						
						bControls = true;
						tbb.fsStyle = TBSTYLE_SEP;   
						tbb.iBitmap = iWidth;

						// create a matching child control
						DWORD dwStyle = WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL |
								CBS_HASSTRINGS | CBS_DROPDOWN;

						if ((pControl) && (pControl->shouldSort()))
						{
							dwStyle |= CBS_SORT;
						}
						
						if (id==AP_TOOLBAR_ID_FMT_FONT)
							dwStyle |= CBS_OWNERDRAWFIXED;

						HWND hwndCombo = UT_CreateWindowEx ( 0L,   // No extended styles.
							L"COMBOBOX",						// Class name.
							L"",								// Default text.
							dwStyle,						// Styles and defaults.
							0, 2, iWidth, 250,				// Size and position.
							m_hwnd,							// Parent window.
							(HMENU) u,						// ID.
							m_pWin32App->getInstance(),		// Current instance.
							NULL );							// No class data.

						UT_ASSERT(hwndCombo);
						
						/*
						 * Hack to create a combobox that is readonly, but which is capable of displaying text in 
						 * the edit control which differs from that in the drop-down list like GTK combos.
						 */
						HWND hwndEdit = FindWindowExW(hwndCombo, 0, NULL, NULL);
						if (!hwndEdit)
							UT_DEBUGMSG(("Toolbar: Failed to get handle of combos edit controls. Not setting read-only.\n"));
						else
							SendMessageW(hwndEdit, EM_SETREADONLY, (WPARAM)TRUE, (LPARAM)0);
						SendMessageW(hwndCombo, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
						SendMessageW(hwndCombo, CB_SETDROPPEDWIDTH,(WPARAM)pControl->getDroppedWidth(), 0);

						// populate it
						if (pControl)
						{
							pControl->populate();

							const UT_GenericVector<const char*> * v = pControl->getContents();
							UT_ASSERT_HARMLESS(v);

							SendMessageW(hwndCombo, WM_SETREDRAW, FALSE,0);

							if (v)
							{
								UT_uint32 items = v->getItemCount();
								int nIndex;
                                UT_Win32LocaleString localised;
								for (UT_uint32 k=0; k < items; k++)
								{
									localised.fromUTF8(v->getNthItem(k));									
    								nIndex = SendMessageW(hwndCombo, CB_ADDSTRING,(WPARAM)0, (LPARAM)localised.c_str());
 	     							SendMessageW(hwndCombo,CB_SETITEMDATA, nIndex, k);
								}
							}

							SendMessageW(hwndCombo, WM_SETREDRAW, TRUE,0);
						}

						// override the window procedure for the combo box
						s_lpfnDefCombo = (WHICHPROC)GetWindowLongPtrW(hwndCombo, GWLP_WNDPROC);
						SetWindowLongPtrW(hwndCombo, GWLP_WNDPROC, (LONG_PTR)_ComboWndProc);
						SetWindowLongPtrW(hwndCombo, GWLP_USERDATA, (LONG_PTR)this);

						// Get the handle to the tooltip window.
						HWND hwndTT = (HWND)SendMessageW(m_hwnd, TB_GETTOOLTIPS, 0, 0);

						if (hwndTT)
						{
                            UT_Win32LocaleString str;	
							const char * szToolTip = pLabel->getToolTip();
							if (!szToolTip || !*szToolTip)
							{
								szToolTip = pLabel->getStatusMsg();
							}

                            str.fromUTF8 (szToolTip);

							// Fill in the TOOLINFO structure.
							TOOLINFOW ti;
							ti.cbSize = sizeof(ti);
							ti.uFlags = TTF_IDISHWND | TTF_CENTERTIP;
							ti.lpszText = (wchar_t *) str.c_str();
							ti.hwnd = m_hwnd;		// TODO: should this be the frame?
							ti.uId = (UINT_PTR)hwndCombo;
							// Set up tooltips for the combo box.
							SendMessageW(hwndTT, TTM_ADDTOOLW, 0, (LPARAM)(LPTOOLINFOW)&ti);
						}
						
						// bind this separator to its control
						tbb.dwData = (DWORD_PTR) hwndCombo;

						// for now, we never repopulate, so can just toss it
						DELETEP(pControl);
					}
					break;
					
				case EV_TBIT_EditText:
				case EV_TBIT_DropDown:
				case EV_TBIT_StaticLabel:
					// TODO do these...
					break;
					
				case EV_TBIT_Spacer:
				case EV_TBIT_BOGUS:
				default:
					UT_ASSERT_HARMLESS(0);
					break;
				}

				if (bButton)
				{
					// TODO figure out who destroys hBitmap...
					//
					// TMN: 20 Nov 2000 - Tests display that we must hang on to the
					// bitmap handle for this to work.
					// Had we been allowed to use C++ I'd say
					//		std::map<std::string, HBITMAP> icons;
					// but currently I don't know how to handle this in a clean way.
					// Would UT_Array help?

					// TODO add code to create these once per application
					// TODO and reference them in each toolbar instance
					// TODO rather than create them once for each window....

					if (bIcons)
					{
						HBITMAP hBitmap;
						UT_DebugOnly<bool> bFoundIcon =
							XAP_Win32Toolbar_Icons::getBitmapForIcon(m_hwnd,
																	MY_MAXIMUM_BITMAP_X,
																	MY_MAXIMUM_BITMAP_Y,
																	&backgroundColor,
																	pLabel->getIconName(),
																	&hBitmap);
						UT_ASSERT(bFoundIcon);

						// TMN: I know, this is really, really bad, but it's
						// currently the only thing we have at our disposal to
						// release these bitmaps at program termination.
						foo_Bitmap_container::instance().addBitmap(hBitmap);

						TBADDBITMAP ab;
						ab.hInst = 0;
						ab.nID = (LPARAM)hBitmap;						
						
						LRESULT iAddedAt = SendMessageW(m_hwnd,TB_ADDBITMAP,1,(LPARAM)&ab);
						UT_ASSERT(iAddedAt != -1);

						tbb.iBitmap = iAddedAt;
					}
					
					if (bText)
					{
						const char * szLabel = pLabel->getToolbarLabel();
						// As long as translators don't cut the text short,
						// we need to autosize just to squize anything insize 1280 :)
						tbb.fsStyle |= TBSTYLE_AUTOSIZE;
						tbb.iString = SendMessageW(m_hwnd, TB_ADDSTRING, (WPARAM) 0, (LPARAM) (LPSTR) szLabel);
					}
					
					if (pAction->getItemType() == EV_TBIT_ColorFore || pAction->getItemType() == EV_TBIT_ColorBack)
					{
						tbb.fsStyle |=TBSTYLE_DROPDOWN;

						//tbb.fsStyle |=BTNS_WHOLEDROPDOWN;

						DWORD dwStyle = SendMessageW(m_hwnd, TB_GETEXTENDEDSTYLE, (WPARAM) 0, (LPARAM) 0);						
						dwStyle |= TBSTYLE_EX_DRAWDDARROWS;																		
						SendMessageW(m_hwnd, TB_SETEXTENDEDSTYLE, (WPARAM) 0, (LPARAM) dwStyle);						
					}
					
				}
			}
			break;
		
		case EV_TLF_Spacer:
			tbb.fsState = TBSTATE_ENABLED; 
			tbb.fsStyle = TBSTYLE_SEP;     
			break;
			
		default:
			UT_ASSERT_HARMLESS(0);
		}

		// add this button to the bar
		SendMessageW(m_hwnd, TB_ADDBUTTONS, (WPARAM) 1, (LPARAM) (LPTBBUTTON) &tbb);
	}

	// figure out bar dimensions now that buttons are all there
	SendMessageW(m_hwnd, TB_AUTOSIZE, 0, 0); 
	
	if (bControls)
	{
		// move each control on top of its associated separator
		for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
		{
			EV_Toolbar_LayoutItem * pLayoutItem = m_pToolbarLayout->getLayoutItem(k);
			UT_continue_if_fail(pLayoutItem);

			XAP_Toolbar_Id id = pLayoutItem->getToolbarId();
			EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
			UT_continue_if_fail(pAction);

			RECT r;
			HWND hwndCtrl;
			int nHeight, nSep;

			switch (pAction->getItemType())
			{
				case EV_TBIT_ComboBox:
					hwndCtrl = _getControlWindow(id);
					UT_ASSERT(hwndCtrl);
					GetWindowRect(hwndCtrl, &r);
					nHeight = r.bottom - r.top;

					SendMessageW(m_hwnd, TB_GETITEMRECT, (WPARAM) k, (LPARAM)(LPRECT) &r);

					nSep = (r.bottom - r.top - nHeight)/2;
					if (nSep < 0)
						nSep = 0;

					MoveWindow(hwndCtrl, r.left, r.top + nSep, r.right - r.left, nHeight, TRUE);

					break;

				case EV_TBIT_EditText:
				case EV_TBIT_DropDown:
				case EV_TBIT_StaticLabel:
					// TODO do these...
					break;
					
				default:
					break;
			}
		}
	}

	UT_ASSERT(last_id > 0);

	_addToRebar();	// add this bar to the rebar

	// I think that this is true due to the WS_VISIBLE flag
	// sent to CreateWindowEx
	m_bVisible = true;

	return true;
}

HWND EV_Win32Toolbar::getWindow(void) const
{
	return m_hwnd;
}

HWND EV_Win32Toolbar::_getControlWindow(XAP_Toolbar_Id id)
{
	TBBUTTON tbb;
	HWND hwndCtrl;
	UINT u = WmCommandFromItemId(id);

	UT_uint32 k = SendMessageW(m_hwnd, TB_COMMANDTOINDEX, (WPARAM) u, 0);
	SendMessageW(m_hwnd, TB_GETBUTTON, (WPARAM) k, (LPARAM)(LPTBBUTTON) &tbb);
	UT_ASSERT(tbb.idCommand == (int) u);
	UT_ASSERT(tbb.fsStyle & TBSTYLE_SEP);

	hwndCtrl = (HWND) tbb.dwData;

	return hwndCtrl;
}

void EV_Win32Toolbar::_releaseListener(void)
{
	if (!m_pViewListener)
		return;
	DELETEP(m_pViewListener);
	m_pViewListener = 0;
	m_lid = 0;
}
	
bool EV_Win32Toolbar::bindListenerToView(AV_View * pView)
{
	if(m_hwnd == NULL)
		return false;

	_releaseListener();
	
	m_pViewListener = new EV_Win32Toolbar_ViewListener(this,pView);
	UT_ASSERT(m_pViewListener);

	UT_DebugOnly<bool> bResult = pView->addListener(static_cast<AV_Listener *>(m_pViewListener),&m_lid);
	UT_ASSERT(bResult);

	if(pView->isDocumentPresent())
	{
		refreshToolbar(pView, AV_CHG_ALL);
	}
	
	return true;
}

bool EV_Win32Toolbar::refreshToolbar(AV_View * pView, AV_ChangeMask mask)
{
	if(m_hwnd == NULL)
		return false;

	// make the toolbar reflect the current state of the document
	// at the current insertion point or selection.
	
	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pWin32App->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pToolbarLayout->getLayoutItemCount();
	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Toolbar_LayoutItem * pLayoutItem = m_pToolbarLayout->getLayoutItem(k);
		UT_continue_if_fail(pLayoutItem);

		XAP_Toolbar_Id id = pLayoutItem->getToolbarId();
		EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
		UT_continue_if_fail(pAction);

		AV_ChangeMask maskOfInterest = pAction->getChangeMaskOfInterest();
		if ((maskOfInterest & mask) == 0)					// if this item doesn't care about
			continue;										// changes of this type, skip it...

		switch (pLayoutItem->getToolbarLayoutFlags())
		{
		case EV_TLF_Normal:
			_refreshItem(pView, pAction, id);
			break;
			
		case EV_TLF_Spacer:
			break;
			
		default:
			UT_ASSERT_HARMLESS(0);
			break;
		}
	}

	return true;
}

bool EV_Win32Toolbar::_refreshID(XAP_Toolbar_Id id)
{
	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pWin32App->getToolbarActionSet();
	UT_return_val_if_fail(pToolbarActionSet,false);

	EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
	UT_ASSERT(pAction);

	AV_View * pView = m_pWin32Frame->getCurrentView();
	UT_ASSERT(pView);

	return _refreshItem(pView, pAction, id);
}

bool EV_Win32Toolbar::_refreshItem(AV_View * pView, const EV_Toolbar_Action * pAction, XAP_Toolbar_Id id)
{
	const char * szState = 0;
	EV_Toolbar_ItemState tis = pAction->getToolbarItemState(pView,&szState);
		
	UINT u = WmCommandFromItemId(id);

	switch (pAction->getItemType())
	{
		// These two are unhandled and for the moment drop down to PushButton
		case EV_TBIT_ColorFore:
		case EV_TBIT_ColorBack:

		case EV_TBIT_PushButton:
			{
				bool bGrayed = EV_TIS_ShouldBeGray(tis);

				SendMessageW(m_hwnd, TB_ENABLEBUTTON, u, (!bGrayed ? 1 : 0)) ;

				//UT_DEBUGMSG(("refreshToolbar: PushButton [%s] is %s\n",
				//			m_pToolbarLabelSet->getLabel(id)->getToolbarLabel(),
				//			((bGrayed) ? "disabled" : "enabled")));
			}
			break;
	
		case EV_TBIT_ToggleButton:
		case EV_TBIT_GroupButton:
			{
				bool bGrayed = EV_TIS_ShouldBeGray(tis);
				bool bToggled = EV_TIS_ShouldBeToggled(tis);
				
				SendMessageW(m_hwnd, TB_ENABLEBUTTON, u, (!bGrayed ? 1 : 0));
				SendMessageW(m_hwnd, TB_CHECKBUTTON, u, (bToggled ? 1 : 0));

				//UT_DEBUGMSG(("refreshToolbar: ToggleButton [%s] is %s and %s\n",
				//			 m_pToolbarLabelSet->getLabel(id)->getToolbarLabel(),
				//			 ((bGrayed) ? "disabled" : "enabled"),
				//			 ((bToggled) ? "pressed" : "not pressed")));
			}
			break;

		case EV_TBIT_ComboBox:
			{
				//bool bGrayed = EV_TIS_ShouldBeGray(tis);
				//bool bString = EV_TIS_ShouldUseString(tis);
				HWND hwndCombo = _getControlWindow(id);
				UT_return_val_if_fail(hwndCombo, true);
				std::string utf8;
				UT_String str;
				
				// NOTE: we always update the control even if !szState
				if (!szState)
				{
					int idx = SendMessageW(hwndCombo, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)szState);
					if (idx==CB_ERR)
						SetWindowTextW(hwndCombo, L"");
					break;
				}					
				
				// Find the proper non-localised text
                UT_Win32LocaleString localised;
				if (id==AP_TOOLBAR_ID_FMT_STYLE)
				{
                        pt_PieceTable::s_getLocalisedStyleName(szState, utf8);
						localised.fromUTF8 (utf8.c_str());
                }       
                 else
    					localised.fromUTF8 (szState);
					
                int idx = SendMessageW(hwndCombo, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)localised.c_str());
				if (idx!=CB_ERR)
					SendMessageW(hwndCombo, CB_SETCURSEL, idx, 0);
				/*
				 * If the string didn't exist within the combos list, we handle things differently for
				 * different combos.
				 */
				if (idx==CB_ERR)
				{
					const XAP_StringSet * pSS;
					
					switch (id)
					{
					/*
					 * In the case of the font and font size combo, this means that the document contains a font not
					 * installed on the system. Add it to the edit control, as is the case with the unix version.
					 */
					case AP_TOOLBAR_ID_FMT_SIZE:
					case AP_TOOLBAR_ID_FMT_FONT:
						idx = SendMessageW(hwndCombo, WM_SETTEXT, (WPARAM)-1, (LPARAM)localised.c_str());
						if (idx == CB_ERR)
						{
							UT_DEBUGMSG(("refreshToolbar: Failed to set text for font combo.\n"));
						}
						break;
					/* 
					 * If this is the zoom combo, select the "Other..." string.
					 */
					case AP_TOOLBAR_ID_ZOOM:
						pSS = XAP_App::getApp()->getStringSet();
                        localised.fromUTF8 (pSS->getValue(XAP_STRING_ID_TB_Zoom_Percent));
						idx = SendMessageW(hwndCombo, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)localised.c_str());
						break;

					case AP_TOOLBAR_ID_FMT_STYLE:
						UT_DEBUGMSG(("refreshToolbar: Unknown string selected in style combo.\n"));
						break;
					default:
						UT_DEBUGMSG(("refreshToolbar: Unknown string selected in unknown combo.\n"));
						break;
					}
				}
	
				//UT_DEBUGMSG(("refreshToolbar: ComboBox [%s] is %s and %s\n",
				//			 m_pToolbarLabelSet->getLabel(id)->getToolbarLabel(),
				//			 ((bGrayed) ? "disabled" : "enabled"),
				//			 ((bString) ? szState : "no state")));
			}
			break;

		case EV_TBIT_EditText:
		case EV_TBIT_DropDown:
		case EV_TBIT_StaticLabel:
		case EV_TBIT_Spacer:
			// TODO do these later...
			break;
			

		case EV_TBIT_BOGUS:
		default:
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			break;
	}

	return true;
}

bool EV_Win32Toolbar::getToolTip(LPARAM lParam)
{
	UT_ASSERT(lParam);
	LPTOOLTIPTEXTW lpttt = (LPTOOLTIPTEXTW) lParam;

	// who's asking?
	UINT idButton = lpttt->hdr.idFrom;
	XAP_Toolbar_Id id = ItemIdFromWmCommand(idButton);
	
	EV_Toolbar_Label * pLabel = m_pToolbarLabelSet->getLabel(id);
	if (!pLabel)
		return false;

	// ok, gotcha
	const char * szToolTip = pLabel->getToolTip();
	if (!szToolTip || !*szToolTip)
	{
		szToolTip = pLabel->getStatusMsg();
	}

	if (!szToolTip || !*szToolTip)
	{
		szToolTip = pLabel->getToolbarLabel();
	}

	if (szToolTip && *szToolTip)
	{
		UT_Win32LocaleString str;
		str.fromUTF8 (szToolTip);
		wcsncpy(lpttt->lpszText, str.c_str(),80);
	}
	else
	{
		lpttt->lpszText[0] = '\0';
	}
	lpttt->hinst=NULL;

	return true;
}


void EV_Win32Toolbar::show(void)
{
	UT_return_if_fail(m_pWin32Frame);
	UT_DebugOnly<HWND> hRebar = static_cast<XAP_Win32FrameImpl*>(m_pWin32Frame->getFrameImpl())->getToolbarWindow();
	UT_ASSERT(hRebar);
	UT_DebugOnly<int> iBand = _getBandForHwnd(m_hwnd);
	UT_ASSERT(iBand < 0);	// It can't already be displayed!
	_addToRebar();

	m_bVisible = true;
}

void EV_Win32Toolbar::hide(void)
{
	UT_return_if_fail(m_pWin32Frame);
	HWND hRebar = static_cast<XAP_Win32FrameImpl*>(m_pWin32Frame->getFrameImpl())->getToolbarWindow();
	UT_ASSERT(hRebar);
	const int iBand = _getBandForHwnd(m_hwnd);
	if (iBand >= 0)
	{
		SendMessageW(hRebar, RB_DELETEBAND, (WPARAM)iBand, 0);
		m_bVisible = false;
	}
}

//
// Returns the index of hToolbar in the rebar hRebar.
// Returns -1 on error or if the toolbar wasn't recognized as a
// child of the specified rebar control.
//
int EV_Win32Toolbar::_getBandForHwnd(HWND hToolbar) const
{
	HWND hRebar = static_cast<XAP_Win32FrameImpl*>(m_pWin32Frame->getFrameImpl())->getToolbarWindow();

	// If we ever get more than 20 toolbars I don't wanna be around.
	REBARBANDINFOW rbi;
	memset(&rbi, 0, sizeof(rbi));
	rbi.cbSize = sizeof(rbi);
	rbi.fMask  = RBBIM_CHILD;
	for (int i = 0; i < 20; ++i)
	{
		if (!SendMessageW(hRebar, RB_GETBANDINFO, (WPARAM)i, (LPARAM)&rbi))
		{
			continue;
		}
		if (hToolbar == rbi.hwndChild)
		{
			return i;
		}
	}
	return -1;
}

void EV_Win32Toolbar::_addToRebar()
{
	// Get the height of the toolbar.
	DWORD dwBtnSize = SendMessageW(m_hwnd, TB_GETBUTTONSIZE, 0,0);

	RECT rc;
	GetWindowRect(m_hwnd, &rc);
//	UINT iWidth = rc.right + 13;
	UINT iWidth = rc.right - rc.left + 13;

	// add this bar to the rebar
	REBARBANDINFOW rbbi;
	memset(&rbbi, 0, sizeof(rbbi));
	// Initialize REBARBANDINFO
	rbbi.cbSize = sizeof(REBARBANDINFOW);
	rbbi.fMask =	RBBIM_COLORS	|	// clrFore and clrBack are valid
					RBBIM_CHILD		|	// hwndChild is valid
					RBBIM_CHILDSIZE	|	// cxMinChild and cyMinChild are valid
					RBBIM_SIZE		|	// cx is valid
					RBBIM_STYLE;		// fStyle is valid
	rbbi.clrFore = GetSysColor(COLOR_BTNTEXT);
	rbbi.clrBack = GetSysColor(COLOR_BTNFACE);
	// NOT using RBBS_CHILDEDGE for fStyle gives us a slimmer look,
	// better with the flat button style.
	rbbi.fStyle =	RBBS_NOVERT			|	// do not display in vertical orientation
					RBBS_BREAK;
	rbbi.hwndChild = m_hwnd;
	rbbi.cxMinChild = LOWORD(dwBtnSize);
	rbbi.cyMinChild = HIWORD(dwBtnSize);
	rbbi.cx = iWidth;

	HWND hRebar = static_cast<XAP_Win32FrameImpl*>(m_pWin32Frame->getFrameImpl())->getToolbarWindow();
	// Add it at the the end
	SendMessageW(hRebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbbi);
}


XAP_Frame * EV_Win32Toolbar::getFrame(void)
{
	return m_pWin32Frame;
}


/*!
 * This method examines the current document and repopulates the Styles
 * Combo box with what is in the document. It returns false if no styles 
 * combo box was found. True if it all worked.
 */
bool EV_Win32Toolbar::repopulateStyles(void)
{
//
// First off find the Styles combobox in a toolbar somewhere
//
	UT_uint32 count = m_pToolbarLayout->getLayoutItemCount();
	UT_uint32 i =0;
	EV_Toolbar_LayoutItem * pLayoutItem = NULL;
	XAP_Toolbar_Id id;
	for(i=0; i < count; i++)
	{
		pLayoutItem = m_pToolbarLayout->getLayoutItem(i);
		id = pLayoutItem->getToolbarId();
	//	wd = (_wd *) getNthItem(i);
		if(id == AP_TOOLBAR_ID_FMT_STYLE)
			break;
	}
	if(i>=count)
		return false;
	


//
// GOT IT!
//
  //	UT_ASSERT(wd->m_id == AP_TOOLBAR_ID_FMT_STYLE);
	XAP_Toolbar_ControlFactory * pFactory = m_pWin32App->getControlFactory();
	UT_return_val_if_fail(pFactory,false);
	EV_Toolbar_Control * pControl = pFactory->getControl(this, id);
	AP_Win32Toolbar_StyleCombo * pStyleC = static_cast<AP_Win32Toolbar_StyleCombo *>(pControl);
	pStyleC->repopulate();
	
	
	HWND hwndCombo = _getControlWindow(id);
	UT_ASSERT(hwndCombo);
	// GtkCombo * item = GTK_COMBO(wd->m_widget);
//
// Now the combo box has to be refilled from this
//						
	const UT_GenericVector<const char*> * v = pControl->getContents();
	UT_return_val_if_fail(v,false);
	
//
// Now  we must remove and delete the old data so we add the new
// list of styles to the combo box.
//
// Try this....
	SendMessageW(hwndCombo, WM_SETREDRAW, FALSE, 0);
	SendMessageW(hwndCombo, CB_RESETCONTENT, 0 , 0);

	SendMessageW(hwndCombo, CB_SETDROPPEDWIDTH,
				(WPARAM)pStyleC->getDroppedWidth(), 0);

//
//	GtkList * oldlist = GTK_LIST(item->list);
//	gtk_list_clear_items(oldlist,0,-1);
//
// Now make a new one.
//
	int	nItem;													    
	std::string utf8;
	UT_String str;

	for (UT_sint32 c=0; c < m_vecOrgStylesNames.getItemCount(); c++)	
		delete m_vecOrgStylesNames.getNthItem(c);

	m_vecOrgStylesNames.clear();
	
	
	for (UT_sint32 k=0; k < v->getItemCount(); k++)
	{
		const char*	sz = (char *)v->getNthItem(k);
		UT_Win32LocaleString localised;
		
		pt_PieceTable::s_getLocalisedStyleName(sz, utf8);
		localised.fromUTF8 (utf8.c_str());
		
		nItem = SendMessageW(hwndCombo, CB_ADDSTRING,(WPARAM)0, (LPARAM)localised.c_str());
		m_vecOrgStylesNames.addItem (new UT_UTF8String ((char *)v->getNthItem(k)));
		SendMessageW(hwndCombo, CB_SETITEMDATA,(WPARAM)nItem, (LPARAM)m_vecOrgStylesNames.getItemCount()-1);
	}
//
// Don't need this anymore and we don't like memory leaks in abi
//
	SendMessageW(hwndCombo, WM_SETREDRAW, TRUE, 0);
	InvalidateRect (hwndCombo, NULL, true);
	delete pStyleC;
//
// I think we've finished!
//
	return true;
}

/*
	This method is called when the user clicks on the arrow next to the
	foreground and background colour seletion buttons on the toolbar
*/
void	EV_Win32Toolbar::onDropArrow(UINT cmd)
{
	AV_View * pView = m_pWin32Frame->getCurrentView();
	XAP_Frame * pFrame = (XAP_Frame *) pView->getParentData();
	UT_return_if_fail(pFrame);

	UT_UCS4String ucs4color;
	int id = ItemIdFromWmCommand(cmd);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_Background * pDialog
		= (AP_Dialog_Background *)(pDialogFactory->requestDialog(AP_DIALOG_ID_BACKGROUND));
	UT_ASSERT_HARMLESS(pDialog);
	if (pDialog)
	{
		//
		// Get Current color
		//

		const EV_Toolbar_ActionSet * pToolbarActionSet = m_pWin32App->getToolbarActionSet();
		UT_return_if_fail(pToolbarActionSet);

		const EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
		if (!pAction)	return;


		XAP_Win32FrameImpl* fimpl = static_cast<XAP_Win32FrameImpl*>(m_pWin32Frame->getFrameImpl());

		if (!fimpl->m_sColorBack.empty() && pAction->getItemType() == EV_TBIT_ColorBack)
			pDialog->setColor(fimpl->m_sColorBack.utf8_str());

		if (!fimpl->m_sColorFore.empty() && pAction->getItemType() == EV_TBIT_ColorFore)
			pDialog->setColor(fimpl->m_sColorFore.utf8_str());

		pDialog->runModal (pFrame);

		AP_Dialog_Background::tAnswer ans = pDialog->getAnswer();
		bool bOK = (ans == AP_Dialog_Background::a_OK);			

		if (bOK)
		{
			UT_UTF8String strColor;
			strColor = (reinterpret_cast<const char *>(pDialog->getColor()));								
			
			if (pAction->getItemType() == EV_TBIT_ColorBack)
				fimpl->m_sColorBack = strColor;

			if (pAction->getItemType() == EV_TBIT_ColorFore)
				fimpl->m_sColorFore = strColor;

			toolbarEvent(id, NULL, 0);
		}

		pDialogFactory->releaseDialog(pDialog);
	}
}

