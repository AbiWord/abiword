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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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
	UT_return_if_fail(pFrame);
	UT_return_if_fail(m_id == XAP_DIALOG_ID_PLUGIN_MANAGER);

	setDialog(this);
	createModal(pFrame,MAKEINTRESOURCEW(XAP_RID_DIALOG_PLUGIN_MANAGER));
}

#define _DS(c,s)	setDlgItemText(XAP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))
#define _DSX(c,s)	setDlgItemText(XAP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL XAP_Win32Dialog_PluginManager::_onInitDialog(HWND /*hWnd*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{		
	const XAP_StringSet* pSS = m_pApp->getStringSet();

	// Update the caption
	setDialogTitle(pSS->getValue(XAP_STRING_ID_DLG_PLUGIN_MANAGER_TITLE));

	// localize controls
	localizeControlText(IDOK,			XAP_STRING_ID_DLG_OK);
	// _DSX(PLUGIN_MANAGER_BTN_OK,		DLG_OK);
	_DS(PLUGIN_MANAGER_BTN_ACTIVE,		DLG_PLUGIN_MANAGER_ACTIVE);
	_DS(PLUGIN_MANAGER_BTN_INSTALL,		DLG_PLUGIN_MANAGER_INSTALL);
	_DS(PLUGIN_MANAGER_LBL_NAME,		DLG_PLUGIN_MANAGER_NAME);
	_DS(PLUGIN_MANAGER_LBL_DESCRIPTION,	DLG_PLUGIN_MANAGER_DESC);
	_DS(PLUGIN_MANAGER_LBL_AUTHOR,		DLG_PLUGIN_MANAGER_AUTHOR);
	_DS(PLUGIN_MANAGER_LBL_VERSION,		DLG_PLUGIN_MANAGER_VERSION);

	refreshPluginList();	

	/* Default */
	selectListItem(XAP_RID_DIALOG_PLUGIN_MANAGER_LBX_LIST, 0);
	centerDialog();

	return 1;							// 1 == we did not call SetFocus()
}

BOOL XAP_Win32Dialog_PluginManager::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	// This handles WM_COMMAND message for the top-level dialog.
	
	WORD wId = LOWORD(wParam);

	switch (wId)
	{
	// LIST TAB
	case XAP_RID_DIALOG_PLUGIN_MANAGER_BTN_ACTIVE:	
		return 0;	

	case XAP_RID_DIALOG_PLUGIN_MANAGER_BTN_INSTALL:
		event_Load();
		return 0;

	case XAP_RID_DIALOG_PLUGIN_MANAGER_LBX_LIST:
	{
		m_curSelection = getListSelectedIndex( wId );
		refreshPluginInfo();
		return 0;
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

BOOL XAP_Win32Dialog_PluginManager::_onDeltaPos(NM_UPDOWN * /*pnmud*/)
{
	return FALSE;
}

void XAP_Win32Dialog_PluginManager::event_Load()
{
	
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *) getApp()->findValidFrame()->getDialogFactory();
	
	XAP_Dialog_FileOpenSaveAs * pDialog
		= (XAP_Dialog_FileOpenSaveAs *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_FILE_OPEN));
	UT_return_if_fail(pDialog);
	
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
	UT_return_if_fail(szDescList);
	const char ** szSuffixList = (const char **) UT_calloc(filterCount + 1,
														sizeof(char *));
	if(!szSuffixList)
	{
		UT_ASSERT_HARMLESS(szSuffixList);
		FREEP(szDescList);
		return;
	}
	IEFileType * nTypeList = (IEFileType *) UT_calloc(filterCount + 1,
												   sizeof(IEFileType));
	if(!nTypeList)
	{
		UT_ASSERT_HARMLESS(nTypeList);
		FREEP(szDescList);
		FREEP(szSuffixList);
		return;
	}
	// we probably shouldn't hardcode this
	// HP-UX uses .sl, for instance
	szDescList[0] = "AbiWord Plugin (.dll)";
	szSuffixList[0] = "*.dll";
	nTypeList[0] = (IEFileType)1;
	
	pDialog->setFileTypeList(szDescList, szSuffixList, 
							 (const UT_sint32 *) nTypeList);
	
	pDialog->setDefaultFileType((IEFileType)1);

	// todo: cd to the proper plugin directory
	
	pDialog->runModal(getApp()->findValidFrame());
	
	XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);
	
	if (bOK)
	{
		const std::string & resultPathname = pDialog->getPathname();
		if (!resultPathname.empty())
		{
			if( activatePlugin(resultPathname.c_str()) )
			{
				// worked!
				refreshPluginList();
			}
			else
			{
				// error message
				getApp()->findValidFrame()->showMessageBox( pSS->getValue(XAP_STRING_ID_DLG_PLUGIN_MANAGER_COULDNT_LOAD),
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
	resetContent( XAP_RID_DIALOG_PLUGIN_MANAGER_LBX_LIST );

	// Populate List Box
	XAP_Module* pModule = 0;
	const UT_GenericVector<class XAP_Module *> *pVec = XAP_ModuleManager::instance().enumModules();

	for (UT_sint32 i = 0; i < pVec->size(); i++)
	{
        pModule = (XAP_Module *)pVec->getNthItem (i);
		addItemToList( XAP_RID_DIALOG_PLUGIN_MANAGER_LBX_LIST, pModule->getModuleInfo()->name );
	}
}

void XAP_Win32Dialog_PluginManager::refreshPluginInfo()
{	

	const char *pName, *pAuthor, *pDesc, *pVersion;
	pName = pAuthor = pDesc = pVersion ="";
	XAP_Module* pModule = NULL;

	if (m_curSelection!=LB_ERR)
	{
		pModule = (XAP_Module *) XAP_ModuleManager::instance().enumModules()->getNthItem(m_curSelection);
	}
	
	if(pModule)
	{	
		const XAP_ModuleInfo * mi = pModule->getModuleInfo();
		if( mi )
		{
		    pName = mi->name;
		    pAuthor = mi->author;
		    pDesc = mi->desc;
		    pVersion = mi->version;
		}
	}
	
	setControlText(XAP_RID_DIALOG_PLUGIN_MANAGER_EBX_NAME, pName);
    setControlText(XAP_RID_DIALOG_PLUGIN_MANAGER_EBX_DESCRIPTION, pDesc);
	setControlText(XAP_RID_DIALOG_PLUGIN_MANAGER_EBX_AUTHOR, pAuthor);
	setControlText(XAP_RID_DIALOG_PLUGIN_MANAGER_EBX_VERSION, pVersion);	
}
