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

#include "ut_types.h"
#include "ut_vector.h"
#include "ut_alphahash.h"
#include "ut_string.h"
#include "xmlparse.h"
#include "xap_App.h"
#include "xap_Prefs_SchemeIds.h"

/*****************************************************************
** XAP_PrefsScheme is a complete set of preferences.  it contains
** all the info for a UI styling set.  use XAP_Prefs to switch
** between schemes.
******************************************************************/

class XAP_PrefsScheme
{
public:
	XAP_PrefsScheme(const XML_Char * szSchemeName);
	~XAP_PrefsScheme(void);

	const XML_Char *	getSchemeName(void) const;
	UT_Bool				setSchemeName(const XML_Char * szNewSchemeName);
	
	UT_Bool				setValue(const XML_Char * szKey, const XML_Char * szValue);
	UT_Bool				getValue(const XML_Char * szKey, const XML_Char ** pszValue) const;
	UT_Bool				getNthValue(UT_uint32 k, const XML_Char ** pszKey, const XML_Char ** pszValue) const;
	
protected:
	XML_Char *			m_szName;
	UT_AlphaHashTable	m_hash;
};

/*****************************************************************/
/*****************************************************************/

class XAP_Prefs
{
public:
	XAP_Prefs(XAP_App * pApp);
	virtual ~XAP_Prefs(void);

	UT_Bool					loadPrefsFile(void);
	UT_Bool					savePrefsFile(void);
	
	XAP_PrefsScheme *		getNthScheme(UT_uint32 k) const;
	XAP_PrefsScheme *		getScheme(const XML_Char * szSchemeName) const;
	UT_Bool					addScheme(XAP_PrefsScheme * pNewScheme);
	XAP_PrefsScheme *		getCurrentScheme(void) const;
	UT_Bool					setCurrentScheme(const XML_Char * szSchemeName);

	UT_Bool					getPrefsValue(const XML_Char * szKey, const XML_Char ** pszValue) const;

	UT_Bool					getAutoSavePrefs(void) const;
	void					setAutoSavePrefs(UT_Bool bAuto);

	UT_uint32				getMaxRecent(void) const;
	void					setMaxRecent(UT_uint32 k);
	UT_uint32				getRecentCount(void) const;
	const char *			getRecent(UT_uint32 k) const;		// one-based
	void					addRecent(const char * szRecent);
	void					removeRecent(UT_uint32 k);			// one-based
	
	virtual UT_Bool				loadBuiltinPrefs(void) = 0;
	virtual const XML_Char *	getBuiltinSchemeName(void) const = 0;
	virtual const char *		getPrefsPathname(void) const = 0;

protected:
	void					_pruneRecent(void);

	XAP_App *				m_pApp;
	UT_Bool					m_bAutoSavePrefs; /* save on any changes or only when user asks */

	UT_Vector				m_vecSchemes;		/* vector of XAP_PrefsScheme */
	XAP_PrefsScheme *		m_currentScheme;
	XAP_PrefsScheme *		m_builtinScheme;

	UT_uint32				m_iMaxRecent;
	UT_Vector				m_vecRecent;		/* vector of (char *) */

public:						/* these 3 are needed by the XML parser interface */
	void					_startElement(const XML_Char *name, const XML_Char **atts);
	void					_endElement(const XML_Char *name);
	void					_charData(const XML_Char *s, int len);

private:
	struct
	{
		UT_Bool				m_parserStatus;
		UT_Bool				m_bFoundAbiPreferences;
		UT_Bool				m_bFoundSelect;
		XML_Char *			m_szSelectedSchemeName;
		UT_Bool				m_bFoundRecent;
	} m_parserState;
};

//////////////////////////////////////////////////////////////////////////////////////
// The following are the default values and limits for various non-scheme-based
// application-independent preferences.

#define XAP_PREF_DEFAULT_AutoSavePrefs		"1" /* TODO this is true for testing, set it to false later. */
#define XAP_PREF_DEFAULT_MaxRecent			"4"

#define XAP_PREF_LIMIT_MaxRecent			9

#endif /* XAP_PREFS_H */
