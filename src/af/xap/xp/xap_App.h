/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998,1999 AbiSource, Inc.
 * Copyright (C) 2004 Hubert Figuiere
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
#include "ut_Language.h"
#include "ut_string_class.h"
#include "xap_AppImpl.h"
#include "xav_Listener.h"
#include <map>
#include <string>
#include <list>

#define NUM_MODELESSID 39

class XAP_DialogFactory;
class XAP_Dialog_Modeless;
class XAP_Toolbar_ControlFactory;
class XAP_Frame;
class EV_EditMethodContainer;
class EV_EditBindingMap;
class EV_EditEventMapper;
class EV_Menu_ActionSet;
class EV_Toolbar_ActionSet;
class XAP_BindingSet;
class XAP_Prefs;
class XAP_StringSet;
class XAP_Dictionary;
class PD_DocumentRange;
class AV_View;
class AD_Document;
class XAP_EncodingManager;
class UT_String;
class XAP_Menu_Factory;
class XAP_Toolbar_Factory;
class UT_UUIDGenerator;
class GR_GraphicsFactory;
class GR_Graphics;
class GR_AllocInfo;
class XAP_InputModes;
class AV_Listener;
class GR_EmbedManager;
class XAP_Module;
class UT_ScriptLibrary;


#define XAP_SD_FILENAME_LENGTH 256
#define XAP_SD_MAX_FILES 5

/*!
    Date for storing state
    Please note that this struction must not contain any pointers; the hildon state saving
    mechanism simply memcopies its contents.
*/
struct ABI_EXPORT XAP_StateData
{
	XAP_StateData();
	UT_uint32 iFileCount;
	char filenames[XAP_SD_MAX_FILES][XAP_SD_FILENAME_LENGTH];
	UT_uint32 iDocPos[XAP_SD_MAX_FILES];
	UT_sint32 iXScroll[XAP_SD_MAX_FILES];
	UT_sint32 iYScroll[XAP_SD_MAX_FILES];
};

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
	static const char* s_szAbiSuite_Home;

public:
	static const char* getBuildId ();
	static const char* getBuildVersion ();
	static const char* getBuildOptions ();
	static const char* getBuildTarget ();
	static const char* getBuildCompileTime ();
	static const char* getBuildCompileDate ();
	static const char* getAbiSuiteHome ();

public:
	XAP_App(const char * szAppName);
	virtual ~XAP_App();

	XAP_AppImpl* getImpl()
			{ return m_pImpl; }

	virtual const char * getDefaultEncoding () const = 0 ;

	virtual bool					initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue);
	virtual bool					rememberFrame(XAP_Frame* pFrame, XAP_Frame* pCloneOf = 0);
	virtual bool					forgetFrame(XAP_Frame * pFrame);
	virtual bool					forgetClones(XAP_Frame * pFrame);
	virtual bool					getClones(UT_GenericVector<XAP_Frame*> *pvClonesCopy, XAP_Frame * pFrame);
	virtual XAP_Frame *				newFrame() = 0;
	virtual void					reallyExit() = 0;

	bool						updateClones(XAP_Frame * pFrame);

	virtual void					notifyFrameCountChange (); // default is empty method
	UT_sint32					getFrameCount() const;
	XAP_Frame * 					getFrame(UT_sint32 ndx) const;
	UT_sint32					findFrame(XAP_Frame * pFrame) const;
	UT_sint32					findFrame(const char * szFilename) const;

	void						enumerateFrames(UT_Vector & v) const;
    std::list< AD_Document* >   getDocuments( const AD_Document * pExclude = 0 ) const;
	void						enumerateDocuments(UT_Vector & v, const AD_Document * pExclude) const;
	const char *					getApplicationTitleForTitleBar() const;
	const char *					getApplicationName() const;

	virtual void                rebuildMenus(void);

	EV_EditMethodContainer *			getEditMethodContainer() const;
	EV_EditBindingMap *				getBindingMap(const char * szName);
	XAP_BindingSet *				getBindingSet(void)
	{ return m_pBindingSet;}		/* the set of binding maps */
	const EV_Menu_ActionSet *			getMenuActionSet() const;
	const EV_Toolbar_ActionSet *			getToolbarActionSet() const;
	const XAP_EncodingManager *			getEncodingManager() const;
	EV_Menu_ActionSet *				getMenuActionSet();
	EV_Toolbar_ActionSet *				getToolbarActionSet();

	// only used in ispell builds because aspell doesn't suck...
	bool						addWordToDict(const UT_UCSChar * pWord, UT_uint32 len);
	bool						isWordInDict(const UT_UCSChar * pWord, UT_uint32 len) const;
	void						suggestWord(UT_GenericVector<UT_UCSChar*> * pVecSuggestions, const UT_UCSChar * pWord, UT_uint32 lenWord);
    XAP_Prefs *						getPrefs() const;
	bool						getPrefsValue(const gchar * szKey, const gchar ** pszValue) const;
	bool						getPrefsValue(const UT_String &stKey, UT_String &stValue) const;
	bool						getPrefsValueBool(const gchar * szKey, bool * pbValue) const;

	static XAP_App *				getApp();

	virtual XAP_DialogFactory *			getDialogFactory() = 0;
	virtual XAP_Toolbar_ControlFactory *		getControlFactory() = 0;

	virtual const XAP_StringSet *			getStringSet() const = 0;
	virtual void						migrate(const char *oldName, const char *newName, const char *path) const;
	virtual const char *				getUserPrivateDirectory() const = 0;
	virtual const char *				getAbiSuiteLibDir() const;
	virtual const char *				getAbiSuiteAppDir() const = 0;
	virtual bool					findAbiSuiteLibFile(std::string & path, const char * filename, const char * subdir = 0);
	virtual bool					findAbiSuiteAppFile(std::string & path, const char * filename, const char * subdir = 0); // doesn't check user-dir
	virtual void					copyToClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard = true) = 0;
	virtual void					pasteFromClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard, bool bHonorFormatting = true) = 0;
	virtual bool					canPasteFromClipboard() = 0;
	virtual void					cacheCurrentSelection(AV_View *) = 0;
	virtual void				addClipboardFmt (const char * /*szFormat*/) {}
	virtual void				deleteClipboardFmt (const char * /*szFormat*/) {}
	void						rememberFocussedFrame(void * pJustFocussedFrame);
	XAP_Frame *					getLastFocussedFrame() const;
	XAP_Frame *					findValidFrame() const;
	bool						safeCompare(XAP_Frame * lff, XAP_Frame * f);
	UT_sint32					safefindFrame(XAP_Frame * f) const;
	void						clearLastFocussedFrame();
	void						clearIdTable();
	bool						setDebugBool(void) { m_bDebugBool = true; return m_bDebugBool; }
	bool						clearDebugBool(void) { m_bDebugBool = false; return m_bDebugBool; }
	bool						isDebug(void) { return m_bDebugBool; }
	void						rememberModelessId(UT_sint32 id, XAP_Dialog_Modeless * pDialog);
	void						forgetModelessId(UT_sint32 id);
	bool						isModelessRunning(UT_sint32 id);
	XAP_Dialog_Modeless *				getModelessDialog(UT_sint32 id);
	void						closeModelessDlgs();
	void						notifyModelessDlgsOfActiveFrame(XAP_Frame *p_Frame);
	void						notifyModelessDlgsCloseFrame(XAP_Frame *p_Frame);

	virtual void					setViewSelection(AV_View * /*pView*/) {}; //subclasses override
	virtual AV_View *				getViewSelection() { return static_cast<AV_View *>(NULL);} ; // subclasses override

	virtual	bool					setGeometry(UT_sint32 x, UT_sint32 y,
									UT_uint32 width, UT_uint32 height, UT_uint32 flags = 0);
	virtual	bool					getGeometry(UT_sint32 *x, UT_sint32 *y,
									UT_uint32 *width, UT_uint32 *height, UT_uint32 *flags = 0);
	virtual void 					parseAndSetGeometry(const char *string);
	virtual UT_sint32				makeDirectory(const char * szPath, const UT_sint32 mode ) const = 0;
	XAP_Menu_Factory *				getMenuFactory(void) const { return m_pMenuFactory; }
	XAP_Toolbar_Factory *				getToolbarFactory(void) const { return m_pToolbarFactory; }

	typedef enum {BIDI_SUPPORT_NONE, BIDI_SUPPORT_GUI, BIDI_SUPPORT_FULL} BidiSupportType;

	virtual BidiSupportType				theOSHasBidiSupport() const {return BIDI_SUPPORT_NONE;}
	void						setEnableSmoothScrolling(bool b);
	bool						isSmoothScrollingEnabled(void) { return m_bEnableSmoothScrolling; }

	void						setBonoboRunning(void) { m_bBonoboRunning = true; }
	bool						isBonoboRunning(void) const { return m_bBonoboRunning; }
	virtual void					getDefaultGeometry(UT_uint32& /*width*/,
													   UT_uint32& /*height*/,
													   UT_uint32& /*flags*/) {}

	const UT_LangRecord *				getKbdLanguage() const { return m_pKbdLang; }
	void						setKbdLanguage(const char * pszLang);

	UT_UUIDGenerator *				getUUIDGenerator() const { return m_pUUIDGenerator; }
    std::string                     createUUIDString() const;

	bool						openURL(const char * url) { return m_pImpl->openURL(url); }
	bool						openHelpURL(const char * url) { return m_pImpl->openHelpURL(url); }
	UT_String					localizeHelpUrl(const char * pathBeforeLang,
									const char * pathAfterLang, const char * remoteURLbase)
							{ return m_pImpl->localizeHelpUrl(pathBeforeLang, pathAfterLang, remoteURLbase); }

	GR_GraphicsFactory *				getGraphicsFactory() const { return m_pGraphicsFactory; }
	void						setDefaultGraphicsId(UT_uint32 i);
	/* primary graphics allocator */
	GR_Graphics *					newGraphics(GR_AllocInfo &ai) const;
	/* secondary graphics allocator; use only in special cases */
	GR_Graphics *					newGraphics(UT_uint32 iClassId, GR_AllocInfo &ai) const;
	virtual GR_Graphics *				newDefaultScreenGraphics() const = 0;

	virtual UT_sint32				setInputMode(const char * szName, bool bForce=false);
	const char *					getInputMode() const;
	EV_EditEventMapper *				getEditEventMapper() const;
	bool						addListener(AV_Listener * pListener, AV_ListenerId * pListenerId);
	bool						removeListener(AV_ListenerId listenerId);
	virtual bool					notifyListeners(AV_View * pView, const AV_ChangeMask hint,void * pPrivateData = NULL);

	bool					registerEmbeddable(GR_EmbedManager * pEmbed, const char *uid = NULL);
	bool						unRegisterEmbeddable(const char *uid);
	GR_EmbedManager *				getEmbeddableManager(GR_Graphics * pG, const char * szObjectType);
	XAP_Module *				getPlugin(const char * szPluginName);

	static const char*			findNearestFont(const char* pszFontFamily,
												const char* pszFontStyle,
												const char* pszFontVariant,
												const char* pszFontWeight,
												const char* pszFontStretch,
												const char* pszFontSize,
												const char* pszLang);

	bool                        saveState(bool bQuit);
	bool                        retrieveState();
	virtual void                clearStateInfo(){};

	bool                        getDisableDoubleBuffering() const;
	void                        setDisableDoubleBuffering( bool v );
	bool                        getNoGUI() const;
	void                        setNoGUI( bool v );

	// signal wrapper
	static void signalWrapper(int sig_num);
	// catch the signals
	virtual void catchSignals(int signum) = 0;
	// Override in AP.
	virtual void saveRecoveryFiles() = 0;

protected:
	void						_setAbiSuiteLibDir(const char * sz);
	virtual const char *				_getKbdLanguage() {return NULL;}
	void						_setUUIDGenerator(UT_UUIDGenerator * pG) { m_pUUIDGenerator = pG; }

	virtual bool                _saveState(XAP_StateData & sd);
	virtual bool                _retrieveState(XAP_StateData & sd);

	const char *					m_szAppName;
	const char *					m_szAbiSuiteLibDir;

	EV_EditMethodContainer *			m_pEMC;			/* the set of all possible EditMethods in the app */
	XAP_BindingSet *				m_pBindingSet;		/* the set of binding maps */
	EV_Menu_ActionSet *				m_pMenuActionSet;	/* the set of all possible menu actions in the app */
	EV_Toolbar_ActionSet *				m_pToolbarActionSet;
	XAP_Dictionary *				m_pDict;
	XAP_Prefs *					m_prefs;		/* populated in AP_<platform>App::initialize() */

	UT_GenericVector<XAP_Frame*>			m_vecFrames;
	typedef std::map<std::string, UT_GenericVector<XAP_Frame*>*> CloneMap;
	CloneMap	m_hashClones;
	XAP_Frame *					m_lastFocussedFrame;
	XAP_Menu_Factory *              	        m_pMenuFactory;
	XAP_Toolbar_Factory *				m_pToolbarFactory;

	struct modeless_pair
	{
		UT_sint32 id;
		XAP_Dialog_Modeless * pDialog;
	} m_IdTable[NUM_MODELESSID+1];

	static XAP_App *				m_pApp;
	bool						m_bAllowCustomizing;
	bool						m_bAreCustomized;
	bool						m_bDebugBool;
	bool						m_bBonoboRunning;
	bool						m_bEnableSmoothScrolling;
	bool						m_bDisableDoubleBuffering;
	bool						m_bNoGUI;
private:
	const UT_LangRecord *				m_pKbdLang;
	UT_UUIDGenerator *				m_pUUIDGenerator;

	GR_GraphicsFactory *				m_pGraphicsFactory;
	UT_uint32					m_iDefaultGraphicsId;

	XAP_InputModes *				m_pInputModes;
	std::map<std::string, GR_EmbedManager *> m_mapEmbedManagers;
	XAP_App(const XAP_App&);			// should not even be called. Just to avoid a warning.
	void operator=(const XAP_App&);
#ifdef DEBUG
	void _fundamentalAsserts() const;
#endif
	XAP_AppImpl* m_pImpl;
	UT_GenericVector<AV_Listener *>			m_vecPluginListeners;
	UT_ScriptLibrary *             m_pScriptLibrary;
};

#endif /* XAP_APP_H */
