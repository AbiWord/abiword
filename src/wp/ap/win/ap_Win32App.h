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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

/*****************************************************************
******************************************************************
** Only one of these is created by the application.
******************************************************************
*****************************************************************/

#ifndef AP_WIN32APP_H
#define AP_WIN32APP_H

#include "ap_App.h"
#include "ap_Win32Prefs.h"
#include "ap_Win32Clipboard.h"
#include "ie_types.h"
#include "ut_string_class.h"
#include "ut_Win32LocaleString.h"

/*
  The following turns on code that tries to place data on the
  clipboard on demand rather than blindly. The idea was that we place
  data on clipboard in RTF format only, cache the document state, and
  indicate that we can provide other formats if required.

  The main advantage, obviously, is being able to provide data for the
  clipboard in a myriad of formats, but without taking up system
  resources.

  At present this code is turned off, since we currently only put on
  the clipboard RTF and text and the latter is quite undemading as far
  as resources go. When we add more export formats, such as AW native,
  this line should be uncommented.

  Tomas, June 28, 2003
*/
//#define COPY_ON_DEMAND


class PD_DocumentRange;

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class ABI_EXPORT AP_Win32App : public AP_App
{
public:
	AP_Win32App(HINSTANCE hInstance, const char * szAppName);
	virtual ~AP_Win32App(void);

	virtual bool					initialize(void);
	virtual XAP_Frame *				newFrame(void);
	virtual bool					shutdown(void);
	virtual bool					getPrefsValueDirectory(bool bAppSpecific,
														   const gchar * szKey, const gchar ** pszValue) const;
	virtual const XAP_StringSet *	getStringSet(void) const;
	virtual const char *			getAbiSuiteAppDir(void) const;
	virtual void					copyToClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard = true);
	virtual GR_Graphics *           newDefaultScreenGraphics() const;


#ifdef COPY_ON_DEMAND
	// see comments in the cpp file
	bool                            copyFmtToClipboardOnDemand(UINT iFmt);
	bool                            copyAllFmtsToClipboardOnDemand();
#endif

	virtual void					pasteFromClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard, bool bHonorFormatting);
	virtual bool					canPasteFromClipboard(void);
	virtual void					cacheCurrentSelection(AV_View *) {};

	virtual void 					errorMsgBadArg(const char *msg);
	virtual void 					errorMsgBadFile(XAP_Frame * pFrame, const char * file, UT_Error error);
	virtual bool 					doWindowlessArgs (const AP_Args *, bool & bSuccess);

	static int WinMain (const char * szAppName, HINSTANCE hInstance,
						HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow);

	virtual HICON							getIcon(void);
	virtual HICON							getSmallIcon(void);

	virtual UT_Error						fileOpen(XAP_Frame * pFrame, const char * pNewFile);
	UT_Vector*								getInstalledUILanguages(void);
	bool									doesStringSetExist(const char* pLocale);

	/*
		Currently we need single byte strings to work with
		win32 UI until we do a Unicode port. It's better to do all UI
		conversions in a single point. You are looking to this point right now. Jordi,
	*/
	static UT_Win32LocaleString 	s_fromUCS4ToWinLocale(const UT_UCS4Char * szIn);
	static UT_UCS4String	s_fromWinLocaleToUCS4(const char* szIn);
	static UT_Win32LocaleString 	s_fromUTF8ToWinLocale(const char* szIn);
	static UT_UTF8String	s_fromWinLocaleToUTF8(const char* szIn);



	bool handleModelessDialogMessage( MSG * msg );

private:
	bool               _copyFmtToClipboard(PD_DocumentRange * pDocRange, UINT iFmt);
	bool               _copyFmtToClipboard(PD_DocumentRange * pDocRange, const char *pszFmt);
#ifdef COPY_ON_DEMAND
	bool               _cacheClipboardDoc(PD_DocumentRange *pDocRange);
	void               _indicateFmtToClipboard(const char * pszFmt) const;
#endif
protected:

	bool					_pasteFormatFromClipboard(PD_DocumentRange * pDocRange, const char * szFormat,
													 const char * szType, bool bWide);

	IEFileType 				_getFileTypeFromDesc(const char *desc);

	XAP_StringSet *			m_pStringSet;
	AP_Win32Clipboard *		m_pClipboard;
	static const bool		m_bForceAnsi = false; // boolean flag used for compatibility with UT_GetMessage
};

#endif /* AP_WIN32APP_H */
