/* AbiSource Application Framework
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


#ifndef AP_APP_H
#define AP_APP_H

#ifdef ABI_OPT_JS
#include <js.h>
#endif /* ABI_OPT_JS */

#include "ut_types.h"
#include "ut_vector.h"
#include "ut_hash.h"

class AP_Args;
class AP_DialogFactory;
class AP_Toolbar_ControlFactory;
class XAP_Frame;
class EV_EditMethodContainer;
class EV_EditBindingMap;
class EV_Menu_ActionSet;
class EV_Toolbar_ActionSet;
class AP_Clipboard;
class GR_ImageFactory;
class XAP_BindingSet;

/*****************************************************************
******************************************************************
** This file defines the base class for the cross-platform 
** application.  This is used to hold all of the application-specific
** data.  Only one of these is created by the application.
******************************************************************
*****************************************************************/

class AP_App
{
public:									/* TODO these should be protected */
	static const char* s_szBuild_ID;
	static const char* s_szBuild_Version;
	static const char* s_szBuild_Options;
	static const char* s_szBuild_Target;
	static const char* s_szBuild_CompileTime;
	static const char* s_szBuild_CompileDate;

public:
	AP_App(AP_Args * pArgs, const char * szAppName);
	virtual ~AP_App(void);

	virtual UT_Bool					initialize(void);
	virtual UT_Bool					rememberFrame(XAP_Frame * pFrame, XAP_Frame * pCloneOf=(XAP_Frame*)NULL);
	virtual UT_Bool					forgetFrame(XAP_Frame * pFrame);
	virtual XAP_Frame * 			newFrame(void) = 0;
	virtual void					reallyExit(void) = 0;

	UT_Bool							updateClones(XAP_Frame * pFrame);

	UT_uint32						getFrameCount(void) const;
	XAP_Frame * 					getFrame(UT_uint32 ndx) const;
	UT_sint32						findFrame(XAP_Frame * pFrame);
	UT_sint32						findFrame(const char * szFilename);

	const char *					getApplicationTitleForTitleBar(void) const;
	const char *					getApplicationName(void) const;
	
	EV_EditMethodContainer *		getEditMethodContainer(void) const;
	EV_EditBindingMap *				getBindingMap(const char * szName);
	const EV_Menu_ActionSet *		getMenuActionSet(void) const;
	const EV_Toolbar_ActionSet *	getToolbarActionSet(void) const;

	AP_Args *						getArgs(void) const;

	virtual AP_DialogFactory *				getDialogFactory(void) = 0;
	virtual AP_Toolbar_ControlFactory *		getControlFactory(void) = 0;

	static AP_Clipboard*					getClipboard(void);
	static GR_ImageFactory*			getImageFactory(void);

protected:
	AP_Args *						m_pArgs;
	const char *					m_szAppName;

	EV_EditMethodContainer *		m_pEMC;				/* the set of all possible EditMethods in the app */
	XAP_BindingSet *				m_pBindingSet;		/* the set of binding maps */
	EV_Menu_ActionSet *				m_pMenuActionSet;	/* the set of all possible menu actions in the app */
	EV_Toolbar_ActionSet *			m_pToolbarActionSet;

	UT_Vector						m_vecFrames;
	UT_HashTable					m_hashClones;

#ifdef ABI_OPT_JS	
public:
	JSInterpPtr			   			getInterp(void) const;

protected:	
	JSInterpPtr 					m_pJSInterp;
	JSInterpOptions 				m_JSOptions;
#endif /* ABI_OPT_JS */

	// TODO give this a s_ prefix like all other static variables
	static AP_Clipboard*			_pClipboard;
	static GR_ImageFactory*			_pImageFactory;
};

#endif /* AP_APP_H */
