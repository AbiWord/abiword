/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <glib.h>

#include "xap_UnixEncodingManager.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_string_class.h"

/* We source this file in order not to replace symbols in gnome-libs.
  (if AW is linked with GNOME).
*/

/* Only this function is used. For $LANG=ru_RU.KOI8-R it returns the following
  list:
  
  ru_RU.KOI8-R
  ru_RU
  ru.KOI8-R
  ru
  C
  ""
  
  It's much more powerful (gives correct result) than plain setlocale since 
  it explicitly reads locale aliases.
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

const GList *
g_i18n_get_language_list (const gchar *category_name);

#ifdef __cplusplus
}
#endif /* __cplusplus */


/*
 * Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation
 * All rights reserved.
 *
 * This file is part of the Gnome Library.
 *
 * The Gnome Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The Gnome Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

static GHashTable *alias_table = NULL;
static GHashTable *category_table= NULL;
bool prepped_table = 0;

/*read an alias file for the locales*/
static void
read_aliases (const char *file)
{
  FILE *fp;
  char buf[256];
  if (!prepped_table) {
    alias_table = g_hash_table_new (g_str_hash, g_str_equal);
    prepped_table = 1;
    }
  fp = fopen (file,"r");
  if (!fp)
    return;
  while (fgets (buf,256,fp))
    {
      char *p;
      g_strstrip(buf);
      if (buf[0]=='#' || buf[0]=='\0')
        continue;
      p = strtok (buf,"\t ");
      if (!p)
	continue;
      p = strtok (NULL,"\t ");
      if(!p)
	continue;
      if (!g_hash_table_lookup (alias_table, buf))
	g_hash_table_insert (alias_table, g_strdup(buf), g_strdup(p));
    }
  fclose (fp);
}

/*return the un-aliased language as a newly allocated string*/
static char *
unalias_lang (char *lang)
{
  char *p;
  int i;
  if (!prepped_table)
    {
      read_aliases ("/usr/lib/locale/locale.alias");
      read_aliases ("/usr/local/lib/locale/locale.alias");
      read_aliases ("/usr/share/locale/locale.alias");
      read_aliases ("/usr/local/share/locale/locale.alias");
      read_aliases ("/usr/lib/X11/locale/locale.alias");
      read_aliases ("/usr/openwin/lib/locale/locale.alias");
    }
  i = 0;
  while ((p=static_cast<char*>(g_hash_table_lookup(alias_table,lang))) && strcmp(p, lang))
    {
      lang = p;
      if (i++ == 30)
        {
          static gboolean said_before = FALSE;
	  if (!said_before)
            g_warning ("Too many alias levels for a locale, "
		       "may indicate a loop");
	  said_before = TRUE;
	  return lang;
	}
    }
  return lang;
}

static void
free_entry (void *ekey,void *eval,void * /*user_data*/)
{
	g_free(ekey);
	g_free(eval);
}

/* Mask for components of locale spec. The ordering here is from
 * least significant to most significant
 */
enum
{
  COMPONENT_CODESET =   1 << 0,
  COMPONENT_TERRITORY = 1 << 1,
  COMPONENT_MODIFIER =  1 << 2
};

/* Break an X/Open style locale specification into components
 */
static guint
explode_locale (const gchar *locale,
		gchar **language, 
		gchar **territory, 
		gchar **codeset, 
		gchar **modifier)
{
  const gchar *uscore_pos;
  const gchar *at_pos;
  const gchar *dot_pos;

  guint mask = 0;

  uscore_pos = strchr (locale, '_');
  dot_pos = strchr (uscore_pos ? uscore_pos : locale, '.');
  at_pos = strchr (dot_pos ? dot_pos : (uscore_pos ? uscore_pos : locale), '@');

  if (at_pos)
    {
      mask |= COMPONENT_MODIFIER;
      *modifier = g_strdup (at_pos);
    }
  else
    at_pos = locale + strlen (locale);

  if (dot_pos)
    {
      mask |= COMPONENT_CODESET;
      *codeset = g_new (gchar, 1 + at_pos - dot_pos);
      strncpy (*codeset, dot_pos, at_pos - dot_pos);
      (*codeset)[at_pos - dot_pos] = '\0';
    }
  else
    dot_pos = at_pos;

  if (uscore_pos)
    {
      mask |= COMPONENT_TERRITORY;
      *territory = g_new (gchar, 1 + dot_pos - uscore_pos);
      strncpy (*territory, uscore_pos, dot_pos - uscore_pos);
      (*territory)[dot_pos - uscore_pos] = '\0';
    }
  else
    uscore_pos = dot_pos;

  *language = g_new (gchar, 1 + uscore_pos - locale);
  strncpy (*language, locale, uscore_pos - locale);
  (*language)[uscore_pos - locale] = '\0';

  return mask;
}

/*
 * Compute all interesting variants for a given locale name -
 * by stripping off different components of the value.
 *
 * For simplicity, we assume that the locale is in
 * X/Open format: language[_territory][.codeset][@modifier]
 *
 * TODO: Extend this to handle the CEN format (see the GNUlibc docs)
 *       as well. We could just copy the code from glibc wholesale
 *       but it is big, ugly, and complicated, so I'm reluctant
 *       to do so when this should handle 99% of the time...
 */
static GList *
compute_locale_variants (const gchar *locale)
{
  GList *retval = NULL;

  gchar *language;
  gchar *territory;
  gchar *codeset;
  gchar *modifier;

  guint mask;
  guint i;

  UT_return_val_if_fail (locale != NULL, NULL);

  mask = explode_locale (locale, &language, &territory, &codeset, &modifier);

  /* Iterate through all possible combinations, from least attractive
   * to most attractive.
   */
  for (i=0; i<=mask; i++)
    if ((i & ~mask) == 0)
      {
	gchar *val = g_strconcat(language,
				 (i & COMPONENT_TERRITORY) ? territory : "",
				 (i & COMPONENT_CODESET) ? codeset : "",
				 (i & COMPONENT_MODIFIER) ? modifier : "",
				 NULL);
	retval = g_list_prepend (retval, val);
      }

  g_free (language);
  if (mask & COMPONENT_CODESET)
    g_free (codeset);
  if (mask & COMPONENT_TERRITORY)
    g_free (territory);
  if (mask & COMPONENT_MODIFIER)
    g_free (modifier);

  return retval;
}

/* The following is (partly) taken from the gettext package.
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.  */

static const gchar *
guess_category_value (const gchar *categoryname)
{
	const gchar *retval;
	
	/* Let's check passed category.  */
	retval = g_getenv (categoryname);
	if (retval != NULL && retval[0] != '\0')
		return retval;
	
	/* The highest priority value after passed category is the `LANGUAGE' 
	environment variable.  This is a GNU extension.  */
	retval = g_getenv ("LANGUAGE");
	if (retval != NULL && retval[0] != '\0')
		return retval;

	/* `LANGUAGE' is not set.  So we have to proceed with the POSIX
		 methods of looking to `LC_ALL', `LC_xxx', and `LANG'.  On some
		 systems this can be done by the `setlocale' function itself.  */

	/* Setting of LC_ALL overwrites all other.  */
	retval = g_getenv ("LC_ALL");
	if (retval != NULL && retval[0] != '\0')
		return retval;


	/* Last possibility is the LANG environment variable.  */
	retval = g_getenv ("LANG");
	if (retval != NULL && retval[0] != '\0')
		return retval;

	return NULL;
}

/**
 * g_i18n_get_language_list:
 * @category_name: Name of category to look up, e.g. "LC_MESSAGES".
 * 
 * This computes a list of language strings.  It searches in the
 * standard environment variables to find the list, which is sorted
 * in order from most desirable to least desirable.  The `C' locale
 * is appended to the list if it does not already appear (other routines depend on this behaviour).
 * If @category_name is %NULL, then LC_ALL is assumed.
 * 
 * Return value: the list of languages, this list should not be freed as it is owned by gnome-i18n
 **/
const GList *
g_i18n_get_language_list (const gchar *category_name)
{
  GList *list;
  prepped_table = 0;

	if (!category_name)
		category_name= "LC_ALL";

	if (category_table)
    {
#if 0
		// we want a fresh reading of the LANG variable every time so we can
		// work out the non-Unicode encoding under UTF-8 locale
		list= static_cast<GList *>(g_hash_table_lookup (category_table, const_cast<const gpointer>(category_name)));
#else
		xxx_UT_DEBUGMSG(("recreating hash table\n"));
		g_hash_table_destroy (category_table);
        category_table= g_hash_table_new (g_str_hash, g_str_equal);
        list= NULL;
		
#endif
    }
	else
    {
		category_table= g_hash_table_new (g_str_hash, g_str_equal);
		list= NULL;
    }

	if (!list)
    {
		gint c_locale_defined= FALSE;
	  
		const gchar *category_value;
		gchar *category_memory, *orig_category_memory;

		category_value = guess_category_value (category_name);
		if (! category_value)
			category_value = "C";
		orig_category_memory = category_memory =
			static_cast<gchar*>(g_malloc (strlen (category_value)+1));
      
		while (category_value[0] != '\0')
		{
			while (category_value[0] != '\0' && category_value[0] == ':')
				++category_value;
	  
			if (category_value[0] != '\0')
			{
				char *cp= category_memory;
	      
				while (category_value[0] != '\0' && category_value[0] != ':')
					*category_memory++= *category_value++;
	      
				category_memory[0]= '\0'; 
				category_memory++;
				  
				cp = unalias_lang(cp);
				  
				if (strcmp (cp, "C") == 0)
					c_locale_defined= TRUE;
	      
				list= g_list_concat (list, compute_locale_variants (cp));
			}
		}

		g_free (orig_category_memory);
      
		if (!c_locale_defined)
			list= g_list_append (list, reinterpret_cast<void*>(const_cast<gchar *>("C")));

		g_hash_table_insert (category_table, reinterpret_cast<gpointer>(const_cast<gchar *>(category_name)), list);
    }

   g_hash_table_foreach(alias_table, free_entry, NULL);
   g_hash_table_destroy(alias_table);  
   prepped_table = 0;

  return list;
}

/************************************************************/

XAP_EncodingManager *XAP_EncodingManager::get_instance()
{
	if (_instance == 0)
	{
		UT_DEBUGMSG(("Building XAP_EncodingManager\n"));
		_instance = new XAP_UnixEncodingManager();
		_instance->initialize();
		UT_DEBUGMSG(("XAP_EncodingManager built\n"));
	}

	return _instance;
}

/************************************************************/

static UT_UTF8String NativeEncodingName;
static UT_UTF8String NativeSystemEncodingName;
static UT_UTF8String Native8BitEncodingName;
static UT_UTF8String NativeNonUnicodeEncodingName;
static UT_UTF8String NativeUnicodeEncodingName;
static UT_UTF8String LanguageISOName;
static UT_UTF8String LanguageISOTerritory;

XAP_UnixEncodingManager::XAP_UnixEncodingManager() 
{
}

XAP_UnixEncodingManager::~XAP_UnixEncodingManager() 
{
}

const char* XAP_UnixEncodingManager::getNativeEncodingName() const
{     
  return NativeEncodingName.utf8_str(); 
}

const char* XAP_UnixEncodingManager::getNativeSystemEncodingName() const
{     
  return NativeSystemEncodingName.utf8_str(); 
}

const char* XAP_UnixEncodingManager::getNative8BitEncodingName() const
{     
  return Native8BitEncodingName.utf8_str();
}

const char* XAP_UnixEncodingManager::getNativeNonUnicodeEncodingName() const
{     
  return NativeNonUnicodeEncodingName.utf8_str();
}

const char* XAP_UnixEncodingManager::getNativeUnicodeEncodingName() const
{     
  return NativeUnicodeEncodingName.utf8_str(); 
}

const char* XAP_UnixEncodingManager::getLanguageISOName() const
{
 	return LanguageISOName.utf8_str(); 
}

const char* XAP_UnixEncodingManager::getLanguageISOTerritory() const
{ 	
  return LanguageISOTerritory.utf8_str(); 
}

void  XAP_UnixEncodingManager::initialize()
{	
	const GList* lst = g_i18n_get_language_list ("LANG");
	const char* locname = static_cast<char*>(lst->data);

	NativeEncodingName =  "ISO-8859-1";
	NativeSystemEncodingName =
	Native8BitEncodingName =
	NativeNonUnicodeEncodingName = NativeEncodingName;
	NativeUnicodeEncodingName = "UTF-8";
	LanguageISOName = "en";
	LanguageISOTerritory = "US";   

	if (!*locname || !strcmp(locname,"C"))
	{ 	/* paranoic case - broken system */
		; /*already initialized*/
	}
	else
	{
		char * lang = NULL;
		char * terr = NULL;
		char * cs   = NULL;
		char * mod  = NULL;

		int mask = explode_locale (locname,&lang,&terr,&cs,&mod);

		LanguageISOName = lang;

		if ((mask & COMPONENT_TERRITORY) && terr)
		{
			LanguageISOTerritory = terr + 1; /* yes, +1 */
		}
		if ((mask & COMPONENT_CODESET) && cs)
		{
			if (cs[1])
			{
				int length = strlen (cs + 1);
				char * name = static_cast<char *>(g_try_malloc (length + 3));
				if (name)
				{
					strcpy (name, cs + 1);
					
					/* make the encoding name upper-case
					 * TODO why not use the UT_islower/UT_isupper?
					 */
					for (int i = 0; i < length; i++)
						if (islower (static_cast<int>(static_cast<unsigned char>(name[i]))))
							name[i] = static_cast<char>(toupper (static_cast<int>(static_cast<unsigned char>(name[i]))));
					
					/* encoding names may be presented as iso88591 or ISO8859-1,
					 * but we need both hyphens for iconv
					 */
					if (strncmp (name, "ISO8859", 7) == 0)
					{
						memmove (name + 4, name + 3, length + 1 - 3);
						length++;
						name[3] = '-';
						if (name[8] != '-')
						{
							memmove (name + 9, name + 8, length + 1 - 8);
							length++;
							name[8] = '-';
						}
					}
					NativeEncodingName = name;
					FREEP(name);
				}
			}
			Native8BitEncodingName = NativeSystemEncodingName = NativeEncodingName;
			
			// need to get non-unicode encoding if encoding is UTF-8
			if(!g_ascii_strcasecmp(NativeEncodingName.utf8_str(), "UTF-8"))
			{
				// we want to get the encoding that would be used for the given
				// language/territory if the UTF-8 encoding was not specified
				// by LANG
				
				UT_UTF8String OLDLANG (getenv("LANG"));
				UT_UTF8String MYLANG (LanguageISOName);
				MYLANG += "_";
				MYLANG += LanguageISOTerritory;
				g_setenv ("LANG", MYLANG.utf8_str(), TRUE);
				if (mask & COMPONENT_CODESET)
				{
					NativeNonUnicodeEncodingName = cs+1;
					xxx_UT_DEBUGMSG(("NativeNonUnicodeEncodingName (1) %s\n", NativeNonUnicodeEncodingName));
					if (!strncmp(cs+1,"ISO8859",strlen("ISO8859")))
					{
						char buf[40];
						strcpy(buf,"ISO-");
						strcat(buf,cs+1+3);
						NativeNonUnicodeEncodingName = buf;
					}
					xxx_UT_DEBUGMSG(("NativeNonUnicodeEncodingName (2) %s\n", NativeNonUnicodeEncodingName));
				}
				g_setenv("LANG", OLDLANG.utf8_str(), TRUE);
			}
			
		}
		FREEP(lang);
		FREEP(terr);
		FREEP(cs);
		FREEP(mod);
	}
	// Moved to end so that we correctly detect if the locale is CJK.
	XAP_EncodingManager::initialize(); 
	describe();
}
