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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_dialogHelper.h"

#include "xap_UnixApp.h"
#include "xap_UnixFontManager.h"

#define DELETEP(p)	do { if (p) delete(p); (p)=NULL; } while (0)
#define FREEP(p)	do { if (p) free(p); (p)=NULL; } while (0)

// TODO get this from some higher-level place
#define FONTS_DIR_FILE	"/fonts.dir"

XAP_UnixFontManager::XAP_UnixFontManager(void) : m_fontHash(256)
{
}

XAP_UnixFontManager::~XAP_UnixFontManager(void)
{
	UT_VECTOR_PURGEALL(char *, m_searchPaths);
	UT_HASH_PURGEDATA(XAP_UnixFont *, m_fontHash);
}

UT_Bool XAP_UnixFontManager::setFontPath(const char * searchpath)
{
	gchar ** table = g_strsplit(searchpath, ";", 0);

	for (gint i = 0; table[i]; i++)
		m_searchPaths.addItem(table[i]);

	// free the table but not its contents
	g_free(table);
	
	return UT_TRUE;
}

UT_Bool XAP_UnixFontManager::scavengeFonts(void)
{
	UT_uint32 i = 0;
	UT_uint32 count = m_searchPaths.getItemCount();

	UT_uint32 totaldirs = 0;
	UT_uint32 totalfonts = 0;
	
	for (i = 0; i < count; i++)
	{
		UT_ASSERT(m_searchPaths.getNthItem(i));

		char * filename = (char *) calloc(strlen((char *) m_searchPaths.getNthItem(i)) +
										  strlen((char *) FONTS_DIR_FILE) + 1, sizeof(char));

		sprintf(filename, "%s%s", (char *) m_searchPaths.getNthItem(i), FONTS_DIR_FILE);

		FILE * file;

		file = fopen(filename, "r");
		if (!file)
		{
			// silently print this message out, since lots of users will have a "default"
			// path which will probably contain bogus directories, but there's always
			// hope later on in the path
			UT_DEBUGMSG(("Cannot open [%s] to read fonts list.\n", filename));
		}
		else
		{
			char buffer[512];

			// first line is a count
			fgets(buffer, 512, file);

			long fontcount = atol(buffer);

			// these should probably not be DEBUG-build only, but reworked
			// for real error messages in a release version (perhaps with
			// friendly messages telling the users where to look to fix
			// a busted font path problem)
#ifdef DEBUG
			if (fontcount < 0)
			{
				char message[512];
				g_snprintf(message, 512, "WARNING: Font directory file [%s]\ndeclares an invalid number of fonts (%ld).",
						   filename, fontcount);
				messageBoxOK(message);
			}
			else
			{
				UT_DEBUGMSG(("File says %d fonts should follow...", fontcount));
			}
#endif
			
			// every line after is a font name / XLFD pair
			long line;
			for (line = 0; line < fontcount; line++)
			{
				if (!fgets(buffer, 512, file))
				{
					// premature EOF (it's always premature if there are more
					// fonts specified than found)
					char message[512];
					g_snprintf(message, 512, "Premature end of file from font directory file [%s];\n"
							   "%ld fonts were supposed to be declared, but I only got %ld.\n"
							   "I will continue, but things may not work correctly.\n",
							   filename, fontcount, line);
					messageBoxOK(message);
					fclose(file);
					return UT_TRUE;
				}
				_allocateThisFont((const char *) buffer,
								  (const char *) m_searchPaths.getNthItem(i));
			}

			totalfonts += line;

			if (line >= 0)
				totaldirs++;
		  
			UT_DEBUGMSG(("Read %ld fonts from directory [%s].", line, filename));
		}
		FREEP(filename);
		if (file)
			fclose(file);
	}

	if (totaldirs <= 0)
	{
		// TODO this is not big enough for a really big list of fonts!
		char message[10240];
		g_snprintf(message, 10240, "Found no font directory files ('fonts.dir') in any directories\n"
				   "in font path:\n\n");
		{
			UT_uint32 dircount = m_searchPaths.getItemCount();
			for (i = 0; i < dircount; i++)
			{
				UT_ASSERT(m_searchPaths.getNthItem(i));
				strcat(message, "    ");
				strcat(message, (const char *) m_searchPaths.getNthItem(i));
				strcat(message, "\n");
			}
		}
		char message2[11264];
		g_snprintf(message2, 11264,
				   "%s"
				   "\n"
				   "Do the directories in this list contain valid font directory\n"
				   "files ('fonts.dir') and the actual font files ('*.pfa', '*.afm') files\n"
				   "they declare?\n"
				   "\n"
				   "See 'http://www.abisource.com/dev_download.phtml#type1'\n"
				   "for instructions on how to install these fonts and properly\n"
				   "set up your environment.", message);
		messageBoxOK(message2);
		return UT_FALSE;
	}

	if (totalfonts <= 0)
	{
		// we have no fonts, just quit
		char message[1024];
		g_snprintf(message, 1024, "Found no actual font data files ('*.pfa', '*.afm') in\n"
				   "the font path ('$ABIWORD_FONTPATH') even though I found %ld font\n"
				   "directory files ('fonts.dir').\n"
				   "\n"
				   "See 'http://www.abisource.com/dev_download.phtml#type1' for\n"
				   "instructions on how to install these fonts and properly\n"
				   "set up your environment.", totaldirs);
		messageBoxOK(message);
		return UT_FALSE;
	}

	// since we now have a good list of fonts totalling more than 0,
	// verify that their metrics data can be loaded by parsing the files
	// now.

	// NOTE this adds more time to program startup and steals it from the
	// NOTE first printing

	// for the above reason (speed) we don't do this now
#if 0	
	XAP_UnixFont ** allfonts = getAllFonts();
	for (UT_uint32 k = 0; k < getCount(); k++)
	{
		// if any of these fails, the user will know about it
		if (!allfonts[k]->getMetricsData())
			return UT_FALSE;
	}
	DELETEP(allfonts);
#endif
	
	return UT_TRUE;
}

UT_uint32 XAP_UnixFontManager::getCount(void)
{
	return (UT_uint32) m_fontHash.getEntryCount();
}

XAP_UnixFont ** XAP_UnixFontManager::getAllFonts(void)
{
	UT_uint32 count = getCount();
	
	XAP_UnixFont ** table = new XAP_UnixFont * [count];

	UT_ASSERT(table);

	UT_HashTable::UT_HashEntry * entry = NULL;
	for (UT_uint32 i = 0; i < count; i++)
	{
		entry = m_fontHash.getNthEntry(i);
		UT_ASSERT(entry && entry->pData);
		
		table[i] = (XAP_UnixFont *) entry->pData;
	}

	return table;
}

XAP_UnixFont * XAP_UnixFontManager::getDefaultFont(void)
{
	// this function (perhaps incorrectly) always assumes
	// it will be able to find this font on the display.
	// this is probably not such a bad assumption, since
	// gtk itself uses it all over (and it ships with every
	// X11R6 I can think of)

	XAP_UnixFont * f = new XAP_UnixFont();

	// do some manual behind-the-back construction
	f->setName("Default");
	f->setStyle(XAP_UnixFont::STYLE_NORMAL);
	f->setXLFD("-*-helvetica-medium-r-normal--0-0-*-*-p-56-iso8859-1");

	return f;
}

XAP_UnixFont * XAP_UnixFontManager::getFont(const char * fontname,
											XAP_UnixFont::style s)
{
	// We use a static buffer because this function gets called a lot.
	// Doing an allocation is really slow on many machines.
	static char keyBuffer[512];

	char * copy;
	UT_cloneString(copy, fontname);
	UT_upperString(copy);
	
	g_snprintf(keyBuffer, 512, "%s@%d", copy, s);

	FREEP(copy);
	
	UT_HashTable::UT_HashEntry * entry = m_fontHash.findEntry(keyBuffer);

	UT_DEBUGMSG(("Found font [%p] in table.\n", entry));
	
	return (entry && entry->pData) ? ((XAP_UnixFont *) entry->pData) : NULL;
}

/*
  If you don't like C-style strings, this function will make you cry.
*/
void XAP_UnixFontManager::_allocateThisFont(const char * line,
											const char * workingdir)
{
	// TODO (this one's easy): Make this function use the xap_UnixFontXLFD
	// TODO (this one's easy): class!  It'll save lots of work.
	/*
	  Each line comes in as:
	  
	  "myfile.pfa  -foundry-family-weight-slant-blah--0-0-0-0-p-0-iso8859-1"

	  We consider everything up to the first chunk of whitespace to be the
	  actual font name.  This name, minus the "pfa" extension is the name
	  of the file containing the font metrics.  These files are both
	  considered to be found in the working directory (workingdir).
	  Since fields in an XLFD can have white space (like the family
	  name--"Times New Roman"), the rest of the string (minus that whitespace)
	  is considered the complete XLFD.

	  The XLFD string also holds the style attributes, which we parse out
	  and use to create the font.

	  We then do our best shot to allocate this font.
	*/

	char * linedup = strdup(line);
	char * fontfile = strtok(linedup, " ");
	if (!fontfile)
	{
		UT_DEBUGMSG(("XAP_UnixFontManager::_allocateThisFont() - missing font "
					 "file name at first position in line.\n"));
		FREEP(linedup);
		return;
	}
	// allocate a real buffer for this
	fontfile = strdup(fontfile);
	// tack on the working directory to get the full path to the font file
	char * newstuff = (char *) calloc(strlen(workingdir) + 1 + strlen(fontfile) + 1,
									  sizeof(char));
	sprintf(newstuff, "%s/%s", workingdir, fontfile);
	FREEP(fontfile);
	fontfile = newstuff;
		
	char * xlfd = strtok(NULL, "\n");
	if (!xlfd)
	{
		UT_DEBUGMSG(("XAP_UnixFontManager::_allocateThisFont() - missing XLFD "
					 "file name at second position in line.\n"));
		FREEP(linedup);
		return;
	}
	// clean up that XLFD by removing surrounding spaces
	xlfd = g_strstrip(xlfd);
	
	// while we're hanging around the XLFD, find both the weight and
	// slant attributes to discern a "style"
	char * ihatexlfds = strdup(xlfd);
	UT_ASSERT(ihatexlfds);
	// first hit will be the leading dash
	strtok(ihatexlfds, "-");
	// ignore first field
	strtok(NULL, "-");
	// get the weight (should be "regular" or "bold")
	char * weight = strtok(NULL, "-");
	UT_ASSERT(weight);
	// get the slant (should be "r" for Roman or "i" for Italic)
	char * slant = strtok(NULL, "-");
	UT_ASSERT(slant);

	XAP_UnixFont::style s = XAP_UnixFont::STYLE_NORMAL;
	
	// sort from most common down
	if (!UT_stricmp(weight, "regular") &&
		!UT_stricmp(slant, "r"))
	{
		s = XAP_UnixFont::STYLE_NORMAL;
	}
	else if (!UT_stricmp(weight, "bold") &&
			!UT_stricmp(slant, "r"))
	{
		s = XAP_UnixFont::STYLE_BOLD;
	}
	else if (!UT_stricmp(weight, "regular") &&
			!UT_stricmp(slant, "i"))
	{
		s = XAP_UnixFont::STYLE_ITALIC;
	}
	else if (!UT_stricmp(weight, "bold") &&
			!UT_stricmp(slant, "i"))
	{
		s = XAP_UnixFont::STYLE_BOLD_ITALIC;
	}
	FREEP(ihatexlfds);
	
	// do some voodoo to get the AFM file from the file name
	char * dot = strrchr(fontfile, '.');
	if (!dot)
	{
		UT_DEBUGMSG(("XAP_UnixFontManager::_allocateThisFont() - can't guess "
					 "at font metrics file from font file name.\n"));
		FREEP(linedup);
		return;
	}

	// where's the last dot at in the filename?
	UT_ASSERT(dot > fontfile);
	unsigned int stemchars = (dot - fontfile) + 1;
	UT_ASSERT(stemchars <= strlen(fontfile));

	// allocate a new buffer for the AFM name
	size_t len = stemchars + 4 + 1;
	char * metricfile = (char *) calloc(len, sizeof(char));
	g_snprintf(metricfile, stemchars, "%s", fontfile);
	strcat(metricfile, ".afm");

	// build a font and load it up
	XAP_UnixFont * font = new XAP_UnixFont;
	UT_ASSERT(font);
	if (font->openFileAs((const char *) fontfile,
						 (const char *) metricfile,
						 (const char *) xlfd,
						 s))
	{
		_addFont(font);
	}
	else
	{
		DELETEP(font);
	}

	// be careful not to free strings built from strtok()
	// or pointer math.
	FREEP(newstuff);
	FREEP(metricfile);
	FREEP(linedup);
}
	  
void XAP_UnixFontManager::_addFont(XAP_UnixFont * font)
{
	// we index fonts by a combined "name" and "style"

	m_fontHash.addEntry(font->getFontKey(), NULL, (void *) font);
}
