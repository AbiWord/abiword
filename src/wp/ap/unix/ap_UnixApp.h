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

#ifndef AP_UNIXAPP_H
#define AP_UNIXAPP_H

#include "xap_Args.h"
#include "xap_UnixApp.h"
#include "ap_UnixPrefs.h"

class XAP_StringSet;

class AP_UnixApp : public XAP_UnixApp
{
public:
	AP_UnixApp(XAP_Args * pArgs, const char * szAppName);
	virtual ~AP_UnixApp(void);

	virtual UT_Bool					initialize(void);
	virtual XAP_Frame *				newFrame(void);
	virtual UT_Bool					shutdown(void);
	virtual XAP_Prefs *				getPrefs(void) const;
	virtual UT_Bool					getPrefsValue(const XML_Char * szKey, const XML_Char ** pszValue) const;
	virtual UT_Bool					getPrefsValueDirectory(const XML_Char * szKey, const XML_Char ** pszValue) const;
	virtual const XAP_StringSet *	getStringSet(void) const;
	
	UT_Bool							parseCommandLine(void);

	static int main (const char * szAppName, int argc, char ** argv);

protected:

	void							_printUsage(void);

	AP_UnixPrefs *			m_prefs;
	XAP_StringSet *			m_pStringSet;
	
};

#endif /* AP_UNIXAPP_H */
