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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define _WIN32_IE 0x0300 // specify minimal comctl.dll v4.70 for toolbars
#include <commctrl.h>   // includes the common control header

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_vector.h"
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
class foo_Bitmap_container
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
		void* p = m_bitmaps_vector.getNthItem(iItem - 1);
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
	m_pFontCtrl(NULL),
	m_hwnd(0)
{
}

EV_Win32Toolbar::~EV_Win32Toolbar(void)
{
	_releaseListener();

	if (m_pFontCtrl)
		delete m_pFontCtrl;
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
	UT_ASSERT(pToolbarActionSet);

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
			UT_ASSERT(UT_TODO);
			
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
		UT_ASSERT(pToolbarActionSet);

		const EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);

		if (!pAction) return true;

		AV_View * pView = m_pWin32Frame->getCurrentView();
		
		const char * szMethodName = pAction->getMethodName();
		if (!szMethodName) return true;
		
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
	UT_ASSERT(pEMC);

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
WHICHPROC s_lpfnDefComboEdit; 


#define COMBO_BUF_LEN 256


LRESULT CALLBACK EV_Win32Toolbar::_ComboWndProc( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
		/* Draws the font preview in the font selection combobox*/
		case WM_DRAWITEM:
		{						
			EV_Win32Toolbar * t = (EV_Win32Toolbar *) GetWindowLong(hWnd, GWL_USERDATA);			
			UINT u = GetDlgCtrlID(hWnd);
			XAP_Toolbar_Id id = t->ItemIdFromWmCommand(u);
						
			if (id!=AP_TOOLBAR_ID_FMT_FONT)	/* Only owner draw the font selection*/
			{
				UT_ASSERT(0);
				break;
			}
								
			DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)lParam;			
			XAP_Toolbar_ControlFactory * pFactory = t->m_pWin32App->getControlFactory();			

			if (!t->m_pFontCtrl)
			{	
				EV_Toolbar_Control * pControl = pFactory->getControl(t, AP_TOOLBAR_ID_FMT_FONT);                                                                         						
				t->m_pFontCtrl = static_cast<AP_Win32Toolbar_FontCombo *>(pControl);
				t->m_pFontCtrl->populate();
			}

			HFONT hFont, hUIFont;
			LOGFONT logfont;
			SIZE size;
			HFONT hfontSave;
			const UT_Vector * v = t->m_pFontCtrl->getContents();
			const UT_Vector * vcharSet = t->m_pFontCtrl->getFontsCharset();						
			const char * sz  = (const char *)v->getNthItem(dis->itemData);			
			const int nCharset = (int)vcharSet->getNthItem(dis->itemData);						
			
			if(dis->itemState & ODS_COMBOBOXEDIT)
			{
				HFONT hUIFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
				hfontSave = (HFONT) SelectObject (dis->hDC, hUIFont);
				ExtTextOut(dis->hDC, dis->rcItem.left, dis->rcItem.top, ETO_OPAQUE | ETO_CLIPPED, 0, sz, lstrlen(sz), 0);
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
				strcpy (logfont.lfFaceName, sz);   
				hFont = CreateFontIndirect (&logfont);			

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
				ExtTextOut(dis->hDC, dis->rcItem.left, dis->rcItem.top, ETO_OPAQUE | ETO_CLIPPED, 0, sz, lstrlen(sz), 0);
				
				/*Font example after the name*/
				const char* szSample="AbCdEfGhIj";
				GetTextExtentPoint32(dis->hDC, sz, lstrlen(sz), &size);
				SelectObject (dis->hDC, hFont);
				ExtTextOut(dis->hDC, dis->rcItem.left+size.cx+5, dis->rcItem.top, ETO_OPAQUE | ETO_CLIPPED, 0, szSample, lstrlen(szSample), 0);					
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

			hWndTT = (HWND)SendMessage(hwndToolbar, TB_GETTOOLTIPS, 0,0);
			SendMessage(hWndTT, TTM_RELAYEVENT, 0, (LPARAM)(LPMSG)&msg);
			break;
		}

		case WM_COMMAND:
		{
			switch (HIWORD(wParam))
			{								
				case CBN_SELCHANGE:
				{
					// are we currently dropped down?
					if (!SendMessage(hWnd, CB_GETDROPPEDSTATE, 0, 0))
						break;

					// yep, we're done 
					SendMessage(hWnd, CB_SHOWDROPDOWN, (WPARAM) FALSE, 0);

					UT_sint32 iSelected = SendMessage(hWnd, CB_GETCURSEL, 0, 0);		
									
					if(iSelected != -1)
					{							
						EV_Win32Toolbar * t = (EV_Win32Toolbar *) GetWindowLong(hWnd, GWL_USERDATA);
						UT_ASSERT(t);

						
						UINT u = GetDlgCtrlID(hWnd);
						XAP_Toolbar_Id id = t->ItemIdFromWmCommand(u);

						static UT_UCSChar ucs_buf[COMBO_BUF_LEN];
						char buf[COMBO_BUF_LEN];

						UT_uint32 dataLength = GetWindowText(hWnd, buf, COMBO_BUF_LEN);

						UT_UCS4_strcpy_char(ucs_buf, buf);
						UT_UCSChar * pData = (UT_UCSChar *) ucs_buf;	// HACK: should be void *
						
						if(dataLength)
						{	
							// If is a STYLE_NAME we should pass the internal one, not the localised				
							if (id==AP_TOOLBAR_ID_FMT_STYLE)
							{									
								UT_sint32 iSelected;					
								int nData;
								
								iSelected = SendMessage(hWnd, CB_GETCURSEL, 0, 0);										

								// Find the proper non-localised text                                                                             					
								XAP_Toolbar_ControlFactory * pFactory = t->m_pWin32App->getControlFactory();
								EV_Toolbar_Control * pControl = pFactory->getControl(t, AP_TOOLBAR_ID_FMT_STYLE);                                                                         
								const UT_Vector * v = pControl->getContents();                                                                          
								AP_Win32Toolbar_StyleCombo * pStyleC = static_cast<AP_Win32Toolbar_StyleCombo *>(pControl);
								pStyleC->repopulate();                                                                                                

								nData  = SendMessage(hWnd, CB_GETITEMDATA, iSelected, 0);                                                         													
								UT_UCS4_strcpy_char(ucs_buf, (char *)v->getNthItem(nData));				
								DELETEP(pControl);
							}
							
							t->toolbarEvent(id, pData, dataLength);
						}

						SetFocus(static_cast<XAP_Win32FrameImpl*>(t->m_pWin32Frame->getFrameImpl())->getTopLevelWindow());
					}	
					else
					{
						PostMessage(hWnd, WM_KEYDOWN, VK_ESCAPE, 0);
					}
						
					break;
				}					
				
							
				break;
				
			} // swich
		} // case command
			
		case WM_KEYDOWN:
		{
			EV_Win32Toolbar * t = (EV_Win32Toolbar *) GetWindowLong(hWnd, GWL_USERDATA);
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
	return (CallWindowProc(s_lpfnDefCombo, hWnd, uMessage, wParam, lParam));
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
	const XML_Char * szValue = NULL;
	m_pWin32App->getPrefsValue(XAP_PREF_KEY_ToolbarAppearance,&szValue);
	UT_ASSERT((szValue) && (*szValue));

	if (UT_XML_stricmp(szValue,"icon") == 0)
	{
		bIcons = true;
		bText = false;
	}
	else if (UT_XML_stricmp(szValue,"text") == 0)
	{
		bIcons = false;
		bText = true;
	}
	else if (UT_XML_stricmp(szValue,"both") == 0)
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

	m_hwnd = CreateWindowEx(0, 
				TOOLBARCLASSNAME,		// window class name
				(LPSTR) NULL,			// window caption
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
				NULL);					// creation parameters

	UT_ASSERT(m_hwnd);

	// override the window procedure 	
	SetWindowLong(m_hwnd, GWL_USERDATA, (LONG)this);


	SendMessage(m_hwnd, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);  

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
		SendMessage(m_hwnd, TB_SETBITMAPSIZE, 0,
					(LPARAM) MAKELONG(MY_MAXIMUM_BITMAP_X,MY_MAXIMUM_BITMAP_Y));
	else
		SendMessage(m_hwnd, TB_SETBITMAPSIZE, 0, (LPARAM) MAKELONG(0,0));

	DWORD dwColor = GetSysColor(COLOR_BTNFACE);
	UT_RGBColor backgroundColor(GetRValue(dwColor),GetGValue(dwColor),GetBValue(dwColor));

	// TODO: is there any advantage to building up all the TBBUTTONs at once
	//		 and then adding them en masse, instead of one at a time? 
	UINT last_id=0;
	bool bControls = false;

	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Toolbar_LayoutItem * pLayoutItem = m_pToolbarLayout->getLayoutItem(k);
		UT_ASSERT(pLayoutItem);

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
						UT_ASSERT(pControl);

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
								CBS_HASSTRINGS | CBS_DROPDOWNLIST;

						if ((pControl) && (pControl->shouldSort()))
						{
							dwStyle |= CBS_SORT;
						}
						
						if (id==AP_TOOLBAR_ID_FMT_FONT)
							dwStyle |= CBS_OWNERDRAWFIXED;
							
							
						HWND hwndCombo = CreateWindowEx ( 0L,   // No extended styles.
							"COMBOBOX",						// Class name.
							"",								// Default text.
							dwStyle,						// Styles and defaults.
							0, 2, iWidth, 250,				// Size and position.
							m_hwnd,							// Parent window.
							(HMENU) u,						// ID.
							m_pWin32App->getInstance(),		// Current instance.
							NULL );							// No class data.

						UT_ASSERT(hwndCombo);
						
						SendMessage(hwndCombo, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
						SendMessage(hwndCombo, CB_SETDROPPEDWIDTH,(WPARAM)pControl->getDroppedWidth(), 0);

						// populate it
						if (pControl)
						{
							pControl->populate();

							const UT_Vector * v = pControl->getContents();
							UT_ASSERT(v);

							SendMessage(hwndCombo, WM_SETREDRAW, FALSE,0);

							if (v)
							{
								UT_uint32 items = v->getItemCount();
								int nIndex;
								for (UT_uint32 k=0; k < items; k++)
								{
									char * sz = (char *)v->getNthItem(k);
									nIndex = SendMessage(hwndCombo, CB_ADDSTRING,(WPARAM)0, (LPARAM)sz);
									SendMessage(hwndCombo,CB_SETITEMDATA, nIndex, k);
								}
							}

							SendMessage(hwndCombo, WM_SETREDRAW, TRUE,0);
						}

						// override the window procedure for the combo box
						s_lpfnDefCombo = (WHICHPROC)GetWindowLong(hwndCombo, GWL_WNDPROC);
						SetWindowLong(hwndCombo, GWL_WNDPROC, (LONG)_ComboWndProc);
						SetWindowLong(hwndCombo, GWL_USERDATA, (LONG)this);

						// Get the handle to the tooltip window.
						HWND hwndTT = (HWND)SendMessage(m_hwnd, TB_GETTOOLTIPS, 0, 0);

						if (hwndTT)
						{
							const char * szToolTip = pLabel->getToolTip();
							if (!szToolTip || !*szToolTip)
							{
								szToolTip = pLabel->getStatusMsg();
							}

							// Fill in the TOOLINFO structure.
							TOOLINFO ti;

							ti.cbSize = sizeof(ti);
							ti.uFlags = TTF_IDISHWND | TTF_CENTERTIP;
							ti.lpszText = (char *) szToolTip;
							ti.hwnd = m_hwnd;		// TODO: should this be the frame?
							ti.uId = (UINT)hwndCombo;
							// Set up tooltips for the combo box.
							SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
						}
						
						// bind this separator to its control
						tbb.dwData = (DWORD) hwndCombo;

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
					UT_ASSERT(0);
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
						const bool bFoundIcon =
							AP_Win32Toolbar_Icons::getBitmapForIcon(m_hwnd,
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
						
						LRESULT iAddedAt = SendMessage(m_hwnd,TB_ADDBITMAP,1,(LPARAM)&ab);
						UT_ASSERT(iAddedAt != -1);

						tbb.iBitmap = iAddedAt;
					}
					
					if (bText)
					{
						const char * szLabel = pLabel->getToolbarLabel();
						// As long as translators don't cut the text short, we need to autosize just to squize anything insize 1280 :)
						tbb.fsStyle |= TBSTYLE_AUTOSIZE;
						tbb.iString = SendMessage(m_hwnd, TB_ADDSTRING, (WPARAM) 0, (LPARAM) (LPSTR) szLabel);
					}
					
					if (pAction->getItemType() == EV_TBIT_ColorFore || pAction->getItemType() == EV_TBIT_ColorBack)
					{
						tbb.fsStyle |=TBSTYLE_DROPDOWN;

						//tbb.fsStyle |=BTNS_WHOLEDROPDOWN;

						DWORD dwStyle = SendMessage(m_hwnd, TB_GETEXTENDEDSTYLE, (WPARAM) 0, (LPARAM) 0);						
						dwStyle |= TBSTYLE_EX_DRAWDDARROWS;																		
						SendMessage(m_hwnd, TB_SETEXTENDEDSTYLE, (WPARAM) 0, (LPARAM) dwStyle);						
					}
					
				}
			}
			break;
		
		case EV_TLF_Spacer:
			tbb.fsState = TBSTATE_ENABLED; 
			tbb.fsStyle = TBSTYLE_SEP;     
			break;
			
		default:
			UT_ASSERT(0);
		}

		// add this button to the bar
		SendMessage(m_hwnd, TB_ADDBUTTONS, (WPARAM) 1, (LPARAM) (LPTBBUTTON) &tbb);
	}

	// figure out bar dimensions now that buttons are all there
	SendMessage(m_hwnd, TB_AUTOSIZE, 0, 0); 
	
	if (bControls)
	{
		// move each control on top of its associated separator
		for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
		{
			EV_Toolbar_LayoutItem * pLayoutItem = m_pToolbarLayout->getLayoutItem(k);
			UT_ASSERT(pLayoutItem);

			XAP_Toolbar_Id id = pLayoutItem->getToolbarId();
			EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
			UT_ASSERT(pAction);

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

					SendMessage(m_hwnd, TB_GETITEMRECT, (WPARAM) k, (LPARAM)(LPRECT) &r);

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

	UT_uint32 k = SendMessage(m_hwnd, TB_COMMANDTOINDEX, (WPARAM) u, 0);
	SendMessage(m_hwnd, TB_GETBUTTON, (WPARAM) k, (LPARAM)(LPTBBUTTON) &tbb);
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

	bool bResult = pView->addListener(static_cast<AV_Listener *>(m_pViewListener),&m_lid);
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
		UT_ASSERT(pLayoutItem);

		XAP_Toolbar_Id id = pLayoutItem->getToolbarId();
		EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
		UT_ASSERT(pAction);

		AV_ChangeMask maskOfInterest = pAction->getChangeMaskOfInterest();
		if ((maskOfInterest & mask) == 0)					// if this item doesn't care about
			continue;										// changes of this type, skip it...

		switch (pLayoutItem->getToolbarLayoutFlags())
		{
		case EV_TLF_Normal:
			{
				_refreshItem(pView, pAction, id);
			}
			break;
			
		case EV_TLF_Spacer:
			break;
			
		default:
			UT_ASSERT(0);
			break;
		}
	}

	return true;
}

bool EV_Win32Toolbar::_refreshID(XAP_Toolbar_Id id)
{
	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pWin32App->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);

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

				SendMessage(m_hwnd, TB_ENABLEBUTTON, u, (LONG)!bGrayed) ;

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
				
				SendMessage(m_hwnd, TB_ENABLEBUTTON, u, (LONG)!bGrayed);
				SendMessage(m_hwnd, TB_CHECKBUTTON, u, (LONG)bToggled);

				//UT_DEBUGMSG(("refreshToolbar: ToggleButton [%s] is %s and %s\n",
				//			 m_pToolbarLabelSet->getLabel(id)->getToolbarLabel(),
				//			 ((bGrayed) ? "disabled" : "enabled"),
				//			 ((bToggled) ? "pressed" : "not pressed")));
			}
			break;

		case EV_TBIT_ComboBox:
			{
				bool bGrayed = EV_TIS_ShouldBeGray(tis);
				bool bString = EV_TIS_ShouldUseString(tis);
				
				HWND hwndCombo = _getControlWindow(id);
				UT_return_val_if_fail(hwndCombo, true);

				
				// NOTE: we always update the control even if !szState
				if (!szState)
				{
					int idx = SendMessage(hwndCombo, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)szState);
					if (idx==CB_ERR)
						SetWindowText(hwndCombo, "");
					break;
				}					
				
				
				// are we currently dropped down?
			
				// Find the proper non-localised text
		
				EV_Win32Toolbar * t = (EV_Win32Toolbar *) GetWindowLong(hwndCombo, GWL_USERDATA);
				XAP_Toolbar_ControlFactory * pFactory = t->m_pWin32App->getControlFactory();			

				EV_Toolbar_Control * pControl = pFactory->getControl(t, AP_TOOLBAR_ID_FMT_STYLE);			
				const UT_Vector * v = pControl->getContents();				
					
				AP_Win32Toolbar_StyleCombo * pStyleC = static_cast<AP_Win32Toolbar_StyleCombo *>(pControl);
				pStyleC->repopulate();	
				
				//
				// Is this a valid text?
				//
				UT_uint32 items = v->getItemCount();
				bool bFound = false;
				UT_uint32 k=0;
				
				for (k=0; k < items; k++)
				{
					if (strcmp((char *)v->getNthItem(k), szState)==0)
					{
						bFound = true;
						break;
					}	
				}				
				
				char* pLocalised = (char *)szState;
				
				if (bFound)
				{
				 	pLocalised = (char *)v->getNthItem(k);					
					pLocalised = (char *) pt_PieceTable::s_getLocalisedStyleName(pLocalised);
				}								
												
				int idx = SendMessage(hwndCombo, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)pLocalised);
				if (idx==CB_ERR)
					SetWindowText(hwndCombo, pLocalised);							
	
				DELETEP(pControl);	
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
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			break;
	}

	return true;
}

bool EV_Win32Toolbar::getToolTip(LPARAM lParam)
{
	UT_ASSERT(lParam);
	LPTOOLTIPTEXT lpttt = (LPTOOLTIPTEXT) lParam;

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
		// here 'tis
		strncpy(lpttt->lpszText, szToolTip, 80);
	}
	else
	{
		lpttt->lpszText[0] = '\0';
	}

	return true;
}


void EV_Win32Toolbar::show(void)
{
	UT_ASSERT(m_pWin32Frame);
	HWND hRebar = static_cast<XAP_Win32FrameImpl*>(m_pWin32Frame->getFrameImpl())->getToolbarWindow();
	UT_ASSERT(hRebar);
	const int iBand = _getBandForHwnd(m_hwnd);
	UT_ASSERT(iBand < 0);	// It can't already be displayed!
	_addToRebar();

	m_bVisible = true;
}

void EV_Win32Toolbar::hide(void)
{
	UT_ASSERT(m_pWin32Frame);
	HWND hRebar = static_cast<XAP_Win32FrameImpl*>(m_pWin32Frame->getFrameImpl())->getToolbarWindow();
	UT_ASSERT(hRebar);
	const int iBand = _getBandForHwnd(m_hwnd);
	if (iBand >= 0)
	{
		SendMessage(hRebar, RB_DELETEBAND, (WPARAM)iBand, 0);
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
	REBARBANDINFO rbi = { 0 };
	rbi.cbSize = sizeof(rbi);
	rbi.fMask  = RBBIM_CHILD;
	for (int i = 0; i < 20; ++i)
	{
		if (!SendMessage(hRebar, RB_GETBANDINFO, (WPARAM)i, (LPARAM)&rbi))
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
	DWORD dwBtnSize = SendMessage(m_hwnd, TB_GETBUTTONSIZE, 0,0);

	RECT rc;
	GetWindowRect(m_hwnd, &rc);
//	UINT iWidth = rc.right + 13;
	UINT iWidth = rc.right - rc.left + 13;

	// add this bar to the rebar
	REBARBANDINFO  rbbi = { 0 };
	// Initialize REBARBANDINFO
	rbbi.cbSize = sizeof(REBARBANDINFO);
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
	SendMessage(hRebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbbi);
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
	//	wd = (_wd *) m_vecToolbarWidgets.getNthItem(i);
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
	UT_ASSERT(pFactory);
	EV_Toolbar_Control * pControl = pFactory->getControl(this, id);
	AP_Win32Toolbar_StyleCombo * pStyleC = static_cast<AP_Win32Toolbar_StyleCombo *>(pControl);
	pStyleC->repopulate();
	
	
	HWND hwndCombo = _getControlWindow(id);
	UT_ASSERT(hwndCombo);
	// GtkCombo * item = GTK_COMBO(wd->m_widget);
//
// Now the combo box has to be refilled from this
//						
	const UT_Vector * v = pControl->getContents();
	UT_ASSERT(v);
	
//
// Now  we must remove and delete the old data so we add the new
// list of styles to the combo box.
//
// Try this....
	SendMessage(hwndCombo, CB_RESETCONTENT, 0 , 0);

	SendMessage(hwndCombo, CB_SETDROPPEDWIDTH,
				(WPARAM)pStyleC->getDroppedWidth(), 0);

//
//	GtkList * oldlist = GTK_LIST(item->list);
//	gtk_list_clear_items(oldlist,0,-1);
//
// Now make a new one.
//
	UT_uint32 items = v->getItemCount();
	int	nItem;													    
	
	for (UT_uint32 k=0; k < items; k++)
	{
		char*	sz = (char *)v->getNthItem(k);
		char*	pLocalised = sz;

		pLocalised = (char *) pt_PieceTable::s_getLocalisedStyleName(sz);
		
		if (pLocalised!=sz)
			int n=1;			

		nItem = SendMessage(hwndCombo, CB_ADDSTRING,(WPARAM)0, (LPARAM)pLocalised);
		SendMessage(hwndCombo, CB_SETITEMDATA,(WPARAM)nItem, (LPARAM)k);
	}
//
// Don't need this anymore and we don't like memory leaks in abi
//
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
	UT_ASSERT(pFrame);

	UT_UCS4String ucs4color;
	int id = ItemIdFromWmCommand(cmd);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_Background * pDialog
		= (AP_Dialog_Background *)(pDialogFactory->requestDialog(AP_DIALOG_ID_BACKGROUND));
	UT_ASSERT(pDialog);
	if (pDialog)
	{
		//
		// Get Current color
		//

		const EV_Toolbar_ActionSet * pToolbarActionSet = m_pWin32App->getToolbarActionSet();
		UT_ASSERT(pToolbarActionSet);

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
		}

		pDialogFactory->releaseDialog(pDialog);
	}
}

