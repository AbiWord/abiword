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

	UT_Bool					getAutoSave(void) const;
	void					setAutoSave(UT_Bool bAuto);
	
	virtual UT_Bool				loadBuiltinPrefs(void) = 0;
	virtual const XML_Char *	getBuiltinSchemeName(void) const = 0;
	virtual const char *		getPrefsPathname(void) const = 0;

protected:
	XAP_App *				m_pApp;
	UT_Bool					m_bAutoSave; /* save on any changes or only when user asks */

	UT_Vector				m_vecSchemes;		/* vector of XAP_PrefsScheme */
	XAP_PrefsScheme *		m_currentScheme;

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
	} m_parserState;
};

// The following are the set of application-independent preference keys.
// Additional keys may be defined by the application.

#define XAP_PREF_KEY_KeyBindings			"KeyBindings"
#define XAP_PREF_KEY_MenuLayout				"MenuLayouts"
#define XAP_PREF_KEY_MenuLabelSet			"MenuLabelSet"
#define XAP_PREF_KEY_ToolbarAppearance		"ToolbarAppearance"
#define XAP_PREF_KEY_ToolbarLabelSet		"ToolbarLabelSet"
#define XAP_PREF_KEY_ToolbarLayouts			"ToolbarLayouts"

// The following are the set of default values for the above set of keys.

#define XAP_PREF_DEFAULT_KeyBindings		"default"
#define XAP_PREF_DEFAULT_MenuLabelSet		"EnUS"
#define XAP_PREF_DEFAULT_MenuLayout			"Main"
#define XAP_PREF_DEFAULT_ToolbarAppearance	"icon"
#define XAP_PREF_DEFAULT_ToolbarLabelSet	"EnUS"
#define XAP_PREF_DEFAULT_ToolbarLayouts		"FileEditOps FormatOps"

#endif /* XAP_PREFS_H */
