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

	virtual bool					initialize(void);
	virtual XAP_Frame *				newFrame(void);
	virtual XAP_Frame *				newFrame(const char *path);
	virtual bool					shutdown(void);
	virtual bool					getPrefsValueDirectory(bool bAppSpecific,
														   const XML_Char * szKey, const XML_Char ** pszValue) const;
	virtual const XAP_StringSet *	getStringSet(void) const;
	virtual const char *			getAbiSuiteAppDir(void) const;
	virtual void					copyToClipboard(PD_DocumentRange * pDocRange);
//	virtual void					pasteFromClipboard(PD_DocumentRange * pDocRange, bool, bool = true);
	virtual void					pasteFromClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard, bool bHonorFormatting = true);
	virtual bool					canPasteFromClipboard(void);
	virtual void					cacheCurrentSelection(AV_View *) {};

	void							ParseCommandLine(void);
	
	static int main (const char * szAppName, int argc, char ** argv);

	void                                             catchSignals(int sig_num);

protected:
	XAP_StringSet *			m_pStringSet;
	AP_BeOSClipboard *		m_pClipboard;
};

// What follows is an ugly hack. It is neccessitated by the 
// C/C++ conflict over pointers to member functions. It is,
// however, what the C++ FAQ reccommends.

void signalWrapper(int);

#endif /* AP_BEOSAPP_H */
