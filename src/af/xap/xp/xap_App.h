/* AbiSource Application Framework
 * Copyright (C) 1998,1999 AbiSource, Inc.
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


#ifndef XAP_APP_H
#define XAP_APP_H

#include "ut_types.h"
#include "ut_vector.h"
#include "ut_hash.h"
#include "xmlparse.h"
#include "xap_Dialog.h"

#define NUM_MODELESSID 39

class XAP_Args;
class XAP_DialogFactory;
class XAP_Toolbar_ControlFactory;
class XAP_Frame;
class EV_EditMethodContainer;
class EV_EditBindingMap;
class EV_Menu_ActionSet;
class EV_Toolbar_ActionSet;
class XAP_BindingSet;
class XAP_Prefs;
class XAP_StringSet;
class XAP_Dictionary;
class PD_DocumentRange;
class AV_View;

/*****************************************************************
******************************************************************
** This file defines the base class for the cross-platform 
** application.  This is used to hold all of the application-specific
** data.  Only one of these is created by the application.
******************************************************************
*****************************************************************/

class XAP_App
{
public:									/* TODO these should be protected */
	static const char* s_szBuild_ID;
	static const char* s_szBuild_Version;
	static const char* s_szBuild_Options;
	static const char* s_szBuild_Target;
	static const char* s_szBuild_CompileTime;
	static const char* s_szBuild_CompileDate;

public:
	XAP_App(XAP_Args * pArgs, const char * szAppName);
	virtual ~XAP_App(void);

	virtual UT_Bool					initialize(void);
	virtual UT_Bool					rememberFrame(XAP_Frame * pFrame, XAP_Frame * pCloneOf=(XAP_Frame*)NULL);
	virtual UT_Bool					forgetFrame(XAP_Frame * pFrame);
	virtual UT_Bool					forgetClones(XAP_Frame * pFrame);
	virtual UT_Bool					getClones(UT_Vector *pvClonesCopy, XAP_Frame * pFrame);
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

	XAP_Args *						getArgs(void) const;

	UT_Bool							addWordToDict(const UT_UCSChar * pWord, UT_uint32 len);
	UT_Bool							isWordInDict(const UT_UCSChar * pWord, UT_uint32 len) const;

	XAP_Prefs *						getPrefs(void) const;
	UT_Bool							getPrefsValue(const XML_Char * szKey, const XML_Char ** pszValue) const;
	UT_Bool							getPrefsValueBool(const XML_Char * szKey, UT_Bool * pbValue) const;

	static XAP_App *				getApp(void) {return m_pApp;}

	virtual XAP_DialogFactory *				getDialogFactory(void) = 0;
	virtual XAP_Toolbar_ControlFactory *	getControlFactory(void) = 0;

	virtual const XAP_StringSet *			getStringSet(void) const = 0;
	virtual const char *					getUserPrivateDirectory(void) = 0;
	virtual const char *					getAbiSuiteLibDir(void) const;
	virtual const char *					getAbiSuiteAppDir(void) const = 0;
	virtual void							copyToClipboard(PD_DocumentRange * pDocRange) = 0;
	virtual void							pasteFromClipboard(PD_DocumentRange * pDocRange, UT_Bool bUseClipboard) = 0;
	virtual UT_Bool							canPasteFromClipboard(void) = 0;
	virtual void							cacheCurrentSelection(AV_View *) = 0;
        void rememberFocussedFrame( void * pJustFocussedFrame);
        XAP_Frame * getLastFocussedFrame( void ) ;
        XAP_Frame * findValidFrame( void ) ;
        UT_Bool safeCompare( XAP_Frame * lff, XAP_Frame * f);
        UT_sint32 safefindFrame( XAP_Frame * f);
        void clearLastFocussedFrame(void);
        void clearIdTable( void);
	void rememberModelessId(  UT_sint32 id, XAP_Dialog_Modeless * pDialog);
	void forgetModelessId( UT_sint32 id );
        UT_Bool isModelessRunning( UT_sint32 id);
	XAP_Dialog_Modeless * getModelessDialog( UT_sint32 id);
	void closeModelessDlgs( void);
	void notifyModelessDlgsOfActiveFrame(XAP_Frame *p_Frame);
	void notifyModelessDlgsCloseFrame(XAP_Frame *p_Frame);

protected:
	void				  _setAbiSuiteLibDir(const char * sz);
	
	XAP_Args *						m_pArgs;
	const char *					m_szAppName;
	const char *					m_szAbiSuiteLibDir;

	EV_EditMethodContainer *		m_pEMC;				/* the set of all possible EditMethods in the app */
	XAP_BindingSet *				m_pBindingSet;		/* the set of binding maps */
	EV_Menu_ActionSet *				m_pMenuActionSet;	/* the set of all possible menu actions in the app */
	EV_Toolbar_ActionSet *			m_pToolbarActionSet;
	XAP_Dictionary *				m_pDict;

	XAP_Prefs *						m_prefs;			/* populated in AP_<platform>App::initialize() */

	UT_Vector						m_vecFrames;
	UT_HashTable					m_hashClones;
        XAP_Frame *                                     m_lastFocussedFrame;

        struct modeless_pair 
        { 
	       UT_sint32 id;
	       XAP_Dialog_Modeless * pDialog;
        } m_IdTable[NUM_MODELESSID+1]; 
        
	static XAP_App *				m_pApp;
};

#endif /* XAP_APP_H */


