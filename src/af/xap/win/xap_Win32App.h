/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * BIDI Copyright (c) 2001,2002 Tomas Frydrych
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


#ifndef XAP_WIN32APP_H
#define XAP_WIN32APP_H

#include <windows.h>
#include <tchar.h>
#include "xap_App.h"
#include "xap_Win32DialogFactory.h"
#include "xap_Win32_TB_CFactory.h"
#include "xap_Strings.h"

#define MAX_CONVBUFFER 256

class XAP_Win32Slurp;
class XAP_Win32Toolbar_Icons;

/*****************************************************************
******************************************************************
** Only one of these is created by the application.
******************************************************************
*****************************************************************/

class ABI_EXPORT XAP_Win32App : public XAP_App
{
public:
	XAP_Win32App(HINSTANCE hInstance, const char * szAppName);
	virtual ~XAP_Win32App(void);

	virtual const char * getDefaultEncoding () const;

	static const WCHAR * getWideString (const char * p_str);
	static const char * getUTF8String (const WCHAR * p_str);

	virtual bool							initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue);
	virtual XAP_Frame *						newFrame(void) = 0;
	virtual void							reallyExit(void);

	virtual HINSTANCE						getInstance() const;

	virtual XAP_DialogFactory *				getDialogFactory(void);
	virtual XAP_Toolbar_ControlFactory *	getControlFactory(void);
	virtual const XAP_StringSet *			getStringSet(void) const = 0;
	virtual const char *					getAbiSuiteAppDir(void) const = 0;
	virtual void							copyToClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard = true) = 0;
	virtual void							pasteFromClipboard(PD_DocumentRange * pDocRange, bool, bool) = 0;
	virtual bool							canPasteFromClipboard(void) = 0;
	virtual void							cacheCurrentSelection(AV_View *) = 0;
	virtual const char *					getUserPrivateDirectory(void) const;

	virtual HICON							getIcon(void) = 0;
	virtual HICON							getSmallIcon(void) = 0;

	virtual UT_Error						fileOpen(XAP_Frame * pFrame, const char * pNewFile) = 0;

	void									enableAllTopLevelWindows(bool);
	virtual UT_sint32 				setupWindowFromPrefs(UT_sint32 iCmdShow, HWND hwndFrame);
    virtual XAP_App::BidiSupportType        theOSHasBidiSupport() const {return m_eBidiOS;}

	void									getDefaultGeometry(UT_uint32& width,
															   UT_uint32& height,
															   UT_uint32& flags);

	void                                    setHKL(HKL hkl) {m_hkl = hkl;}
	HKL                                     getHKL()const {return m_hkl;}
	void                                    setKbdLanguage(HKL hkl);

protected:
	UT_uint32								_getExeDir(LPWSTR pDirBuf, UT_uint32 iBufLen);
	void									_setAbiSuiteLibDir(void);
	void									_setBidiOS(void);
	virtual const char *                    _getKbdLanguage();

	HINSTANCE								m_hInstance;
	AP_Win32DialogFactory					m_dialogFactory;
	AP_Win32Toolbar_ControlFactory			m_controlFactory;

	XAP_Win32Slurp *						m_pSlurp;
	static char m_buffer[MAX_CONVBUFFER*6];
	static WCHAR m_wbuffer[MAX_CONVBUFFER];

private:
	XAP_App::BidiSupportType		        m_eBidiOS;
	HKL                                     m_hkl; // kbd layout
												   // handle
};

#endif /* XAP_WIN32APP_H */
