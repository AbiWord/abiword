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

#ifndef AP_BEOSAPP_H
#define AP_BEOSAPP_H

#include "xap_Args.h"
#include "xap_BeOSApp.h"
#include "ap_BeOSPrefs.h"
#include "ap_BeOSClipboard.h"
class PD_DocumentRange;
class XAP_StringSet;


class AP_BeOSApp : public XAP_BeOSApp
{
public:
	AP_BeOSApp(XAP_Args * pArgs, const char * szAppName);
	virtual ~AP_BeOSApp(void);

	virtual UT_Bool					initialize(void);
	virtual XAP_Frame *				newFrame(void);
	virtual XAP_Frame *				newFrame(const char *path);
	virtual UT_Bool					shutdown(void);
	virtual XAP_Prefs *				getPrefs(void) const;
	virtual UT_Bool					getPrefsValue(const XML_Char * szKey, const XML_Char ** pszValue) const;
	virtual UT_Bool					getPrefsValueDirectory(const XML_Char * szKey, const XML_Char ** pszValue) const;
	virtual const XAP_StringSet *	getStringSet(void) const;
	virtual const char *			getAbiSuiteAppDir(void) const;
	virtual void					copyToClipboard(PD_DocumentRange * pDocRange);
	virtual void					pasteFromClipboard(PD_DocumentRange * pDocRange);
	virtual UT_Bool					canPasteFromClipboard(void);
	
	void							ParseCommandLine(void);
	
	static int local_main (const char * szAppName, int argc, char ** argv);

protected:
	AP_BeOSPrefs *			m_prefs;
	XAP_StringSet *			m_pStringSet;
	AP_BeOSClipboard *		m_pClipboard;
};

#endif /* AP_BEOSAPP_H */
