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

// TODO add code to do an auto save anytime anything is changed.

#include <stdlib.h>
#include <stdio.h>

#include "ut_debugmsg.h"
#include "xap_Prefs.h"

/*****************************************************************/

XAP_PrefsScheme::XAP_PrefsScheme(const XML_Char * szSchemeName)
	: m_hash(13)
{
	if (szSchemeName && *szSchemeName)
		UT_XML_cloneString((XML_Char *&)m_szName,szSchemeName);
	else
		m_szName = NULL;
}

XAP_PrefsScheme::~XAP_PrefsScheme(void)
{
	FREEP(m_szName);
}

const XML_Char * XAP_PrefsScheme::getSchemeName(void) const
{
	return m_szName;
}

UT_Bool XAP_PrefsScheme::setSchemeName(const XML_Char * szNewSchemeName)
{
	FREEP(m_szName);
	return UT_XML_cloneString(m_szName,szNewSchemeName);
}

UT_Bool XAP_PrefsScheme::setValue(const XML_Char * szKey, const XML_Char * szValue)
{
	UT_HashEntry * pEntry = m_hash.findEntry(szKey);
	if (pEntry)
	{
		if (UT_stricmp(szValue,pEntry->pszRight) == 0)
			return UT_TRUE;				// equal values, no changes required
		
		m_hash.setEntry(pEntry, szValue, NULL); // update with new value
		return UT_TRUE;
	}

	// otherwise, need to add a new entry

	m_hash.addEntry(szKey,szValue,NULL);
	return UT_TRUE;
}

UT_Bool XAP_PrefsScheme::setValueBool(const XML_Char * szKey, UT_Bool bValue)
{
	return setValue(szKey, ((bValue) ? "1" : "0"));
}

UT_Bool XAP_PrefsScheme::getValue(const XML_Char * szKey, const XML_Char ** pszValue) const
{
	UT_HashEntry * pEntry = m_hash.findEntry(szKey);
	if (!pEntry)
		return UT_FALSE;

	if (pszValue)
		*pszValue = pEntry->pszRight;
	return UT_TRUE;
}

UT_Bool XAP_PrefsScheme::getValueBool(const XML_Char * szKey, UT_Bool * pbValue) const
{
	*pbValue = UT_FALSE;				// assume something
	
	const XML_Char * szValue = NULL;
	if (!getValue(szKey,&szValue))
		return UT_FALSE;				// bogus keyword ??

	if (!szValue || !*szValue)
		return UT_FALSE;				// no value for known keyword ??

	switch (szValue[0])
	{
	case '1':
	case 't':
	case 'T':
	case 'y':
	case 'Y':
		*pbValue = UT_TRUE;
		return UT_TRUE;

	default:
		*pbValue = UT_FALSE;
		return UT_TRUE;
	}
}

UT_Bool XAP_PrefsScheme::getNthValue(UT_uint32 k, const XML_Char ** pszKey, const XML_Char ** pszValue) const
{
	// TODO we should fix hash to use ut_uint32 rather than int
	
	if (k >= (UT_uint32)m_hash.getEntryCount())
		return UT_FALSE;
	
	UT_HashEntry * pEntry = m_hash.getNthEntryAlpha(k);

	if (!pEntry)
		return UT_FALSE;

	if (pszKey)
		*pszKey = pEntry->pszLeft;
	if (pszValue)
		*pszValue = pEntry->pszRight;

	return UT_TRUE;
}

/*****************************************************************/

UT_Bool XAP_Prefs::getAutoSavePrefs(void) const
{
	return m_bAutoSavePrefs;
}

void XAP_Prefs::setAutoSavePrefs(UT_Bool bAuto)
{
	m_bAutoSavePrefs = bAuto;

	// TODO if turning autosave on, we should do a save now....
	// TODO if was on and turning off, should we save it now ??
}

/*****************************************************************/

UT_Bool XAP_Prefs::getUseEnvLocale(void) const
{
	return m_bUseEnvLocale;
}

void XAP_Prefs::setUseEnvLocale(UT_Bool bUse)
{
	m_bUseEnvLocale = bUse;
}

/*****************************************************************/

UT_uint32 XAP_Prefs::getMaxRecent(void) const
{
	return m_iMaxRecent;
}

void XAP_Prefs::setMaxRecent(UT_uint32 k)
{
	UT_ASSERT(k<=XAP_PREF_LIMIT_MaxRecent);

	if (k > XAP_PREF_LIMIT_MaxRecent)
		k = XAP_PREF_LIMIT_MaxRecent;

	m_iMaxRecent = k;
}

UT_uint32 XAP_Prefs::getRecentCount(void) const
{
	return m_vecRecent.getItemCount();
}

const char * XAP_Prefs::getRecent(UT_uint32 k) const
{
	// NB: k is one-based
	UT_ASSERT(k <= m_iMaxRecent);

	const char * pRecent = NULL;
	
	if (k <= m_vecRecent.getItemCount())
	{
		pRecent = (const char *) m_vecRecent.getNthItem(k - 1);
	}

	return pRecent;
}
	
void XAP_Prefs::addRecent(const char * szRecent)
{
	const char * sz;
	UT_Bool bFound = UT_FALSE;

	if (m_iMaxRecent == 0)
		return;		// NOOP

	// was it already here? 
	for (UT_uint32 i=0; i<m_vecRecent.getItemCount(); i++)
	{
		sz = (const char *)m_vecRecent.getNthItem(i);
		if ((sz==szRecent) || !UT_strcmp(sz, szRecent))
		{
			// yep, we're gonna move it up
			m_vecRecent.deleteNthItem(i);
			bFound = UT_TRUE;
			break;
		}
	}

	if (!bFound)
	{
		// nope.  make a new copy to store
		UT_cloneString((char *&)sz, szRecent);
	}

	m_vecRecent.insertItemAt((void *)sz, 0);
	_pruneRecent();
}

void XAP_Prefs::removeRecent(UT_uint32 k)
{
	UT_ASSERT(k>0);
	UT_ASSERT(k<=getRecentCount());

	char * sz = (char *) m_vecRecent.getNthItem(k-1);
	FREEP(sz);

	m_vecRecent.deleteNthItem(k-1);
}

void XAP_Prefs::_pruneRecent(void)
{
	UT_sint32 i;
	UT_uint32 count = getRecentCount();

	if (m_iMaxRecent == 0)
	{
		// nuke the whole thing
		for (i = (signed) count; i > 0 ; i--)
		{
			char * sz = (char *) m_vecRecent.getNthItem(i-1);
			FREEP(sz);
		}

		m_vecRecent.clear();
	}
	else if (count > m_iMaxRecent)
	{
		// prune entries past m_iMaxRecent
		for (i = (signed) count; i > (signed) m_iMaxRecent; i--)
			removeRecent(i);
	}
}

/*****************************************************************/

XAP_Prefs::XAP_Prefs(XAP_App * pApp)
{
	m_pApp = pApp;
	m_bAutoSavePrefs = atoi(XAP_PREF_DEFAULT_AutoSavePrefs);
	m_bUseEnvLocale = atoi(XAP_PREF_DEFAULT_UseEnvLocale);
	m_currentScheme = NULL;
	m_builtinScheme = NULL;
	m_iMaxRecent = atoi(XAP_PREF_DEFAULT_MaxRecent);

	// NOTE: since constructors cannot report malloc
	// NOTE: failures (and since it is virtual back
	// NOTE: to the application), our creator must call
	// NOTE: loadBuiltinPrefs().

	// NOTE: we do not initialize the values in m_parserState
	// NOTE: since they are only used by the parser, it can
	// NOTE: initialize them.
}

XAP_Prefs::~XAP_Prefs(void)
{
	UT_VECTOR_PURGEALL(XAP_PrefsScheme *, m_vecSchemes);
	UT_VECTOR_PURGEALL(char *, m_vecRecent);
}

/*****************************************************************/

XAP_PrefsScheme * XAP_Prefs::getNthScheme(UT_uint32 k) const
{
	UT_uint32 kLimit = m_vecSchemes.getItemCount();
	if (k < kLimit)
		return (XAP_PrefsScheme *)m_vecSchemes.getNthItem(k);
	else
		return NULL;
}

XAP_PrefsScheme * XAP_Prefs::getScheme(const XML_Char * szSchemeName) const
{
	UT_uint32 kLimit = m_vecSchemes.getItemCount();
	UT_uint32 k;

	for (k=0; k<kLimit; k++)
	{
		XAP_PrefsScheme * p = getNthScheme(k);
		UT_ASSERT(p);
		if (UT_XML_stricmp(szSchemeName,p->getSchemeName()) == 0)
			return p;
	}

	return NULL;
}

UT_Bool XAP_Prefs::addScheme(XAP_PrefsScheme * pNewScheme)
{
	const XML_Char * szBuiltinSchemeName = getBuiltinSchemeName();
	const XML_Char * szThisSchemeName = pNewScheme->getSchemeName();
	
	if (UT_XML_stricmp(szThisSchemeName, szBuiltinSchemeName) == 0)
	{
		UT_ASSERT(m_builtinScheme == NULL);
		m_builtinScheme = pNewScheme;
	}
	
	return (m_vecSchemes.addItem(pNewScheme) == 0);
}


XAP_PrefsScheme * XAP_Prefs::getCurrentScheme(void) const
{
	return m_currentScheme;
}

UT_Bool XAP_Prefs::setCurrentScheme(const XML_Char * szSchemeName)
{
	// set the current scheme.

	// TODO notify the application that the scheme has changed

	XAP_PrefsScheme * p = getScheme(szSchemeName);
	if (!p)
		return UT_FALSE;

	UT_DEBUGMSG(("Preferences::setCurrentScheme [%s].\n",szSchemeName));
	
	m_currentScheme = p;
	return UT_TRUE;
}

/*****************************************************************/

UT_Bool XAP_Prefs::getPrefsValue(const XML_Char * szKey, const XML_Char ** pszValue) const
{
	// a convenient routine to get a name/value pair from the current scheme

	UT_ASSERT(m_currentScheme);

	if (m_currentScheme->getValue(szKey,pszValue))
		return UT_TRUE;
	if (m_builtinScheme->getValue(szKey,pszValue))
		return UT_TRUE;

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return UT_FALSE;
}

UT_Bool XAP_Prefs::getPrefsValueBool(const XML_Char * szKey, UT_Bool * pbValue) const
{
	// a convenient routine to get a name/value pair from the current scheme

	UT_ASSERT(m_currentScheme);

	if (m_currentScheme->getValueBool(szKey,pbValue))
		return UT_TRUE;
	if (m_builtinScheme->getValueBool(szKey,pbValue))
		return UT_TRUE;

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return UT_FALSE;
}

/*****************************************************************
******************************************************************
** C-style callback functions that we register with the XML parser
******************************************************************
*****************************************************************/

static void startElement(void *userData, const XML_Char *name, const XML_Char **atts)
{
	XAP_Prefs * pPrefs = (XAP_Prefs *)userData;
	pPrefs->_startElement(name,atts);
}

static void endElement(void *userData, const XML_Char *name)
{
	XAP_Prefs * pPrefs = (XAP_Prefs *)userData;
	pPrefs->_endElement(name);
}

static void charData(void* userData, const XML_Char *s, int len)
{
	XAP_Prefs * pPrefs = (XAP_Prefs *)userData;
	pPrefs->_charData(s,len);
}

static void startElement_SystemDefaultFile(void *userData, const XML_Char *name, const XML_Char **atts)
{
	XAP_Prefs * pPrefs = (XAP_Prefs *)userData;
	pPrefs->_startElement_SystemDefaultFile(name,atts);
}

/*****************************************************************/

void XAP_Prefs::_startElement(const XML_Char *name, const XML_Char **atts)
{
	XAP_PrefsScheme * pNewScheme = NULL; // must be freed
	
	if (!m_parserState.m_parserStatus)		// eat if already had an error
		return;

	if (UT_XML_stricmp(name, "AbiPreferences") == 0)
	{
		m_parserState.m_bFoundAbiPreferences = UT_TRUE;

		// we expect something of the form:
		// <AbiPreferences app="AbiWord" ver="1.0">...</AbiPreferences>

		const XML_Char ** a = atts;
		while (*a)
		{
			UT_ASSERT(a[1] && *a[1]);	// require a value for each attribute keyword

			if (UT_XML_stricmp(a[0], "app") == 0)
			{
				// TODO the following test will fail if you are running
				// TODO both an AbiWord (release) build and an AbiWord
				// TODO Personal (development/personal) build.  That is,
				// TODO you'll lose your MRU list if you alternate between
				// TODO the two types of executables.
				
				const char * szThisApp = m_pApp->getApplicationName();
				UT_DEBUGMSG(("Found preferences for application [%s] (this is [%s]).\n",
							a[1],szThisApp));
				if (UT_XML_stricmp(a[1],szThisApp) != 0)
				{
					UT_DEBUGMSG(("Preferences file does not match this application.\n"));
					goto InvalidFileError;
				}
			}
			else if (UT_XML_stricmp(a[0], "ver") == 0)
			{
				// TODO test version number
			}

			a += 2;
		}
	}
	else if (UT_XML_stricmp(name, "Select") == 0)
	{
		m_parserState.m_bFoundSelect = UT_TRUE;
		
		// we expect something of the form:
		// <Select
		//     scheme="myScheme"
		//     autosaveprefs="1"
		//     useenvlocale="1"
		//     />

		const XML_Char ** a = atts;
		while (*a)
		{
			UT_ASSERT(a[1] && *a[1]);	// require a value for each attribute keyword

			if (UT_XML_stricmp(a[0], "scheme") == 0)
			{
				FREEP(m_parserState.m_szSelectedSchemeName);
				if (!UT_cloneString((char *&)m_parserState.m_szSelectedSchemeName,a[1]))
					goto MemoryError;
			}
			else if (UT_XML_stricmp(a[0], "autosaveprefs") == 0)
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
			else if (UT_XML_stricmp(a[0], "useenvlocale") == 0)
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
	}
	else if (UT_XML_stricmp(name, "Scheme") == 0)
	{
		// we found a preferences scheme.  we expect something of the form:
		// <Scheme name="myScheme" n0="v0" n1="v1" ... />
		// where the [nk,vk] are arbitrary name/value pairs that mean
		// something to the application.
		//
		// if there are duplicated name/value pairs our behavior is
		// undefined -- we remember the last one that the XML parser
		// give us.

		UT_Bool bIsNamed = UT_FALSE;
		
		pNewScheme = new XAP_PrefsScheme(NULL);
		if (!pNewScheme)
			goto MemoryError;
		
		const XML_Char ** a = atts;
		while (*a)
		{
			UT_ASSERT(a[1] && *a[1]);	// require a value for each attribute keyword

			if (UT_XML_stricmp(a[0], "name") == 0)
			{
				bIsNamed = UT_TRUE;
				
				const XML_Char * szBuiltinSchemeName = getBuiltinSchemeName();

				if (UT_XML_stricmp(a[1], szBuiltinSchemeName) == 0)
				{
					UT_DEBUGMSG(("Reserved scheme name [%s] found in file; ignoring.\n",a[1]));
					goto IgnoreThisScheme;
				}

				if (getScheme(a[1]))
				{
					UT_DEBUGMSG(("Duplicate scheme [%s]; ignoring latter instance.\n",a[1]));
					goto IgnoreThisScheme;
				}

				if (!pNewScheme->setSchemeName(a[1]))
					goto MemoryError;

				UT_DEBUGMSG(("Found Preferences scheme [%s].\n",a[1]));
			}
			else
			{
				if (!pNewScheme->setValue(a[0],a[1]))
					goto MemoryError;
			}

			a += 2;
		}

		if (!addScheme(pNewScheme))
			goto MemoryError;
		pNewScheme = NULL;				// we don't own it anymore
	}
	else if (UT_XML_stricmp(name, "Recent") == 0)
	{
		m_parserState.m_bFoundRecent = UT_TRUE;
		
		// we expect something of the form:
		// <Recent max="4" name1="v1" name2="v2" ... />

		const XML_Char ** a = atts;
		while (*a)
		{
			UT_ASSERT(a[1] && *a[1]);	// require a value for each attribute keyword

			if (UT_XML_stricmp(a[0], "max") == 0)
			{
				m_iMaxRecent = atoi(a[1]);
			}
			else if (UT_strnicmp(a[0], "name", 4) == 0)
			{
				// NOTE: taking advantage of the fact that XML_Char == char
				UT_ASSERT((sizeof(XML_Char) == sizeof(char)));
				XML_Char * sz;
				UT_XML_cloneString((XML_Char *&)sz, a[1]);

				// NOTE: we keep the copied string in the vector
				m_vecRecent.addItem((void *)sz);
			}

			a += 2;
		}

		_pruneRecent();
	}

	// successful parse of tag...
	
IgnoreThisScheme:
	DELETEP(pNewScheme);
	return;								// success

MemoryError:
	UT_DEBUGMSG(("Memory error parsing preferences file.\n"));
InvalidFileError:
	m_parserState.m_parserStatus = UT_FALSE;			// cause parser driver to bail
	DELETEP(pNewScheme);
	return;
}

void XAP_Prefs::_endElement(const XML_Char * /* name */)
{
	// everything in this file is contained in start-tags
	return;
}

void XAP_Prefs::_charData(const XML_Char * /* s */, int /* len */)
{
	// everything in this file is contained in start-tags
	return;
}

/*****************************************************************/

UT_Bool XAP_Prefs::loadPrefsFile(void)
{
	UT_Bool bResult = UT_FALSE;			// assume failure
	const char * szFilename;
	FILE * fp = NULL;
	XML_Parser parser = NULL;
	int done = 0;
	char buf[4096];

	m_parserState.m_parserStatus = UT_TRUE;
	m_parserState.m_bFoundAbiPreferences = UT_FALSE;
	m_parserState.m_bFoundSelect = UT_FALSE;
	m_parserState.m_szSelectedSchemeName = NULL;
	m_parserState.m_bFoundRecent = UT_FALSE;

	szFilename = getPrefsPathname();
	if (!szFilename)
	{
		UT_DEBUGMSG(("could not get pathname for preferences file.\n"));
		goto Cleanup;
	}

	fp = fopen(szFilename, "r");
	if (!fp)
	{
		UT_DEBUGMSG(("could not open preferences file [%s].\n",szFilename));
		goto Cleanup;
	}
	
	parser = XML_ParserCreate(NULL);
	if (!parser)
	{
		UT_DEBUGMSG(("could not create parser for preferences file.\n"));
		goto Cleanup;
	}
	
	XML_SetUserData(parser, this);
	XML_SetElementHandler(parser, startElement, endElement);
	XML_SetCharacterDataHandler(parser, charData);

	while (!done)
	{
		size_t len = fread(buf, 1, sizeof(buf), fp);
		done = (len < sizeof(buf));

		if (!XML_Parse(parser, buf, len, done)) 
		{
			UT_DEBUGMSG(("%s at line %d\n",
						XML_ErrorString(XML_GetErrorCode(parser)),
						XML_GetCurrentLineNumber(parser)));
			goto Cleanup;
		}

		if (!m_parserState.m_parserStatus)
		{
			UT_DEBUGMSG(("Problem reading document\n"));
			goto Cleanup;
		}
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

	UT_ASSERT(m_parserState.m_szSelectedSchemeName);
	if (!setCurrentScheme(m_parserState.m_szSelectedSchemeName))
	{
		UT_DEBUGMSG(("Selected scheme [%s] not found in preferences file.\n",
					m_parserState.m_szSelectedSchemeName));
		goto Cleanup;
	}

	bResult = UT_TRUE;

Cleanup:
	FREEP(m_parserState.m_szSelectedSchemeName);
	if (parser)
		XML_ParserFree(parser);
	if (fp)
		fclose(fp);
	return bResult;
}

UT_Bool XAP_Prefs::savePrefsFile(void)
{
	UT_Bool bResult = UT_FALSE;			// assume failure
	const char * szFilename;
	FILE * fp = NULL;

	szFilename = getPrefsPathname();
	if (!szFilename)
	{
		UT_DEBUGMSG(("could not get pathname for preferences file.\n"));
		goto Cleanup;
	}

	fp = fopen(szFilename, "w");
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

#ifdef DEBUG
	if (XAP_App::s_szBuild_ID && XAP_App::s_szBuild_ID[0])
	{
		fprintf(fp,"<!--         Build_ID          = ");
		fprintf(fp,XAP_App::s_szBuild_ID);
		fprintf(fp," -->\n");
	}
	if (XAP_App::s_szBuild_Version && XAP_App::s_szBuild_Version[0])
	{
		fprintf(fp,"<!--         Build_Version     = ");
		fprintf(fp,XAP_App::s_szBuild_Version);
		fprintf(fp," -->\n");
	}
	if (XAP_App::s_szBuild_Options && XAP_App::s_szBuild_Options[0])
	{
		fprintf(fp,"<!--         Build_Options     = ");
		fprintf(fp,XAP_App::s_szBuild_Options);
		fprintf(fp," -->\n");
	}
	if (XAP_App::s_szBuild_Target && XAP_App::s_szBuild_Target[0])
	{
		fprintf(fp,"<!--         Build_Target      = ");
		fprintf(fp,XAP_App::s_szBuild_Target);
		fprintf(fp," -->\n");
	}
	if (XAP_App::s_szBuild_CompileTime && XAP_App::s_szBuild_CompileTime[0])
	{
		fprintf(fp,"<!--         Build_CompileTime = ");
		fprintf(fp,XAP_App::s_szBuild_CompileTime);
		fprintf(fp," -->\n");
	}
	if (XAP_App::s_szBuild_CompileDate && XAP_App::s_szBuild_CompileDate[0])
	{
		fprintf(fp,"<!--         Build_CompileDate = ");
		fprintf(fp,XAP_App::s_szBuild_CompileDate);
		fprintf(fp," -->\n");
	}
#endif
	
	// end of prolog.
	// now we begin the actual document.

	UT_ASSERT(m_builtinScheme);
	
	fprintf(fp,"\n<AbiPreferences app=\"%s\" ver=\"%s\">\n",
			m_pApp->getApplicationName(),
			"1.0");
	{
		fprintf(fp,("\n"
					"\t<Select\n"
					"\t    scheme=\"%s\"\n"
					"\t    autosaveprefs=\"%d\"\n"
					"\t    useenvlocale=\"%d\"\n"
					"\t/>\n"),
				m_currentScheme->getSchemeName(),
				(UT_uint32)m_bAutoSavePrefs,
				(UT_uint32)m_bUseEnvLocale);

		UT_uint32 kLimit = m_vecSchemes.getItemCount();
		UT_uint32 k;

		const XML_Char * szBuiltinSchemeName = getBuiltinSchemeName();

		for (k=0; k<kLimit; k++)
		{
			XAP_PrefsScheme * p = getNthScheme(k);
			UT_ASSERT(p);

			const XML_Char * szThisSchemeName = p->getSchemeName();
			UT_Bool bIsBuiltin = (p == m_builtinScheme);

			if (bIsBuiltin)
			{
				fprintf(fp,("\n"
							"\t<!-- The following scheme, %s, contains the built-in application\n"
							"\t**** defaults and adjusted by the installation system defaults.  This scheme\n"
							"\t**** is only written here as a reference.  Any schemes following this one\n"
							"\t**** only list values that deviate from the built-in values.\n"
							"\t-->\n"),
						szBuiltinSchemeName);
			}

			fprintf(fp,"\n\t<Scheme\n\t\tname=\"%s\"\n",szThisSchemeName);

			const XML_Char * szKey;
			const XML_Char * szValue;
			UT_uint32 j;
			for (j=0; (p->getNthValue(j,&szKey,&szValue)); j++)
			{
				if (bIsBuiltin)
				{
					// for the builtin set, we print every value
					fprintf(fp,"\t\t%s=\"%s\"\n",szKey,szValue);
				}
				else
				{
					// for non-builtin sets, we only print the values which are different
					// from the builtin set.
					const XML_Char * szBuiltinValue;
					UT_Bool bHaveBuiltinValue = m_builtinScheme->getValue(szKey,&szBuiltinValue);
					UT_ASSERT(bHaveBuiltinValue);
					if (UT_XML_strcmp(szValue,szBuiltinValue) != 0)
						fprintf(fp,"\t\t%s=\"%s\"\n",szKey,szValue);
				}
			}
				
			fprintf(fp,"\t\t/>\n");
		}

		fprintf(fp,"\n\t<Recent\n\t\tmax=\"%d\"\n",
				(UT_uint32)m_iMaxRecent);

		kLimit = m_vecRecent.getItemCount();

		for (k=0; k<kLimit; k++)
		{
			const char * szRecent = getRecent(k+1);

			fprintf(fp,"\t\tname%d=\"%s\"\n",k+1,szRecent);
		}
				
		fprintf(fp,"\t\t/>\n");
	}

	fprintf(fp,"\n</AbiPreferences>\n");
	
Cleanup:
	if (fp)
		fclose(fp);
	return bResult;

}

/*****************************************************************/

void XAP_Prefs::_startElement_SystemDefaultFile(const XML_Char *name, const XML_Char **atts)
{
	// routine to parse system default preferences file and
	// overlay values onto the builtin scheme.
	
	if (!m_parserState.m_parserStatus)		// eat if already had an error
		return;

	if (UT_XML_stricmp(name, "SystemDefaults") == 0)
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

		const XML_Char ** a = atts;
		while (*a)
		{
			UT_ASSERT(a[1] && *a[1]);	// require a value for each attribute keyword

			// we ignore "name=<schemename>" just incase they copied and
			// pasted a user-profile into the system file.
			
			if (UT_XML_stricmp(a[0], "name") != 0)
				if (!m_builtinScheme->setValue(a[0],a[1]))
					goto MemoryError;

			a += 2;
		}
	}
	else
	{
		UT_DEBUGMSG(("Ignoring tag [%s] in system default preferences file.\n",name));
	}

	return;								// success

MemoryError:
	UT_DEBUGMSG(("Memory error parsing preferences file.\n"));
	m_parserState.m_parserStatus = UT_FALSE;			// cause parser driver to bail
	return;
}

/*****************************************************************/

UT_Bool XAP_Prefs::loadSystemDefaultPrefsFile(const char * szSystemDefaultPrefsPathname)
{
	UT_ASSERT(szSystemDefaultPrefsPathname && *szSystemDefaultPrefsPathname);
	
	UT_Bool bResult = UT_FALSE;			// assume failure
	FILE * fp = NULL;
	XML_Parser parser = NULL;
	int done = 0;
	char buf[4096];

	m_parserState.m_parserStatus = UT_TRUE;

	fp = fopen(szSystemDefaultPrefsPathname, "r");
	if (!fp)
	{
		UT_DEBUGMSG(("could not open system default preferences file [%s].\n",szSystemDefaultPrefsPathname));
		goto Cleanup;
	}
	
	parser = XML_ParserCreate(NULL);
	if (!parser)
	{
		UT_DEBUGMSG(("could not create parser for system default preferences file.\n"));
		goto Cleanup;
	}
	
	XML_SetUserData(parser, this);
	XML_SetElementHandler(parser, startElement_SystemDefaultFile, endElement);
	XML_SetCharacterDataHandler(parser, charData);

	while (!done)
	{
		size_t len = fread(buf, 1, sizeof(buf), fp);
		done = (len < sizeof(buf));

		if (!XML_Parse(parser, buf, len, done)) 
		{
			UT_DEBUGMSG(("%s at line %d\n",
						XML_ErrorString(XML_GetErrorCode(parser)),
						XML_GetCurrentLineNumber(parser)));
			goto Cleanup;
		}

		if (!m_parserState.m_parserStatus)
		{
			UT_DEBUGMSG(("Problem reading document\n"));
			goto Cleanup;
		}
	} 

	// we succeeded in parsing the file,

	bResult = UT_TRUE;

Cleanup:
	if (parser)
		XML_ParserFree(parser);
	if (fp)
		fclose(fp);
	return bResult;
}
