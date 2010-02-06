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
 
/*
	In versions previous to Windows Vista we use owner-draw menus. 
	In Vista we use SetMenuItemBitmaps that is more powerful that previous Windows editions
*/

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "ut_types.h"
#include "ut_stack.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_debugmsg.h"
#include "ut_Language.h"
#include "xap_Types.h"
#include "ev_Win32Menu.h"
#include "xap_Win32App.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Actions.h"
#include "ev_Menu_Labels.h"
#include "ev_EditEventMapper.h"
#include "xap_Frame.h"
#include "ap_Win32Resources.rc2"
#include "ap_Menu_Id.h"
#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32Toolbar_Icons.h"
#include "ap_Win32App.h"
#include "ut_Win32OS.h"

#define SPACE_ICONTEXT	4	// Pixels between the icon and the text
#define BITMAP_WITDH	16
#define BITMAP_HEIGHT	16


/*
	This table assigns an icon to every menu entry
*/
static const EV_Menu_Bitmap s_bitmaps[] = 
{	  

	
    // File
	{AP_MENU_ID_FILE_CLOSE,  "Menu_AbiWord_Close"},
	{AP_MENU_ID_FILE_EXIT,		"Menu_AbiWord_Exit"},	
	{AP_MENU_ID_FILE_EXPORT, "Menu_AbiWord_Export"},
	{AP_MENU_ID_FILE_IMPORT, "Menu_AbiWord_Import"},
	{AP_MENU_ID_FILE_NEW,	 "Menu_AbiWord_New"},
	{AP_MENU_ID_FILE_OPEN,	 "Menu_AbiWord_Open"},
	{AP_MENU_ID_FILE_PRINT,		"Menu_AbiWord_Print"}, 
	{AP_MENU_ID_FILE_SAVE,		"Menu_AbiWord_Save"}, 
	{AP_MENU_ID_FILE_SAVEAS,	"Menu_AbiWord_SaveAs"}, 
	{AP_MENU_ID_FILE_REVERT, "Menu_AbiWord_Revert"},
	{AP_MENU_ID_FILE_PROPERTIES, "Menu_AbiWord_Properties"},
	{AP_MENU_ID_FILE_PAGESETUP, "Menu_AbiWord_Print_Setup"},
    
	// Edit
	{AP_MENU_ID_EDIT_PASTE, "Menu_AbiWord_Paste"},				
	{AP_MENU_ID_EDIT_CUT, "Menu_AbiWord_Cut"},				
	{AP_MENU_ID_EDIT_COPY, "Menu_AbiWord_Copy"},				
	{AP_MENU_ID_EDIT_UNDO, "Menu_AbiWord_Undo"},				
	{AP_MENU_ID_EDIT_REDO, "Menu_AbiWord_Redo"},				
	{AP_MENU_ID_EDIT_PASTE_SPECIAL, "Menu_AbiWord_Paste"},
	{AP_MENU_ID_EDIT_CLEAR, "Menu_AbiWord_Clear"},
	{AP_MENU_ID_EDIT_FIND, "Menu_AbiWord_Search"},
	{AP_MENU_ID_EDIT_REPLACE, "Menu_AbiWord_Replace"},
	{AP_MENU_ID_EDIT_GOTO, "Menu_AbiWord_Goto"},

	// Insert
	{AP_MENU_ID_INSERT_SYMBOL,	"Menu_AbiWord_Insert_Symbol"},
	{AP_MENU_ID_INSERT_GRAPHIC, "Menu_AbiWord_Img"},
	{AP_MENU_ID_INSERT_HYPERLINK,	"Menu_AbiWord_Hyperlink"},	

	// Format
	{AP_MENU_ID_FMT_FONT, "Menu_AbiWord_Font"},
	{AP_MENU_ID_TABLE_INSERT_TABLE, "Menu_AbiWord_Insert_Table"},
	{AP_MENU_ID_TOOLS_SPELL, "Menu_AbiWord_Spellcheck"},			
	{AP_MENU_ID_TOOLS_OPTIONS, "Menu_AbiWord_Preferences"},
	{AP_MENU_ID_TOOLS_SCRIPTS, "Menu_AbiWord_Execute"},
	{AP_MENU_ID_TOOLS_LANGUAGE, "Menu_AbiWord_Book"},
	{AP_MENU_ID_FMT_LANGUAGE, "Menu_AbiWord_Book"},


	// Table
	{AP_MENU_ID_TABLE_INSERT_TABLE, "Menu_AbiWord_Insert_Table"},
	{AP_MENU_ID_TABLE_DELETE_TABLE, "Menu_AbiWord_Delete_Table"},
	{AP_MENU_ID_TABLE_INSERTTABLE, "Menu_AbiWord_Insert_Table"},
	{AP_MENU_ID_TABLE_DELETETABLE, "Menu_AbiWord_Delete_Table"},
	{AP_MENU_ID_TABLE_MERGE_CELLS, "Menu_AbiWord_Merge_Cells"},
	{AP_MENU_ID_TABLE_SPLIT_CELLS, "Menu_AbiWord_Split_Cells"},
	{AP_MENU_ID_TABLE_INSERTCOLUMN, "Menu_AbiWord_Add_Column"},
	{AP_MENU_ID_TABLE_INSERTROW, "Menu_AbiWord_Add_Row"},
	{AP_MENU_ID_TABLE_DELETECOLUMN, "Menu_AbiWord_Delete_Column"},
	{AP_MENU_ID_TABLE_DELETEROW, "Menu_AbiWord_Delete_Row"},
	{AP_MENU_ID_TABLE_DELETE_COLUMNS, "Menu_AbiWord_Delete_Column"},
	{AP_MENU_ID_TABLE_DELETE_ROWS, "Menu_AbiWord_Delete_Row"},
	

	// Help
	{AP_MENU_ID_HELP_SEARCH, "Menu_AbiWord_Search"},
	{AP_MENU_ID_HELP_ABOUT, "Menu_AbiWord_About"},
	{AP_MENU_ID_HELP_CREDITS, "Menu_AbiWord_Credits"},
	
	{0, NULL}

		
};



/*

*/
static const char * _ev_GetLabelName(XAP_Win32App * pWin32App,
									 XAP_Frame * /*pFrame*/, 
									 const EV_EditEventMapper * pEEM,
									 const EV_Menu_Action * pAction,
									 EV_Menu_Label * pLabel)
{
	const char * szLabelName;
	
	if (pAction->hasDynamicLabel())
		szLabelName = pAction->getDynamicLabel(pLabel);
	else
		szLabelName = pLabel->getMenuLabel();

	if (!szLabelName || !*szLabelName)
		return NULL;

	UT_String str = AP_Win32App::s_fromUTF8ToWinLocale(szLabelName);
	szLabelName = str.c_str();

	
	const char * szShortcut = NULL;
	int len = 0;

	if (pEEM)
	{
		// see if this has an associated keybinding
		const char * szMethodName = pAction->getMethodName();

		if (szMethodName)
		{
			const EV_EditMethodContainer * pEMC = pWin32App->getEditMethodContainer();
			UT_return_val_if_fail(pEMC, NULL);

			EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
			UT_ASSERT(pEM);					// make sure it's bound to something

			szShortcut = pEEM->getShortcutFor(pEM);
			if (szShortcut && *szShortcut)
				len = strlen(szShortcut) + 1;	// extra char is for the tab
		}
	}
	
	if (pAction->raisesDialog())
		len += 4;

	if (!len)
	{
		static char buf[128];
		strcpy(buf,szLabelName);
		return buf;
	}

	static char buf[128];
	memset(buf,0,G_N_ELEMENTS(buf));
	strncpy(buf,szLabelName,G_N_ELEMENTS(buf)-len);

	// append "..." to menu item if it raises a dialog
	if (pAction->raisesDialog())
		strcat(buf,"...");

	// append shortcut mnemonic, if any
	if (szShortcut && *szShortcut)
	{
		strcat(buf, "\t");
		strcat(buf, szShortcut);
	}
										  
	return buf;
}



/*****************************************************************/

EV_Win32Menu::EV_Win32Menu(XAP_Win32App * pWin32App,
						   const EV_EditEventMapper * pEEM,
						   const char * szMenuLayoutName,
						   const char * szMenuLabelSetName)
:	EV_Menu(pWin32App, pWin32App->getEditMethodContainer(), szMenuLayoutName, szMenuLabelSetName),
	m_pWin32App(pWin32App),
	m_pEEM(pEEM),
	m_myMenu(NULL),
	m_iDIR(0)
{
		    
	NONCLIENTMETRICSA ncm;
	ncm.cbSize = sizeof (NONCLIENTMETRICSA);
	m_bTrack = false;

    SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSA), &ncm, 0);
	m_hFont = CreateFontIndirectA(&ncm.lfMenuFont);	

	m_nBitmapCX = BITMAP_WITDH + 2;
	m_nBitmapCY = BITMAP_HEIGHT + 2;

	UT_return_if_fail( m_pMenuLabelSet );
	
	UT_Language l;
	if(l.getDirFromCode(m_pMenuLabelSet->getLanguage()) == UTLANG_RTL)
		m_iDIR = DT_RTLREADING;
}

void	EV_Win32Menu::destroy()
{		

	UT_VECTOR_PURGEALL(EV_Menu_Item*,m_vecItems);

	if (m_myMenu)
	{
		DestroyMenu(m_myMenu);
		m_myMenu = 0;
	}

	if (m_hFont)
		DeleteObject(m_hFont);
	
}

EV_Win32Menu::~EV_Win32Menu()
{	
	// we let the derived classes handle destruction of m_myMenu if appropriate.
	// TODO: this is never colld
	
	for (std::vector<HBITMAP>::const_iterator i = m_vechBitmaps.begin(); 
		 i != m_vechBitmaps.end(); i++)
	{	    
	    DeleteObject (*i);
	}
		
}

bool EV_Win32Menu::onCommand(AV_View * pView,
							 HWND /*hWnd*/, WPARAM wParam)
{
	// TODO do we need the hWnd parameter....

	// map the windows WM_COMMAND command-id into one of our XAP_Menu_Id.
	// we don't need to range check it, getAction() will puke if it's
	// out of range.
	
	XAP_Menu_Id id = MenuIdFromWmCommand(LOWORD(wParam));

	// user selected something from the menu.
	// invoke the appropriate function.
	// return true iff handled.

	const EV_Menu_ActionSet * pMenuActionSet = m_pWin32App->getMenuActionSet();
	UT_return_val_if_fail(pMenuActionSet, false);

	const EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
	if (!pAction)
		return false;

	const char * szMethodName = pAction->getMethodName();
	UT_return_val_if_fail(szMethodName, false);
	
	const EV_EditMethodContainer * pEMC = m_pWin32App->getEditMethodContainer();
	UT_return_val_if_fail(pEMC, false);

	EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
	UT_ASSERT(pEM);						// make sure it's bound to something

	UT_String script_name(pAction->getScriptName());
	invokeMenuMethod(pView, pEM, script_name);
	return true;
}

/*

	Create a Win32 menu from the info provided.

*/
bool EV_Win32Menu::synthesizeMenu(XAP_Frame * pFrame, HMENU menuRoot)
{	
	bool bResult;
	
	const EV_Menu_ActionSet * pMenuActionSet = m_pWin32App->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);
	
	const UT_uint32 nrLabelItemsInLayout = m_pMenuLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	// we keep a stack of the submenus so that we can properly
	// parent the menu items and deal with nested pull-rights.
	
	UT_Stack stack;
	stack.push(menuRoot);
	//UT_DEBUGMSG(("Menu::synthesize [menuRoot 0x%08lx]\n",menuRoot));
	
	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		
		EV_Menu_LayoutItem * pLayoutItem = m_pMenuLayout->getLayoutItem(k);
		UT_continue_if_fail(pLayoutItem);

		XAP_Menu_Id id = pLayoutItem->getMenuId();
		const EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
		UT_ASSERT(pAction);
		EV_Menu_Label * pLabel = m_pMenuLabelSet->getLabel(id);
		UT_ASSERT(pLabel);

		// get the name for the menu item

		const char * szLabelName = _ev_GetLabelName(m_pWin32App,pFrame,m_pEEM,pAction,pLabel);
		
		switch (pLayoutItem->getMenuLayoutFlags())
		{
		case EV_MLF_BeginSubMenu:
			UT_ASSERT(szLabelName && *szLabelName);
			// Fall Thru Intended

		case EV_MLF_Normal:
			{
				UT_DEBUGMSG(("menu::synthesize [name %s]\n",szLabelName));

				HMENU m;
				bResult = stack.viewTop((void **)&m);
				UT_ASSERT(bResult);

				// set standard flags on the item, we'll update the
				// state on an onInitMenu().
				// map our XAP_Menu_Id into a windows WM_COMMAND id.
				
				UINT flags = MF_STRING | MF_ENABLED | MF_UNCHECKED;
				UINT u = WmCommandFromMenuId(id);

				/*
					This a menu bar
				*/
				if (pLayoutItem->getMenuLayoutFlags() == EV_MLF_BeginSubMenu)
				{
					HMENU sub = CreateMenu();	// TODO NOTE: Leaking handle!
					UT_ASSERT(sub);

					UT_DEBUGMSG(("menu::synthesize [name %s][subMenu 0x%08lx] %u\n",szLabelName,u, stack.getDepth()));

					flags |= MF_POPUP;
					stack.push(sub);
					u = (UINT) sub;					
					
					if (szLabelName && *szLabelName)					
					{
						EV_Menu_Item*	item = new EV_Menu_Item;
						item->id = id;					
						item->pMenu= this;							
						
						strcpy (item->szText, szLabelName);					
						m_vecItems.addItem(item);
							
						if (!m_bTrack && stack.getDepth()==2)
							AppendMenu(m, flags,u, szLabelName);
						else {
							if (UT_IsWinVista()) {
								AppendMenu(m, flags|MF_STRING,u, szLabelName);
								_setBitmapforID(m, id, u);
							}
							else
								AppendMenu(m, flags|MF_OWNERDRAW,u, (const char*) item);
						}
					}		
						
				}
				else
				{					
					if (szLabelName && *szLabelName)
					{		  
						EV_Menu_Item*	item = new EV_Menu_Item;
						item->id = id;
						item->pMenu= this;						
						strcpy (item->szText, szLabelName);
						m_vecItems.addItem(item);
						
						if (UT_IsWinVista()) {
							AppendMenu(m, MF_STRING, u, szLabelName);
							_setBitmapforID (m, id, u);
						}
						else
							AppendMenu(m, MF_OWNERDRAW, u, (const char*) item);
					}
				}

				
			}
			break;
	
		case EV_MLF_EndSubMenu:
			{				
				HMENU m = NULL;
				bResult = stack.pop((void **)&m);
				UT_ASSERT(bResult);
				UT_DEBUGMSG(("menu::synthesize [endSubMenu 0x%08lx]\n",m));
			}
			break;
			
		case EV_MLF_Separator:
			{
				HMENU m;
				bResult = stack.viewTop((void **)&m);
				UT_ASSERT(bResult);
				UT_ASSERT(m);

				AppendMenu(m, MF_SEPARATOR, 0, NULL);
				UT_DEBUGMSG(("menu::synthesize [separator appended to submenu 0x%08lx]\n",m));
			}
			break;

		case EV_MLF_BeginPopupMenu:
		case EV_MLF_EndPopupMenu:
			break;

		default:
			UT_ASSERT_HARMLESS(0);
			break;
		}
	}

#ifdef DEBUG
	HMENU wDbg = NULL;
	bResult = stack.pop((void **)&wDbg);
	UT_ASSERT(bResult);
	UT_ASSERT(wDbg == menuRoot);
#endif

	return true;
}

bool EV_Win32Menu::onInitMenu(XAP_Frame * pFrame, AV_View * pView, HWND /*hWnd*/, HMENU hMenuBar)
{
	// deal with WM_INITMENU.

	if (hMenuBar != m_myMenu)			// these are not different when they
		return false;				// right-click on us on the menu bar.
	
	const EV_Menu_ActionSet * pMenuActionSet = m_pWin32App->getMenuActionSet();
	UT_uint32 nrLabelItemsInLayout = m_pMenuLayout->getLayoutItemCount();
	

	UT_uint32 pos = 0;
	bool bResult;
	UT_Stack stackPos;
	stackPos.push((void*)pos);
	UT_Stack stackMenu;
	stackMenu.push(hMenuBar);

	HMENU mTemp;
	HMENU m = hMenuBar;
	//UT_DEBUGMSG(("menu::onInitMenu: [menubar 0x%08lx]\n",m));

	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Menu_LayoutItem * pLayoutItem = m_pMenuLayout->getLayoutItem(k);
		XAP_Menu_Id id = pLayoutItem->getMenuId();
		const EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
		EV_Menu_Label * pLabel = m_pMenuLabelSet->getLabel(id);

		UINT cmd = WmCommandFromMenuId(id);

		switch (pLayoutItem->getMenuLayoutFlags())
		{
		case EV_MLF_Normal:
			{
				// see if we need to enable/disable and/or check/uncheck it.
				
				UINT uEnable = MF_BYCOMMAND | MF_ENABLED;
				UINT uCheck = MF_BYCOMMAND | MF_UNCHECKED;
				UINT uBold = 0;
				if (pAction->hasGetStateFunction())
				{
 					EV_Menu_ItemState mis = pAction->getMenuItemState(pView);
					if (mis & EV_MIS_Gray)
						uEnable |= MF_GRAYED;
					if (mis & EV_MIS_Toggled)
						uCheck |= MF_CHECKED;
					if (mis & EV_MIS_Bold)
						uBold = MFS_DEFAULT;
				}

				if (!pAction->hasDynamicLabel())
				{
					// if no dynamic label, all we need to do
					// is enable/disable and/or check/uncheck it.
					pos++;

					// need to check before disabling ...
					CheckMenuItem(hMenuBar,cmd,uCheck);
					EnableMenuItem(hMenuBar,cmd,uEnable);
					break;
				}

				// get the current menu info for this item.
				
				MENUITEMINFO mif;
				char bufMIF[128];
				mif.cbSize = sizeof(mif);
				mif.dwTypeData = bufMIF;
				mif.cch = G_N_ELEMENTS(bufMIF)-1;
				mif.fMask = MIIM_STATE | MIIM_TYPE | MIIM_ID;
				BOOL bPresent = GetMenuItemInfo(hMenuBar,cmd,FALSE,&mif);

				// this item has a dynamic label...
				// compute the value for the label.
				// if it is blank, we remove the item from the menu.

				const char * szLabelName = _ev_GetLabelName(m_pWin32App,pFrame,m_pEEM,pAction,pLabel);

				BOOL bRemoveIt = (!szLabelName || !*szLabelName);

				if (bRemoveIt)			// we don't want it to be there
				{
					if (bPresent)
						RemoveMenu(hMenuBar,cmd,MF_BYCOMMAND);
					
					break;
				}

				// we want the item in the menu.
				pos++;			

				if (bPresent)			// just update the label on the item.
				{					
					// dynamic label has changed
					XAP_Menu_Id id = MenuIdFromWmCommand(cmd);
					EV_Menu_Item*	item;
					for(UT_sint32 i = 0; i< m_vecItems.getItemCount(); i++)
					{
						item = (EV_Menu_Item*)m_vecItems.getNthItem(i);
						if (id==item->id)
						{
							strcpy (item->szText, szLabelName);					
							//UT_DEBUGMSG(("Menu changing text->%s\n",szLabelName));
							if (UT_IsWinVista()) 
							{
								mif.fState = uCheck | uEnable | uBold;
								mif.fType = MFT_STRING;
								mif.dwTypeData = (LPTSTR)szLabelName;
								SetMenuItemInfo (hMenuBar,cmd,FALSE,&mif);
							}
							break;
						}
					}
				}
				else	
				{
					EV_Menu_Item*	item = new EV_Menu_Item;
					item->id = id;					
					item->pMenu= this;													
					strcpy (item->szText, szLabelName);					
					m_vecItems.addItem(item);
					//UT_DEBUGMSG(("Menu adding menu->%s\n",szLabelName));
										
					if (UT_IsWinVista()) {
						AppendMenu(m, MF_STRING, cmd, szLabelName);
						_setBitmapforID(m, id, cmd);
					}
					else
						AppendMenu(m, MF_OWNERDRAW,cmd, (const char*) item);
				}
				
				EV_Menu_ItemState mis = pAction->getMenuItemState(pView);
				
				if (mis & EV_MIS_Gray)
						uEnable |= MF_GRAYED;
						
				if (mis & EV_MIS_Toggled)
						uCheck |= MF_CHECKED;
						
				EnableMenuItem(m,cmd,uEnable);
				CheckMenuItem(m,cmd,uCheck);
				
			}
			break;
	
		case EV_MLF_Separator:
			pos++;
			break;

		case EV_MLF_BeginSubMenu:
			mTemp = m;
			pos++;
			stackMenu.push(mTemp);
			stackPos.push((void*)pos);

			m = GetSubMenu(mTemp, pos-1);
			//UT_DEBUGMSG(("menu::onInitMenu: [menu 0x%08lx] at [pos %d] has [submenu 0x%08lx]\n",mTemp,pos-1,m));
			//UT_ASSERT(m);
			pos = 0;
			break;

		case EV_MLF_EndSubMenu:
			bResult = stackMenu.pop((void **)&mTemp);
			UT_ASSERT(bResult);
			bResult = stackPos.pop((void **)&pos);
			UT_ASSERT(bResult);

			//UT_DEBUGMSG(("menu::onInitMenu: endSubMenu [popping to menu 0x%08lx pos %d] from 0x%08lx\n",mTemp,pos,m));
			m = mTemp;
			break;

		case EV_MLF_BeginPopupMenu:
		case EV_MLF_EndPopupMenu:
			break;
			
		default:
			UT_ASSERT_HARMLESS(0);
			break;
		}
	}

#ifdef DEBUG
	HMENU wDbg = NULL;
	bResult = stackMenu.pop((void **)&wDbg);
	UT_ASSERT(bResult);
	UT_ASSERT(wDbg == hMenuBar);
#endif
		
 	return true;
}

/*
	Caller should DeleteObject the handle returned	
*/
HBITMAP EV_Win32Menu::_loadBitmap(XAP_Menu_Id id, int width, int height, UT_RGBColor color)
{
	
	HBITMAP hBitmap = NULL;
	EV_Menu_Bitmap* pBitmaps = (EV_Menu_Bitmap*)&s_bitmaps;		
							 
	for (; pBitmaps->id;pBitmaps++)
	{							
		if (pBitmaps->id==id)
			break;	
	}

	if (!pBitmaps->id) return hBitmap;	

	AP_Win32Toolbar_Icons::getBitmapForIcon(GetDesktopWindow(), width, height, &color, pBitmaps->szName,	&hBitmap);					
	
	return hBitmap; 
}

// Sets the Bitmap in Windows Vista
void EV_Win32Menu::_setBitmapforID (HMENU hMenu, XAP_Menu_Id id, UINT cmd)
{
	DWORD dwColor = GetSysColor (COLOR_MENU);
	UT_RGBColor Color(GetRValue(dwColor),GetGValue(dwColor),GetBValue(dwColor));
	HBITMAP hBitmap =  EV_Win32Menu::_loadBitmap(id, BITMAP_WITDH, BITMAP_HEIGHT, Color);
							
	if (hBitmap != NULL) {
		SetMenuItemBitmaps (hMenu, cmd, MF_BYCOMMAND, hBitmap, hBitmap);		
		m_vechBitmaps.push_back (hBitmap);
	}
}

/*
	It tells us which are menu bars
*/
bool EV_Win32Menu::_isAMenuBar(XAP_Menu_Id id, HMENU hMenu)
{
	XAP_Menu_Id ids[] = { AP_MENU_ID_FILE, AP_MENU_ID_EDIT, AP_MENU_ID_VIEW,AP_MENU_ID_INSERT,
		AP_MENU_ID_TOOLS, AP_MENU_ID_WINDOW, AP_MENU_ID_HELP,AP_MENU_ID_TABLE, AP_MENU_ID_FORMAT, NULL};

	for (UT_uint32 i=0; i<(sizeof(ids)/sizeof(XAP_Menu_Id)); i++)
	{
		if (ids[i]==id)
		{		
			
			MENUITEMINFO menuInfo;	 
			memset (&menuInfo, 0, sizeof(MENUITEMINFO));
			menuInfo.cbSize = sizeof(MENUITEMINFO);
			menuInfo.fMask = MIIM_DATA;
			GetMenuItemInfo(hMenu, 0, TRUE, &menuInfo);		
			EV_Menu_Item*	item = (EV_Menu_Item *) menuInfo.dwItemData;            			           				

			if (item && id==item->id)		
			{
				UT_DEBUGMSG(("EV_Win32Menu::_isAMenuBar->%s, 1\n", item->szText));
				return true;
			}
		}
	}

	UT_DEBUGMSG(("EV_Win32Menu::_isAMenuBar-> 0\n"));
	return false;
}



/*
	Process message WM_MEASUREITEM
*/
void EV_Win32Menu::onMeasureItem(HWND hwnd, WPARAM /*wParam*/, LPARAM lParam)
{
	SIZE size;
	LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT) lParam; 
	HDC hdc = GetDC(hwnd); 
	
	EV_Menu_Item*	item = (EV_Menu_Item *) lpmis->itemData;            	
	HFONT hfontOld = (HFONT) SelectObject(hdc, m_hFont); 

	// Retrieve the width and height of the item's string 
	GetTextExtentPoint32(hdc, item->szText, lstrlen(item->szText), &size); 
		
	if (size.cy < item->pMenu->m_nBitmapCY)
		lpmis->itemHeight = item->pMenu->m_nBitmapCY;
	else
		lpmis->itemHeight = size.cy;

	lpmis->itemWidth =  size.cx + item->pMenu->m_nBitmapCX + SPACE_ICONTEXT;

	SelectObject(hdc, hfontOld); 
	ReleaseDC(hwnd, hdc); 
}

/*
	Process message  WM_MENUCHAR. Process the hotkey messages	
*/
LPARAM EV_Win32Menu::onMenuChar(HWND /*hwnd*/, WPARAM wParam, LPARAM lParam)
{	
	HMENU hMenu = (HMENU) lParam;
	MENUITEMINFO	menuInfo;
	int nItems = GetMenuItemCount(hMenu);
	char szBuff[1024];
	
	for (int i=0; i<nItems; i++)
	{					
		memset (&menuInfo, 0, sizeof(MENUITEMINFO));
		menuInfo.cbSize = sizeof(MENUITEMINFO);
		menuInfo.fMask = MIIM_DATA;

		GetMenuItemInfo(hMenu, i, TRUE, &menuInfo);		

		EV_Menu_Item*	item = (EV_Menu_Item *) menuInfo.dwItemData;            			           	

		if (item)
		{
			strcpy (szBuff, item->szText);
			strlwr(szBuff);			

			char* pHotKeyPos = strchr (szBuff, '&');
				
			if (pHotKeyPos)
			{								
				char n = (char)wParam & 0x000000ff;

				pHotKeyPos++;

				if (toupper(*pHotKeyPos)==toupper(n))			
					return MAKELRESULT(i, MNC_EXECUTE);														 
				
			}
		}			
	}

	return MNC_IGNORE;
}

/*
	Process message  WM_DRAWITEM
*/
void EV_Win32Menu::onDrawItem(HWND /*hwnd*/, WPARAM /*wParam*/, LPARAM lParam)
{

	LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam;  
	EV_Menu_Item*	item = (EV_Menu_Item *) lpdis->itemData;            			           	
    COLORREF crText;    
    COLORREF crBkgnd;   
	DWORD dwColor;
	RECT rect;
	UT_String sTextRight, sTextLeft;
	HBITMAP hBitmap;	
	int colorID;
	UT_ASSERT(lpdis->CtlType==ODT_MENU); 						

	/* Rect to draw the text */	
	rect.top =  lpdis->rcItem.top;
	rect.bottom = lpdis->rcItem.bottom;

	if(m_iDIR)
	{
		rect.right = lpdis->rcItem.right - 20;
		rect.left = lpdis->rcItem.left;
	}
	else
	{
		rect.right = lpdis->rcItem.right;
		rect.left = /*m_nBitmapCX*/ 20  + lpdis->rcItem.left;
	}
	
	
	if (lpdis->itemState & ODS_GRAYED) 
		crText = SetTextColor(lpdis->hDC, GetSysColor(COLOR_GRAYTEXT));	
	else
		if (lpdis->itemState & ODS_SELECTED)
			crText = SetTextColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));		
		else
			crText = SetTextColor(lpdis->hDC, GetSysColor(COLOR_MENUTEXT));			

	if (lpdis->itemState & ODS_SELECTED)     
		colorID = COLOR_HIGHLIGHT;
	else
		colorID = COLOR_MENU;		
	
	dwColor = GetSysColor(colorID);
	
	UT_RGBColor Color(GetRValue(dwColor),GetGValue(dwColor),GetBValue(dwColor));
	hBitmap =  EV_Win32Menu::_loadBitmap(item->id, BITMAP_WITDH, BITMAP_HEIGHT, Color);

	if (hBitmap)	
	{
		BITMAP bitmap;		
		GetObjectA(hBitmap, sizeof(BITMAP), &bitmap);	
	}	

	/*Draw the background of the item*/
	FillRect(lpdis->hDC, &lpdis->rcItem, GetSysColorBrush(colorID));

	if(m_iDIR)
		rect.right = rect.right - SPACE_ICONTEXT;
	else
		rect.left = rect.left;
		
	
	// Select the font associated with the item into the 
    // item's device context, and then draw the string.  
    HFONT hfontOld = (HFONT) SelectObject(lpdis->hDC, item->pMenu->m_hFont); 

	
	/* 
		Process tabs
	*/	
	char* pTabPos = strchr (item->szText, '\t');

	if (pTabPos)
	{
		char szTmp[255];
		char* pTmp;
		
		strncpy (szTmp, item->szText, pTabPos-item->szText);
		pTmp = szTmp; pTmp+=pTabPos-item->szText; *pTmp=NULL;
		sTextLeft = szTmp;
		
		strcpy (szTmp, pTabPos+1);
		sTextRight = szTmp;
		sTextRight +="  ";
	}
	else
		sTextLeft = item->szText;		
	
	if (lpdis->itemState & ODS_SELECTED) 
		crBkgnd = SetBkColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHT));

	/* Draw text*/
	if(m_iDIR)
	{
		DrawText(lpdis->hDC, sTextLeft.c_str(),  sTextLeft.length() , &rect,
				 DT_RIGHT | DT_VCENTER | DT_SINGLELINE | m_iDIR);
		
		if (sTextRight.length())
			DrawText(lpdis->hDC, sTextRight.c_str(), sTextRight.length(), &rect,
					 DT_LEFT | DT_VCENTER | DT_SINGLELINE | m_iDIR);
	}
	else
	{
		DrawText(lpdis->hDC, sTextLeft.c_str(),  sTextLeft.length() , &rect,
				 DT_LEFT | DT_VCENTER | DT_SINGLELINE | m_iDIR);
		
		if (sTextRight.length())
			DrawText(lpdis->hDC, sTextRight.c_str(), sTextRight.length(), &rect,
					 DT_RIGHT | DT_VCENTER | DT_SINGLELINE | m_iDIR);
	}
	
	if (lpdis->itemState & ODS_SELECTED) 
		SetBkColor(lpdis->hDC, crBkgnd); 
	
	/* Draw bitmap*/
	if (hBitmap)
	{	
					  		
		HDC hdcMem = CreateCompatibleDC(lpdis->hDC);
		SelectObject(hdcMem,(void *)hBitmap);				

		if(m_iDIR)
			BitBlt(lpdis->hDC, lpdis->rcItem.right - 20, lpdis->rcItem.top+1, BITMAP_WITDH, BITMAP_HEIGHT, hdcMem, 0, 0, SRCCOPY );
		else
			BitBlt(lpdis->hDC, lpdis->rcItem.left+1, lpdis->rcItem.top+1, BITMAP_WITDH, BITMAP_HEIGHT, hdcMem, 0, 0, SRCCOPY );
		
		DeleteDC(hdcMem);					
				
	}
	else 
		if (lpdis->itemState & ODS_CHECKED)
		{	
			UINT nWidth = GetSystemMetrics(SM_CXMENUCHECK);
			UINT nHeight = GetSystemMetrics(SM_CYMENUCHECK);
			RECT r;
			HBITMAP bm = CreateBitmap(nWidth, nHeight, 1, 1, NULL );
			HDC hdcMem = CreateCompatibleDC(lpdis->hDC);

			SetBkColor(lpdis->hDC, GetSysColor(colorID));
			SelectObject(hdcMem, bm);
			SetRect(&r, 0, 0, nWidth, nHeight);
			DrawFrameControl(hdcMem, &r, DFC_MENU,DFCS_MENUCHECK);			

			if(m_iDIR)
				BitBlt(lpdis->hDC, lpdis->rcItem.right - 20, lpdis->rcItem.top+2, nWidth, nHeight,
					   hdcMem, 0, 0, SRCCOPY);
			else
				BitBlt(lpdis->hDC, lpdis->rcItem.left+2, lpdis->rcItem.top+2, nWidth, nHeight,
					   hdcMem, 0, 0, SRCCOPY);
				
			DeleteDC(hdcMem);
			DeleteObject(bm);
		}	

    SelectObject(lpdis->hDC, hfontOld); 
    SetTextColor(lpdis->hDC, crText); 	
	if (hBitmap)
		DeleteObject(hBitmap);	
}

								
bool EV_Win32Menu::onMenuSelect(XAP_Frame * pFrame, AV_View * /*pView*/,
								   HWND /*hWnd*/, HMENU hMenu, WPARAM wParam)
{
	UINT nItemID = (UINT)LOWORD(wParam);
	UINT nFlags  = (UINT)HIWORD(wParam);

	if ( (nFlags==0xffff) && (hMenu==0) )
	{
		//UT_DEBUGMSG(("ClearMessage 1\n"));
		pFrame->setStatusMessage(NULL);
		return true;
	}

	if ( (nItemID==0) || (nFlags & (MF_SEPARATOR|MF_POPUP)) )
	{
		//UT_DEBUGMSG(("ClearMessage 2\n"));
		pFrame->setStatusMessage(NULL);
		return true;
	}

	if (nFlags & (MF_SYSMENU))
	{
		//UT_DEBUGMSG(("SysMenu [%x]\n",nItemID));
		pFrame->setStatusMessage(NULL);
		return true;
	}
	
	XAP_Menu_Id id = MenuIdFromWmCommand(nItemID);
	EV_Menu_Label * pLabel = m_pMenuLabelSet->getLabel(id);
	if (!pLabel)
	{
		//UT_DEBUGMSG(("ClearMessage 3 [%d %d]\n",nItemID,id));
		pFrame->setStatusMessage(NULL);
		return true;
	}

	const char * szMsg = pLabel->getMenuStatusMessage();
	if (!szMsg || !*szMsg)
		szMsg = "TODO This menu item doesn't have a StatusMessage defined.";
	
	UT_String str = AP_Win32App::s_fromUTF8ToWinLocale(szMsg);
	pFrame->setStatusMessage(str.c_str());
	return true;
}



/*****************************************************************/
/*****************************************************************/

EV_Win32MenuBar::EV_Win32MenuBar(XAP_Win32App * pWin32App,
								 const EV_EditEventMapper * pEEM,
								 const char * szMenuLayoutName,
								 const char * szMenuLabelSetName)
	: EV_Win32Menu(pWin32App,pEEM,szMenuLayoutName,szMenuLabelSetName)
{
}

EV_Win32MenuBar::~EV_Win32MenuBar(void)
{
	destroy();
	// TODO should we destroy m_myMenu if set ??
}

bool EV_Win32MenuBar::synthesizeMenuBar(XAP_Frame * pFrame)
{
	m_myMenu = CreateMenu();

	bool bRet = synthesizeMenu(pFrame, m_myMenu);

	// when dealing with RTL interface language, we need to tell do some tricks here to
	// make the menubar to layout RTL
	// see http://www.microsoft.com/middleeast/msdn/faq.aspx
	MENUITEMINFO mii;
	char buff[81];
	memset(buff, 0, 81);
	
	mii.cbSize = sizeof(mii);
	mii.dwTypeData = buff;
	mii.fType = MF_STRING;
	mii.cch = 80;
	mii.fState = MFS_DEFAULT;
	mii.fMask = MIIM_ID | MIIM_DATA | MIIM_TYPE | MIIM_SUBMENU;
	
	if(m_iDIR && GetMenuItemInfo(m_myMenu, 0, 1, &mii))
	{
		mii.fType |= MFT_RIGHTORDER;
		SetMenuItemInfo(m_myMenu, 0, 1, &mii);
	}
	
	return bRet;
}

/*****************************************************************/
/*****************************************************************/

EV_Win32MenuPopup::EV_Win32MenuPopup(XAP_Win32App * pWin32App,
									 const char * szMenuLayoutName,
									 const char * szMenuLabelSetName)
	: EV_Win32Menu(pWin32App,NULL,szMenuLayoutName,szMenuLabelSetName)
{
}

EV_Win32MenuPopup::~EV_Win32MenuPopup(void)
{
	destroy();
	
	if (m_myMenu)
		DestroyMenu(m_myMenu);
}

bool EV_Win32MenuPopup::synthesizeMenuPopup(XAP_Frame * pFrame)
{
	m_myMenu = CreatePopupMenu();

	bool bRet = synthesizeMenu(pFrame, m_myMenu);

	// when dealing with RTL interface language, we need to tell do some tricks here to
	// make the menubar to layout RTL
	// see http://www.microsoft.com/middleeast/msdn/faq.aspx
	MENUITEMINFO mii;
	char buff[81];
	memset(buff,80,' ');
	buff[80] = 0;
	
	mii.cbSize = sizeof(mii);
	mii.dwTypeData = buff;
	mii.fType = MF_STRING;
	mii.cch = 80;
	mii.fState = MFS_DEFAULT;
	mii.fMask = MIIM_ID | MIIM_DATA | MIIM_TYPE | MIIM_SUBMENU;
	
	if(m_iDIR && GetMenuItemInfo(m_myMenu, 0, 1, &mii))
	{
		mii.fType |= MFT_RIGHTORDER;
		SetMenuItemInfo(m_myMenu, 0, 1, &mii);
	}
	
	return bRet;
	
}

