/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_vector.h"
#include "ut_hash.h"

#define NUM_MODELESSID 39

class XAP_Args;
class XAP_DialogFactory;
class XAP_Dialog_Modeless;
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
class XAP_EncodingManager;
class UT_String;
class XAP_Menu_Factory;
class XAP_Toolbar_Factory;
class XAP_HashDownloader;

/*****************************************************************
******************************************************************
** This file defines the base class for the cross-platform 
** application.  This is used to hold all of the application-specific
** data.  Only one of these is created by the application.
******************************************************************
*****************************************************************/

class ABI_EXPORT XAP_App
{
public:									/* TODO these should be protected */
	static const char* s_szBuild_ID;
	static const char* s_szBuild_Version;
	static const char* s_szBuild_Options;
	static const char* s_szBuild_Target;
	static const char* s_szBuild_CompileTime;
	static const char* s_szBuild_CompileDate;

 public:
	static const char* getBuildId ();
	static const char* getBuildVersion ();
	static const char* getBuildOptions ();
	static const char* getBuildTarget ();
	static const char* getBuildCompileTime ();
	static const char* getBuildCompileDate ();

public:
	XAP_App(XAP_Args * pArgs, const char * szAppName);
	virtual ~XAP_App();

	virtual const char * getDefaultEncoding () const = 0 ;

	virtual bool					initialize();
	virtual bool					rememberFrame(XAP_Frame* pFrame, XAP_Frame* pCloneOf = 0);
	virtual bool					forgetFrame(XAP_Frame * pFrame);
	virtual bool					forgetClones(XAP_Frame * pFrame);
	virtual bool					getClones(UT_Vector *pvClonesCopy, XAP_Frame * pFrame);
	virtual XAP_Frame * 			newFrame() = 0;
	virtual void					reallyExit() = 0;

	bool							updateClones(XAP_Frame * pFrame);

	UT_uint32						getFrameCount() const;
	XAP_Frame * 					getFrame(UT_uint32 ndx) const;
	UT_sint32						findFrame(XAP_Frame * pFrame);
	UT_sint32						findFrame(const char * szFilename);

	const char *					getApplicationTitleForTitleBar() const;
	const char *					getApplicationName() const;
	
	EV_EditMethodContainer *		getEditMethodContainer() const;
	EV_EditBindingMap *				getBindingMap(const char * szName);
	const EV_Menu_ActionSet *		getMenuActionSet() const;
	const EV_Toolbar_ActionSet *	getToolbarActionSet() const;
	const XAP_EncodingManager *		getEncodingManager() const;
	EV_Menu_ActionSet *				getMenuActionSet();
	EV_Toolbar_ActionSet *			getToolbarActionSet();

	XAP_Args *			getArgs() const;

	// only used in ispell builds because aspell doesn't suck...
	bool				addWordToDict(const UT_UCSChar * pWord, UT_uint32 len);
	bool				isWordInDict(const UT_UCSChar * pWord, UT_uint32 len) const;
	void                            suggestWord(UT_Vector * pVecSuggestions, const UT_UCSChar * pWord, UT_uint32 lenWord);
    XAP_Prefs *						getPrefs() const;
	bool							getPrefsValue(const XML_Char * szKey, const XML_Char ** pszValue) const;
	bool							getPrefsValue(const UT_String &stKey, UT_String &stValue) const;
	bool							getPrefsValueBool(const XML_Char * szKey, bool * pbValue) const;

	static XAP_App *				getApp();

	virtual XAP_DialogFactory *				getDialogFactory() = 0;
	virtual XAP_Toolbar_ControlFactory *	getControlFactory() = 0;

	virtual const XAP_StringSet *			getStringSet() const = 0;
	virtual const char *					getUserPrivateDirectory() = 0;
	virtual const char *					getAbiSuiteLibDir() const;
	virtual const char *					getAbiSuiteAppDir() const = 0;
	virtual void							copyToClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard = true) = 0;
	virtual void							pasteFromClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard, bool bHonorFormatting = true) = 0;
	virtual bool							canPasteFromClipboard() = 0;
	virtual void							cacheCurrentSelection(AV_View *) = 0;
	void									rememberFocussedFrame(void * pJustFocussedFrame);
	XAP_Frame *								getLastFocussedFrame();
	XAP_Frame *								findValidFrame();
	bool									safeCompare(XAP_Frame * lff, XAP_Frame * f);
	UT_sint32								safefindFrame(XAP_Frame * f);
	void									clearLastFocussedFrame();
	void									clearIdTable();
	bool                                    setDebugBool(void)
		{m_bDebugBool = true; return m_bDebugBool;}
	bool                                    clearDebugBool(void)
		{m_bDebugBool = false; return m_bDebugBool;}
	bool                                    isDebug(void)
		{return m_bDebugBool;}
	void									rememberModelessId(UT_sint32 id, XAP_Dialog_Modeless * pDialog);
	void									forgetModelessId(UT_sint32 id );
	bool									isModelessRunning(UT_sint32 id);
	XAP_Dialog_Modeless *					getModelessDialog(UT_sint32 id);
	void									closeModelessDlgs();
	void									notifyModelessDlgsOfActiveFrame(XAP_Frame *p_Frame);
	void									notifyModelessDlgsCloseFrame(XAP_Frame *p_Frame);

	virtual void							setViewSelection(AV_View * pView) {}; //subclasses override
	virtual AV_View *						getViewSelection() { return (AV_View *)  NULL;} ; // subclasses override
	
	virtual	bool							setGeometry(UT_sint32 x, UT_sint32 y, 
														UT_uint32 width, UT_uint32 height, UT_uint32 flags = 0);
	virtual	bool							getGeometry(UT_sint32 *x, UT_sint32 *y, 
														UT_uint32 *width, UT_uint32 *height, UT_uint32 *flags = 0);
	virtual void 							parseAndSetGeometry(const char *string);
	virtual UT_sint32						makeDirectory(const char * szPath, const UT_sint32 mode ) const = 0;
	XAP_Menu_Factory *                      getMenuFactory(void) const { return m_pMenuFactory;}
	XAP_Toolbar_Factory *                   getToolbarFactory(void) const { return m_pToolbarFactory;}

	typedef enum {BIDI_SUPPORT_NONE, BIDI_SUPPORT_GUI, BIDI_SUPPORT_FULL} BidiSupportType;
	
    virtual BidiSupportType                 theOSHasBidiSupport() const {return BIDI_SUPPORT_NONE;}
	bool                                    areToolbarsCustomizable(void) const 
		                                             { return m_bAllowCustomizing;}
    void                                    setToolbarsCustomizable(bool b);
    
	virtual bool 							getDisplayStatus(void) const;
	virtual bool 							setDisplayStatus(bool b);

	XAP_HashDownloader *			getHashDownloader(void) const { return m_pHashDownloader; };

protected:
	void									_setAbiSuiteLibDir(const char * sz);
	virtual void							_printUsage();   
	
	XAP_Args *								m_pArgs;
	const char *							m_szAppName;
	const char *							m_szAbiSuiteLibDir;

	EV_EditMethodContainer *				m_pEMC;				/* the set of all possible EditMethods in the app */
	XAP_BindingSet *						m_pBindingSet;		/* the set of binding maps */
	EV_Menu_ActionSet *						m_pMenuActionSet;	/* the set of all possible menu actions in the app */
	EV_Toolbar_ActionSet *					m_pToolbarActionSet;
	XAP_Dictionary *						m_pDict;
	XAP_Prefs *								m_prefs;			/* populated in AP_<platform>App::initialize() */

	UT_Vector								m_vecFrames;
	UT_StringPtrMap							m_hashClones;
	XAP_Frame *								m_lastFocussedFrame;
	XAP_Menu_Factory *                      m_pMenuFactory;
	XAP_Toolbar_Factory *                   m_pToolbarFactory;

	XAP_HashDownloader *			        m_pHashDownloader;

	static bool 							s_bShowDisplay;

	struct modeless_pair 
	{ 
		UT_sint32 id;
		XAP_Dialog_Modeless * pDialog;
	} m_IdTable[NUM_MODELESSID+1]; 
        
	static XAP_App *						m_pApp;
	bool                                    m_bAllowCustomizing;
	bool                                    m_bDebugBool;
};

#endif /* XAP_APP_H */









