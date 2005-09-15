/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <glib.h>

#ifdef HAVE_GNOME
#include <gnome.h>
#endif

#include "xap_UnixAppImpl.h"
#include "ut_files.h"
#include "ut_string_class.h"

UT_String XAP_UnixAppImpl::localizeHelpUrl (const char * pathBefore, 
						   const char * pathAfter,
						   const char * remoteURLbase)
{
#if 0 //def HAVE_GNOME
	UT_String path (pathAfter);
	path += ".html";
	return path;
#else
	return XAP_AppImpl::localizeHelpUrl (pathBefore, pathAfter, remoteURLbase);
#endif
}



bool XAP_UnixAppImpl::openHelpURL(const char * url)
{
#if 0 //def HAVE_GNOME
	GError * err = NULL;	

	UT_DEBUGMSG(("DOM: Help URL: %s\n", szURL));

	gnome_help_display (szURL, NULL, &err);
	if (err != NULL) {
		UT_DEBUGMSG(("DOM: help error: %s\n", err->message));
		g_error_free (err);
	}
	return false;
#else
	return openURL (url);
#endif
}


bool XAP_UnixAppImpl::openURL(const char * szURL)
{
  //
  // Need this to make AbiGimp Load!!!!!
  //
        if(progExists("epiphany"))
	{
	}
#ifdef HAVE_GNOME
	gnome_url_show(szURL, NULL);
	return false;
#else
	xstatic char *fmtstring = NULL;
  	char *execstring = NULL;

	if(fmtstring)			// lookup browser result when we have
	  {				// already calculated it once before
		if(strstr(fmtstring, "netscape"))
			execstring = g_strdup_printf(fmtstring, szURL, szURL);
		else
		  execstring = g_strdup_printf(fmtstring, szURL);

		system(execstring);
		g_free (execstring);
		return false;					// why do we return false?
	}

	// TODO: we should move this into a preferences item,
	// TODO: but every other platform has a default browser
	// TODO: or text/html handler in a global registry

	// ORDER:
	// Use value of environment variable BROWSER, if valid; otherwise:
	// 1) konqueror
	// 2) mozilla
	// 3) netscape
	// 4) kdehelp
	// 5) lynx in an xterm
  	char *env_browser = getenv ("BROWSER");
  	if (env_browser)
  	{
	        bPExists = progExists(env_browser);
		if(bPExists)
		{
			if (strstr (env_browser, "netscape"))
			{
				fmtstring = g_strdup_printf("%s -remote openURL\\('%%s'\\) || %s '%%s' &", env_browser, env_browser);
				if (fmtstring) execstring = g_strdup_printf(fmtstring, szURL, szURL);
			}
			else
			{
				fmtstring = g_strdup_printf("%s '%%s' &", env_browser);
				if (fmtstring) execstring = g_strdup_printf(fmtstring, szURL);
			}
		}
  	}
	if (fmtstring == 0)
	{
		if(progExists("galeon"))
		{
		  	fmtstring = "galeon '%s' &";
			execstring = g_strdup_printf(fmtstring, szURL);
		}
		else if(progExists("epiphany"))
		{
		  	fmtstring = "epiphany '%s' &";
			execstring = g_strdup_printf(fmtstring, szURL);
		}
		// Anyone know how to find out where it might be, regardless?
		else if(progExists("mozilla"))
		{
		        fmtstring = "mozilla '%s' &";
			execstring = g_strdup_printf(fmtstring, szURL);
		}
		// Anyone know how to find out where it might be, regardless?
		else if(progExists("phoenix"))
		{
		        fmtstring = "phoenix '%s' &";
			execstring = g_strdup_printf(fmtstring, szURL);
		}
		else if(progExists("firefox"))
		{
		        fmtstring = "firefox '%s' &";
			execstring = g_strdup_printf(fmtstring, szURL);
		}
		else if(progExists("netscape"))
		{
			// Try to connect to a running Netscape, if not, start new one
			fmtstring = "netscape -remote openURL\\('%s'\\) || netscape '%s' &";
			execstring = g_strdup_printf(fmtstring, szURL, szURL);
		}
		else if(progExists("konqueror"))
		{
			fmtstring = "konqueror '%s' &";
			execstring = g_strdup_printf(fmtstring, szURL);
		}
		else if(progExists("khelpcenter"))
		{
			fmtstring = "khelpcenter '%s' &";
			execstring = g_strdup_printf(fmtstring, szURL);
		}
		else if(progExists("gnome-help-browser"))
		{
			fmtstring = "gnome-help-browser '%s' &";
			execstring = g_strdup_printf(fmtstring, szURL);
		}
		else if(progExists("lynx"))
		{
			fmtstring = "xterm -e lynx '%s' &";
			execstring = g_strdup_printf(fmtstring, szURL);
		}
		else if(progExists("w3m"))
		{
			fmtstring = "xterm -e w3m '%s' &";
	 		execstring = g_strdup_printf(fmtstring, szURL);
	 	}
	}
	if (execstring)
	{
		system (execstring);
		g_free (execstring);
	}

	return false;
#endif
}

