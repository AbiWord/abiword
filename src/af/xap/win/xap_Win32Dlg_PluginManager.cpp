/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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
#include <commctrl.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_Win32OS.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"

#include "xap_Dialog_Id.h"
#include "xap_Win32Dlg_FileOpenSaveAs.h"
#include "xap_Win32Dlg_PluginManager.h"
#include "xap_Win32DialogHelper.h"
#include "xap_Module.h"
#include "xap_ModuleManager.h"

#include "ie_types.h"
#include "ut_string_class.h"

#include "xap_Win32Resources.rc2"

/*****************************************************************/

XAP_Dialog * XAP_Win32Dialog_PluginManager::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_Win32Dialog_PluginManager * p = new XAP_Win32Dialog_PluginManager(pFactory,id);
	return p;
}

XAP_Win32Dialog_PluginManager::XAP_Win32Dialog_PluginManager(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: XAP_Dialog_PluginManager(pDlgFactory,id),
	m_curSelection(LB_ERR)
{
}

XAP_Win32Dialog_PluginManager::~XAP_Win32Dialog_PluginManager(void)
{
}

void XAP_Win32Dialog_PluginManager::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	m_pFrame = pFrame;

	// raise the dialog
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);

	LPCTSTR lpTemplate = NULL;

	UT_ASSERT(m_id == XAP_DIALOG_ID_PLUGIN_MANAGER);

	lpTemplate = MAKEINTRESOURCE(XAP_RID_DIALOG_PLUGIN_MANAGER);

	int result = DialogBoxParam(pWin32App->getInstance(),lpTemplate,
						static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow(),
						(DLGPROC)s_dlgProc,(LPARAM)this);
	UT_ASSERT((result != -1));
}

/*****************************************************************/

#define GWL(hwnd)		(XAP_Win32Dialog_PluginManager*)GetWindowLong((hwnd), DWL_USER)
#define SWL(hwnd, d)	(XAP_Win32Dialog_PluginManager*)SetWindowLong((hwnd), DWL_USER,(LONG)(d))

/*****************************************************************/

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

BOOL CALLBACK XAP_Win32Dialog_PluginManager::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.
	// This is the dialog procedure for the top-level dialog (that contains
	// the Close button and the Tab-control).

	XAP_Win32Dialog_PluginManager * pThis;
	
	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (XAP_Win32Dialog_PluginManager *)lParam;
		SWL(hWnd,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);
		
	case WM_COMMAND:
		pThis = GWL(hWnd);
		return pThis->_onCommand(hWnd,wParam,lParam);
		
	case WM_NOTIFY:
		pThis = GWL(hWnd);
		return pThis->_onNotify(hWnd,lParam);
		
	default:
		return 0;
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
// As Tabbed Dialogs have problems with HotKeys, this macro have been replaced to remove &
//#define _DSX(c,s)	SetDlgItemText(hWnd,XAP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))
#define _DSX(c,s) { \
                    XML_Char* p = NULL; \
                    UT_XML_cloneNoAmpersands( p, pSS->getValue(XAP_STRING_ID_##s));\
                    SetDlgItemText(hWnd,XAP_RID_DIALOG_##c,p); \
					FREEP(p); \
                  }
#define _GVX(s)		(pSS->getValue(XAP_STRING_ID_##s))

// the order of the tabs

#define LIST_INDEX		0
#define DETAILS_INDEX	1

// this little struct gets passed into s_tabProc
// it's on the stack so don't rely on it to be valid later.
typedef struct _tabParam 
{
	XAP_Win32Dialog_PluginManager*	pThis;
	WORD which;
} TabParam;


BOOL XAP_Win32Dialog_PluginManager::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// This handles the WM_INITDIALOG message for the top-level dialog.
	
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	SetWindowText(hWnd, pSS->getValue(XAP_STRING_ID_DLG_PLUGIN_MANAGER_TITLE));

	// localize controls
	_DSX(PLUGIN_MANAGER_BTN_CLOSE, DLG_Close);

	// setup the tabs
	{
		TabParam tp;
		TCITEM tie; 

		XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);
		HINSTANCE hinst = pWin32App->getInstance();
		DLGTEMPLATE * pTemplate = NULL;
		HWND w = NULL;

		tp.pThis = this;

		// remember the windows we're using 
		m_hwndDlg = hWnd;
		m_hwndTab = GetDlgItem(hWnd, XAP_RID_DIALOG_PLUGIN_MANAGER_TAB);

		// add a tab for each of the child dialog boxes
    
		tie.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM; 
		tie.iImage = -1; 

		tie.pszText = (LPSTR) _GVX(DLG_PLUGIN_MANAGER_LIST); 
		tie.lParam = XAP_RID_DIALOG_PLUGIN_MANAGER_LIST;
		TabCtrl_InsertItem(m_hwndTab, LIST_INDEX, &tie); 

		tie.pszText = (LPSTR) _GVX(DLG_PLUGIN_MANAGER_DETAILS); 
		tie.lParam = XAP_RID_DIALOG_PLUGIN_MANAGER_DETAILS;
		TabCtrl_InsertItem(m_hwndTab, DETAILS_INDEX, &tie); 

		// finally, create the (modeless) child dialogs
		
		tp.which = XAP_RID_DIALOG_PLUGIN_MANAGER_LIST;
		pTemplate = UT_LockDlgRes(hinst, MAKEINTRESOURCE(tp.which));
		w = CreateDialogIndirectParam(hinst, pTemplate, m_hwndTab, 
										(DLGPROC)s_tabProc, (LPARAM)&tp);
		UT_ASSERT((w
				   && (m_vecSubDlgHWnd.getItemCount()>0)
				   && (w == m_vecSubDlgHWnd.getLastItem())));

		tp.which = XAP_RID_DIALOG_PLUGIN_MANAGER_DETAILS;
		pTemplate = UT_LockDlgRes(hinst, MAKEINTRESOURCE(tp.which));
		w = CreateDialogIndirectParam(hinst, pTemplate, m_hwndTab, 
										(DLGPROC)s_tabProc, (LPARAM)&tp); 
		UT_ASSERT((w
				   && (m_vecSubDlgHWnd.getItemCount()>0)
				   && (w == m_vecSubDlgHWnd.getLastItem())));

	}

	// let XP code tell us what all of the values should be.
//	_populateWindowData();

	// This has to follow the call to _populateWindowData()
//	_initializeTransperentToggle();

	// make sure first tab is selected.
	ShowWindow((HWND)m_vecSubDlgHWnd.getNthItem(0), SW_SHOW);
	XAP_Win32DialogHelper::s_centerDialog(hWnd);	
	return 1;							// 1 == we did not call SetFocus()
}

BOOL XAP_Win32Dialog_PluginManager::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// This handles WM_COMMAND message for the top-level dialog.
	
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	case IDOK:
	case IDCANCEL:
	case XAP_RID_DIALOG_PLUGIN_MANAGER_BTN_CLOSE:		
		EndDialog(hWnd,0);
		return 0;

	default:									// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;								// return zero to let windows take care of it.
	}
}

BOOL XAP_Win32Dialog_PluginManager::_onNotify(HWND hWnd, LPARAM lParam)
{
	// This handles WM_NOTIFY messages for the top-level dialog.
	
	LPNMHDR pNmhdr = (LPNMHDR)lParam;

	switch (pNmhdr->code)
	{
	case TCN_SELCHANGING:
		// TODO: consider validating data before leaving page
		break;

	case TCN_SELCHANGE:
		{
			UT_uint32 iTo = TabCtrl_GetCurSel(pNmhdr->hwndFrom); 
			if( iTo == DETAILS_INDEX )
			{
				SetDlgItemText( (HWND)m_vecSubDlgHWnd.getNthItem(DETAILS_INDEX),
					            XAP_RID_DIALOG_PLUGIN_MANAGER_EBX_NAME,
								"" );
				SetDlgItemText( (HWND)m_vecSubDlgHWnd.getNthItem(DETAILS_INDEX),
					            XAP_RID_DIALOG_PLUGIN_MANAGER_EBX_DESCRIPTION, 
								"" );
				SetDlgItemText( (HWND)m_vecSubDlgHWnd.getNthItem(DETAILS_INDEX),
					            XAP_RID_DIALOG_PLUGIN_MANAGER_EBX_AUTHOR,      
								"" );
				SetDlgItemText( (HWND)m_vecSubDlgHWnd.getNthItem(DETAILS_INDEX),
					            XAP_RID_DIALOG_PLUGIN_MANAGER_EBX_VERSION,     
								"" );
				if( m_curSelection != LB_ERR )
				{
					XAP_Module* pModule = 0;
					pModule = (XAP_Module *) XAP_ModuleManager::instance().enumModules()->getNthItem(m_curSelection);
					if( pModule )
					{
						const XAP_ModuleInfo * mi = pModule->getModuleInfo ();
						if( mi )
						{
							SetDlgItemText( (HWND)m_vecSubDlgHWnd.getNthItem(DETAILS_INDEX),
								            XAP_RID_DIALOG_PLUGIN_MANAGER_EBX_NAME,        
											mi->name );
							SetDlgItemText( (HWND)m_vecSubDlgHWnd.getNthItem(DETAILS_INDEX),
											XAP_RID_DIALOG_PLUGIN_MANAGER_EBX_DESCRIPTION, 
											mi->desc );
							SetDlgItemText( (HWND)m_vecSubDlgHWnd.getNthItem(DETAILS_INDEX),
											XAP_RID_DIALOG_PLUGIN_MANAGER_EBX_AUTHOR,      
											mi->author );
							SetDlgItemText( (HWND)m_vecSubDlgHWnd.getNthItem(DETAILS_INDEX),
											XAP_RID_DIALOG_PLUGIN_MANAGER_EBX_VERSION,     
											mi->version );
						}
					}
				}

			}
			for (UT_uint32 k=0; k<m_vecSubDlgHWnd.getItemCount(); k++)
				ShowWindow((HWND)m_vecSubDlgHWnd.getNthItem(k), ((k==iTo) ? SW_SHOW : SW_HIDE));
			break;
		}
		
	// Process other notifications here
	default:
		break;
	} 

	return 0;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

BOOL CALLBACK XAP_Win32Dialog_PluginManager::s_tabProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.
	// This is a pseudo-dialog procedure for the tab-control.

	XAP_Win32Dialog_PluginManager * pThis;

	switch (msg)
	{
	case WM_INITDIALOG:
		{
			TabParam * pTP = (TabParam *) lParam;

			// from now on, we can just remember pThis 
			pThis = pTP->pThis;
			SWL(hWnd,pThis);
			return pThis->_onInitTab(hWnd,wParam,lParam);
		}
		
	case WM_COMMAND:
		pThis = GWL(hWnd);
		UT_DEBUGMSG(("s_tabProc: received WM_COMMAND [wParam 0x%08lx][lParam 0x%08lx]\n",wParam,lParam));
		return pThis->_onCommandTab(hWnd,wParam,lParam);

	default:
		return 0;
	}
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
BOOL XAP_Win32Dialog_PluginManager::_onInitTab(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// This handles the WM_INITDIALOG message for the tab-control

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// position ourselves w.r.t. containing tab

	RECT r;
	GetClientRect(m_hwndTab, &r);
	TabCtrl_AdjustRect(m_hwndTab, FALSE, &r);
    SetWindowPos(hWnd, HWND_TOP, r.left, r.top, 0, 0, SWP_NOSIZE); 

	m_vecSubDlgHWnd.addItem(hWnd);
	
	TabParam * pTP = (TabParam *) lParam;
	switch (pTP->which)
	{
	case XAP_RID_DIALOG_PLUGIN_MANAGER_LIST:
		{
			_DSX(PLUGIN_MANAGER_BTN_ACTIVE,			DLG_PLUGIN_MANAGER_ACTIVE);
			_DSX(PLUGIN_MANAGER_BTN_DEACTIVATE,		DLG_PLUGIN_MANAGER_DEACTIVATE);
			_DSX(PLUGIN_MANAGER_BTN_DEACTIVATEALL,	DLG_PLUGIN_MANAGER_DEACTIVATE_ALL);
			_DSX(PLUGIN_MANAGER_BTN_INSTALL,		DLG_PLUGIN_MANAGER_INSTALL);			

			refresh_Tab1();
		}
		break;
			
	case XAP_RID_DIALOG_PLUGIN_MANAGER_DETAILS:
		{
			// localize controls
			_DSX(PLUGIN_MANAGER_LBL_NAME,			DLG_PLUGIN_MANAGER_NAME);
			_DSX(PLUGIN_MANAGER_LBL_DESCRIPTION,	DLG_PLUGIN_MANAGER_DESC);
			_DSX(PLUGIN_MANAGER_LBL_AUTHOR,		 	DLG_PLUGIN_MANAGER_AUTHOR);
			_DSX(PLUGIN_MANAGER_LBL_VERSION,		DLG_PLUGIN_MANAGER_VERSION);
		}
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return 1;							// 1 == we did not call SetFocus()
}

BOOL XAP_Win32Dialog_PluginManager::_onCommandTab(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// This handles WM_COMMAND message for all of the sub-dialogs.
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	// LIST TAB
	case XAP_RID_DIALOG_PLUGIN_MANAGER_BTN_ACTIVE:	
		return 0;	

	case XAP_RID_DIALOG_PLUGIN_MANAGER_BTN_INSTALL:
		event_Load();
		return 0;

	case XAP_RID_DIALOG_PLUGIN_MANAGER_BTN_DEACTIVATE:
		if( m_curSelection != LB_ERR )
		{
			XAP_Module * pModule = 0;
			pModule = (XAP_Module *) XAP_ModuleManager::instance().enumModules()->getNthItem(m_curSelection);
			if (pModule)
			{
				if( deactivatePlugin(pModule) )
				{
					SendDlgItemMessage( hWnd,
                        				XAP_RID_DIALOG_PLUGIN_MANAGER_LBX_LIST,
                            			LB_DELETESTRING ,
										(WPARAM) m_curSelection,
										(LPARAM) 0 );
				}
			}
		}
		return 0;

	case XAP_RID_DIALOG_PLUGIN_MANAGER_BTN_DEACTIVATEALL:
		// Clear the List Box
		SendDlgItemMessage( hWnd,
                            XAP_RID_DIALOG_PLUGIN_MANAGER_LBX_LIST,
                            LB_RESETCONTENT,
							(WPARAM) 0,
							(LPARAM) 0 );
		deactivateAllPlugins();
		return 0;

	case XAP_RID_DIALOG_PLUGIN_MANAGER_LBX_LIST:
		m_curSelection = SendDlgItemMessage( hWnd,
                            				 XAP_RID_DIALOG_PLUGIN_MANAGER_LBX_LIST,
                            				 LB_GETCURSEL,
											 (WPARAM) 0,
											 (LPARAM) 0 );
		return 0;

	// DETAILS TAB	

		// No controls to work with at the moment

	default:
		UT_DEBUGMSG(("WM_Command for id %ld for sub-dialog\n",wId));
		return 0;
	}
}

void XAP_Win32Dialog_PluginManager::refresh_Tab1()
{
	// Clear List Box
	SendDlgItemMessage( (HWND)m_vecSubDlgHWnd.getNthItem(LIST_INDEX),
                        XAP_RID_DIALOG_PLUGIN_MANAGER_LBX_LIST,
                        LB_RESETCONTENT,
						(WPARAM) 0,
						(LPARAM) 0 );

	// Populate List Box
	XAP_Module* pModule = 0;
	const UT_Vector* pVec = XAP_ModuleManager::instance().enumModules();

	for (UT_uint32 i = 0; i < pVec->size(); i++)
	{
		pModule = (XAP_Module *)pVec->getNthItem (i);
		SendDlgItemMessage( (HWND)m_vecSubDlgHWnd.getNthItem(LIST_INDEX),
                            XAP_RID_DIALOG_PLUGIN_MANAGER_LBX_LIST,
                            LB_ADDSTRING,
							(WPARAM) 0,
							(LPARAM) pModule->getModuleInfo()->name );
	}
}

void XAP_Win32Dialog_PluginManager::event_Load()
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *) m_pFrame->getDialogFactory();
	
	XAP_Dialog_FileOpenSaveAs * pDialog
		= (XAP_Dialog_FileOpenSaveAs *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_FILE_OPEN));
	UT_ASSERT(pDialog);
	
	// set the intial plugin directory to the user-local plugin directory
	// could also set to: XAP_App::getApp()->getUserPrivateDirectory()\plugins
	// could also set to: XAP_App::getApp()->getAbiSuiteLibDir()\plugins
	UT_String pluginDir (XAP_App::getApp()->getAbiSuiteAppDir());
	pluginDir += "\\plugins";
	pDialog->setCurrentPathname (pluginDir.c_str());
	pDialog->setSuggestFilename(false);
	
	UT_uint32 filterCount = 1;
	const char ** szDescList = (const char **) UT_calloc(filterCount + 1,
													  sizeof(char *));
	const char ** szSuffixList = (const char **) UT_calloc(filterCount + 1,
														sizeof(char *));
	IEFileType * nTypeList = (IEFileType *) UT_calloc(filterCount + 1,
												   sizeof(IEFileType));
	
	// we probably shouldn't hardcode this
	// HP-UX uses .sl, for instance
	szDescList[0] = "AbiWord Plugin (.dll)";
	szSuffixList[0] = "*.dll";
	nTypeList[0] = (IEFileType)1;
	
	pDialog->setFileTypeList(szDescList, szSuffixList, 
							 (const UT_sint32 *) nTypeList);
	
	pDialog->setDefaultFileType((IEFileType)1);

	// todo: cd to the proper plugin directory
	
	pDialog->runModal(m_pFrame);
	
	XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);
	
	if (bOK)
	{
		const char * szResultPathname = pDialog->getPathname();
		if (szResultPathname && *szResultPathname)
		{
			if( activatePlugin(szResultPathname) )
			{
				// worked!
				refresh_Tab1();
			}
			else
			{
				// error message
				m_pFrame->showMessageBox( pSS->getValue(XAP_STRING_ID_DLG_PLUGIN_MANAGER_COULDNT_LOAD),
                                          XAP_Dialog_MessageBox::b_O,
                                          XAP_Dialog_MessageBox::a_OK );
			}
		}
	}
	
	FREEP(szDescList);
	FREEP(szSuffixList);
	FREEP(nTypeList);
	
	pDialogFactory->releaseDialog(pDialog);
}
