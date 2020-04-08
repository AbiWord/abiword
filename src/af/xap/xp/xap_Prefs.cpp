/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; ident-tabs-mode:t; -*- */
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

// TODO add code to do an auto save anytime anything is changed.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <vector>

#include "ut_debugmsg.h"
#include "ut_growbuf.h"
#include "ut_std_string.h"
#include "ut_std_vector.h"
#include "ut_string_class.h"
#ifdef _WIN32
#include <ut_Win32LocaleString.h>
#endif
#include "ut_go_file.h"
#include "xap_Prefs.h"
#include "xap_App.h"

struct xmlToIdMapping {
	const char *m_name;
	int m_type;
};

enum
{
	TT_ABIPREFERENCES,
	TT_GEOMETRY,
	TT_FACE,
	TT_FONTS,
	TT_LOG,
	TT_PLUGIN,
	TT_RECENT,
	TT_SCHEME,
	TT_SELECT
};

// keep these in alphabetical order
static struct xmlToIdMapping s_Tokens[] =
{
	{ "AbiPreferences",		TT_ABIPREFERENCES },
	{ "Face",               TT_FACE },
	{ "Fonts",              TT_FONTS },
	{ "Geometry",			TT_GEOMETRY },
	{ "Log",                TT_LOG},
	{ "Plugin",				TT_PLUGIN },
	{ "Recent",				TT_RECENT },
	{ "Scheme",				TT_SCHEME },
	{ "Select",				TT_SELECT }
};

/*****************************************************************/

XAP_PrefsScheme::XAP_PrefsScheme( XAP_Prefs *pPrefs, const gchar * szSchemeName)
	: m_szName(szSchemeName ? szSchemeName : "")
    , m_pPrefs(pPrefs)
{
}

XAP_PrefsScheme::~XAP_PrefsScheme(void)
{
}

const std::string& XAP_PrefsScheme::getSchemeName(void) const
{
	return m_szName;
}

void XAP_PrefsScheme::setSchemeName(const gchar* szNewSchemeName)
{
	m_szName = (szNewSchemeName ? szNewSchemeName : "");
}

void XAP_PrefsScheme::setValue(const std::string& key, const std::string& value)
{
	auto iter = m_hash.find(key);
	if (iter == m_hash.end()) {
		m_hash.insert({key, value});
	} else {
		if (iter->second == value) {
			return;
		}
		iter->second = value;
	}
	m_pPrefs->_markPrefChange(key);
}

void XAP_PrefsScheme::setValueBool(const std::string& key, bool bValue)
{
	setValue(key, (bValue) ? "1" : "0");
}

void XAP_PrefsScheme::setValueInt(const std::string& key, int nValue)
{
	gchar szValue[32];
	snprintf(szValue, 32, "%d", nValue);
	setValue(key, szValue);
}

bool XAP_PrefsScheme::getValue(const std::string& key, std::string &value) const
{
    auto iter = m_hash.find(key);
    if (iter == m_hash.end()) {
        return false;
    }

    value = iter->second;
	return true;
}

bool XAP_PrefsScheme::getValueInt(const std::string& key, int& nValue) const
{
    std::string value;
	if (!getValue(key, value)) {
		return false;
    }

	nValue = atoi(value.c_str());
	return true;
}

bool XAP_PrefsScheme::getValueBool(const std::string& key, bool& bValue) const
{
	bValue = false;				// assume something

	std::string value;
	if (!getValue(key, value) || value.empty()) {
		return false;
    }

	switch (value[0])
	{
	case '1':
	case 't':
	case 'T':
	case 'y':
	case 'Y':
		bValue = true;
		return true;

	default:
		bValue = false;
		return true;
	}
}

/*****************************************************************/

bool XAP_Prefs::getAutoSavePrefs(void) const
{
	return m_bAutoSavePrefs;
}

void XAP_Prefs::setAutoSavePrefs(bool bAuto)
{
	m_bAutoSavePrefs = bAuto;

	// TODO if turning autosave on, we should do a save now....
	// TODO if was on and turning off, should we save it now ??
}

/*****************************************************************/

bool XAP_Prefs::getUseEnvLocale(void) const
{
	return m_bUseEnvLocale;
}

void XAP_Prefs::setUseEnvLocale(bool bUse)
{
	m_bUseEnvLocale = bUse;
}

/*****************************************************************/

UT_uint32 XAP_Prefs::getMaxRecent(void) const
{
	return m_maxRecent;
}

void XAP_Prefs::setMaxRecent(UT_uint32 k)
{
	UT_ASSERT_HARMLESS(k <= XAP_PREF_LIMIT_MaxRecent);

	if (k > XAP_PREF_LIMIT_MaxRecent) {
		k = XAP_PREF_LIMIT_MaxRecent;
	}

	m_maxRecent = k;
}

UT_uint32 XAP_Prefs::getRecentCount(void) const
{
	return m_vecRecent.size();
}

const char* XAP_Prefs::getRecent(UT_uint32 k) const
{
	// NB: k is one-based
	UT_return_val_if_fail(k > 0, nullptr);
	UT_return_val_if_fail(k <= m_maxRecent, nullptr);

	if (k <= m_vecRecent.size()) {
		return m_vecRecent.at(k - 1).c_str();
	}

	return nullptr;
}

void XAP_Prefs::addRecent(const char * szRecent)
{
	UT_return_if_fail(szRecent);

	if (m_maxRecent == 0) {
		return;		// NOOP
	}

	if(m_bIgnoreThisOne) {
		m_bIgnoreThisOne = false;
		return;
	}
	// was it already here?
	for (auto iter = m_vecRecent.begin(); iter != m_vecRecent.end(); ++iter) {
		if (*iter == szRecent) {
			// yep, we're gonna move it up
			m_vecRecent.erase(iter);
			break;
		}
	}

	m_vecRecent.insert(m_vecRecent.begin(), szRecent);
	_pruneRecent();
}

void XAP_Prefs::removeRecent(UT_uint32 k)
{
	UT_return_if_fail(k > 0);
	UT_return_if_fail(k <= getRecentCount());

	m_vecRecent.erase(m_vecRecent.begin() + (k - 1));
}

void XAP_Prefs::_pruneRecent(void)
{
	UT_uint32 count = getRecentCount();

	if (m_maxRecent == 0) {
		m_vecRecent.clear();
	} else if (count > m_maxRecent) {
		// prune entries past m_maxRecent
		for (UT_uint32 i = count; i > m_maxRecent; i--) {
			removeRecent(i);
		}
	}
}
/*****************************************************************/

bool XAP_Prefs::setGeometry(UT_sint32 posx, UT_sint32 posy, UT_uint32 width, UT_uint32 height, UT_uint32 flags) 
{
	m_parserState.m_bFoundGeometry = true;
	m_geom.m_width = width;
	m_geom.m_height = height;
	m_geom.m_posx = posx;
	m_geom.m_posy = posy;
	m_geom.m_flags = flags;

	//For now we turn on the autosave of prefs so that we can save this setting ... is this bad?
	setAutoSavePrefs(true);	
	
	return true;
}

bool XAP_Prefs::getGeometry(UT_sint32 *posx, UT_sint32 *posy, UT_uint32 *width, UT_uint32 *height, UT_uint32 *flags)
{
	if (m_parserState.m_bFoundGeometry == false) {
		return false;
	}
	if (width) { 
		*width = m_geom.m_width; 
	}
	if (height) {
		*height = m_geom.m_height;
	}
	if (posx) {
		*posx = m_geom.m_posx;
	}
	if (posy) {
		*posy = m_geom.m_posy;
	}
	if (flags) {
		*flags = m_geom.m_flags;
	}
	return true;
}

void XAP_Prefs::log(const char * where, const char * what, XAPPrefsLog_Level level)
{
	UT_return_if_fail(where && what);

	time_t t = time(NULL);

	// we are inserting the log entries as comments, so we have to
	// ensure we have no "--" in there (it anoys the parser)
	UT_UTF8String sWhere(where);
	UT_UTF8String sWhat(what);
	UT_UTF8String sDashdash = "--";
	UT_UTF8String sDash = "-";

	while(strstr(sWhat.utf8_str(), "--")) {
		sWhat.escape(sDashdash, sDash);
	}

	while(strstr(sWhere.utf8_str(), "--")) {
		sWhere.escape(sDashdash, sDash);
	}

	char b[50];
	strftime(b, 50, "<!-- [%c] ", localtime(&t));
	std::string s(b);

	switch(level) {
	case Warning:
		s += "warning: ";
		break;

	case Error:
		s += "error:   ";
		break;
	case Log:
	default:
		s += "message: ";
	}

	sWhere.escapeXML();
	sWhat.escapeXML();

	s += sWhere.utf8_str();
	s += " - ";
	s += sWhat.utf8_str();
	s += " -->";

	m_vecLog.push_back(s);
}


/*****************************************************************/

XAP_Prefs::XAP_Prefs()
	: m_bAutoSavePrefs(atoi(XAP_PREF_DEFAULT_AutoSavePrefs) ? true : false)
	, m_bUseEnvLocale(atoi(XAP_PREF_DEFAULT_UseEnvLocale) ? true : false)
	, m_currentScheme(nullptr)
	, m_builtinScheme(nullptr)
	, m_maxRecent(atoi(XAP_PREF_DEFAULT_MaxRecent))
	, m_bInChangeBlock(false)
	, m_bIgnoreThisOne(false)
	, m_geom({0, 0, 0, 0, 0})
{
	// NOTE: since constructors cannot report g_try_malloc
	// NOTE: failures (and since it is virtual back
	// NOTE: to the application), our creator must call
	// NOTE: loadBuiltinPrefs().

	// NOTE: we do not initialize the values in m_parserState
	// NOTE: since they are only used by the parser, it can
	// NOTE: initialize them.
}

XAP_Prefs::~XAP_Prefs(void)
{
	UT_std_vector_purgeall(m_vecSchemes);
	UT_std_vector_purgeall(m_vecPluginSchemes);
}

/*****************************************************************/

XAP_PrefsScheme* XAP_Prefs::_getNthScheme(UT_uint32 k, const std::vector<XAP_PrefsScheme *> &vecSchemes) const
{
	UT_uint32 kLimit = vecSchemes.size();
	if (k < kLimit) {
		return vecSchemes.at(k);
	}
	return nullptr;
}

XAP_PrefsScheme* XAP_Prefs::getNthScheme(UT_uint32 k) const
{
	return _getNthScheme(k, m_vecSchemes);
}

XAP_PrefsScheme* XAP_Prefs::getNthPluginScheme(UT_uint32 k) const
{
	return _getNthScheme(k, m_vecPluginSchemes);
}

XAP_PrefsScheme* XAP_Prefs::getScheme(const gchar * szSchemeName) const
{
	UT_uint32 kLimit = m_vecSchemes.size();

	for (UT_uint32 k = 0; k < kLimit; k++) {
		XAP_PrefsScheme * p = getNthScheme(k);
		if (!p) {
			UT_ASSERT_HARMLESS(p);
			continue;
		}
		if (p->getSchemeName() == szSchemeName) {
			return p;
		}
	}

	return NULL;
}

XAP_PrefsScheme* XAP_Prefs::getPluginScheme(const gchar * szSchemeName) const
{
	UT_uint32 kLimit = m_vecPluginSchemes.size();

	for (UT_uint32 k = 0; k < kLimit; k++) {
		XAP_PrefsScheme * p = getNthPluginScheme(k);
		if (!p) {
			UT_ASSERT_HARMLESS(p);
			continue;
		}
		if (p->getSchemeName() == szSchemeName) {
			return p;
		}
	}

	return NULL;
}

void XAP_Prefs::addScheme(XAP_PrefsScheme * pNewScheme)
{
	const gchar * szBuiltinSchemeName = getBuiltinSchemeName();
	const std::string& thisSchemeName = pNewScheme->getSchemeName();

	if (thisSchemeName == szBuiltinSchemeName) {
		UT_ASSERT(m_builtinScheme == NULL);
		m_builtinScheme = pNewScheme;
	}

	m_vecSchemes.push_back(pNewScheme);
}

void XAP_Prefs::addPluginScheme(XAP_PrefsScheme * pNewScheme)
{
	m_vecPluginSchemes.push_back(pNewScheme);
}

XAP_PrefsScheme * XAP_Prefs::getCurrentScheme() const
{
	return m_currentScheme;
}

XAP_PrefsScheme * XAP_Prefs::getCurrentScheme(bool bCreate)
{
	if (bCreate) {
		// the builtin scheme is not updatable,
		// so we may need to create one that is
		if (m_currentScheme->getSchemeName() == "_builtin_") {

			const gchar* new_name = "_custom_";

			if (setCurrentScheme(new_name))	{
				// unused _custom_ scheme is lying around, so recycle it
				UT_ASSERT_HARMLESS(UT_TODO);

				// HYP: reset the current scheme's hash table contents?
				// ALT: replace the existing scheme with new empty one
			} else {
				// we need to create it
				XAP_PrefsScheme * pNewScheme = new XAP_PrefsScheme(this, new_name);
				UT_ASSERT(pNewScheme);
				addScheme(pNewScheme);
				setCurrentScheme(new_name);
			}
		}
	}

	return m_currentScheme;
}

bool XAP_Prefs::setCurrentScheme(const gchar * szSchemeName)
{
	// set the current scheme.

	// TODO notify the application that the scheme has changed

	XAP_PrefsScheme * p = getScheme(szSchemeName);
	if (!p)
		return false;

	UT_DEBUGMSG(("Preferences::setCurrentScheme [%s].\n",szSchemeName));
	
	m_currentScheme = p;
	return true;
}

/*****************************************************************/
static const gchar DEBUG_PREFIX[] = "DeBuG";  // case insensitive
static const gchar NO_PREF_VALUE[] = "";

bool XAP_Prefs::getPrefsValue(const std::string& key, std::string& value, bool bAllowBuiltin) const
{
	// a convenient routine to get a name/value pair from the current scheme
	UT_return_val_if_fail(m_currentScheme, false);

	if (m_currentScheme->getValue(key, value)) {
		return true;
	}
	if (bAllowBuiltin && m_builtinScheme->getValue(key, value)) {
		return true;
	}
	// It is legal for there to be arbitrary preference tags that start with
	// "Debug", and Abi apps won't choke.  The idea is that developers can use
	// these to selectively trigger development-time behaviors.
	if (g_ascii_strncasecmp(key.c_str(), DEBUG_PREFIX, sizeof(DEBUG_PREFIX) - 1) == 0) {
		value = NO_PREF_VALUE;
		return true;
	}

	return false;
}

bool XAP_Prefs::getPrefsValueBool(const std::string& key, bool& bValue, bool bAllowBuiltin) const
{
	// a convenient routine to get a name/value pair from the current scheme
	UT_return_val_if_fail(m_currentScheme, false);

	if (m_currentScheme->getValueBool(key, bValue)) {
		return true;
	}
	if (bAllowBuiltin && m_builtinScheme->getValueBool(key, bValue)) {
		return true;
	}
	// It is legal for there to be arbitrary preference tags that start with
	// "Debug", and Abi apps won't choke.  The idea is that developers can use
	// these to selectively trigger development-time behaviors.
	if (g_ascii_strncasecmp(key.c_str(), DEBUG_PREFIX, sizeof(DEBUG_PREFIX) - 1) == 0) {
		bValue = false;
		return true;
	}

	return false;
}

bool XAP_Prefs::getPrefsValueInt(const std::string &key, int& nValue, bool bAllowBuiltin) const
{
	// a convenient routine to get a name/value pair from the current scheme
	UT_return_val_if_fail(m_currentScheme, false);

	if (m_currentScheme->getValueInt(key, nValue)) {
		return true;
	}
	if (bAllowBuiltin && m_builtinScheme->getValueInt(key, nValue)) {
		return true;
	}
	// It is legal for there to be arbitrary preference tags that start with
	// "Debug", and Abi apps won't choke.  The idea is that developers can use
	// these to selectively trigger development-time behaviors.
	if (g_ascii_strncasecmp(key.c_str(), DEBUG_PREFIX, sizeof(DEBUG_PREFIX) - 1) == 0) {
		nValue = -1;
		return true;
	}

	return false;
}


static int n_compare (const char *name, const xmlToIdMapping *id)
{
	return strcmp (name, id->m_name);
}


/*****************************************************************/

void XAP_Prefs::startElement(const gchar *name, const gchar **atts)
{
	if (m_bLoadSystemDefaultFile) /* redirection - used to happen earlier */
	{
		_startElement_SystemDefaultFile (name, atts);
		return;
	}
	UT_DEBUGMSG(("Looking for %s \n",name));
	XAP_PrefsScheme * pNewScheme = NULL; // must be freed
	
	if (!m_parserState.m_parserStatus)		// eat if already had an error
		return;

	xmlToIdMapping * id = NULL;
	id = static_cast<xmlToIdMapping *>(bsearch (static_cast<const void*>(name), static_cast<const void*>(s_Tokens),
									sizeof(s_Tokens)/sizeof(xmlToIdMapping),
									sizeof (xmlToIdMapping),
									(int (*)(const void*, const void*))
									n_compare));
	if (!id)
	{
		UT_DEBUGMSG(("Didin't find it! Abort! \n"));
		UT_ASSERT_HARMLESS(0);
		return;
	}
	switch (id->m_type)
	{
		case TT_LOG: // ignore
			break;

		case TT_FONTS:
		{
			m_parserState.m_bFoundFonts = true;
			const gchar ** a = atts;
			
			while (a && *a)
			{
				if (!strcmp (a[0], "include"))
				{
					if (!strcmp (a[1], "1") || !strcmp (a[1], "true"))
						m_fonts.setIncludeFlag(true);
					else
						m_fonts.setIncludeFlag(false);
				}
				else
				{
					UT_DEBUGMSG(("Preferences: unknown attribute [%s] "
								 "for <Fontface>\n",
								 a[0]));
				}
				
				a += 2;
			}
		}
		break;
			
		case TT_FACE:
		{
			if (!m_parserState.m_bFoundFonts)
			{
				UT_ASSERT_HARMLESS (UT_SHOULD_NOT_HAPPEN);
				break;
			}
			
			const gchar ** a = atts;
			const gchar * pName = NULL;
			
			while (a && *a)
			{
				if (!strcmp (a[0], "name"))
				{
					pName = a[1];
				}
				else
				{
					UT_DEBUGMSG(("Preferences: unknown attribute [%s] "
								 "for <Fontface>\n",
								 a[0]));
				}
				
				a += 2;
			}

			if (pName)
				m_fonts.addFont (pName);
			
		}
		break;
		
		case TT_ABIPREFERENCES:
		{
		m_parserState.m_bFoundAbiPreferences = true;

		// we expect something of the form:
		// <AbiPreferences app="AbiWord" ver="1.0">...</AbiPreferences>

		const gchar ** a = atts;
		while (a && *a)
		{
			UT_ASSERT(a[1] && *a[1]);	// require a value for each attribute keyword

			if (strcmp(static_cast<const char*>(a[0]), "app") == 0)
			{
				// TODO the following test will fail if you are running
				// TODO both an AbiWord (release) build and an AbiWord
				// TODO Personal (development/personal) build.  That is,
				// TODO you'll lose your MRU list if you alternate between
				// TODO the two types of executables.
				
				const char * szThisApp = XAP_App::getApp()->getApplicationName();
				UT_DEBUGMSG(("Found preferences for application [%s] (this is [%s]).\n",
							a[1],szThisApp));
				if (strcmp(static_cast<const char*>(a[1]),szThisApp) != 0)
				{
					UT_DEBUGMSG(("Preferences file does not match this application.\n"));
					goto InvalidFileError;
				}
			}
			else if (strcmp(static_cast<const char*>(a[0]), "ver") == 0)
			{
				// TODO test version number
			}

			a += 2;
		}
		break;
		}
		case TT_SELECT:
		{
		m_parserState.m_bFoundSelect = true;
		
		// we expect something of the form:
		// <Select
		//     scheme="myScheme"
		//     autosaveprefs="1"
		//     useenvlocale="1"
		//     />

		const gchar ** a = atts;
		while (a && *a)
		{
			UT_ASSERT(a[1] && *a[1]);	// require a value for each attribute keyword

			if (strcmp(static_cast<const char*>(a[0]), "scheme") == 0)
			{
				FREEP(m_parserState.m_szSelectedSchemeName);
				if (!(m_parserState.m_szSelectedSchemeName = g_strdup(static_cast<const char*>(a[1]))))
					goto MemoryError;
			}
			else if (strcmp(static_cast<const char*>(a[0]), "autosaveprefs") == 0)
			{
				// m_bAutoSavePrefs controls whether we automatically
				// save any changes in the preferences during
				// interactive use/manipulation of the UI or if
				// we wait until the user explicitly does a save.
				// MSFT has this annoying tendency to session-persist
				// almost everything in the UI -- we can do that,
				// but lets not make it the default....
				
				m_bAutoSavePrefs = (*a[1] == '1');
			}
			else if (strcmp(static_cast<const char*>(a[0]), "useenvlocale") == 0)
			{
				m_bUseEnvLocale = (*a[1] == '1');
			}
			
			a += 2;
		}

		if (!m_parserState.m_szSelectedSchemeName)
		{
			UT_DEBUGMSG(("No scheme selected in <Select...>\n"));
			goto InvalidFileError;
		}
		break;
		}
		case TT_SCHEME:
		{
		// we found a preferences scheme.  we expect something of the form:
		// <Scheme name="myScheme" n0="v0" n1="v1" ... />
		// where the [nk,vk] are arbitrary name/value pairs that mean
		// something to the application.
		//
		// if there are duplicated name/value pairs our behavior is
		// undefined -- we remember the last one that the XML parser
		// give us.

			//		bool bIsNamed = false;
		
		pNewScheme = new XAP_PrefsScheme(this, NULL);
		if (!pNewScheme)
			goto MemoryError;
		
		const gchar ** a = atts;
		while (*a)
		{
			UT_ASSERT(a[1] && *a[1]);	// require a value for each attribute keyword

			if (strcmp(static_cast<const char*>(a[0]), "name") == 0)
			{
				//				bIsNamed = true;
				
				const gchar * szBuiltinSchemeName = getBuiltinSchemeName();

				if (strcmp(static_cast<const char*>(a[1]), static_cast<const char*>(szBuiltinSchemeName)) == 0)
				{
					UT_DEBUGMSG(("Reserved scheme name [%s] found in file; ignoring.\n",a[1]));
					goto IgnoreThisScheme;
				}

				if (getScheme(a[1]))
				{
					UT_DEBUGMSG(("Duplicate scheme [%s]; ignoring latter instance.\n",a[1]));
					goto IgnoreThisScheme;
				}

				pNewScheme->setSchemeName(a[1]);

				UT_DEBUGMSG(("Found Preferences scheme [%s].\n",a[1]));
			}
			else
			{
				pNewScheme->setValue(a[0], a[1]);
			}

			a += 2;
		}

		addScheme(pNewScheme);

		pNewScheme = NULL;				// we don't own it anymore
		break;
		}
		case TT_PLUGIN:
		{
		// Almost the same as TT_SCHEME, except is denoted by <Plugin ... /> 
		// instead of <Scheme ... /> and has no builtin to deal with

			//		bool bIsNamed = false;
		
		pNewScheme = new XAP_PrefsScheme(this, NULL);
		if (!pNewScheme)
			goto MemoryError;
		
		const gchar ** a = atts;
		while (*a)
		{
			UT_ASSERT(a[1] && *a[1]);	// require a value for each attribute keyword

			if (strcmp(static_cast<const char*>(a[0]), "name") == 0)
			{
				//				bIsNamed = true;
				
				if (getPluginScheme(a[1]))
				{
					UT_DEBUGMSG(("Duplicate Plugin scheme [%s]; ignoring latter instance.\n",a[1]));
					goto IgnoreThisScheme;
				}

				pNewScheme->setSchemeName(a[1]);

				UT_DEBUGMSG(("Found Preferences Plugin scheme [%s].\n",a[1]));
			}
			else
			{
				pNewScheme->setValue(a[0], a[1]);
			}

			a += 2;
		}

		addPluginScheme(pNewScheme);

		pNewScheme = NULL;				// we don't own it anymore
		break;
		}
		case TT_RECENT:
		{
		m_parserState.m_bFoundRecent = true;
		
		// we expect something of the form:
		// <Recent max="4" name1="v1" name2="v2" ... />

		const gchar ** a = atts;
		while (*a) {
			UT_ASSERT(a[1] && *a[1]);	// require a value for each attribute keyword

			if (strcmp(a[0], "max") == 0) {
				m_maxRecent = atoi(a[1]);
			} else if (strncmp(a[0], "name", 4) == 0) {
				// see bug 10709 - Non-URI paths aren't displayed correctly in the file menu
				// this provides a seamless migration

				std::string recent;
				if (UT_go_path_is_uri(a[1])) {
					recent = a[1];
				} else {
					gchar* uri = UT_go_filename_to_uri(a[1]);
					recent = uri;
					g_free (uri);
				}

				m_vecRecent.push_back(recent);
			}

			a += 2;
		}

		_pruneRecent();
		break;
		}
		case TT_GEOMETRY:
		{
			if (m_geom.m_flags &  PREF_FLAG_GEOMETRY_NOUPDATE) break;
			m_parserState.m_bFoundGeometry = true;
		
			// we expect something of the form:
			// <Geometry width="xxx" height="xxx" posx="xxx" posy="xxx" />
			
			// These are the fall-back defaults which will be applied
			// if the user does NOT specify a geometry argument or
			// have set preference values. We only want to obey the
			// size not a position.

			// (Both m_geom and the temporary variables are
			// initialized so partial/invalid preference data from the
			// file will still have sensible fall-back defaults).
			UT_uint32 width = 800, height = 600, flags = PREF_FLAG_GEOMETRY_SIZE;
			UT_sint32 posx = 0, posy = 0;
			
			XAP_App::getApp()->getDefaultGeometry(width,height, flags);

			m_geom.m_width = width;
			m_geom.m_height = height;
			m_geom.m_posx = posx;
			m_geom.m_posy = posy;
			m_geom.m_flags = flags;
			
			const gchar ** a = atts;
			while (*a)
			{
				UT_ASSERT(a[1] && *a[1]);	// require a value for each attribute keyword
				
				if (strcmp(static_cast<const char*>(a[0]), "width") == 0)
				{
					width = atoi(static_cast<const char*>(a[1]));
				}
				else if (strcmp(static_cast<const char*>(a[0]), "height") == 0)
				{
					height = atoi(static_cast<const char*>(a[1]));
				}
				else if (strcmp(static_cast<const char*>(a[0]), "posx") == 0)
				{
					posx = atoi(static_cast<const char*>(a[1]));
				}
				else if (strcmp(static_cast<const char*>(a[0]), "posy") == 0)
				{
					posy = atoi(static_cast<const char*>(a[1]));
				}
				else if (strcmp(static_cast<const char*>(a[0]), "flags") == 0)
				{
					flags = atoi(static_cast<const char*>(a[1])) & ~PREF_FLAG_GEOMETRY_NOUPDATE;
				}
				
				a += 2;
			}

			// Override the fall-backs as appropriate with preference
			// settings.
			if (flags & PREF_FLAG_GEOMETRY_SIZE)
			{
				m_geom.m_width = width;
				m_geom.m_height = height;
				m_geom.m_flags |= PREF_FLAG_GEOMETRY_SIZE;
			}
			if (flags & PREF_FLAG_GEOMETRY_POS)
			{
				m_geom.m_posx = posx;
				m_geom.m_posy = posy;
				m_geom.m_flags |= PREF_FLAG_GEOMETRY_POS;
			}
			
			if (!(flags & PREF_FLAG_GEOMETRY_MAXIMIZED))
				m_geom.m_flags &= ~PREF_FLAG_GEOMETRY_MAXIMIZED;
			
			
		}
	}
	// successful parse of tag...
IgnoreThisScheme:
	DELETEP(pNewScheme);
	return;								// success

MemoryError:
	UT_DEBUGMSG(("Memory error parsing preferences file.\n"));
InvalidFileError:
	m_parserState.m_parserStatus = false;			// cause parser driver to bail
	DELETEP(pNewScheme);
	return;
}

void XAP_Prefs::endElement(const gchar * /* name */)
{
	// everything in this file is contained in start-tags
	return;
}

void XAP_Prefs::charData(const gchar * /* s */, int /* len */)
{
	// everything in this file is contained in start-tags
	return;
}

/*****************************************************************/


bool XAP_Prefs::loadPrefsFile(void)
{
	bool bResult = false;			// assume failure
	const char * szFilename;

	m_parserState.m_parserStatus = true;
	m_parserState.m_bFoundAbiPreferences = false;
	m_parserState.m_bFoundSelect = false;
	m_parserState.m_szSelectedSchemeName = NULL;
	m_parserState.m_bFoundRecent = false;
	m_parserState.m_bFoundGeometry = false;
	m_parserState.m_bFoundFonts = false;
	m_bLoadSystemDefaultFile = false;

	UT_XML parser;

	szFilename = getPrefsPathname();
	if (!szFilename)
	{
		UT_DEBUGMSG(("could not get pathname for preferences file.\n"));
		goto Cleanup;
	}

	parser.setListener (this);
	if ((parser.parse (szFilename) != UT_OK) || (!m_parserState.m_parserStatus))
	{
		UT_DEBUGMSG(("Problem reading (Preferences) document\n"));
		goto Cleanup;
	}

	// we succeeded in parsing the file,
	// now check for higher-level consistency.

	if (!m_parserState.m_bFoundAbiPreferences)
	{
		UT_DEBUGMSG(("Did not find <AbiPreferences...>\n"));
		goto Cleanup;
	}
	if (!m_parserState.m_bFoundSelect)
	{
		UT_DEBUGMSG(("Did not find <Select...>\n"));
		goto Cleanup;
	}
	if (!m_parserState.m_bFoundRecent)
	{
		UT_DEBUGMSG(("Did not find <Recent...>\n"));
		// Note: it's ok if we didn't find it...
	}
	if (!m_parserState.m_bFoundGeometry)
	{
		UT_DEBUGMSG(("Did not find <Geometry...>\n"));
		// Note: it's ok if we didn't find it...
	}

	UT_ASSERT(m_parserState.m_szSelectedSchemeName);
	if (!setCurrentScheme(m_parserState.m_szSelectedSchemeName))
	{
		UT_DEBUGMSG(("Selected scheme [%s] not found in preferences file.\n",
					m_parserState.m_szSelectedSchemeName));
		goto Cleanup;
	}

	bResult = true;
Cleanup:
	FREEP(m_parserState.m_szSelectedSchemeName);

	return bResult;
}

bool XAP_Prefs::savePrefsFile(void)
{
	bool bResult = false;			// assume failure
	const char * szFilename;
	FILE * fp = NULL;
#ifdef _WIN32
	UT_Win32LocaleString str;
#endif

	szFilename = getPrefsPathname();
	if (!szFilename)
	{
		UT_DEBUGMSG(("could not get pathname for preferences file.\n"));
		goto Cleanup;
	}

#ifdef _WIN32
	// TODO: something more elegant
	str.fromUTF8(szFilename);
	fp = _wfopen(str.c_str(), L"w");
#else
	fp = fopen(szFilename, "w");
#endif
	if (!fp)
	{
		UT_DEBUGMSG(("could not open preferences file [%s].\n",szFilename));
		goto Cleanup;
	}

	// write a comment block as a prolog.
	// NOTE: this is human readable information only.
	
	fprintf(fp,"<!-- =====================================================================  -->\n");
	fprintf(fp,"<!-- This file contains AbiSuite Preferences.  AbiSuite is a suite of Open  -->\n");
	fprintf(fp,"<!-- Source desktop applications developed by AbiSource, Inc.  Information  -->\n");
	fprintf(fp,"<!-- about this application can be found at http://www.abisource.com        -->\n");
	fprintf(fp,"<!-- You should not edit this file by hand.                                 -->\n");
	fprintf(fp,"<!-- =====================================================================  -->\n");
	fprintf(fp,"\n");

#if 1 //def DEBUG -- want this always in, it helps debugging
	if (XAP_App::s_szBuild_ID && XAP_App::s_szBuild_ID[0])
	{
		fprintf(fp,"<!--         Build_ID          = ");
		fprintf(fp,"%s",XAP_App::s_szBuild_ID);
		fprintf(fp," -->\n");
	}
	if (XAP_App::s_szBuild_Version && XAP_App::s_szBuild_Version[0])
	{
		fprintf(fp,"<!--         Build_Version     = ");
		fprintf(fp,"%s",XAP_App::s_szBuild_Version);
		fprintf(fp," -->\n");
	}
	if (XAP_App::s_szBuild_Options && XAP_App::s_szBuild_Options[0])
	{
		fprintf(fp,"<!--         Build_Options     = ");
		fprintf(fp,"%s",XAP_App::s_szBuild_Options);
		fprintf(fp," -->\n");
	}
	if (XAP_App::s_szBuild_Target && XAP_App::s_szBuild_Target[0])
	{
		fprintf(fp,"<!--         Build_Target      = ");
		fprintf(fp,"%s",XAP_App::s_szBuild_Target);
		fprintf(fp," -->\n");
	}
	if (XAP_App::s_szBuild_CompileTime && XAP_App::s_szBuild_CompileTime[0])
	{
		fprintf(fp,"<!--         Build_CompileTime = ");
		fprintf(fp,"%s",XAP_App::s_szBuild_CompileTime);
		fprintf(fp," -->\n");
	}
	if (XAP_App::s_szBuild_CompileDate && XAP_App::s_szBuild_CompileDate[0])
	{
		fprintf(fp,"<!--         Build_CompileDate = ");
		fprintf(fp,"%s",XAP_App::s_szBuild_CompileDate);
		fprintf(fp," -->\n");
	}
#endif
	
	// end of prolog.
	// now we begin the actual document.

	UT_ASSERT(m_builtinScheme);
	
	fprintf(fp,"\n<AbiPreferences app=\"%s\" ver=\"%s\">\n",
			XAP_App::getApp()->getApplicationName(),
			"1.0");
	{
		fprintf(fp,("\n"
					"\t<Select\n"
					"\t    scheme=\"%s\"\n"
					"\t    autosaveprefs=\"%d\"\n"
					"\t    useenvlocale=\"%d\"\n"
					"\t/>\n"),
				m_currentScheme->getSchemeName().c_str(),
				static_cast<UT_uint32>(m_bAutoSavePrefs),
				static_cast<UT_uint32>(m_bUseEnvLocale));

		UT_uint32 kLimit = m_vecSchemes.size();
		UT_uint32 k;

		const gchar * szBuiltinSchemeName = getBuiltinSchemeName();

		for (k=0; k<kLimit; k++)
		{
			XAP_PrefsScheme * p = getNthScheme(k);
			if(!p)
			{
				UT_ASSERT_HARMLESS(p);
				continue;
			}

			const std::string& thisSchemeName = p->getSchemeName();
			bool bIsBuiltin = (p == m_builtinScheme);

			if (bIsBuiltin)
			{
				fprintf(fp,("\n"
							"\t<!-- The following scheme, %s, contains the built-in application\n"
							"\t**** defaults and adjusted by the installation system defaults.  This scheme\n"
							"\t**** is only written here as a reference.  Any schemes following this one\n"
							"\t**** only list values that deviate from the built-in values.\n"
							"\t**** Items values must observe XML encoding for double quote (&quot;),\n"
							"\t**** ampersand (&amp;), and angle brackets (&lt; and &gt;).\n"
							"\t-->\n"),
						szBuiltinSchemeName);
			}

			fprintf(fp, "\n\t<Scheme\n\t\tname=\"%s\"\n", thisSchemeName.c_str());

			for (auto entry : *p) {
				bool need_print;
				need_print = false;
				if (bIsBuiltin)
				{
					// for the builtin set, we print every value
					need_print = true;
				}
				else
				{
					// for non-builtin sets, we only print the values which are different
					// from the builtin set.
					std::string builtinValue = NO_PREF_VALUE;
					m_builtinScheme->getValue(entry.first, builtinValue);
					if (entry.second != builtinValue ||
						// Always print debug values
						strncmp(entry.first.c_str(), DEBUG_PREFIX, sizeof(DEBUG_PREFIX) - 1) == 0)
					{
						need_print = true;
					}
				}
				if (need_print == true)
				{
					// szValue is UTF-8.  Convert to Unicode and then
					// do XML-encoding of XML-special characters and
					// non-ASCII characters.  The printed value
					// strings will get XML parsing and conversion to
					// UTF-8 the next time the application reads the
					// prefs file.
					UT_GrowBuf gb;
					UT_decodeUTF8string(entry.second.c_str(), entry.second.size(), &gb);
					UT_uint32 length = gb.getLength();
					fprintf(fp, "\t\t%s=\"", entry.first.c_str());
					for (UT_uint32 udex=0; udex<length; ++udex)
					{
						UT_UCSChar ch = *(gb.getPointer(udex));
						switch (ch)
						{
						case '&':   fputs("&amp;", fp);  break;
						case '<':   fputs("&lt;", fp);  break;
						case '>':   fputs("&gt;", fp);  break;
						case '"':   fputs("&quot;", fp);  break;
						default:
							if (ch < ' ' || ch >= 128)
							{
								fprintf(fp, "&#x%x;", ch);
							}
							else
							{
								putc(ch, fp);
							}
						}
					}
					fputs("\"\n", fp);
				}
			}
				
			fprintf(fp,"\t\t/>\n");
		}

		// add Plugin Scheme (plugin specific preferences) if they exist
		kLimit = m_vecPluginSchemes.size();
		for (k = 0; k < kLimit; k++) {
			XAP_PrefsScheme * p = getNthPluginScheme(k);
			if (!p) {
				UT_ASSERT_HARMLESS(p);
				continue;
			}

			const std::string& thisSchemeName = p->getSchemeName();
			fprintf(fp, "\n\t<Plugin\n\t\tname=\"%s\"\n", thisSchemeName.c_str());

			for (auto entry : *p) {
					// the value is UTF-8.  Convert to Unicode and then
					// do XML-encoding of XML-special characters and
					// non-ASCII characters.  The printed value
					// strings will get XML parsing and conversion to
					// UTF-8 the next time the application reads the
					// prefs file.
					UT_GrowBuf gb;
					UT_decodeUTF8string(entry.second.c_str(), entry.second.size(), &gb);
					UT_uint32 length = gb.getLength();
					fprintf(fp, "\t\t%s=\"", entry.first.c_str());
					for (UT_uint32 udex=0; udex<length; ++udex)
					{
						UT_UCSChar ch = *(gb.getPointer(udex));
						switch (ch)
						{
						case '&':   fputs("&amp;", fp);  break;
						case '<':   fputs("&lt;", fp);  break;
						case '>':   fputs("&gt;", fp);  break;
						case '"':   fputs("&quot;", fp);  break;
						default:
							if (ch < ' ' || ch >= 128)
							{
								fprintf(fp, "&#x%x;", ch);
							}
							else
							{
								putc(ch, fp);
							}
						}
					}
					fputs("\"\n", fp);
			}

			fprintf(fp,"\t\t/>\n");
		}
		// end Plugin preferences

		fprintf(fp,"\n\t<Recent\n\t\tmax=\"%u\"\n", m_maxRecent);

		kLimit = m_vecRecent.size();

		for (k=0; k<kLimit; k++)
		{
			const char* szRecent = getRecent(k + 1);
			if (!szRecent) {
				UT_DEBUGMSG(("getRecent returned nullptr\n"));
				continue;
			}
			auto entry = UT_escapeXML(szRecent);

			fprintf(fp, "\t\tname%d=\"%s\"\n", k + 1, entry.c_str());
		}

		fprintf(fp,"\t\t/>\n");

		fprintf(fp,"\n\t<Geometry\n");
		fprintf(fp,"\t\twidth=\"%u\"\n", m_geom.m_width);
		fprintf(fp,"\t\theight=\"%u\"\n", m_geom.m_height);
		fprintf(fp,"\t\tposx=\"%d\"\n", m_geom.m_posx);
		fprintf(fp,"\t\tposy=\"%d\"\n", m_geom.m_posy);
		fprintf(fp,"\t\tflags=\"%d\"\n", m_geom.m_flags);
		fprintf(fp,"\t\t/>\n");
	}

	// now the log
	fprintf(fp, "\n\t<Log>\n");

	for (auto line : m_vecLog) {
		fprintf(fp,"\t%s\n", line.c_str());
	}

	fprintf(fp, "\t</Log>\n");

	{
		fprintf(fp, "\n\t<Fonts include=\"%d\">\n", m_fonts.getIncludeFlag());
		fprintf(fp,
				"\t<!--"
				"\n\t     Here you can put a list of fonts to limit the fonts that appear "
				"\n\t     in the font UI:\n"
				"\n\t\t<Face name=\"some face\"/>\n"
				"\n\t     The include attribute of 'Fonts' controls the significance of "
				"\n\t     the list:"
				"\n\t     include=\"1\" - limit fonts to those listed"
				"\n\t     include=\"0\" - exclude the listed fonts from the system font list"
				"\n\t-->");

		const std::vector<std::string> & v = m_fonts.getFonts();
		
		for (std::vector<std::string>::const_iterator k = v.begin(); 
			 k != v.end() ; ++k)
		{
			fprintf(fp,"\n\t\t<Face name=\"%s\"/>",
					(*k).c_str());
		}

		fprintf(fp, "\n\t</Fonts>\n");
	}
	
	fprintf(fp,"\n</AbiPreferences>\n");
	
Cleanup:
	if (fp)
		fclose(fp);
	return bResult;

}

/*****************************************************************/

void XAP_Prefs::_startElement_SystemDefaultFile(const gchar *name, const gchar **atts)
{
	// routine to parse system default preferences file and
	// overlay values onto the builtin scheme.
	
	if (!m_parserState.m_parserStatus)		// eat if already had an error
		return;

	if (strcmp(static_cast<const char*>(name), "SystemDefaults") == 0)
	{
		// we found the system default preferences scheme.
		//
		// we expect something of the form:
		// <SystemDefaults n0="v0" n1="v1" ... />
		// where the [nk,vk] are arbitrary name/value pairs
		// that mean something to the application.
		//
		// if there are duplicated name/value pairs our behavior is
		// undefined -- we remember the last one that the XML parser
		// give us.

		const gchar ** a = atts;
		while (a && *a)
		{
			UT_ASSERT(a[1] && *a[1]);	// require a value for each attribute keyword

			// we ignore "name=<schemename>" just incase they copied and
			// pasted a user-profile into the system file.

			if (strcmp(a[0], "name") != 0) {
				m_builtinScheme->setValue(a[0], a[1]);
			}

			a += 2;
		}
	}
	else
	{
		UT_DEBUGMSG(("Ignoring tag [%s] in system default preferences file.\n",name));
	}
}

/*****************************************************************/

bool XAP_Prefs::loadSystemDefaultPrefsFile(const char * szSystemDefaultPrefsPathname)
{
	UT_ASSERT(szSystemDefaultPrefsPathname && *szSystemDefaultPrefsPathname);
	
	bool bResult = false;			// assume failure
	m_parserState.m_parserStatus = true;

	m_bLoadSystemDefaultFile = true;

	UT_XML parser;
	parser.setListener (this);
	if ((parser.parse (szSystemDefaultPrefsPathname) != UT_OK) || (!m_parserState.m_parserStatus))
	{
		UT_DEBUGMSG(("Problem reading (System Default Preferences) document\n"));
		goto Cleanup;
	}

	// we succeeded in parsing the file,

	bResult = true;
Cleanup:
	return bResult;
}

/*****************************************************************/

void XAP_Prefs::addListener	  ( PrefsListener pFunc, void *data )
{
	m_prefsListeners.push_back(tPrefsListenersPair(pFunc, data));
}

// optional parameter, data.  If given (i.e., != 0), will try to match data,
// otherwise, will delete all calls to pFunc
void XAP_Prefs::removeListener ( PrefsListener pFunc, void *data )
{
	for (PrefsListenersList::iterator iter = m_prefsListeners.begin();
		 iter != m_prefsListeners.end(); )
	{
		const tPrefsListenersPair & pPair = *iter;
		if ( pPair.m_pFunc == pFunc && (!data || pPair.m_pData == data) ) {
			iter = m_prefsListeners.erase(iter);
		}
		else {
			++iter;
		}
	}
}

void XAP_Prefs::_markPrefChange(const std::string& key)
{
	if (m_bInChangeBlock) {
		m_ahashChanges.insert(key);
		// notify later
	} else {
		XAP_PrefsChangeSet changes;
		changes.insert(key);
		_sendPrefsSignal(changes);
	}
}

void XAP_Prefs::startBlockChange()
{
	m_bInChangeBlock = true;
}

void XAP_Prefs::endBlockChange()
{
	if ( m_bInChangeBlock )
	{
		m_bInChangeBlock = false;
		_sendPrefsSignal( m_ahashChanges );
	}
}

void XAP_Prefs::_sendPrefsSignal(const XAP_PrefsChangeSet& hash)
{
	for (PrefsListenersList::const_iterator iter = m_prefsListeners.begin();
		 iter != m_prefsListeners.end(); ++iter)
	{
		const tPrefsListenersPair & p = *iter;

		UT_ASSERT_HARMLESS(p.m_pFunc);
		if(!p.m_pFunc)
			continue;

		(p.m_pFunc)(this, &hash, p.m_pData);
	}
}

bool XAP_FontSettings::isOnExcludeList (const char * name) const
{
	if (m_bInclude)
		return false;

	if (!m_vecFonts.size())
		return false;

	std::vector<std::string>::const_iterator i =
		std::find(m_vecFonts.begin(), m_vecFonts.end(), name);

	return i != m_vecFonts.end();
}

