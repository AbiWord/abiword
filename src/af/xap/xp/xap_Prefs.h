/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
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
#include "ut_string_class.h"
#include "ut_xml.h"

#include "xap_Prefs_SchemeIds.h"

#include <vector>

/* For handling the position of windows on the screen, this sets up preferences */
enum {
	PREF_FLAG_GEOMETRY_POS = 0x1,     // Position is valid
	PREF_FLAG_GEOMETRY_SIZE = 0x2,    // Size is valid
	PREF_FLAG_GEOMETRY_NOUPDATE = 0x4, // User specified, don't update it
	PREF_FLAG_GEOMETRY_MAXIMIZED = 0x8 // Maximize at start
};

class XAP_Prefs;

typedef void (*PrefsListener) (
	XAP_Prefs			*pPrefs,
	UT_StringPtrMap	    *phChanges,
	void				*data
	);

/*****************************************************************
** XAP_PrefsScheme is a complete set of preferences.  it contains
** all the info for a UI styling set.  use XAP_Prefs to switch
** between schemes.
******************************************************************/

enum XAPPrefsLog_Level
{
	Log,
	Warning,
	Error
};

class ABI_EXPORT XAP_FontSettings
{
  public:
	XAP_FontSettings()
		:m_bInclude(false){};

	const std::vector<UT_UTF8String> & getFonts() const
		{return m_vecFonts;}

	void addFont (const char * pFace)
		{
			m_vecFonts.push_back (pFace);
		}

	bool haveFontsToExclude () const {return (!m_bInclude && m_vecFonts.size());}
	bool haveFontsToInclude () const {return (m_bInclude && m_vecFonts.size());}

	bool isOnExcludeList (const char * name) const;

	void setIncludeFlag (bool bInclude) {m_bInclude = bInclude;}
	bool getIncludeFlag () const {return m_bInclude;}

  private:
	std::vector<UT_UTF8String> m_vecFonts;
	bool m_bInclude;
};

class ABI_EXPORT XAP_PrefsScheme
{
public:
	XAP_PrefsScheme(XAP_Prefs *pPrefs, const gchar * szSchemeName);
	~XAP_PrefsScheme(void);

	const gchar *	getSchemeName(void) const;
	bool				setSchemeName(const gchar * szNewSchemeName);
	// The idea of the tick is that some object can cache a preference
	// value if it makes a performance difference.  It should also save
	// a copy of the tick count and the scheme pointer.  If the scheme
	// pointer and the tick count are the same, the cached preference
	// value is current.  If either is changed, the object can refresh
	// its cached value.  The scheme pointer can be different because
	// the preference scheme has changed.  The tick count bumps up once
	// every time any preference value in the scheme is changed.
    UT_uint32			getTickCount() {return m_uTick;}

	bool				setValue(const gchar * szKey, const gchar * szValue);
	bool				setValueBool(const gchar * szKey, bool bValue);
	bool				setValueInt(const gchar * szKey, const int nValue);

	// the get*Value*() functions return the answer in the last
	// argument; they return error information as the function
	// return value.
	bool				getValue(const gchar * szKey, const gchar ** pszValue) const;
	bool				getValue(const UT_String &szKey, UT_String &szValue) const;
	bool				getValue(const std::string &szKey, std::string &szValue) const;
	bool                getValue(const gchar* szKey, std::string &szValue) const;
	bool				getValueInt(const gchar * szKey, int& nValue) const;
	bool				getValueBool(const gchar * szKey, bool * pbValue) const;
	bool				getNthValue(UT_uint32 k, const gchar ** pszKey, const gchar ** pszValue);

protected:
	gchar *			m_szName;
	UT_GenericStringMap<gchar*> m_hash;
	UT_GenericVector<const gchar*> m_sortedKeys;
	bool				m_bValidSortedKeys;
	XAP_Prefs *			m_pPrefs;
	UT_uint32			m_uTick;   // ticks up every time setValue() or setValueBool() is called
};

/*****************************************************************/
/*****************************************************************/

class ABI_EXPORT XAP_Prefs : public UT_XML::Listener
{
public:
	XAP_Prefs();
	virtual ~XAP_Prefs(void);

	bool					loadPrefsFile(void);
	bool					loadSystemDefaultPrefsFile(const char * szSystemDefaultPrefsPathname);
	bool					savePrefsFile(void);

	XAP_PrefsScheme *		getNthScheme(UT_uint32 k) const;
	XAP_PrefsScheme *		getNthPluginScheme(UT_uint32 k) const;
	XAP_PrefsScheme *		getScheme(const gchar * szSchemeName) const;
	XAP_PrefsScheme *		getPluginScheme(const gchar * szSchemeName) const;
	bool					addScheme(XAP_PrefsScheme * pNewScheme);
	bool					addPluginScheme(XAP_PrefsScheme * pNewScheme);
	XAP_PrefsScheme *		getCurrentScheme(bool bCreate = false);
	bool					setCurrentScheme(const gchar * szSchemeName);

	bool					getPrefsValue(const gchar * szKey, const gchar ** pszValue, bool bAllowBuiltin = true) const;
	bool					getPrefsValue(const UT_String &stKey, UT_String &stValue, bool bAllowBuiltin = true) const;
	bool					getPrefsValue(const gchar* szKey, std::string &stValue, bool bAllowBuiltin = true) const;
	bool					getPrefsValueBool(const gchar * szKey, bool * pbValue, bool bAllowBuiltin = true) const;
	bool					getPrefsValueInt(const gchar * szKey, int& nValue, bool bAllowBuiltin = true) const;

	bool					getAutoSavePrefs(void) const;
	void					setAutoSavePrefs(bool bAuto);

	bool					getUseEnvLocale(void) const;
	void					setUseEnvLocale(bool bUse);

	UT_sint32				getMaxRecent(void) const;
	void					setMaxRecent(UT_sint32 k);
	UT_sint32				getRecentCount(void) const;
	const char *			getRecent(UT_sint32 k) const;		// one-based
	void					addRecent(const char * szRecent);
	void					removeRecent(UT_sint32 k);			// one-based
	void                    setIgnoreNextRecent(void)
		{ m_bIgnoreThisOne = true;}
	bool                    isIgnoreRecent(void)
	{ return m_bIgnoreThisOne;}

	bool					setGeometry(UT_sint32 posx, UT_sint32 posy, UT_uint32 width, UT_uint32 height, UT_uint32 flags = 0);
	bool					getGeometry(UT_sint32 *posx, UT_sint32 *posy, UT_uint32 *width, UT_uint32 *height, UT_uint32 *flags = 0);

	virtual void			fullInit(void) = 0;
	virtual bool			loadBuiltinPrefs(void) = 0;
	virtual const gchar *	getBuiltinSchemeName(void) const = 0;
	virtual const char *	getPrefsPathname(void) const = 0;

	void					addListener	  ( PrefsListener pFunc, void *data );
	void					removeListener ( PrefsListener pFunc, void *data = 0 );
	void					startBlockChange();
	void					endBlockChange();

	void                    log(const char * where, const char * what, XAPPrefsLog_Level level = Log);

	XAP_FontSettings &      getFontSettings () {return m_fonts;}

	// a only-to-be-used-by XAP_PrefsScheme::setValue
	void					_markPrefChange	( const gchar *szKey );
protected:
	void					_pruneRecent(void);
	XAP_PrefsScheme * 		_getNthScheme(UT_uint32 k,
										  const UT_GenericVector<XAP_PrefsScheme *> &vecSchemes) const;

	bool					m_bAutoSavePrefs; /* save on any changes or only when user asks */
	bool					m_bUseEnvLocale; /* use POSIX env vars to set locale */

	UT_GenericVector<XAP_PrefsScheme *>	m_vecSchemes;		/* vector of XAP_PrefsScheme */
	UT_GenericVector<XAP_PrefsScheme *>	m_vecPluginSchemes;	/* vector of XAP_PrefsScheme */
	XAP_PrefsScheme *		m_currentScheme;
	XAP_PrefsScheme *		m_builtinScheme;

	UT_sint32				m_iMaxRecent;
	UT_GenericVector<char*>	m_vecRecent;		/* vector of (char *) */
	UT_GenericVector<UT_UTF8String *> m_vecLog; /* vector of UT_UTF8String */

	XAP_FontSettings        m_fonts;

	typedef struct					/* used to store the listener/data pairs in the vector  */
	{
		PrefsListener	m_pFunc;
		void			*m_pData;
	} tPrefsListenersPair;

	UT_GenericVector<tPrefsListenersPair *>	m_vecPrefsListeners;	/* vectory of struct PrefListnersPair */
	UT_StringPtrMap		m_ahashChanges;
	bool				m_bInChangeBlock;
	void				_sendPrefsSignal( UT_StringPtrMap *hash );

	typedef struct {
		UT_uint32		m_width, m_height;	/* Default width and height */
		UT_sint32		m_posx, m_posy;		/* Default position */
		UT_uint32		m_flags;			/* What is valid, what is not */
	} Pref_Geometry;

	Pref_Geometry		m_geom;


	/* Implementation of UT_XML::Listener */
public:
	void				startElement(const gchar *name, const gchar **atts);
	void				endElement(const gchar *name);
	void				charData(const gchar *s, int len);
private:
	void				_startElement_SystemDefaultFile(const gchar *name,
														const gchar **atts);
	bool				m_bLoadSystemDefaultFile;
    bool                m_bIgnoreThisOne;
private:
	struct
	{
		bool			m_parserStatus;
		bool			m_bFoundAbiPreferences;
		bool			m_bFoundSelect;
		gchar *			m_szSelectedSchemeName;
		bool			m_bFoundRecent;
		bool			m_bFoundGeometry;
		bool            m_bFoundFonts;
	} m_parserState;

};

//////////////////////////////////////////////////////////////////////////////////////
// The following are the default values and limits for various non-scheme-based
// application-independent preferences.

#define XAP_PREF_DEFAULT_AutoSavePrefs		"1"
#define XAP_PREF_DEFAULT_MaxRecent			"9"
#define XAP_PREF_DEFAULT_UseEnvLocale		"1"

#define XAP_PREF_LIMIT_MaxRecent			9

#endif /* XAP_PREFS_H */
