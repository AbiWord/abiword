/* AbiWord
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

/*****************************************************************
******************************************************************
** Only one of these is created by the application.
******************************************************************
*****************************************************************/

#ifndef AP_WIN32APP_H
#define AP_WIN32APP_H

#include "xap_Args.h"
#include "xap_Win32App.h"
#include "ap_Win32Prefs.h"
#include "ap_Win32Clipboard.h"
class PD_DocumentRange;

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class AP_Win32App : public XAP_Win32App
{
public:
	AP_Win32App(HINSTANCE hInstance, XAP_Args * pArgs, const char * szAppName);
	virtual ~AP_Win32App(void);

	virtual UT_Bool					initialize(void);
	virtual XAP_Frame *				newFrame(void);
	virtual UT_Bool					shutdown(void);
	virtual XAP_Prefs *				getPrefs(void) const;
	virtual UT_Bool					getPrefsValue(const XML_Char * szKey, const XML_Char ** pszValue) const;
	virtual UT_Bool					getPrefsValueDirectory(UT_Bool bAppSpecific,
														   const XML_Char * szKey, const XML_Char ** pszValue) const;
	virtual const XAP_StringSet *	getStringSet(void) const;
	virtual const char *			getAbiSuiteAppDir(void) const;
	virtual void					copyToClipboard(PD_DocumentRange * pDocRange);
	virtual void					pasteFromClipboard(PD_DocumentRange * pDocRange, UT_Bool);
	virtual UT_Bool					canPasteFromClipboard(void);
	virtual void					cacheCurrentSelection(AV_View *) {};

	void							ParseCommandLine(int iCmdShow);
	
	static int WinMain (const char * szAppName, HINSTANCE hInstance, 
						HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow);

	virtual HICON							getIcon(void);
	virtual HICON							getSmallIcon(void);
	
protected:
	AP_Win32Prefs *			m_prefs;
	XAP_StringSet *			m_pStringSet;
	AP_Win32Clipboard *		m_pClipboard;
};

#endif /* AP_WIN32APP_H */
