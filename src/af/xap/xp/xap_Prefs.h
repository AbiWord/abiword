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

#ifndef XAP_PREFS_H
#define XAP_PREFS_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_vector.h"
#include "ut_hash.h"
#include "ut_string.h"
#include "ut_xml.h"

#include "xap_App.h"
#include "xap_Prefs_SchemeIds.h"

class UT_String;

/* For handling the position of windows on the screen, this sets up preferences */
enum {
	PREF_FLAG_GEOMETRY_POS = 0x1,     // Position is valid
	PREF_FLAG_GEOMETRY_SIZE = 0x2,    // Size is valid
	PREF_FLAG_GEOMETRY_NOUPDATE = 0x4, // User specified, don't update it
	PREF_FLAG_GEOMETRY_MAXIMIZED = 0x8 // Maximize at start
};

typedef void (*PrefsListener) (
	XAP_App				*pApp,
	XAP_Prefs			*pPrefs,
	UT_StringPtrMap	    *phChanges,
	void				*data
	);

/*****************************************************************
** XAP_PrefsScheme is a complete set of preferences.  it contains
** all the info for a UI styling set.  use XAP_Prefs to switch
** between schemes.
******************************************************************/

class ABI_EXPORT XAP_PrefsScheme
{
public:
	XAP_PrefsScheme(XAP_Prefs *pPrefs, const XML_Char * szSchemeName);
	~XAP_PrefsScheme(void);

	const XML_Char *	getSchemeName(void) const;
	bool				setSchemeName(const XML_Char * szNewSchemeName);
	// The idea of the tick is that some object can cache a preference
	// value if it makes a performance difference.  It should also save
	// a copy of the tick count and the scheme pointer.  If the scheme
	// pointer and the tick count are the same, the cached preference
	// value is current.  If either is changed, the object can refresh
	// its cached value.  The scheme pointer can be different because
	// the preference scheme has changed.  The tick count bumps up once
	// every time any preference value in the scheme is changed.
    UT_uint32			getTickCount() {return m_uTick;}
	
	bool				setValue(const XML_Char * szKey, const XML_Char * szValue);
	bool				setValueBool(const XML_Char * szKey, bool bValue);
	
	// the get*Value*() functions return the answer in the last
	// argument; they return error information as the function
	// return value.
	bool				getValue(const XML_Char * szKey, const XML_Char ** pszValue) const;
	bool				getValue(const UT_String &szKey, UT_String &szValue) const;
	bool				getValueBool(const XML_Char * szKey, bool * pbValue) const;
	bool				getNthValue(UT_uint32 k, const XML_Char ** pszKey, const XML_Char ** pszValue) const;
	
protected:
	XML_Char *			m_szName;
	UT_StringPtrMap	    m_hash;
	XAP_Prefs *			m_pPrefs;
	UT_uint32			m_uTick;   // ticks up every time setValue() or setValueBool() is called
};

/*****************************************************************/
/*****************************************************************/

class ABI_EXPORT XAP_Prefs : public UT_XML::Listener
{
public:
	XAP_Prefs(XAP_App * pApp);
	virtual ~XAP_Prefs(void);

	bool					loadPrefsFile(void);
	bool					loadSystemDefaultPrefsFile(const char * szSystemDefaultPrefsPathname);
	bool					savePrefsFile(void);
	
	XAP_PrefsScheme *		getNthScheme(UT_uint32 k) const;
	XAP_PrefsScheme *		getNthPluginScheme(UT_uint32 k) const;
	XAP_PrefsScheme *		getScheme(const XML_Char * szSchemeName) const;
	XAP_PrefsScheme *		getPluginScheme(const XML_Char * szSchemeName) const;
	bool					addScheme(XAP_PrefsScheme * pNewScheme);
	bool					addPluginScheme(XAP_PrefsScheme * pNewScheme);
	XAP_PrefsScheme *		getCurrentScheme(bool bCreate = false);
	bool					setCurrentScheme(const XML_Char * szSchemeName);

	bool					getPrefsValue(const XML_Char * szKey, const XML_Char ** pszValue) const;
	bool					getPrefsValue(const UT_String &stKey, UT_String &stValue) const;
	bool					getPrefsValueBool(const XML_Char * szKey, bool * pbValue) const;

	bool					getAutoSavePrefs(void) const;
	void					setAutoSavePrefs(bool bAuto);

	bool					getUseEnvLocale(void) const;
	void					setUseEnvLocale(bool bUse);

	UT_uint32				getMaxRecent(void) const;
	void					setMaxRecent(UT_uint32 k);
	UT_uint32				getRecentCount(void) const;
	const char *			getRecent(UT_uint32 k) const;		// one-based
	void					addRecent(const char * szRecent);
	void					removeRecent(UT_uint32 k);			// one-based

	bool					setGeometry(UT_sint32 posx, UT_sint32 posy, UT_uint32 width, UT_uint32 height, UT_uint32 flags = 0);
	bool					getGeometry(UT_sint32 *posx, UT_sint32 *posy, UT_uint32 *width, UT_uint32 *height, UT_uint32 *flags = 0);
	
	virtual void				fullInit(void) = 0;
	virtual bool				loadBuiltinPrefs(void) = 0;
	virtual const XML_Char *	getBuiltinSchemeName(void) const = 0;
	virtual const char *		getPrefsPathname(void) const = 0;

	void					addListener	  ( PrefsListener pFunc, void *data );
	void					removeListener ( PrefsListener pFunc, void *data = 0 );
	void					startBlockChange();
	void					endBlockChange();

	// a only-to-be-used-by XAP_PrefsScheme::setValue
	void					_markPrefChange	( const XML_Char *szKey );
protected:
	void					_pruneRecent(void);
	XAP_PrefsScheme * 		_getNthScheme(UT_uint32 k, const UT_Vector &vecSchemes) const;

	XAP_App *				m_pApp;
	bool					m_bAutoSavePrefs; /* save on any changes or only when user asks */
	bool					m_bUseEnvLocale; /* use POSIX env vars to set locale */

	UT_Vector				m_vecSchemes;		/* vector of XAP_PrefsScheme */
	UT_Vector				m_vecPluginSchemes;	/* vector of XAP_PrefsScheme */
	XAP_PrefsScheme *		m_currentScheme;
	XAP_PrefsScheme *		m_builtinScheme;

	UT_uint32				m_iMaxRecent;
	UT_Vector				m_vecRecent;		/* vector of (char *) */

	UT_Vector				m_vecPrefsListeners;	/* vectory of struct PrefListnersPair */
	UT_StringPtrMap		    m_ahashChanges;
	bool					m_bInChangeBlock;
	void					_sendPrefsSignal( UT_StringPtrMap *hash );

	typedef struct {
		UT_uint32				m_width, m_height;	/* Default width and height */
		UT_sint32				m_posx, m_posy;		/* Default position */
		UT_uint32				m_flags;			/* What is valid, what is not */
	} Pref_Geometry;
	Pref_Geometry				m_geom;
	

	/* Implementation of UT_XML::Listener */
public:
	void					startElement(const XML_Char *name, const XML_Char **atts);
	void					endElement(const XML_Char *name);
	void					charData(const XML_Char *s, int len);
private:
	void					_startElement_SystemDefaultFile(const XML_Char *name, const XML_Char **atts);
	bool					m_bLoadSystemDefaultFile;

private:
	struct
	{
		bool				m_parserStatus;
		bool				m_bFoundAbiPreferences;
		bool				m_bFoundSelect;
		XML_Char *			m_szSelectedSchemeName;
		bool				m_bFoundRecent;
		bool				m_bFoundGeometry;
	} m_parserState;

	typedef struct					/* used to store the listener/data pairs in the vector  */
	{
		PrefsListener	m_pFunc;
		void			*m_pData;
	} tPrefsListenersPair;
};

//////////////////////////////////////////////////////////////////////////////////////
// The following are the default values and limits for various non-scheme-based
// application-independent preferences.

#define XAP_PREF_DEFAULT_AutoSavePrefs		"1"
#define XAP_PREF_DEFAULT_MaxRecent			"4"
#define XAP_PREF_DEFAULT_UseEnvLocale		"1"

#define XAP_PREF_LIMIT_MaxRecent			9

#endif /* XAP_PREFS_H */
