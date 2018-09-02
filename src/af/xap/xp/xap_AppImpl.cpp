/* AbiSource Application Framework
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

#include "xap_AppImpl.h"
#include "ut_assert.h"
#include "ut_path.h"
#include "xap_App.h"
#include "xap_Prefs.h"

/*!
	Open a help URL
	
	\param url the URL to open
	
	This method opens a help page designated by URL. By default it does
	the same as opening and URL, ie likely to be in the web browser.
	Override it to open this in a different viewer like Help Viewer (MacOS X),
	gnome-help (GNOME), etc.
	It should check the protcol (file: vs http:)
 */
bool XAP_AppImpl::openHelpURL(const char * url)
{
	return openURL(url);
}


inline static void _catPath(std::string& st, const char* st2)
{
	if (st.size() > 0)
	{
		if (st[st.size() - 1] != '/')
			st += '/';
	}
	else
		st += '/';

	st += st2;
}

/*!
	Localize and URL for help.
	
	\param pathBeforeLang the path for the help file that prefix
	\param pathAfterLang the path inside the localized directory
	\param remoteURLbase remote URL if help files are not found.
	
	Override in subclasses if platform needs specific work
 */
std::string XAP_AppImpl::localizeHelpUrl (const char * pathBeforeLang,
					  const char * pathAfterLang,
					  const char * remoteURLbase)
{
	XAP_App* pApp = XAP_App::getApp();

	UT_return_val_if_fail(pApp, "");
	XAP_Prefs* pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, "");

	const char* abiSuiteLibDir = pApp->getAbiSuiteLibDir();
	const gchar* abiSuiteLocString = NULL;
	std::string url;

	// evil...
	pPrefs->getPrefsValue("StringSet", &abiSuiteLocString);

	// 1st try file on user's computer (local file), if not exist try remote help
	std::string path(abiSuiteLibDir);
	_catPath(path, pathBeforeLang);

	std::string localized_path(path);
	_catPath(localized_path, abiSuiteLocString);

	if (UT_directoryExists(localized_path.c_str()))
	{
		// the localised help exists, so use it
		path = localized_path;
	}
	else
	{
		// the localised help directory does not exist, so fall back to the
		// en-US help location, which is the default lang, so usually available
		localized_path = path;
		_catPath(localized_path, "en-US");
	}

	_catPath(localized_path, pathAfterLang);
	localized_path += ".html";

	if (remoteURLbase && !UT_isRegularFile(localized_path.c_str()))
	{
		// not found, so build localized path for remote URL (but we can't verify remote URL)
		url = remoteURLbase;
		
		// HACK: Not all help files are localized. 
		// HACK: Hard code the available translations here instead of 404-ing.
		if (!(
			!strcmp(abiSuiteLocString, "en-US") ||
			!strcmp(abiSuiteLocString, "fr-FR") ||
			!strcmp(abiSuiteLocString, "pl-PL")
			))
			_catPath(url, "en-US");
		else
			_catPath(url, abiSuiteLocString);
		_catPath(url, pathAfterLang);
		url += ".html";
	}
	else
	{
		url = "file://";
		url += localized_path;
	}

	return url;
}
