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


#ifndef AP_APP_H
#define AP_APP_H

#ifdef ABI_OPT_JS
#include <js.h>
#endif /* ABI_OPT_JS */

#include "ut_types.h"
#include "ut_vector.h"
#include "ut_hash.h"

class AP_Args;
class AP_Frame;
class EV_EditMethodContainer;
class EV_Menu_ActionSet;
class EV_Toolbar_ActionSet;

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
	static const char* s_szBuild_CompileTime;
	static const char* s_szBuild_CompileDate;

public:
	AP_App(AP_Args * pArgs, const char * szAppName);
	virtual ~AP_App(void);

	virtual UT_Bool					initialize(void);
	virtual UT_Bool					rememberFrame(AP_Frame * pFrame, AP_Frame * pCloneOf=(AP_Frame*)NULL);
	virtual UT_Bool					forgetFrame(AP_Frame * pFrame);
	virtual AP_Frame * 				newFrame(void) = 0;

	UT_Bool							updateClones(AP_Frame * pFrame);

	UT_uint32						getFrameCount(void) const;
	AP_Frame * 						getFrame(UT_uint32 ndx) const;
	UT_sint32						findFrame(AP_Frame * pFrame);
	UT_sint32						findFrame(const char * szFilename);

	const char *					getApplicationTitleForTitleBar(void) const;
	const char *					getApplicationName(void) const;
	
	EV_EditMethodContainer *		getEditMethodContainer(void) const;
	const EV_Menu_ActionSet *		getMenuActionSet(void) const;
	const EV_Toolbar_ActionSet *	getToolbarActionSet(void) const;

	AP_Args *						getArgs(void) const;

protected:
	AP_Args *						m_pArgs;
	const char *					m_szAppName;

	EV_EditMethodContainer *		m_pEMC;				/* the set of all possible EditMethods in the app */
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
};

#endif /* AP_APP_H */
