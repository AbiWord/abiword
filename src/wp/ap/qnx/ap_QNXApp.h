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

#ifndef AP_QNXAPP_H
#define AP_QNXAPP_H

#include "ap_App.h"
#include "ut_bytebuf.h"
#include "xap_Args.h"
#include "xap_QNXApp.h"
#include "ap_QNXPrefs.h"
#include "ap_QNXClipboard.h"
#include "pt_Types.h"

class XAP_StringSet;
class AV_View;
class GR_Image;
class AP_Args;

class AP_QNXApp : public AP_App
{
public:
	AP_QNXApp(XAP_Args * pArgs, const char * szAppName);
	virtual ~AP_QNXApp(void);

	virtual bool					initialize(void);
	virtual XAP_Frame *		newFrame(void);
	virtual void 					reallyExit(void);
	virtual bool					shutdown(void);
	virtual bool					getPrefsValueDirectory(bool bAppSpecific,
														   const gchar * szKey, const gchar ** pszValue) const;
	virtual const XAP_StringSet *	getStringSet(void) const;
	virtual const char *	getAbiSuiteAppDir(void) const;
	virtual void					copyToClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard = true);
	virtual void					pasteFromClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard, bool bHonorFormatting);
	virtual bool					canPasteFromClipboard(void);
  virtual void          cacheCurrentSelection(AV_View *) {};
	



	virtual	void					catchSignals(int sig_num);
	void 									loadAllPlugins ();

	virtual void errorMsgBadArg(AP_Args * Args, int nextopt);
	virtual void errorMsgBadFile(XAP_Frame * pFrame, const char * file, 
								 UT_Error error);
	virtual bool doWindowlessArgs (const AP_Args *, bool & bSuccess);
	virtual XAP_Frame * newFrame(AP_App *);
	
	static int main (const char * szAppName, int argc,const char ** argv);

protected:
	XAP_StringSet *			m_pStringSet;
	AP_QNXClipboard *		m_pClipboard;

};

#endif /* AP_QNXAPP_H */
