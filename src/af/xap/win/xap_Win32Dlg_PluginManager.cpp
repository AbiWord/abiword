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
		
	default:
		return 0;
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
// As Tabbed Dialogs have problems with HotKeys, this macro have been replaced to remove &
#define _DSX(c,s)   (SetDlgItemText(hWnd,XAP_RID_DIALOG_##c, pSS->getValue(XAP_STRING_ID_##s)))
                  
//#define _GVX(s)		(pSS->getValue(XAP_STRING_ID_##s))



// This handles the WM_INITDIALOG message for the top-level dialog.
BOOL XAP_Win32Dialog_PluginManager::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{		
	m_hwndDlg = hWnd;
	
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	SetWindowText(hWnd, pSS->getValue(XAP_STRING_ID_DLG_PLUGIN_MANAGER_TITLE));

	/* Localise controls */	
	_DSX(PLUGIN_MANAGER_BTN_OK,				DLG_OK);
	_DSX(PLUGIN_MANAGER_BTN_ACTIVE,			DLG_PLUGIN_MANAGER_ACTIVE);
	_DSX(PLUGIN_MANAGER_BTN_DEACTIVATE,		DLG_PLUGIN_MANAGER_DEACTIVATE);
	_DSX(PLUGIN_MANAGER_BTN_DEACTIVATEALL,	DLG_PLUGIN_MANAGER_DEACTIVATE_ALL);
	_DSX(PLUGIN_MANAGER_BTN_INSTALL,		DLG_PLUGIN_MANAGER_INSTALL);			

	_DSX(PLUGIN_MANAGER_LBL_NAME,			DLG_PLUGIN_MANAGER_NAME);
	_DSX(PLUGIN_MANAGER_LBL_DESCRIPTION,	DLG_PLUGIN_MANAGER_DESC);
	_DSX(PLUGIN_MANAGER_LBL_AUTHOR,		 	DLG_PLUGIN_MANAGER_AUTHOR);
	_DSX(PLUGIN_MANAGER_LBL_VERSION,		DLG_PLUGIN_MANAGER_VERSION);

	refreshPluginList();	

	/* Default */
	SendDlgItemMessage(m_hwndDlg, XAP_RID_DIALOG_PLUGIN_MANAGER_LBX_LIST, LB_SETCURSEL, 0, 0);
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
										
					refreshPluginInfo();										
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
		refreshPluginInfo();
		return 0;

	case XAP_RID_DIALOG_PLUGIN_MANAGER_LBX_LIST:
	{
		m_curSelection = SendDlgItemMessage( hWnd,
                            				 XAP_RID_DIALOG_PLUGIN_MANAGER_LBX_LIST,
                            				 LB_GETCURSEL,
											 (WPARAM) 0,
											 (LPARAM) 0 );


		refreshPluginInfo();
		break;
	}
	
	case IDOK:
	case IDCANCEL:	
		EndDialog(hWnd,0);
		return 0;

	default:									// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;								// return zero to let windows take care of it.
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
				refreshPluginList();
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

void XAP_Win32Dialog_PluginManager::refreshPluginList()
{	
	
	// Clear List Box
	SendDlgItemMessage(m_hwndDlg, XAP_RID_DIALOG_PLUGIN_MANAGER_LBX_LIST,
                LB_RESETCONTENT,  (WPARAM) 0, (LPARAM) 0 );

	// Populate List Box
	XAP_Module* pModule = 0;
	const UT_Vector* pVec = XAP_ModuleManager::instance().enumModules();

	for (UT_uint32 i = 0; i < pVec->size(); i++)
	{
        pModule = (XAP_Module *)pVec->getNthItem (i);
        SendDlgItemMessage(m_hwndDlg,
                    XAP_RID_DIALOG_PLUGIN_MANAGER_LBX_LIST,
                    LB_ADDSTRING,
                                                (WPARAM) 0,
                                                (LPARAM) pModule->getModuleInfo()->name );
	}
}

void XAP_Win32Dialog_PluginManager::refreshPluginInfo()
{	
	
	char *pName, *pAutor, *pDesc, *pVersion;
	
	pName = pAutor = pDesc = pVersion ="";
	 
	XAP_Module* pModule = (XAP_Module *) XAP_ModuleManager::instance().enumModules()->getNthItem(m_curSelection);		
	
	if(pModule)
	{	
		const XAP_ModuleInfo * mi = pModule->getModuleInfo();
		if( mi )
		{
		    pName = mi->name;
		    pAutor = mi->author;
		    pDesc = mi->desc;
		    pVersion = mi->version;
		}
	}
	
	SetDlgItemText(m_hwndDlg, XAP_RID_DIALOG_PLUGIN_MANAGER_EBX_NAME, pName);
    SetDlgItemText(m_hwndDlg, XAP_RID_DIALOG_PLUGIN_MANAGER_EBX_DESCRIPTION, pDesc);
	SetDlgItemText(m_hwndDlg, XAP_RID_DIALOG_PLUGIN_MANAGER_EBX_AUTHOR, pAutor);
	SetDlgItemText(m_hwndDlg, XAP_RID_DIALOG_PLUGIN_MANAGER_EBX_VERSION, pVersion);	
	
}