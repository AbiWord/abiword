/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 1999 John Brewer DBA Jera Design
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

#ifndef AP_MACAPP_H
#define AP_MACAPP_H

#include "xap_Args.h"
#include "xap_MacApp.h"
#include "ap_MacPrefs.h"
#include "ap_MacClipboard.h"

class AP_MacApp : public XAP_MacApp
{
public:
	AP_MacApp(XAP_Args * pArgs, const char * szAppName);
	virtual ~AP_MacApp(void);

	virtual bool			initialize(void);
	virtual XAP_Frame *		newFrame(void);
	virtual bool			shutdown(void);

	virtual const XAP_StringSet *	getStringSet(void) const;
	virtual const char *			getAbiSuiteAppDir(void) const;
	virtual void					copyToClipboard(PD_DocumentRange * pDocRange);
	virtual void					pasteFromClipboard(PD_DocumentRange * pDocRange, bool);
	virtual bool					canPasteFromClipboard(void);
	virtual void					cacheCurrentSelection(AV_View *) { UT_ASSERT (UT_NOT_IMPLEMENTED); };
    virtual void                    setViewSelection( AV_View * pView) 
    												{ m_pViewSelection = pView; };
    virtual AV_View *               getViewSelection( void) 
    												{ return m_pViewSelection; };

	static int MacMain (const char * szAppName, int argc, char **argv);

protected:
	XAP_StringSet *			m_pStringSet;
	AP_MacClipboard *		m_pClipboard;
    AV_View *				m_pViewSelection;
};

#endif /* XAP_MACAPP_H */
