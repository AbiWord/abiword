/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode:t; -*- */
/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (c) 2020 Hubert Figuière
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

#pragma once

#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_xml.h"

#include "xap_Prefs_SchemeIds.h"

#include <list>
#include <vector>
#include <set>

/* For handling the position of windows on the screen, this sets up preferences */
enum {
	PREF_FLAG_GEOMETRY_POS = 0x1,     // Position is valid
	PREF_FLAG_GEOMETRY_SIZE = 0x2,    // Size is valid
	PREF_FLAG_GEOMETRY_NOUPDATE = 0x4, // User specified, don't update it
	PREF_FLAG_GEOMETRY_MAXIMIZED = 0x8 // Maximize at start
};

class XAP_Prefs;

typedef std::set<std::string> XAP_PrefsChangeSet;

typedef void (*PrefsListener) (
	XAP_Prefs			*pPrefs,
	const XAP_PrefsChangeSet*	    phChanges,
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
		: m_bInclude(false)
	{}

	const std::vector<std::string> & getFonts() const
		{return m_vecFonts;}

	void addFont (const char * pFace)
		{
			m_vecFonts.push_back(pFace);
		}

	bool haveFontsToExclude() const {return (!m_bInclude && m_vecFonts.size());}
	bool haveFontsToInclude() const {return (m_bInclude && m_vecFonts.size());}

	bool isOnExcludeList (const char * name) const;

	void setIncludeFlag(bool bInclude) {m_bInclude = bInclude;}
	bool getIncludeFlag() const {return m_bInclude;}

  private:
	std::vector<std::string> m_vecFonts;
	bool m_bInclude;
};

class ABI_EXPORT XAP_PrefsScheme
{
public:
	// map because we want the keys to be sorted.
	typedef std::map<std::string, std::string> MapType;

	XAP_PrefsScheme(XAP_Prefs *pPrefs, const gchar * szSchemeName);
	~XAP_PrefsScheme(void);

	const std::string& getSchemeName(void) const;
	void setSchemeName(const gchar * szNewSchemeName);

	void setValue(const std::string& key, const std::string& value);
	void setValueBool(const std::string& key, bool bValue);
	void setValueInt(const std::string& key, int nValue);

	// the get*Value*() functions return the answer in the last
	// argument; they return error information as the function
	// return value.
	bool getValue(const std::string& key, std::string &value) const;
	bool getValueInt(const std::string& key, int& nValue) const;
	bool getValueBool(const std::string& key, bool& pbValue) const;

	MapType::const_iterator begin() const
	{
		return m_hash.begin();
	}
	MapType::const_iterator end() const
	{
		return m_hash.end();
	}

protected:
	std::string	m_szName;
	MapType m_hash;
	XAP_Prefs* m_pPrefs;
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
	void addScheme(XAP_PrefsScheme* pNewScheme);
	void addPluginScheme(XAP_PrefsScheme* pNewScheme);
	XAP_PrefsScheme* getCurrentScheme() const;
	XAP_PrefsScheme* getCurrentScheme(bool bCreate);
	bool					setCurrentScheme(const gchar * szSchemeName);

	bool getPrefsValue(const std::string& key, std::string& value, bool bAllowBuiltin = true) const;
	bool getPrefsValueBool(const std::string& key, bool& pbValue, bool bAllowBuiltin = true) const;
	bool getPrefsValueInt(const std::string& key, int& nValue, bool bAllowBuiltin = true) const;

	bool					getAutoSavePrefs(void) const;
	void					setAutoSavePrefs(bool bAuto);

	bool					getUseEnvLocale(void) const;
	void					setUseEnvLocale(bool bUse);

	UT_uint32 getMaxRecent(void) const;
	void setMaxRecent(UT_uint32 k);
	UT_uint32 getRecentCount(void) const;
	const char* getRecent(UT_uint32 k) const;		// one-based
	void addRecent(const char * szRecent);
	void removeRecent(UT_uint32 k);			// one-based
	void                    setIgnoreNextRecent(void)
		{ m_bIgnoreThisOne = true;}
	bool                    isIgnoreRecent(void)
	{ return m_bIgnoreThisOne;}

	bool					setGeometry(UT_sint32 posx, UT_sint32 posy, UT_uint32 width, UT_uint32 height, UT_uint32 flags = 0);
	bool					getGeometry(UT_sint32 *posx, UT_sint32 *posy, UT_uint32 *width, UT_uint32 *height, UT_uint32 *flags = nullptr);

	virtual void			fullInit(void) = 0;
	virtual bool			loadBuiltinPrefs(void) = 0;
	virtual const gchar *	getBuiltinSchemeName(void) const = 0;
	virtual const char *	getPrefsPathname(void) const = 0;

	void					addListener	  ( PrefsListener pFunc, void *data );
	void					removeListener(PrefsListener pFunc, void *data = nullptr);
	void					startBlockChange();
	void					endBlockChange();

	void                    log(const char * where, const char * what, XAPPrefsLog_Level level = Log);

	XAP_FontSettings &      getFontSettings () {return m_fonts;}

	// a only-to-be-used-by XAP_PrefsScheme::setValue
	void _markPrefChange(const std::string& key);
protected:
	void					_pruneRecent(void);
	XAP_PrefsScheme * 		_getNthScheme(UT_uint32 k,
										  const std::vector<XAP_PrefsScheme *> &vecSchemes) const;

	bool					m_bAutoSavePrefs; /* save on any changes or only when user asks */
	bool					m_bUseEnvLocale; /* use POSIX env vars to set locale */

	std::vector<XAP_PrefsScheme*> m_vecSchemes;
	std::vector<XAP_PrefsScheme*> m_vecPluginSchemes;
	XAP_PrefsScheme *		m_currentScheme;
	XAP_PrefsScheme *		m_builtinScheme;

	UT_uint32 m_maxRecent;
	std::vector<std::string> m_vecRecent;
	std::vector<std::string> m_vecLog;

	XAP_FontSettings        m_fonts;

	/* used to store the listener/data pairs in the vector  */
	struct tPrefsListenersPair
	{
		PrefsListener	m_pFunc;
		void			*m_pData;
		tPrefsListenersPair(PrefsListener f, void* d)
			: m_pFunc(f)
			, m_pData(d)
			{}
		tPrefsListenersPair()
			: m_pFunc(nullptr)
			, m_pData(nullptr)
			{}
	};

	typedef std::list<tPrefsListenersPair> PrefsListenersList;
	PrefsListenersList m_prefsListeners;	/* list of struct PrefListnersPair */
	XAP_PrefsChangeSet	m_ahashChanges;
	bool				m_bInChangeBlock;
	void				_sendPrefsSignal(const XAP_PrefsChangeSet& hash );

	struct Pref_Geometry {
		UT_uint32		m_width, m_height;	/* Default width and height */
		UT_sint32		m_posx, m_posy;		/* Default position */
		UT_uint32		m_flags;			/* What is valid, what is not */
	};

	Pref_Geometry		m_geom;


	/* Implementation of UT_XML::Listener */
public:
	virtual void startElement(const gchar *name, const gchar **atts) override;
	virtual void endElement(const gchar *name) override;
	virtual void charData(const gchar *s, int len) override;
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
